///////////////////////////////////////////////////////////////////////////////
// $Id: LicensePropPage.cpp,v 1.2 2003-05-06 16:36:27 adcockj Exp $
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
// Revision 1.1  2003/05/01 12:34:41  adcockj
// Added headers and new license page
//
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

STDMETHODIMP CLicensePropPage::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
	HRESULT hr = IPropertyPageImpl<CLicensePropPage>::Activate(hWndParent,pRect,bModal);
	if(FAILED(hr))
	{
		return hr;
	}

    BSTR Name = NULL;
    eFreeLicense LicenceType;
    BSTR Authors = NULL;

    if(m_License == NULL)
    {
        return E_UNEXPECTED;
    }

    hr = m_License->GetName(&Name);
    if(FAILED(hr)) return hr;
	hr = m_License->GetLicense(&LicenceType);
    if(FAILED(hr)) return hr;
    hr = m_License->GetAuthors(&Authors);
    if(FAILED(hr)) return hr;
    
    SendMessageW(GetDlgItem(IDC_NAME), WM_SETTEXT, 0, (LPARAM)Name);
    CComBSTR LicenseText;

    SendMessageW(GetDlgItem(IDC_COPYRIGHT), WM_SETTEXT, 0, (LPARAM)Authors);

    if(LicenceType == GPL)
    {
        LicenseText.LoadString(IDS_GPL);
    }
    else if(LicenceType == LGPL)
    {
        LicenseText.LoadString(IDS_LGPL);
    }
    else
    {
        LicenseText = L"Unknown License";
    }
    SendMessageW(GetDlgItem(IDC_LICENSE), WM_SETTEXT, 0, (LPARAM)(BSTR)LicenseText);

    SendMessageW(GetDlgItem(IDC_HOMEPAGE), WM_SETTEXT, 0, (LPARAM)L"");

    SysFreeString(Name);
    SysFreeString(Authors);
    return S_OK;
}
