///////////////////////////////////////////////////////////////////////////////
// $Id: GenDMOPropPage.h,v 1.11 2004-11-04 16:09:41 adcockj Exp $
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

EXTERN_C const CLSID CLSID_GenDMOPropPage;

/////////////////////////////////////////////////////////////////////////////
// CGenDMOPropPage
class ATL_NO_VTABLE CGenDMOPropPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CGenDMOPropPage, &CLSID_GenDMOPropPage>,
	public IPropertyPageImpl<CGenDMOPropPage>,
	public CDialogImpl<CGenDMOPropPage>
{
public:
	CGenDMOPropPage();
    ~CGenDMOPropPage();

	enum {IDD = IDD_GENDMOPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_GENDMOPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGenDMOPropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CGenDMOPropPage)
	COMMAND_HANDLER(IDC_PARAMETERLIST, LBN_SELCHANGE, OnListSelChange)
    COMMAND_HANDLER(IDC_EDIT, EN_CHANGE, OnEditChange);
    COMMAND_HANDLER(IDC_COMBO, CBN_SELCHANGE, OnComboChange);
    COMMAND_HANDLER(IDC_CHECK, BN_CLICKED, OnCheckBoxClick);
    COMMAND_HANDLER(IDC_RESETDEFAULTS, BN_CLICKED, OnBnClickedResetdefaults)
    CHAIN_MSG_MAP(IPropertyPageImpl<CGenDMOPropPage>)
END_MSG_MAP()
// Handler prototypes:
    LRESULT OnListSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnEditChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnComboChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCheckBoxClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBnClickedResetdefaults(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	STDMETHOD(Activate)(HWND hWndParent,LPCRECT pRect,BOOL bModal);
	STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
	STDMETHOD(Deactivate)(void);
    BOOL HasAnythingChanged();
    void SetDirty(BOOL bDirty);

private:
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
    CComQIPtr<IMediaParamInfo> m_MediaParamInfo;
    CComQIPtr<IMediaParams> m_MediaParams;
    CComQIPtr<ISaveDefaults> m_SaveDefaults;
    CWindow m_EditBox;
    CWindow m_ListBox;
    CWindow m_CheckBox;
    CWindow m_Slider;
    CWindow m_Scrollbar;
    CWindow m_Combo;
    CWindow m_DefaultsBtn;
	CWindow m_BoolDesc;
    MP_DATA* m_Params;
    MP_PARAMINFO* m_ParamInfos;
    WCHAR** m_ParamTexts;
    DWORD m_NumParams;
    DWORD m_CurrentParam;
};

#endif //__GENDMOPROPPAGE_H_
