// GenDMOPropPage.h : Declaration of the CGenDMOPropPage

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

	enum {IDD = IDD_GENDMOPROPPAGE};

DECLARE_REGISTRY_RESOURCEID(IDR_GENDMOPROPPAGE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CGenDMOPropPage) 
	COM_INTERFACE_ENTRY(IPropertyPage)
END_COM_MAP()

BEGIN_MSG_MAP(CGenDMOPropPage)
	CHAIN_MSG_MAP(IPropertyPageImpl<CGenDMOPropPage>)
END_MSG_MAP()
// Handler prototypes:
//  LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
//  LRESULT CommandHandler(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
//  LRESULT NotifyHandler(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

	STDMETHOD(Apply)(void);
    STDMETHOD(SetObjects)(ULONG cObjects,IUnknown **ppUnk);

private:
    CComQIPtr<IMediaParamInfo> m_MediaParamInfo;
    CComQIPtr<IMediaParams> m_MediaParams;
    long m_CountParams;
    float* m_Values;
};

#endif //__GENDMOPROPPAGE_H_
