///////////////////////////////////////////////////////////////////////////////
// $Id: StatisticsPropPage.h,v 1.1 2004-12-15 13:04:10 adcockj Exp $
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

#ifndef __STATISTICSPROPPAGE_H_
#define __STATISTICSPROPPAGE_H_

#include "resource.h"       // main symbols

EXTERN_C const CLSID CLSID_StatisticsPropPage;

/////////////////////////////////////////////////////////////////////////////
// CStatisticsPropPage
class ATL_NO_VTABLE CStatisticsPropPage :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CStatisticsPropPage, &CLSID_LicensePropPage>,
	public IPropertyPageImpl<CStatisticsPropPage>,
	public CDialogImpl<CStatisticsPropPage>
{
public:
	CStatisticsPropPage();

	enum {IDD = IDD_STATISTICSPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_STATISTICSPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStatisticsPropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CStatisticsPropPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CStatisticsPropPage>)
END_MSG_MAP()

	STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);
    STDMETHOD(Activate)(HWND hWndParent,LPCRECT pRect,BOOL bModal);

private:
    CComQIPtr<IHaveStatistics> m_Statistics;
};

#endif //__LICENSEPROPPAGE_H_
