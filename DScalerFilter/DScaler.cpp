///////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.5 2003-05-02 16:05:22 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.4  2003/05/02 10:53:07  adcockj
// Returns test parameter for testing property page
//
// Revision 1.3  2003/05/01 18:15:17  adcockj
// Moved IMedaiSeeking to output pin
//
// Revision 1.2  2003/05/01 16:19:02  adcockj
// Changed property pages ready for generic page
//
// Revision 1.1.1.1  2003/04/30 13:01:20  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DScaler.h"
#include "EnumTwoPins.h"
#include "InputPin.h"
#include "OutputPin.h"

CDScaler::CDScaler()
{
	m_pUnkMarshaler = NULL;
    m_State = State_Stopped;
    m_Graph = NULL;
    m_IsDirty = FALSE;
    wcscpy(m_Name, L"DScaler Filter");
}

HRESULT CDScaler::FinalConstruct()
{
    LOG(DBGLOG_FLOW, ("CDScaler::FinalConstruct\n"));
    m_InputPin = new CComObject<CInputPin>;
    m_InputPin->AddRef();
    m_OutputPin = new CComObject<COutputPin>;
    m_OutputPin->AddRef();
    m_InputPin->m_Filter = this;
    m_InputPin->m_OutputPin = m_OutputPin;
    m_OutputPin->m_Filter = this;
    m_OutputPin->m_InputPin = m_InputPin;
	return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pUnkMarshaler.p);
}

void CDScaler::FinalRelease()
{
    LOG(DBGLOG_FLOW, ("CDScaler::FinalRelease\n"));
    m_InputPin->Release();
    m_OutputPin->Release();
	m_pUnkMarshaler.Release();
}

STDMETHODIMP CDScaler::EnumPins(IEnumPins **ppEnum)
{
    LOG(DBGLOG_FLOW, ("CDScaler::EnumPins\n"));
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CComObject<CEnumTwoPins>* NewEnum = new CComObject<CEnumTwoPins>;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

    NewEnum->SetPins(m_InputPin, m_OutputPin);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    
    return S_OK;
}

STDMETHODIMP CDScaler::FindPin(LPCWSTR Id, IPin **ppPin)
{
    LOG(DBGLOG_FLOW, ("CDScaler::FindPin\n"));
    if(wcscmp(Id, L"Input") == 0)
    {
        *ppPin = m_InputPin;
        (*ppPin)->AddRef();
        return S_OK;
    }
    else if(wcscmp(Id, L"Output") == 0)
    {
        *ppPin = m_OutputPin;
        (*ppPin)->AddRef();
        return S_OK;
    }
    else
    {
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }
}

STDMETHODIMP CDScaler::QueryFilterInfo(FILTER_INFO *pInfo)
{
    LOG(DBGLOG_FLOW, ("CDScaler::QueryFilterInfo\n"));
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    wcscpy(pInfo->achName,m_Name);
    pInfo->pGraph = m_Graph;
    if(m_Graph != NULL)
    {
        pInfo->pGraph->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CDScaler::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    LOG(DBGLOG_FLOW, ("CDScaler::JoinFilterGraph\n"));
    if(pGraph != NULL)
    {
        // we need the FilterGraph2 interface
        // so that we can use ReconnectEx for dynamic connections
        if(FAILED(pGraph->QueryInterface(__uuidof(IFilterGraph2), (void**)&m_Graph)))
        {
            return VFW_E_NO_INTERFACE;
        }
        // need to release here to avoid circular references
        m_Graph->Release();
        if(pName != NULL && wcslen(pName) < MAX_FILTER_NAME)
        {
            wcscpy(m_Name, pName);
        }
    }
    else
    {
        m_Graph = NULL;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::QueryVendorInfo(LPWSTR *pVendorInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::Stop(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Stop\n"));
    if(m_OutputPin->m_Allocator != NULL)
    {
        HRESULT hr = m_OutputPin->m_Allocator->Decommit();
    }
    m_State = State_Stopped;
    return S_OK;
}

STDMETHODIMP CDScaler::Pause(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Pause\n"));
    if(m_OutputPin->m_Allocator != NULL)
    {
        if(m_State == State_Stopped)
        {
            HRESULT hr = m_OutputPin->m_Allocator->Commit();
        }
    }
    m_State = State_Paused;
    return S_OK;
}

STDMETHODIMP CDScaler::Run(REFERENCE_TIME tStart)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Run\n"));
    m_State = State_Running;
    return S_OK;
}

STDMETHODIMP CDScaler::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetState\n"));
    if(State == NULL)
    {
        return E_POINTER;
    }
    *State = m_State;
    return S_OK;
}

STDMETHODIMP CDScaler::SetSyncSource(IReferenceClock *pClock)
{
    LOG(DBGLOG_FLOW, ("CDScaler::SetSyncSource\n"));
    m_RefClock = pClock;
    return S_OK;
}

STDMETHODIMP CDScaler::GetSyncSource(IReferenceClock **pClock)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetSyncSource\n"));
    m_RefClock.CopyTo(pClock);
    return S_OK;
}

STDMETHODIMP CDScaler::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = __uuidof(CDScaler);
    return S_OK;
}

STDMETHODIMP CDScaler::GetPages(CAUUID *pPages)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetPages\n"));
    if(pPages == NULL)
    {
        return E_POINTER;
    }
    pPages->cElems = 2;
    pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
    if(pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }
    // \todo get property pages working
    pPages->pElems[0] = CLSID_GenDMOPropPage;
    pPages->pElems[1] = CLSID_LicensePropPage;
    return S_OK;
}

STDMETHODIMP CDScaler::IsDirty(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::IsDirty\n"));
    return m_IsDirty?S_OK:S_FALSE;
}

STDMETHODIMP CDScaler::Load(IStream __RPC_FAR *pStm)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Load\n"));
    if(pStm == NULL)
    {
        return E_POINTER;
    }
    // \todo load up any settings
    return S_OK;
}

STDMETHODIMP CDScaler::Save(IStream __RPC_FAR *pStm, BOOL fClearDirty)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Save\n"));
    if(pStm == NULL)
    {
        return E_POINTER;
    }
    // \todo save up any settings
    if(fClearDirty)
    {
        m_IsDirty = FALSE;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::GetSizeMax(ULARGE_INTEGER __RPC_FAR *pcbSize)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetSizeMax\n"));
    if(pcbSize == NULL)
    {
        return E_POINTER;
    }
    // \todo work out real size
    pcbSize->QuadPart = 100;
    return S_OK;
}

STDMETHODIMP CDScaler::GetParam(DWORD dwParamIndex, MP_DATA *pValue)
{
    if(pValue == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex == 0)
    {
        *pValue = 5;
        return S_OK;
    }
    if(dwParamIndex == 1)
    {
        *pValue = 4.5;
        return S_OK;
    }
    if(dwParamIndex == 2)
    {
        *pValue = 1;
        return S_OK;
    }
    if(dwParamIndex == 3)
    {
        *pValue = 2;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CDScaler::SetParam(DWORD dwParamIndex,MP_DATA value)
{
    // \todo get working
    if(dwParamIndex < 4)
    {
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CDScaler::AddEnvelope(DWORD dwParamIndex,DWORD cPoints,MP_ENVELOPE_SEGMENT *ppEnvelope)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::FlushEnvelope( DWORD dwParamIndex,REFERENCE_TIME refTimeStart,REFERENCE_TIME refTimeEnd)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::SetTimeFormat( GUID guidTimeFormat,MP_TIMEDATA mpTimeData)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetParamCount(DWORD *pdwParams)
{
    if(pdwParams == NULL)
    {
        return E_POINTER;
    }
    *pdwParams = 4;
    return S_OK;
}

STDMETHODIMP CDScaler::GetParamInfo(DWORD dwParamIndex,MP_PARAMINFO *pInfo)
{
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex == 0)
    {
        pInfo->mpType = MPT_INT;
        pInfo->mopCaps = 0;
        pInfo->mpdMinValue = -10;
        pInfo->mpdMaxValue = 20;
        pInfo->mpdNeutralValue = 0;
        wcscpy(pInfo->szUnitText, L"Units1");
        wcscpy(pInfo->szLabel, L"Name2");
        return S_OK;
    }
    else if(dwParamIndex == 1)
    {
        pInfo->mpType = MPT_FLOAT;
        pInfo->mopCaps = 0;
        pInfo->mpdMinValue = -10;
        pInfo->mpdMaxValue = 20;
        pInfo->mpdNeutralValue = 0;
        wcscpy(pInfo->szUnitText, L"Units2");
        wcscpy(pInfo->szLabel, L"Name2");
        return S_OK;
    }
    else if(dwParamIndex == 2)
    {
        pInfo->mpType = MPT_BOOL;
        pInfo->mopCaps = 0;
        pInfo->mpdMinValue = 0;
        pInfo->mpdMaxValue = 1;
        pInfo->mpdNeutralValue = 0;
        wcscpy(pInfo->szUnitText, L"Units3");
        wcscpy(pInfo->szLabel, L"Name3");
        return S_OK;
    }
    else if(dwParamIndex == 3)
    {
        pInfo->mpType = MPT_ENUM;
        pInfo->mopCaps = 0;
        pInfo->mpdMinValue = 0;
        pInfo->mpdMaxValue = 3;
        pInfo->mpdNeutralValue = 0;
        wcscpy(pInfo->szUnitText, L"Units4");
        wcscpy(pInfo->szLabel, L"Name4");
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CDScaler::GetParamText(DWORD dwParamIndex,WCHAR **ppwchText)
{
    // \todo get working
    if(ppwchText == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex == 0)
    {
        *ppwchText = (WCHAR*)CoTaskMemAlloc(200);
        if(*ppwchText == NULL) return E_OUTOFMEMORY;
        memcpy(*ppwchText, L"Name1\0Units1\0", 28);
        return S_OK;
    }
    else if(dwParamIndex == 1)
    {
        *ppwchText = (WCHAR*)CoTaskMemAlloc(200);
        if(*ppwchText == NULL) return E_OUTOFMEMORY;
        memcpy(*ppwchText, L"Name2\0Units2\0", 28);
        return S_OK;
    }
    else if(dwParamIndex == 2)
    {
        *ppwchText = (WCHAR*)CoTaskMemAlloc(200);
        if(*ppwchText == NULL) return E_OUTOFMEMORY;
        memcpy(*ppwchText, L"Name3\0Units3\0", 28);
        return S_OK;
    }
    else if(dwParamIndex == 3)
    {
        *ppwchText = (WCHAR*)CoTaskMemAlloc(200);
        if(*ppwchText == NULL) return E_OUTOFMEMORY;
        memcpy(*ppwchText, L"Name4\0Units4\0Val1\0Val2\0Val3\0Val4\0", 68);
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::GetNumTimeFormats(DWORD *pdwNumTimeFormats)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetSupportedTimeFormat(DWORD dwFormatIndex,GUID *pguidTimeFormat)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetCurrentTimeFormat( GUID *pguidTimeFormat,MP_TIMEDATA *pTimeData)
{
    // \todo get working
    return E_NOTIMPL;
}
