///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock
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

#ifndef __SIMPLEPROPERTYPAGE_H_
#define __SIMPLEPROPERTYPAGE_H_

typedef struct {
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} MYDLGTEMPLATEEX;

/////////////////////////////////////////////////////////////////////////////
// StatisticsPropPage
class SimplePropertyPage :
      public IPropertyPage
{
public:
    STDMETHOD(SetPageSite)(IPropertyPageSite *pPageSite);
    STDMETHOD(Activate)(HWND hWndParent, LPCRECT pRect, BOOL bModal);
    STDMETHOD(Deactivate)(void);
    STDMETHOD(GetPageInfo)(PROPPAGEINFO *pPageInfo);
    STDMETHOD(Show)(UINT nCmdShow);
    STDMETHOD(Move)(LPCRECT pRect);
    STDMETHOD(IsPageDirty)(void);
    STDMETHOD(Help)(LPCOLESTR pszHelpDir);
    STDMETHOD(TranslateAccelerator)(MSG *pMsg);
protected:
    SimplePropertyPage(LPCTSTR ResourceId, UINT TitleStringId, UINT DocStringId, UINT HelpStringId, DWORD HelpContext);
    virtual ~SimplePropertyPage();
    void SetIsDirty(BOOL IsDirty);
    LPOLESTR LoadResourceString(UINT StringId);
    HWND m_hWnd;
private:
    SIZE GetDialogBoxSize();
    virtual INT_PTR DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
    static INT_PTR CALLBACK MasterModalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    virtual HRESULT OnActivate() = 0;
    virtual HRESULT OnDeactivate() = 0;
    SI(IPropertyPageSite) m_PageSite;
    MYDLGTEMPLATEEX*  m_DlgTemplate;
    UINT m_TitleStringId;
    UINT m_DocStringId;
    UINT m_HelpStringId;
    DWORD m_HelpContext;
    int m_HelpID;
    BOOL m_IsDirty;
};

#endif
