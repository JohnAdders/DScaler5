////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the
// DMO project wizard.  That code is (c) Microsoft Corporation
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\..\DMO_Lib\DMO_Lib.h"
#include "..\..\DMO_Lib\CInPlaceDMO.h"
#include "resource.h"


DEFINE_GUID(CLSID_CFD_DScaler, 0x81ee9ffc, 0x301, 0x4e0c, 0xa5, 0x69, 0xfe, 0x13, 0x80, 0xea, 0xc4, 0x8f);

class CFD_DScaler :
    public CSimpleInPlaceVideoDMO,
    public IFilmDetect,
    public IAmFreeSoftwareLicensed
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CFD_DScaler, "{81EE9FFC-0301-4e0c-A569-FE1380EAC48F}", "Film Detect DScaler Class", "DMO.FD_DScaler.1", "DMO.FD_DScaler", "both")
    IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
    IMPLEMENTS_INTERFACE(IMediaObject)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IMediaObjectInPlace)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IDScalerVideoFilterPlugin)
    IMPLEMENTS_INTERFACE(IDScalerFilterPlugin)
    IMPLEMENTS_INTERFACE(IFilmDetect)
END_INTERFACE_TABLE()

public:
    CFD_DScaler();
    ~CFD_DScaler();

BEGIN_PARAM_LIST()
END_PARAM_LIST()

public:
    STDMETHODIMP STDMETHODCALLTYPE GetClassID(CLSID *pClsid);

    STDMETHOD(DetectFilm)(IInterlacedBufferStack* Stack, eDeinterlaceType* DetectedType, DWORD* DetectedIndex);
    STDMETHOD(ResetDectection)();


// IAmFreeSoftwareLicensed
public:
    STDMETHOD(get_Name)(BSTR* Name);
    STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
private:
    void CheckVideo(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field);
    void Check32(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field);
    void Check22(eDetectionHint Hint, DWORD FieldNumber, IInterlacedField* Field);
    eDeinterlaceType m_DetectedType;
    DWORD m_DetectedIndex;
};

