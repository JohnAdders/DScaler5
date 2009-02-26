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
#include "DScaler.h"
#include "Utils.h"

#include <initguid.h>
#include <uuids.h>
#include "..\GenDMOProp\GenDMOProp_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(__uuidof(CDScaler), CDScaler)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, NULL);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    CComPtr<IFilterMapper2> FilterMapper;

    REGPINTYPES Types[] = {&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2,
                            &MEDIATYPE_Video, &MEDIASUBTYPE_YV12, 
                            &MEDIATYPE_Video, &MEDIASUBTYPE_NV12 };
    
    REGFILTERPINS2 Pins[2] = {{ 0, 1, countof(Types), Types, 0, NULL, &GUID_NULL}, 
                              { REG_PINFLAG_B_OUTPUT , 1, countof(Types), Types, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;
   
    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_PREFERRED + 5;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;
    
    HRESULT hr = FilterMapper.CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC_SERVER);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to create Filter Mapper %08x\n", hr));
        return hr;
    }
    hr = FilterMapper->RegisterFilter(__uuidof(CDScaler), L"DScaler Filter", NULL, NULL, NULL, &RegInfo);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to Register Filter %08x\n", hr));
        return hr;
    }

    // registers object, 
    // no typelib or interfaces at the moment hense FALSE as 
    // parameter
    return _Module.RegisterServer(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    CComPtr<IFilterMapper2> FilterMapper;
    HRESULT hr = FilterMapper.CoCreateInstance(CLSID_FilterMapper, NULL, CLSCTX_INPROC_SERVER);
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to create Filter Mapper %08x\n", hr));
        return hr;
    }
    hr = FilterMapper->UnregisterFilter(NULL, NULL, __uuidof(CDScaler));
    if(FAILED(hr))
    {
        LOG(DBGLOG_ERROR, ("Failed to Unregister Filter %08x\n", hr));
        return hr;
    }

    return _Module.UnregisterServer(TRUE);
}


