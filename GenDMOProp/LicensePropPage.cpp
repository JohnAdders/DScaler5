///////////////////////////////////////////////////////////////////////////////
// $Id: LicensePropPage.cpp,v 1.1 2003-05-01 12:34:41 adcockj Exp $
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
#include "LicensePropPage.h"

/////////////////////////////////////////////////////////////////////////////
// CLicensePropPage

CLicensePropPage::CLicensePropPage() 
{
	m_dwTitleID = IDS_TITLELicensePropPage;
	m_dwHelpFileID = IDS_HELPFILELicensePropPage;
	m_dwDocStringID = IDS_DOCSTRINGLicensePropPage;
}

STDMETHODIMP CLicensePropPage::Apply(void)
{
	ATLTRACE(_T("CLicensePropPage::Apply\n"));
	m_bDirty = FALSE;
	return S_OK;
}

STDMETHODIMP CLicensePropPage::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
    if(cObjects != 1)
    {
        return E_UNEXPECTED;
    }
    m_License = *ppUnk;

    if(m_License != NULL)
    {
        return S_OK;
    }
    else
    {
        m_License.Release();
        return E_NOINTERFACE;
    }
}   