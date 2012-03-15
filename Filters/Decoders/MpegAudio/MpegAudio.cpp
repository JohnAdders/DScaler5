///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// MpegAudio.dll - DirectShow filter for decoding audio
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "AudioDecoder.h"
#include "MoreUuids.h"
#include "CPUID.h"

#include "..\GenDMOProp\GenDMOProp_i.c"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CAudioDecoder)
END_COCLASS_TABLE()

IMPLEMENT_DLL_MODULE_ROUTINES()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        g_hInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
        CPU_SetupFeatureFlag();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    return ClassTableGetClassObject(Classes, rclsid, riid, ppv);
}

STDAPI DllCanUnloadNow(void)
{
    return ModuleIsIdle() ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{

    REGPINTYPES InputTypes[] = {
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MP3},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1AudioPayload},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1Payload},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG1Packet},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_AUDIO},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_MPEG2_AUDIO},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_MPEG2_AUDIO},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG2_AUDIO},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MPEG2_AUDIO_MPCBUG},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_AC3},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_DOLBY_AC3},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_DOLBY_AC3},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_DOLBY_AC3},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DOLBY_AC3},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_DTS},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_DTS},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_DTS},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_WAVE_DTS},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_AAC},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_AAC},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_AAC},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_AAC},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MP4A},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_MP4A},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_MP4A},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_MP4A},
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_mp4a},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_mp4a},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_mp4a},
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_mp4a},
    };

    REGPINTYPES OutputTypes[] = {
        {&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
    };

    REGFILTERPINS2 Pins[2] = {{ 0, 1, countof(InputTypes), InputTypes, 0, NULL, &GUID_NULL},
                              { REG_PINFLAG_B_OUTPUT , 1, countof(OutputTypes), OutputTypes, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;

    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_PREFERRED;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;


    HRESULT hr = RegisterFilter(CLSID_CAudioDecoder, L"DScaler Audio Decoder", &RegInfo);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = UnregisterFilter(CLSID_CAudioDecoder);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// DllInstall - Adds support for per user registration

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    HRESULT hr = E_FAIL;
    static const wchar_t szUserSwitch[] = L"user";

    if (pszCmdLine != NULL)
    {
        if (_wcsnicmp(pszCmdLine, szUserSwitch, _countof(szUserSwitch)) == 0)
        {
            UsePerUserRegistration = true;
        }
    }

    if (bInstall)
    {
        hr = DllRegisterServer();
        if (FAILED(hr))
        {
            DllUnregisterServer();
        }
    }
    else
    {
        hr = DllUnregisterServer();
    }
    return hr;
}
