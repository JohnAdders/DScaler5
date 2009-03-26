///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
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

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "DScaler.h"

#include <uuids.h>
#include "..\GenDMOProp\GenDMOProp_i.c"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CDScaler)
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
    REGPINTYPES Types[] = {&CLSID_CDScaler, &MEDIASUBTYPE_YUY2,
                            &CLSID_CDScaler, &MEDIASUBTYPE_YV12,
                            &MEDIATYPE_Video, &MEDIASUBTYPE_YUY2,
                            &MEDIATYPE_Video, &MEDIASUBTYPE_YV12};

    REGFILTERPINS2 Pins[2] = {{ 0, 1, countof(Types), Types, 0, NULL, &GUID_NULL},
                              { REG_PINFLAG_B_OUTPUT , 1, countof(Types), Types, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;

    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_UNLIKELY;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;

    HRESULT hr = RegisterFilter(CLSID_CDScaler, L"DScaler Filter", &RegInfo);
    CHECK(hr);

    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = UnregisterFilter(CLSID_CDScaler);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}


