///////////////////////////////////////////////////////////////////////////////
// $Id: MpegVideo.cpp,v 1.1 2004-02-06 12:17:16 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// MpegVideo.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
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
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "MpegDecoder.h"
#include "MoreUuids.h"
#include "CPUID.h"

#include <uuids.h>
#include "..\GenDMOProp\GenDMOProp_i.c"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CMpegDecoder)
END_COCLASS_TABLE()

IMPLEMENT_DLL_MODULE_ROUTINES()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
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
    SI(IFilterMapper2) FilterMapper;

	REGPINTYPES InputTypes[] = {	
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
    	{&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
	    {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_MPEG2_VIDEO},
	    {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG2_VIDEO},
	    {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Packet},
	    {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Payload},
    };

	REGPINTYPES OutputTypes[] = {	
    	{&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2},
    };
    
    REGFILTERPINS2 Pins[2] = {{ 0, 1, sizeof(InputTypes)/sizeof(REGPINTYPES), InputTypes, 0, NULL, &GUID_NULL}, 
                              { REG_PINFLAG_B_OUTPUT , 1, sizeof(OutputTypes)/sizeof(REGPINTYPES), OutputTypes, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;
   
    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_PREFERRED;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;
    
    HRESULT hr = FilterMapper.CreateInstance(CLSID_FilterMapper, CLSCTX_INPROC_SERVER);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to create Filter Mapper %08x\n", hr));
        return hr;
    }
    hr = FilterMapper->RegisterFilter(CLSID_CMpegDecoder, L"Mpeg2 Video Decoder", NULL, NULL, NULL, &RegInfo);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to Register Filter %08x\n", hr));
        return hr;
    }

    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    SI(IFilterMapper2) FilterMapper;
    HRESULT hr = FilterMapper.CreateInstance(CLSID_FilterMapper, CLSCTX_INPROC_SERVER);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to create Filter Mapper %08x\n", hr));
        return hr;
    }
    hr = FilterMapper->UnregisterFilter(NULL, NULL, CLSID_CMpegDecoder);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to Unregister Filter %08x\n", hr));
        return hr;
    }

    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}


