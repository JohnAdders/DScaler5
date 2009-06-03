///////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "stdafx.h"
#include "GenDMOProp.h"
#include "LicensePropPage.h"

/////////////////////////////////////////////////////////////////////////////
// LicensePropPage

extern HINSTANCE g_hInstance;


LicensePropPage::LicensePropPage() :
    SimplePropertyPage(MAKEINTRESOURCE(IDD_LICENSEPROPPAGE), IDS_TITLELicensePropPage, IDS_DOCSTRINGLicensePropPage, IDS_HelpFile, 0)
{
}

STDMETHODIMP LicensePropPage::Apply(void)
{
    SetIsDirty(FALSE);
    return S_OK;
}

STDMETHODIMP LicensePropPage::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
    if(cObjects != 1)
    {
        return E_UNEXPECTED;
    }

    m_License = *ppUnk;

    return S_OK;
}

HRESULT LicensePropPage::OnActivate()
{
    if(m_License == NULL)
    {
        return E_UNEXPECTED;
    }

    BSTR Name = NULL;
    eFreeLicense LicenceType;
    BSTR Authors = NULL;

    HRESULT hr = m_License->get_Name(&Name);
    if(FAILED(hr)) return hr;
    hr = m_License->get_License(&LicenceType);
    if(FAILED(hr)) return hr;
    hr = m_License->get_Authors(&Authors);
    if(FAILED(hr)) return hr;

    SendMessageW(GetDlgItem(m_hWnd, IDC_NAME), WM_SETTEXT, 0, (LPARAM)Name);
    std::vector<wchar_t> LicenseText(512);

    SendMessageW(GetDlgItem(m_hWnd, IDC_COPYRIGHT), WM_SETTEXT, 0, (LPARAM)Authors);

    if(LicenceType == GPL)
    {
        LoadStringW(g_hInstance, IDS_GPL, &LicenseText[0], 512);
        SendMessageW(GetDlgItem(m_hWnd, IDC_LICENSE), WM_SETTEXT, 0, (LPARAM)&LicenseText[0]);
    }
    else if(LicenceType == LGPL)
    {
        LoadStringW(g_hInstance, IDS_LGPL, &LicenseText[0], 512);
        SendMessageW(GetDlgItem(m_hWnd, IDC_LICENSE), WM_SETTEXT, 0, (LPARAM)&LicenseText[0]);
    }
    else
    {
        SendMessageW(GetDlgItem(m_hWnd, IDC_LICENSE), WM_SETTEXT, 0, (LPARAM)L"Unknown License");
    }

    SendMessageW(GetDlgItem(m_hWnd, IDC_HOMEPAGE), WM_SETTEXT, 0, (LPARAM)L"");

    SysFreeString(Name);
    SysFreeString(Authors);
    return S_OK;
}

HRESULT LicensePropPage::OnDeactivate()
{
    return S_OK;
}

INT_PTR LicensePropPage::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}
