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
#include "GenDMOProp.h"
#include "SimplePropertyPage.h"


/////////////////////////////////////////////////////////////////////////////
// LicensePropPage
class LicensePropPage :
      public SimplePropertyPage
{
public:
    LicensePropPage();

IMPLEMENT_AGGREGATABLE_COCLASS(LicensePropPage, "{FDA2243F-7BAA-11D7-B84B-0002A5623377}", "'LicensePropPage Class", "GenDMOProp.'LicensePropPage.1", "GenDMOProp.'LicensePropPage", "both")
    IMPLEMENTS_INTERFACE(IPropertyPage)
    IMPLEMENTS_INTERFACE(IUnknown)
END_INTERFACE_TABLE()

    STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);

private:
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnActivate();
    virtual HRESULT OnDeactivate();
    SI(IAmFreeSoftwareLicensed) m_License;
};

#endif //__LICENSEPROPPAGE_H_
