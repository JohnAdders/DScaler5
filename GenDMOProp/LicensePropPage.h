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

#ifndef __LICENSEPROPPAGE_H_
#define __LICENSEPROPPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_LicensePropPage;

/////////////////////////////////////////////////////////////////////////////
// CLicensePropPage
class ATL_NO_VTABLE CLicensePropPage :
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CLicensePropPage, &CLSID_LicensePropPage>,
	public IPropertyPageImpl<CLicensePropPage>,
	public CDialogImpl<CLicensePropPage>
{
public:
	CLicensePropPage();

	enum {IDD = IDD_LICENSEPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_LICENSEPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CLicensePropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CLicensePropPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CLicensePropPage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
    STDMETHOD(Activate)(HWND hWndParent,LPCRECT pRect,BOOL bModal);

private:
    CComQIPtr<IAmFreeSoftwareLicensed> m_License;
};

#endif //__LICENSEPROPPAGE_H_
