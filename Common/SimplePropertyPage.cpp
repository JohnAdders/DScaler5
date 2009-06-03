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

#include "stdafx.h"
#include "SimplePropertyPage.h"

extern HINSTANCE g_hInstance;

SimplePropertyPage::SimplePropertyPage(LPCTSTR ResourceId, UINT TitleStringId, UINT DocStringId, UINT HelpStringId, DWORD HelpContext) :
    m_TitleStringId(TitleStringId),
    m_DocStringId(DocStringId),
    m_HelpStringId(HelpStringId),
    m_HelpContext(HelpContext),
    m_hWnd(0),
    m_HelpID(0),
    m_IsDirty(FALSE)
{
    HRSRC hResource = FindResource(g_hInstance, ResourceId, RT_DIALOG);
    HGLOBAL hDialog = LoadResource(g_hInstance, hResource);
    m_DlgTemplate = (MYDLGTEMPLATEEX*)LockResource(hDialog);

    if(!m_DlgTemplate)
    {
        throw std::exception("Can't load resource");
    }
    if(m_DlgTemplate->signature != 0xFFFF)
    {
        throw std::exception("Dialog resource isn't extended format");
    }
 }

SimplePropertyPage::~SimplePropertyPage()
{
}


STDMETHODIMP SimplePropertyPage::SetPageSite(IPropertyPageSite *pPageSite)
{
    m_PageSite = pPageSite;
    return S_OK;
}

STDMETHODIMP SimplePropertyPage::Activate(HWND hWndParent, LPCRECT pRect, BOOL bModal)
{
    m_hWnd = CreateDialogIndirectParam(g_hInstance,(DLGTEMPLATE*)m_DlgTemplate, hWndParent, MasterModalDialogProc, (LPARAM)this);
    if(!m_hWnd)
    {
        return E_UNEXPECTED;
    }
    Move(pRect);
    return OnActivate();
}

STDMETHODIMP SimplePropertyPage::Deactivate(void)
{
    DestroyWindow(m_hWnd);
    m_hWnd = NULL;
    return OnDeactivate();
}

SIZE SimplePropertyPage::GetDialogBoxSize()
{
    RECT Rect = {0, 0, m_DlgTemplate->cx, m_DlgTemplate->cy};
    if(IsWindow(m_hWnd))
    {
        MapDialogRect(m_hWnd,&Rect);
    }
    else
    {
        // we need to account for the font of the dialog box
        // the simplest way is just to use MapDialogRect so we create a
        // temporary dialog box just so that we can call it.
        // problem is that we need a parent window, hopefully there
        // will always be a Desktop window, otherwise this will fail
        HWND hWnd = CreateDialogIndirectParam(g_hInstance,(DLGTEMPLATE*)m_DlgTemplate, GetDesktopWindow(), MasterModalDialogProc, (LPARAM)this);
        MapDialogRect(hWnd,&Rect);
        DestroyWindow(hWnd);
    }
    SIZE Result = {Rect.right, Rect.bottom};
    return Result;
}


STDMETHODIMP SimplePropertyPage::GetPageInfo(PROPPAGEINFO *pPageInfo)
{
    pPageInfo->cb = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle = LoadResourceString(m_TitleStringId);
    if(!pPageInfo->pszTitle)
    {
        return E_UNEXPECTED;
    }

    pPageInfo->size = GetDialogBoxSize();

    pPageInfo->pszDocString = LoadResourceString(m_DocStringId);
    if(!pPageInfo->pszDocString)
    {
        return E_UNEXPECTED;
    }
    pPageInfo->pszHelpFile = LoadResourceString(m_HelpStringId);
    if(!pPageInfo->pszHelpFile)
    {
        return E_UNEXPECTED;
    }
    pPageInfo->dwHelpContext = m_HelpContext;
    return S_OK;
}

STDMETHODIMP SimplePropertyPage::Show(UINT nCmdShow)
{
    if(IsWindow(m_hWnd))
    {
        ShowWindow(m_hWnd, nCmdShow);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP SimplePropertyPage::Move(LPCRECT pRect)
{
    if(IsWindow(m_hWnd))
    {
        MoveWindow(m_hWnd, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, TRUE);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP SimplePropertyPage::IsPageDirty(void)
{
    if(m_IsDirty)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP SimplePropertyPage::Help(LPCOLESTR pszHelpDir)
{
    // just return the one set in info
    return E_NOTIMPL;
}

STDMETHODIMP SimplePropertyPage::TranslateAccelerator(MSG *pMsg)
{
    // we don't support accelerators
    return E_NOTIMPL;
}

void SimplePropertyPage::SetIsDirty(BOOL IsDirty)
{
    m_IsDirty = IsDirty;
    if(m_PageSite)
    {
        m_PageSite->OnStatusChange(m_IsDirty ? PROPPAGESTATUS_DIRTY : 0);
    }
}

INT_PTR CALLBACK SimplePropertyPage::MasterModalDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_INITDIALOG)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
    }
    SimplePropertyPage* myObject = (SimplePropertyPage*)GetWindowLongPtr(hDlg, DWLP_USER);
    if(myObject)
    {
        return myObject->DialogProc(hDlg, message, wParam, lParam);
    }
    else
    {
        return FALSE;
    }
}

LPOLESTR SimplePropertyPage::LoadResourceString(UINT StringId)
{
    DWORD Len(256);
    LPOLESTR Result = (LPOLESTR)CoTaskMemAlloc(Len * sizeof(OLECHAR));
    if(!Result)
    {
        return 0;
    }
    if(LoadStringW(g_hInstance, StringId, Result, Len) != 0)
    {
        return Result;
    }
    CoTaskMemFree(Result);
    return 0;
}
