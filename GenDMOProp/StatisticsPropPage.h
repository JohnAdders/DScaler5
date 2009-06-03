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

#ifndef __STATISTICSPROPPAGE_H_
#define __STATISTICSPROPPAGE_H_

#include "resource.h"       // main symbols
#include "GenDMOProp.h"
#include "SimplePropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// StatisticsPropPage
class StatisticsPropPage :
      public SimplePropertyPage
{
public:
    StatisticsPropPage();

IMPLEMENT_AGGREGATABLE_COCLASS(StatisticsPropPage, "{3DDF3FE5-24D4-4289-9143-E1FFD70CD934}", "StatisticsPropPage Class", "GenDMOProp.StatisticsPropPage.1", "GenDMOProp.StatisticsPropPage", "both")
    IMPLEMENTS_INTERFACE(IPropertyPage)
    IMPLEMENTS_INTERFACE(IUnknown)
END_INTERFACE_TABLE()

    STDMETHOD(SetObjects)(ULONG cObjects, IUnknown **ppUnk);
    STDMETHOD(Apply)(void);

private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnActivate();
    virtual HRESULT OnDeactivate();
    SI(IHaveStatistics) m_Statistics;
};

#endif //__LICENSEPROPPAGE_H_
