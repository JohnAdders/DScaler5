///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// GenDMOProp.dll - Generic DirectShow property page
// Copyright (c) 2004 John Adcock
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
#include "StatisticsPropPage.h"

/////////////////////////////////////////////////////////////////////////////
// StatisticsPropPage

StatisticsPropPage::StatisticsPropPage() :
    SimplePropertyPage(MAKEINTRESOURCE(IDD_STATISTICSPROPPAGE), IDS_TITLEStatisticsPropPage, IDS_DOCSTRINGStatisticsPropPage, IDS_HelpFile, 0)
{
}

STDMETHODIMP StatisticsPropPage::Apply(void)
{
    SetIsDirty(FALSE);
    return S_OK;
}

STDMETHODIMP StatisticsPropPage::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
    if(cObjects != 1)
    {
        return E_UNEXPECTED;
    }

    m_Statistics = *ppUnk;

    return S_OK;
}

HRESULT StatisticsPropPage::OnActivate()
{
    if(m_Statistics == NULL)
    {
        return E_UNEXPECTED;
    }

    DWORD NumProps;

    HRESULT hr = m_Statistics->get_NumStatistics(&NumProps);
    if(FAILED(hr)) return hr;

    SendMessageW(GetDlgItem(m_hWnd, IDC_LIST1), LB_RESETCONTENT, 0, 0);

    for(DWORD i = 0; i < NumProps; ++i)
    {
        BSTR Name = NULL;
        hr = m_Statistics->get_StatisticName(i, &Name);
        if(FAILED(hr)) return hr;
        SendMessageW(GetDlgItem(m_hWnd, IDC_LIST1), LB_INSERTSTRING , -1, (LPARAM)Name);
        SysFreeString(Name);
        BSTR Value = NULL;
        hr = m_Statistics->get_StatisticValue(i, &Value);
        if(FAILED(hr)) return hr;
        SendMessageW(GetDlgItem(m_hWnd, IDC_LIST1), LB_INSERTSTRING , -1, (LPARAM)Value);
        SysFreeString(Value);
    }

    return S_OK;
}

HRESULT StatisticsPropPage::OnDeactivate()
{
    return S_OK;
}

INT_PTR StatisticsPropPage::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    return FALSE;
}

