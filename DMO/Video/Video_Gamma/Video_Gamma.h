////////////////////////////////////////////////////////////////////////////
// $Id: Video_Gamma.h,v 1.1 2003-05-21 17:06:01 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "..\..\DMO_Lib\DMO_Lib.h"
#include "..\..\DMO_Lib\CInPlaceDMO.h"
#include "resource.h"


class ATL_NO_VTABLE DECLSPEC_UUID("99AE5B7F-8B9C-11d7-B851-0002A5623377") CVideo_Gamma : 
	public CSimpleInPlaceVideoDMO,
    public IAmFreeSoftwareLicensed,
	public CComCoClass<CVideo_Gamma, &__uuidof(CVideo_Gamma)>
{
public:
    CVideo_Gamma();
    ~CVideo_Gamma();

    DECLARE_REGISTRY( CVideo_Gamma, _T("DMO.Video_Gamma"), _T("DMO.Video_Gamma.1"), IDS_DESCRIPTION, THREADFLAGS_BOTH);

BEGIN_COM_MAP(CVideo_Gamma)
	COM_INTERFACE_ENTRY(IAmFreeSoftwareLicensed)
    COM_INTERFACE_ENTRY_CHAIN(CSimpleInPlaceVideoDMO)
END_COM_MAP()

public:
    STDMETHODIMP STDMETHODCALLTYPE GetClassID(CLSID *pClsid);

// IAmFreeSoftwareLicensed
public:
	STDMETHOD(get_Name)(BSTR* Name);
	STDMETHOD(get_License)(eFreeLicense* License);
	STDMETHOD(get_Authors)(BSTR* Authors);

protected:
	HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager);
private:
};
