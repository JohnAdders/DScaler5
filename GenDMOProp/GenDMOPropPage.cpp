// GenDMOPropPage.cpp : Implementation of CGenDMOPropPage
#include "stdafx.h"
#include "GenDMOProp.h"
#include "GenDMOPropPage.h"

/////////////////////////////////////////////////////////////////////////////
// CGenDMOPropPage

CGenDMOPropPage::CGenDMOPropPage() 
{
	m_dwTitleID = IDS_TITLEGenDMOPropPage;
	m_dwHelpFileID = IDS_HELPFILEGenDMOPropPage;
	m_dwDocStringID = IDS_DOCSTRINGGenDMOPropPage;
}

STDMETHODIMP CGenDMOPropPage::Apply(void)
{
	ATLTRACE(_T("CGenDMOPropPage::Apply\n"));
	for (UINT i = 0; i < m_nObjects; i++)
	{
		// Do something interesting here
		// ICircCtl* pCirc;
		// m_ppUnk[i]->QueryInterface(IID_ICircCtl, (void**)&pCirc);
		// pCirc->put_Caption(CComBSTR("something special"));
		// pCirc->Release();
	}
	m_bDirty = FALSE;
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

    if(m_MediaParamInfo != NULL && m_MediaParams != NULL)
    {
        return S_OK;
    }
    else
    {
        m_MediaParamInfo.Release();
        m_MediaParams.Release();
        return E_NOINTERFACE;
    }
}   