///////////////////////////////////////////////////////////////////////////////
// $Id: GenDMOPropPage.cpp,v 1.16 2004-10-29 11:39:34 adcockj Exp $
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
// Revision 1.15  2004/07/30 20:13:20  adcockj
// Make property page behave sensibly when there are multiple changes
//
// Revision 1.14  2004/07/24 21:45:51  adcockj
// Fixed asset errors (thanks Torbjörn)
//
// Revision 1.13  2004/07/20 16:37:57  adcockj
// Fixes for main issues raised in testing of 0.0.1
//  - Improved parameter handling
//  - Fixed some overlay issues
//  - Auto aspect ratio with VMR
//  - Fixed some overlay stutters
//  - Fixed some push filter issues
//  - ffdshow and DirectVobSub connection issues
//
// Added
//  - Hardcode for PAL setting for ffdshow
//  - Added choice of IDCT for testing
//
// Revision 1.12  2004/07/01 21:16:55  adcockj
// Another load of fixes to recent changes
//
// Revision 1.11  2004/07/01 16:12:48  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.10  2004/05/10 16:50:26  adcockj
// Added apply before save defaults to make UI a bit more intuituve
//
// Revision 1.9  2004/03/15 17:17:06  adcockj
// Basic registry saving support
//
// Revision 1.8  2003/07/21 08:44:41  adcockj
// Added HScroll patch from Torbjorn
//
// Revision 1.7  2003/05/17 11:29:36  adcockj
// Fixed crashing
//
// Revision 1.6  2003/05/16 16:27:45  adcockj
// Added a bit of a safety for the property page
//
// Revision 1.5  2003/05/07 16:27:03  adcockj
// fixes for odd behaviour with multiple pages
//
// Revision 1.4  2003/05/06 07:01:05  adcockj
// Fixes for crashing with multiple pages
//
// Revision 1.3  2003/05/02 15:52:25  adcockj
// Initial limited version of Generic prop page
//
// Revision 1.2  2003/05/01 12:34:41  adcockj
// Added headers and new license page
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GenDMOProp.h"
#include "GenDMOPropPage.h"
#include ".\gendmoproppage.h"

/////////////////////////////////////////////////////////////////////////////
// CGenDMOPropPage

CGenDMOPropPage::CGenDMOPropPage() 
{
    m_dwTitleID = IDS_TITLEGenDMOPropPage;
    m_dwHelpFileID = IDS_HELPFILEGenDMOPropPage;
    m_dwDocStringID = IDS_DOCSTRINGGenDMOPropPage;
    m_NumParams = 0;
    m_ParamTexts = 0;
    m_Params = 0;
    m_ParamInfos = 0;
}

CGenDMOPropPage::~CGenDMOPropPage() 
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

STDMETHODIMP CGenDMOPropPage::Deactivate()
{
    // as a side effect this updates the
    // values from the controls before 
    // we destroy the window
    SetDirty(HasAnythingChanged());

    m_ListBox.Detach();
    m_EditBox.Detach();
    m_CheckBox.Detach();
    m_Slider.Detach();
    m_Scrollbar.Detach();
    m_Combo.Detach();
    m_DefaultsBtn.Detach();

    return IPropertyPageImpl<CGenDMOPropPage>::Deactivate();
}

STDMETHODIMP CGenDMOPropPage::Apply(void)
{
    ATLTRACE(_T("CGenDMOPropPage::Apply\n"));

    if(IsWindow())
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
                // since we tell the user that it will be set next time
                // we had better save it so that it actually is set next time
                hr = m_SaveDefaults->SaveDefaultToRegistry(i);
            }
        }
    }

    if(ShowMessage)
    {
        MessageBox("Some of the properties you attempted to change cannot be altered while filter is connected, you will need to restart the program for the change to take effect.", "Warning", MB_OK); 
    }

    m_bDirty = FALSE;

    if(::IsWindow(m_hWnd))
    {
        SetupControls();
    }

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

        return S_OK;
    }
    else
    {
        m_MediaParamInfo.Release();
        m_MediaParams.Release();
        return E_NOINTERFACE;
    }
}   

STDMETHODIMP CGenDMOPropPage::Activate(HWND hWndParent,LPCRECT pRect,BOOL bModal)
{
    HRESULT hr = IPropertyPageImpl<CGenDMOPropPage>::Activate(hWndParent,pRect,bModal);
    if(FAILED(hr))
    {
        return hr;
    }
    if(::IsWindow(m_hWnd))
    {
        // Initialize the dialog
        m_ListBox.Attach(GetDlgItem(IDC_PARAMETERLIST));
        m_EditBox.Attach(GetDlgItem(IDC_EDIT));
        m_CheckBox.Attach(GetDlgItem(IDC_CHECK));
        m_Slider.Attach(GetDlgItem(IDC_SLIDER));
        m_Scrollbar.Attach(GetDlgItem(IDC_SCROLLBAR));
        m_Combo.Attach(GetDlgItem(IDC_COMBO));
        m_DefaultsBtn.Attach(GetDlgItem(IDC_SAVEDEFAULTS));

        m_EditBox.ShowWindow(SW_HIDE);
        m_CheckBox.ShowWindow(SW_HIDE);
        m_Slider.ShowWindow(SW_HIDE);
        m_Scrollbar.ShowWindow(SW_HIDE);
        m_Combo.ShowWindow(SW_HIDE);

        if(!m_SaveDefaults)
        {
            m_DefaultsBtn.ShowWindow(SW_HIDE);
        }

        // load up the names into the list box
        m_ListBox.SendMessage(LB_RESETCONTENT, 0, 0);
        m_ListBox.SendMessage(LB_SETHORIZONTALEXTENT,0,0);

        int MaxWidth=0;
        for(DWORD i(0); i < m_NumParams; ++i)
        {
            SendMessageW(m_ListBox.m_hWnd, LB_ADDSTRING, 0, (LPARAM)m_ParamTexts[i]);

            // get the size of the text and adjust the horizontal scrollbar if nessesary
            SIZE size;
            GetTextSize(m_ParamTexts[i], size);
            if(size.cx > MaxWidth)
            {
                MaxWidth = size.cx;
                m_ListBox.SendMessage(LB_SETHORIZONTALEXTENT, MaxWidth + 4, 0);
            }
        }
        m_CurrentParam = 0;
        
        SetupControls();
    }

    return hr;
}

void CGenDMOPropPage::GetTextSize(WCHAR *wcItem, SIZE &size)
{
    HDC hDC = m_ListBox.GetDC();
    HFONT hListBoxFont = m_ListBox.GetFont();
    if(hListBoxFont != NULL)
    {
        HFONT hOldFont = (HFONT)SelectObject(hDC,(HGDIOBJ)hListBoxFont);
        GetTextExtentPoint32W(hDC, wcItem, wcslen(wcItem), &size);
        SelectObject(hDC,hOldFont);
    }
    ReleaseDC(hDC);
}



void CGenDMOPropPage::SetupControls()
{
    if(m_NumParams == 0)
    {   
        return;
    }
    m_EditBox.ShowWindow(SW_HIDE);
    m_CheckBox.ShowWindow(SW_HIDE);
    m_Slider.ShowWindow(SW_HIDE);
    m_Scrollbar.ShowWindow(SW_HIDE);
    m_Combo.ShowWindow(SW_HIDE);

    if(m_CurrentParam > m_NumParams)
    {
        // avoid crashing
        return;
    }
    m_ListBox.SendMessage(LB_SETCURSEL, m_CurrentParam, 0);

    WCHAR* CurParamText = &m_ParamTexts[m_CurrentParam][0];

    switch(m_ParamInfos[m_CurrentParam].mpType)
    {
    case MPT_INT:
        m_EditBox.ShowWindow(SW_SHOW);
        //m_Slider.ShowWindow(SW_SHOW);
        //m_Scrollbar.ShowWindow(SW_SHOW);
        SetupIntValue();
        break;
    case MPT_FLOAT:
        m_EditBox.ShowWindow(SW_SHOW);
        //m_Slider.ShowWindow(SW_SHOW);
        //m_Scrollbar.ShowWindow(SW_SHOW);
        SetupFloatValue();
        break;
    case MPT_BOOL:
        m_CheckBox.ShowWindow(SW_SHOW);
        SendMessageW(m_CheckBox.m_hWnd, WM_SETTEXT, 0, (LPARAM)CurParamText);
        SetupBoolValue();
        break;
    case MPT_ENUM:
        m_Combo.ShowWindow(SW_SHOW);
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
    SendMessageW(GetDlgItem(IDC_UNITS), WM_SETTEXT, 0, (LPARAM)CurParamText);

}

void CGenDMOPropPage::SetupIntValue()
{
    BSTR Text = NULL;
    HRESULT hr = VarBstrFromI4((long)m_Params[m_CurrentParam], 0x0409, LOCALE_NOUSEROVERRIDE, &Text);
    if(SUCCEEDED(hr))
    {
        SendMessageW(m_EditBox.m_hWnd, WM_SETTEXT, 0, (LPARAM)Text);
        SysFreeString(Text);
    }
}

void CGenDMOPropPage::SetupFloatValue()
{
    BSTR Text = NULL;
    HRESULT hr = VarBstrFromR4((float)m_Params[m_CurrentParam], 0x0409, LOCALE_NOUSEROVERRIDE, &Text);
    if(SUCCEEDED(hr))
    {
        SendMessageW(m_EditBox.m_hWnd, WM_SETTEXT, 0, (LPARAM)Text);
        SysFreeString(Text);
    }
}

void CGenDMOPropPage::SetupBoolValue()
{
    if(m_Params[m_CurrentParam] != 0)
    {
        m_CheckBox.SendMessage(BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
        m_CheckBox.SendMessage(BM_SETCHECK, BST_UNCHECKED, 0);
    }
}

void CGenDMOPropPage::SetupEnumValue()
{
    long Sel = (long)m_Params[m_CurrentParam];
    m_CheckBox.SendMessage(CB_SETCURSEL, Sel, 0);
}

void CGenDMOPropPage::SetupEnumCombo()
{
    //empty combo
    m_Combo.SendMessage(CB_RESETCONTENT, 0, 0);

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
        SendMessageW(m_Combo.m_hWnd, CB_ADDSTRING, 0, (LPARAM)CurString);
        CurString += wcslen(CurString) + 1;
    }
    long Items = m_Combo.SendMessage(CB_GETCOUNT, 0, 0);
    if((long)m_Params[m_CurrentParam] < Items)
    {
        m_Combo.SendMessage(CB_SETCURSEL, (long)m_Params[m_CurrentParam], 0);
    }
}

void CGenDMOPropPage::GetValueFromControls()
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

void CGenDMOPropPage::GetIntValue()
{
    BSTR Text = NULL;
    m_EditBox.GetWindowText(Text);
    long NewValue;
    HRESULT hr = VarI4FromStr(Text, 0x0409, LOCALE_NOUSEROVERRIDE, &NewValue);
    if(SUCCEEDED(hr))
    {
        m_Params[m_CurrentParam] = (MP_DATA)NewValue;
    }
    SysFreeString(Text);
}

void CGenDMOPropPage::GetFloatValue()
{
    BSTR Text = NULL;
    m_EditBox.GetWindowText(Text);
    float NewValue;
    HRESULT hr = VarR4FromStr(Text, 0x0409, LOCALE_NOUSEROVERRIDE, &NewValue);
    if(SUCCEEDED(hr))
    {
        m_Params[m_CurrentParam] = (MP_DATA)NewValue;
    }
    SysFreeString(Text);
}

void CGenDMOPropPage::GetBoolValue()
{
    m_Params[m_CurrentParam] = (MP_DATA)(BOOL)(m_CheckBox.SendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED);
}

void CGenDMOPropPage::GetEnumValue()
{
    m_Params[m_CurrentParam] = (MP_DATA)(long)m_Combo.SendMessage(CB_GETCURSEL, 0, 0);
}

LRESULT CGenDMOPropPage::OnListSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    GetValueFromControls();
    if(!m_bDirty)
    {
        SetDirty(HasAnythingChanged());
    }
    m_CurrentParam = m_ListBox.SendMessage(LB_GETCURSEL, 0, 0);
    SetupControls();
    return 0;
}

BOOL CGenDMOPropPage::HasAnythingChanged()
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

LRESULT CGenDMOPropPage::OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(!m_bDirty)
    {
        SetDirty(HasAnythingChanged());
    }
    return 0;
}

LRESULT CGenDMOPropPage::OnComboChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(!m_bDirty)
    {
        SetDirty(HasAnythingChanged());
    }
    return 0;
}

LRESULT CGenDMOPropPage::OnCheckBoxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(!m_bDirty)
    {
        SetDirty(HasAnythingChanged());
    }
    return 0;
}

LRESULT CGenDMOPropPage::OnSaveDefaultsClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if(m_SaveDefaults)
    {
        // save the current values before setting them as default
        Apply();
        m_SaveDefaults->SaveDefaultsToRegistry();
    }
    return 0;
}

// we need to override this to get 
// the normal apply and cancel  behaviour
void CGenDMOPropPage::SetDirty(BOOL bDirty)
{
    if (m_bDirty != bDirty)
    {
        m_bDirty = bDirty;
        m_pPageSite->OnStatusChange(bDirty ? PROPPAGESTATUS_DIRTY : 0);
    }
}

LRESULT CGenDMOPropPage::OnBnClickedResetdefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
    for (DWORD i(0); i < m_NumParams; ++i)
    {
        m_Params[i] = m_ParamInfos[i].mpdNeutralValue;
    }
    SetDirty(HasAnythingChanged());
    SetupControls();
    return 0;
}
