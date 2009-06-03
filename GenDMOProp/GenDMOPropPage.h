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

#ifndef __GENDMOPROPPAGE_H_
#define __GENDMOPROPPAGE_H_

#include "resource.h"       // main symbols
#include "GenDMOProp.h"
#include "SimplePropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// GenDMOPropPage
class GenDMOPropPage :
      public SimplePropertyPage
{
public:
    GenDMOPropPage();
    ~GenDMOPropPage();

IMPLEMENT_AGGREGATABLE_COCLASS(GenDMOPropPage, "{0E938757-7AED-11D7-B84B-0002A5623377}", "GenDMOPropPage Class", "GenDMOProp.GenDMOPropPage.1", "GenDMOProp.GenDMOPropPage", "both")
    IMPLEMENTS_INTERFACE(IPropertyPage)
    IMPLEMENTS_INTERFACE(IUnknown)
END_INTERFACE_TABLE()

// Handler prototypes:
    void OnListSelChange(UINT wNotifyCode, int wID, HWND hWndCtl);
    void OnEditChange(UINT wNotifyCode, int wID, HWND hWndCtl);
    void OnComboChange(UINT wNotifyCode, int wID, HWND hWndCtl);
    void OnCheckBoxClick(UINT wNotifyCode, int wID, HWND hWndCtl);
    void OnBnClickedResetdefaults(UINT wNotifyCode, int wID, HWND hWndCtl);

    STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
    BOOL HasAnythingChanged();

private:
    void OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify);
    void GetValueFromControls();
    void GetIntValue();
    void GetFloatValue();
    void GetBoolValue();
    void GetEnumValue();
    void SetupControls();
    void SetupIntValue();
    void SetupFloatValue();
    void SetupBoolValue();
    void SetupEnumValue();
    void SetupEnumCombo();
    void GetTextSize(WCHAR *wcItem,SIZE &size);

private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnActivate();
    virtual HRESULT OnDeactivate();

    SI(IMediaParamInfo) m_MediaParamInfo;
    SI(IMediaParams) m_MediaParams;
    SI(ISaveDefaults) m_SaveDefaults;
    HWND m_EditBox;
    HWND m_ListBox;
    HWND m_CheckBox;
    HWND m_Slider;
    HWND m_Scrollbar;
    HWND m_Combo;
    HWND m_DefaultsBtn;
    HWND m_BoolDesc;
    MP_DATA* m_Params;
    MP_PARAMINFO* m_ParamInfos;
    WCHAR** m_ParamTexts;
    DWORD m_NumParams;
    DWORD m_CurrentParam;
};

#endif //__GENDMOPROPPAGE_H_
