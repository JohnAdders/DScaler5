///////////////////////////////////////////////////////////////////////////////
// $Id: MpegVideo.cpp,v 1.10 2005-10-05 16:21:16 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// MpegVideo.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.9  2005/01/04 17:53:43  adcockj
// added option to force dscalewr filter to be loaded2
//
// Revision 1.8  2004/10/28 15:52:24  adcockj
// Moved video output pin code into new class
//
// Revision 1.7  2004/07/16 15:58:01  adcockj
// Fixed compilation issues under .NET
// Changed name of filter
// Some performance improvements to libmpeg2
//
// Revision 1.6  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.5  2004/03/05 15:56:29  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.4  2004/02/25 17:14:02  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/17 16:51:33  adcockj
// Added countof define
//
// Revision 1.2  2004/02/10 13:24:12  adcockj
// Lots of bug fixes + corrected interlaced YV12 upconversion
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "moreuuids.h"
#include <initguid.h>
#include "MpegDecoder.h"
#include "CPUID.h"

#include "..\GenDMOProp\GenDMOProp_i.c"
#include "yacl\include\combook.cpp"

HINSTANCE g_hInstance = NULL;

BEGIN_COCLASS_TABLE(Classes)
    IMPLEMENTS_COCLASS(CMpegDecoder)
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
        {&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
        {&MEDIATYPE_MPEG2_PACK, &MEDIASUBTYPE_MPEG2_VIDEO},
        {&MEDIATYPE_MPEG2_PES, &MEDIASUBTYPE_MPEG2_VIDEO},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG2_VIDEO},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Packet},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_MPEG1Payload},
    };

    REGPINTYPES OutputTypes[] = {   
        {&MEDIATYPE_Video, &MEDIASUBTYPE_YUY2},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_YV12},
        {&MEDIATYPE_Video, &MEDIASUBTYPE_NV12},
        {&CLSID_CDScaler, &MEDIASUBTYPE_YUY2},
        {&CLSID_CDScaler, &MEDIASUBTYPE_YV12},
    };
    
    REGFILTERPINS2 Pins[2] = {{ 0, 1, countof(InputTypes), InputTypes, 0, NULL, &GUID_NULL}, 
                              { REG_PINFLAG_B_OUTPUT , 1, countof(OutputTypes), OutputTypes, 0, NULL, &GUID_NULL}};

    REGFILTER2 RegInfo;
   
    RegInfo.dwVersion = 2;
    RegInfo.dwMerit = MERIT_PREFERRED;
    RegInfo.cPins2 = 2;
    RegInfo.rgPins2 = Pins;
    
  
    HRESULT hr = RegisterFilter(CLSID_CMpegDecoder, L"DScaler Mpeg2 Video Decoder", &RegInfo);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    HRESULT hr = UnregisterFilter(CLSID_CMpegDecoder);
    CHECK(hr);
    return ClassTableUpdateRegistry(GetThisInstance(), Classes, 0, FALSE, FALSE);
}


