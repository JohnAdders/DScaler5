///////////////////////////////////////////////////////////////////////////////
// $Id: GenDMOPropPage.cpp,v 1.2 2003-05-01 12:34:41 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// GenDMOProp.dll - Generic DirectShow property page using IMediaParams
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
#include "GenDMOProp.h"
#include "GenDMOPropPage.h"

/////////////////////////////////////////////////////////////////////////////
// CGenDMOPropPage

CGenDMOPropPage::CGenDMOPropPage() 
{
	m_dwTitleID = IDS_TITLEGenDMOPropPage;
	m_dwHelpFileID = IDS_HELPFILEGenDMOPropPage;
	m_dwDocStringID = IDS_DOCSTRINGGenDMOPropPage;
}

STDMETHODIMP CGenDMOPropPage::Apply(void)
{
	ATLTRACE(_T("CGenDMOPropPage::Apply\n"));
	for (UINT i = 0; i < m_nObjects; i++)
	{
		// Do something interesting here
		// ICircCtl* pCirc;
		// m_ppUnk[i]->QueryInterface(IID_ICircCtl, (void**)&pCirc);
		// pCirc->put_Caption(CComBSTR("something special"));
		// pCirc->Release();
	}
	m_bDirty = FALSE;
	return S_OK;
}

STDMETHODIMP CGenDMOPropPage::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
    if(cObjects != 1)
    {
        return E_UNEXPECTED;
    }
    m_MediaParamInfo = *ppUnk;
    m_MediaParams = *ppUnk;

    if(m_MediaParamInfo != NULL && m_MediaParams != NULL)
    {
        return S_OK;
    }
    else
    {
        m_MediaParamInfo.Release();
        m_MediaParams.Release();
        return E_NOINTERFACE;
    }
}   