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
#include "GenDMOPropPage.h"
#include ".\gendmoproppage.h"

/////////////////////////////////////////////////////////////////////////////
// GenDMOPropPage

GenDMOPropPage::GenDMOPropPage() :
    SimplePropertyPage(MAKEINTRESOURCE(IDD_GENDMOPROPPAGE), IDS_TITLEGenDMOPropPage, IDS_DOCSTRINGGenDMOPropPage, IDS_HelpFile, 0)
{
    m_NumParams = 0;
    m_ParamTexts = 0;
    m_Params = 0;
    m_ParamInfos = 0;

    m_ListBox = NULL;
    m_EditBox = NULL;
    m_CheckBox = NULL;
    m_Slider = NULL;
    m_Scrollbar = NULL;
    m_Combo = NULL;
    m_DefaultsBtn = NULL;
    m_BoolDesc = NULL;

}

GenDMOPropPage::~GenDMOPropPage()
{
    for(DWORD i(0); i < m_NumParams; ++i)
    {
        CoTaskMemFree(m_ParamTexts[i]);
    }
    m_NumParams = 0;
    CoTaskMemFree(m_ParamTexts);
    m_ParamTexts = NULL;
    CoTaskMemFree(m_Params);
    m_Params = NULL;
    CoTaskMemFree(m_ParamInfos);
    m_ParamInfos = NULL;
}

HRESULT GenDMOPropPage::OnDeactivate()
{
    // as a side effect this updates the
    // values from the controls before
    // we destroy the window
    SetIsDirty(HasAnythingChanged());

    m_ListBox = NULL;
    m_EditBox = NULL;
    m_CheckBox = NULL;
    m_Slider = NULL;
    m_Scrollbar = NULL;
    m_Combo = NULL;
    m_DefaultsBtn = NULL;
    m_BoolDesc = NULL;

    return S_OK;
}

HRESULT GenDMOPropPage::Apply(void)
{
    if(IsWindow(m_hWnd) && m_MediaParams)
    {
        GetValueFromControls();
    }

    bool ShowMessage = false;

    for (DWORD i(0); i < m_NumParams; ++i)
    {
        MP_DATA CurrentValue;
        HRESULT hr = m_MediaParams->GetParam(i, &CurrentValue);
        if(FAILED(hr)) return hr;
        if(CurrentValue != m_Params[i])
        {
            hr = m_MediaParams->SetParam(i, m_Params[i]);
            if(hr == S_FALSE)
            {
                ShowMessage = true;
            }
        }
    }

    // save the values to the registry on apply
    if(m_SaveDefaults)
    {
        m_SaveDefaults->SaveDefaultsToRegistry();
    }

    if(ShowMessage)
    {
        MessageBoxW(m_hWnd, L"Some of the properties you attempted to change cannot be altered "
                            L"while filter is connected, you will need to restart the program "
                            L"for the change to take effect.", L"Warning", MB_OK);
    }

    SetIsDirty(FALSE);

    if(IsWindow(m_hWnd))
    {
        SetupControls();
    }

    return S_OK;
}

STDMETHODIMP GenDMOPropPage::SetObjects(ULONG cObjects,IUnknown **ppUnk)
{
    if(cObjects != 1)
    {
        return E_UNEXPECTED;
    }
    m_MediaParamInfo = *ppUnk;
    m_MediaParams = *ppUnk;
    m_SaveDefaults = *ppUnk;

    if(m_MediaParamInfo != NULL && m_MediaParams != NULL)
    {
        // How many parameters have we got
        HRESULT hr = m_MediaParamInfo->GetParamCount(&m_NumParams);
        if(FAILED(hr)) return hr;

        // if there are no parameters then just return
        if(m_NumParams == 0)
        {
            m_Params = NULL;
            m_ParamInfos = NULL;
            return S_OK;
        }

        // allocate enough memory for the information structures
        m_Params = (MP_DATA*)CoTaskMemAlloc(m_NumParams * sizeof(MP_DATA));
        if(m_Params == NULL)
        {
            return E_OUTOFMEMORY;
        }
        m_ParamInfos = (MP_PARAMINFO*)CoTaskMemAlloc(m_NumParams * sizeof(MP_PARAMINFO));
        if(m_ParamInfos == NULL)
        {
            return E_OUTOFMEMORY;
        }
        m_ParamTexts = (WCHAR**)CoTaskMemAlloc(m_NumParams * sizeof(WCHAR*));
        if(m_ParamTexts == NULL)
        {
            return E_OUTOFMEMORY;
        }

        // go and get the current state and information
        // and load up the names into the list box
        for(DWORD i(0); i < m_NumParams; ++i)
        {
            hr = m_MediaParamInfo->GetParamInfo(i, &m_ParamInfos[i]);
            if(FAILED(hr)) return hr;
            hr = m_MediaParams->GetParam(i, &m_Params[i]);
            if(FAILED(hr)) return hr;
            hr = m_MediaParamInfo->GetParamText(i, &m_ParamTexts[i]);
            if(FAILED(hr)) return hr;
        }
    }
    else
    {
        m_MediaParamInfo.Detach();
        m_MediaParams.Detach();
    }

    return S_OK;
}

HRESULT GenDMOPropPage::OnActivate()
{
    if(m_MediaParamInfo == NULL || m_MediaParams == NULL)
    {
        return E_UNEXPECTED;
    }

    if(IsWindow(m_hWnd))
    {
        // Initialize the dialog
        m_ListBox = GetDlgItem(m_hWnd, IDC_PARAMETERLIST);
        m_EditBox = GetDlgItem(m_hWnd, IDC_EDIT);
        m_CheckBox = GetDlgItem(m_hWnd, IDC_CHECK);
        m_Slider = GetDlgItem(m_hWnd, IDC_SLIDER);
        m_Scrollbar = GetDlgItem(m_hWnd, IDC_SCROLLBAR);
        m_Combo = GetDlgItem(m_hWnd, IDC_COMBO);
        m_DefaultsBtn = GetDlgItem(m_hWnd, IDC_SAVEDEFAULTS);
        m_BoolDesc = GetDlgItem(m_hWnd, IDC_BOOLDESC);

        ShowWindow(m_EditBox, SW_HIDE);
        ShowWindow(m_CheckBox, SW_HIDE);
        ShowWindow(m_Slider, SW_HIDE);
        ShowWindow(m_Scrollbar, SW_HIDE);
        ShowWindow(m_Combo, SW_HIDE);
        ShowWindow(m_BoolDesc, SW_HIDE);

        if(!m_SaveDefaults)
        {
            ShowWindow(m_DefaultsBtn, SW_HIDE);
        }

        // load up the names into the list box
        SendMessage(m_ListBox, LB_RESETCONTENT, 0, 0);
        SendMessage(m_ListBox, LB_SETHORIZONTALEXTENT,0,0);

        int MaxWidth=0;
        for(DWORD i(0); i < m_NumParams; ++i)
        {
            SendMessageW(m_ListBox, LB_ADDSTRING, 0, (LPARAM)m_ParamTexts[i]);

            // get the size of the text and adjust the horizontal scrollbar if nessesary
            SIZE size;
            GetTextSize(m_ParamTexts[i], size);
            if(size.cx > MaxWidth)
            {
                MaxWidth = size.cx;
                SendMessage(m_ListBox, LB_SETHORIZONTALEXTENT, MaxWidth + 4, 0);
            }
        }
        m_CurrentParam = 0;

        SetupControls();
    }

    return S_OK;
}

void GenDMOPropPage::GetTextSize(WCHAR *wcItem, SIZE &size)
{
    HDC hDC = GetDC(m_ListBox);
    HFONT hListBoxFont = (HFONT)SendMessage(m_ListBox, WM_GETFONT, 0, 0);
    if(hListBoxFont != NULL)
    {
        HFONT hOldFont = (HFONT)SelectObject(hDC,(HGDIOBJ)hListBoxFont);
        GetTextExtentPoint32W(hDC, wcItem, (int)wcslen(wcItem), &size);
        SelectObject(hDC,hOldFont);
    }
    ReleaseDC(m_ListBox, hDC);
}



void GenDMOPropPage::SetupControls()
{
    if(m_NumParams == 0)
    {
        return;
    }
    ShowWindow(m_EditBox, SW_HIDE);
    ShowWindow(m_CheckBox, SW_HIDE);
    ShowWindow(m_Slider, SW_HIDE);
    ShowWindow(m_Scrollbar, SW_HIDE);
    ShowWindow(m_Combo, SW_HIDE);
    ShowWindow(m_BoolDesc, SW_HIDE);

    if(m_CurrentParam > m_NumParams)
    {
        // avoid crashing
        return;
    }
    SendMessage(m_ListBox, LB_SETCURSEL, m_CurrentParam, 0);

    WCHAR* CurParamText = &m_ParamTexts[m_CurrentParam][0];

    switch(m_ParamInfos[m_CurrentParam].mpType)
    {
    case MPT_INT:
        ShowWindow(m_EditBox, SW_SHOW);
        SetupIntValue();
        break;
    case MPT_FLOAT:
        ShowWindow(m_EditBox, SW_SHOW);
        SetupFloatValue();
        break;
    case MPT_BOOL:
        ShowWindow(m_CheckBox, SW_SHOW);
        ShowWindow(m_BoolDesc, SW_SHOW);
        SendMessageW(m_BoolDesc, WM_SETTEXT, 0, (LPARAM)CurParamText);
        SetupBoolValue();
        break;
    case MPT_ENUM:
        ShowWindow(m_Combo, SW_SHOW);
        SetupEnumCombo();
        SetupEnumValue();
        break;
    default:
        break;
    }

    // skip past name and get to units
    CurParamText += wcslen(CurParamText) + 1;
    if(CurParamText == L'\0') return;

    // select the units
    SendMessageW(GetDlgItem(m_hWnd, IDC_UNITS), WM_SETTEXT, 0, (LPARAM)CurParamText);

}

void GenDMOPropPage::SetupIntValue()
{
    BSTR Text = NULL;
    HRESULT hr = VarBstrFromI4((long)m_Params[m_CurrentParam], 0x0409, LOCALE_NOUSEROVERRIDE, &Text);
    if(SUCCEEDED(hr))
    {
        SendMessageW(m_EditBox, WM_SETTEXT, 0, (LPARAM)Text);
        SysFreeString(Text);
    }
}

void GenDMOPropPage::SetupFloatValue()
{
    BSTR Text = NULL;
    HRESULT hr = VarBstrFromR4((float)m_Params[m_CurrentParam], 0x0409, LOCALE_NOUSEROVERRIDE, &Text);
    if(SUCCEEDED(hr))
    {
        SendMessageW(m_EditBox, WM_SETTEXT, 0, (LPARAM)Text);
        SysFreeString(Text);
    }
}

void GenDMOPropPage::SetupBoolValue()
{
    if(m_Params[m_CurrentParam] != 0)
    {
        SendMessage(m_CheckBox, BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
        SendMessage(m_CheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    }
}

void GenDMOPropPage::SetupEnumValue()
{
    long Sel = (long)m_Params[m_CurrentParam];
    SendMessage(m_CheckBox, CB_SETCURSEL, Sel, 0);
}

void GenDMOPropPage::SetupEnumCombo()
{
    //empty combo
    SendMessage(m_Combo, CB_RESETCONTENT, 0, 0);

    WCHAR* CurString = &m_ParamTexts[m_CurrentParam][0];

    // skip past name and units
    CurString += wcslen(CurString) + 1;
    if(CurString == L'\0') return;
    CurString += wcslen(CurString) + 1;
    if(CurString == L'\0') return;

    // the text values are in a double null terminated string
    // so we keep going until we find an empty string
    while(*CurString != L'\0')
    {
        SendMessageW(m_Combo, CB_ADDSTRING, 0, (LPARAM)CurString);
        CurString += wcslen(CurString) + 1;
    }
    LRESULT Items = SendMessage(m_Combo, CB_GETCOUNT, 0, 0);
    if((LRESULT)m_Params[m_CurrentParam] < Items)
    {
        SendMessage(m_Combo, CB_SETCURSEL, (long)m_Params[m_CurrentParam], 0);
    }
}

void GenDMOPropPage::GetValueFromControls()
{
    if(m_ParamInfos == NULL)
    {
        return;
    }
    switch(m_ParamInfos[m_CurrentParam].mpType)
    {
    case MPT_INT:
        GetIntValue();
        break;
    case MPT_FLOAT:
        GetFloatValue();
        break;
    case MPT_BOOL:
        GetBoolValue();
        break;
    case MPT_ENUM:
        GetEnumValue();
        break;
    default:
        break;
    }
}

void GenDMOPropPage::GetIntValue()
{
    std::vector<wchar_t> Text(128);
    GetWindowTextW(m_EditBox, &Text[0], 128);
    long NewValue;
    HRESULT hr = VarI4FromStr(&Text[0], 0x0409, LOCALE_NOUSEROVERRIDE, &NewValue);
    if(SUCCEEDED(hr))
    {
        m_Params[m_CurrentParam] = (MP_DATA)NewValue;
    }
}

void GenDMOPropPage::GetFloatValue()
{
    std::vector<wchar_t> Text(128);
    GetWindowTextW(m_EditBox, &Text[0], 128);
    float NewValue;
    HRESULT hr = VarR4FromStr(&Text[0], 0x0409, LOCALE_NOUSEROVERRIDE, &NewValue);
    if(SUCCEEDED(hr))
    {
        m_Params[m_CurrentParam] = (MP_DATA)NewValue;
    }
}

void GenDMOPropPage::GetBoolValue()
{
    m_Params[m_CurrentParam] = (MP_DATA)(BOOL)(SendMessage(m_CheckBox, BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void GenDMOPropPage::GetEnumValue()
{
    m_Params[m_CurrentParam] = (MP_DATA)(long)SendMessage(m_Combo, CB_GETCURSEL, 0, 0);
}

void GenDMOPropPage::OnListSelChange(UINT wNotifyCode, int wID, HWND hWndCtl)
{
    GetValueFromControls();
    SetIsDirty(HasAnythingChanged());
    m_CurrentParam = SendMessage(m_ListBox, LB_GETCURSEL, 0, 0);
    SetupControls();
}

BOOL GenDMOPropPage::HasAnythingChanged()
{
    GetValueFromControls();

    for (DWORD i(0); i < m_NumParams; ++i)
    {
        MP_DATA CurrentValue;
        HRESULT hr = m_MediaParams->GetParam(i, &CurrentValue);
        if(FAILED(hr)) return hr;
        if(CurrentValue != m_Params[i])
        {
            return TRUE;
        }
    }
    return FALSE;
}

void GenDMOPropPage::OnEditChange(UINT wNotifyCode, int wID, HWND hWndCtl)
{
    SetIsDirty(HasAnythingChanged());
}

void GenDMOPropPage::OnComboChange(UINT wNotifyCode, int wID, HWND hWndCtl)
{
    SetIsDirty(HasAnythingChanged());
}

void GenDMOPropPage::OnCheckBoxClick(UINT wNotifyCode, int wID, HWND hWndCtl)
{
    SetIsDirty(HasAnythingChanged());
}

void GenDMOPropPage::OnBnClickedResetdefaults(UINT wNotifyCode, int wID, HWND hWndCtl)
{
    for (DWORD i(0); i < m_NumParams; ++i)
    {
        m_Params[i] = m_ParamInfos[i].mpdNeutralValue;
    }

    SetupControls();
    SetIsDirty(HasAnythingChanged());
}

void GenDMOPropPage::OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify)
{
    switch(id)
    {
    case IDC_PARAMETERLIST:
        if(LBN_SELCHANGE == codeNotify)
        {
            OnListSelChange(codeNotify, id, hwndCtl);
        }
        break;
    case IDC_EDIT:
        if(EN_CHANGE == codeNotify)
        {
            OnEditChange(codeNotify, id, hwndCtl);
        }
        break;
    case IDC_COMBO:
        if(CBN_SELCHANGE == codeNotify)
        {
            OnComboChange(codeNotify, id, hwndCtl);
        }
        break;
    case IDC_CHECK:
        if(BN_CLICKED == codeNotify)
        {
            OnCheckBoxClick(codeNotify, id, hwndCtl);
        }
        break;
    case IDC_RESETDEFAULTS:
        if(BN_CLICKED == codeNotify)
        {
            OnBnClickedResetdefaults(codeNotify, id, hwndCtl);
        }
        break;
    }
}


INT_PTR GenDMOPropPage::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    HANDLE_MSG(hDlg, WM_COMMAND, OnCommand);
    default:
        return FALSE;
    }
}

