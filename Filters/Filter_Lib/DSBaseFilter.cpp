///////////////////////////////////////////////////////////////////////////////
// $Id: DSBaseFilter.cpp,v 1.9 2004-07-20 16:37:57 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock 
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
// Revision 1.8  2004/07/07 14:09:01  adcockj
// removed tabs
//
// Revision 1.7  2004/05/25 16:59:30  adcockj
// fixed issues with new buffered pin
//
// Revision 1.6  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.5  2004/03/15 17:17:05  adcockj
// Basic registry saving support
//
// Revision 1.4  2004/03/01 13:04:28  adcockj
// Fixed another locking problem
//
// Revision 1.3  2004/02/27 17:08:16  adcockj
// Improved locking at state changes
// Better error handling at state changes
//
// Revision 1.2  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DSBaseFilter.h"
#include "DSBasePin.h"
#include "EnumPins.h"

CDSBaseFilter::CDSBaseFilter(LPCWSTR Name, int NumberOfInputPins, int NumberOfOutputPins) :
    CParams(Name)
{
    m_State = State_Stopped;
    m_Graph = NULL;
    m_rtStartTime = 0;
    wcscpy(m_Name, Name);
    m_NumInputPins = NumberOfInputPins;
    m_NumOutputPins = NumberOfOutputPins;
    m_InputPins = new CDSInputPin*[m_NumInputPins];
    if(!m_InputPins)
    {
        throw(std::runtime_error("Can't create memory for pins"));
    }
    m_OutputPins = new CDSOutputPin*[m_NumOutputPins];
    if(!m_OutputPins)
    {
        throw(std::runtime_error("Can't create memory for pins"));
    }
    m_IsDiscontinuity = false;
}

CDSBaseFilter::~CDSBaseFilter()
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::~CDSBaseFilter\n"));
    int i;
    for(i = 0; i < m_NumInputPins; ++i)
    {
        ((IPin*)m_InputPins[i])->Release();
    }
    for(i = 0; i < m_NumOutputPins; ++i)
    {
        ((IPin*)m_OutputPins[i])->Release();
    }
    delete [] m_InputPins;
    delete [] m_OutputPins;
}

STDMETHODIMP CDSBaseFilter::EnumPins(IEnumPins **ppEnum)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::EnumPins\n"));
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CEnumPins* NewEnum = new CEnumPins;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

    NewEnum->SetFilter(this);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    
    return S_OK;
}

CDSBasePin* CDSBaseFilter::GetPin(int PinIndex)
{
    if(PinIndex < m_NumInputPins)
    {
        return m_InputPins[PinIndex];
    }
    else if(PinIndex < m_NumInputPins + m_NumOutputPins)
    {
        return m_OutputPins[PinIndex - m_NumInputPins];
    }
    else
    {
        return NULL;
    }
}


HRESULT CDSBaseFilter::GetPin(ULONG PinNum, IPin** pPin)
{
    *pPin = GetPin(PinNum);
    if(*pPin != NULL)
    {
        (*pPin)->AddRef();
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}


STDMETHODIMP CDSBaseFilter::FindPin(LPCWSTR Id, IPin **ppPin)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::FindPin\n"));
    int i;
    for(i = 0; i < GetNumPins(); ++i)
    {
        LPWSTR PinName = NULL;
        HRESULT hr = GetPin(i)->QueryId(&PinName);
        CHECK(hr);
        if(wcscmp(Id, PinName) == 0)
        {
            *ppPin = GetPin(i);
            (*ppPin)->AddRef();
            CoTaskMemFree(PinName);
            return S_OK;
        }
        else
        {
            CoTaskMemFree(PinName);
        }
    }
    return VFW_E_NOT_FOUND;
}

STDMETHODIMP CDSBaseFilter::QueryFilterInfo(FILTER_INFO *pInfo)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::QueryFilterInfo\n"));
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

STDMETHODIMP CDSBaseFilter::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::JoinFilterGraph\n"));
    if(pGraph != NULL)
    {
        LoadDefaultsFromRegistry();

        // we need the FilterGraph2 interface
        // so that we can use ReconnectEx for dynamic connections
        if(FAILED(pGraph->QueryInterface(IID_IFilterGraph2, (void**)&m_Graph)))
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

STDMETHODIMP CDSBaseFilter::QueryVendorInfo(LPWSTR *pVendorInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDSBaseFilter::Stop(void)
{
    LOG(DBGLOG_FLOW, ("CDSBaseFilter::Stop\n"));

    CProtectCode WhileVarInScope(this);
    
    int i;
    
    FILTER_STATE OldState = m_State;
    m_State = State_Stopped;

    LockAllPins();

    if(OldState != State_Stopped)
    {
        for(i = 0; i < m_NumInputPins; ++i)
        {
            CProtectCode WhileVarInScope(m_InputPins[i]);
            m_InputPins[i]->Deactivate();
        }
        for(i = 0; i < m_NumOutputPins; ++i)
        {
            CProtectCode WhileVarInScope(m_OutputPins[i]);
            m_OutputPins[i]->Deactivate();
        }
        Deactivate();
    }

    UnlockAllPins();
    LOG(DBGLOG_FLOW, ("CDSBaseFilter::End Stop\n"));

    return S_OK;
}

STDMETHODIMP CDSBaseFilter::Pause(void)
{
    LOG(DBGLOG_FLOW, ("CDSBaseFilter::Pause\n"));

    CProtectCode WhileVarInScope(this);


    if(m_State == State_Stopped)
    {
        LockAllPins();
    
        int i;
        for(i = 0; i < m_NumInputPins; ++i)
        {
            m_InputPins[i]->Activate();
        }
        for(i = 0; i < m_NumOutputPins; ++i)
        {
            m_OutputPins[i]->Activate();
        }
        Activate();

        UnlockAllPins();
    }

    m_State = State_Paused;


    LOG(DBGLOG_FLOW, ("CDSBaseFilter::Pause end\n"));
    return S_OK;
}

STDMETHODIMP CDSBaseFilter::Run(REFERENCE_TIME tStart)
{
    LOG(DBGLOG_FLOW, ("CDSBaseFilter::Run %010I64d\n", tStart));

    CProtectCode WhileVarInScope(this);


    if(m_State == State_Stopped)
    {
        LockAllPins();
        int i;
        for(i = 0; i < m_NumInputPins; ++i)
        {
            m_InputPins[i]->Activate();
        }
        for(i = 0; i < m_NumOutputPins; ++i)
        {
            m_OutputPins[i]->Activate();
        }
        Activate();
        UnlockAllPins();
    }

    m_State = State_Running;

    m_rtStartTime = tStart;


    LOG(DBGLOG_FLOW, ("CDSBaseFilter::Run end\n"));
    return S_OK;
}

STDMETHODIMP CDSBaseFilter::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::GetState\n"));
    if(State == NULL)
    {
        return E_POINTER;
    }
    *State = m_State;
    return S_OK;
}

STDMETHODIMP CDSBaseFilter::SetSyncSource(IReferenceClock *pClock)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::SetSyncSource\n"));
    m_RefClock = pClock;
    return S_OK;
}

STDMETHODIMP CDSBaseFilter::GetSyncSource(IReferenceClock **pClock)
{
    LOG(DBGLOG_ALL, ("CDSBaseFilter::GetSyncSource\n"));
    *pClock = m_RefClock.GetAddRefedInterface();
    return S_OK;
}

STDMETHODIMP CDSBaseFilter::GetPages(CAUUID* pPages)
{
    pPages->cElems = 2;
    pPages->pElems = (CLSID*)CoTaskMemAlloc(sizeof(CLSID) * 2);
    if(pPages->pElems == NULL)
        return E_OUTOFMEMORY;
    pPages->pElems[0] = CLSID_GenDMOPropPage;
    pPages->pElems[1] = CLSID_LicensePropPage;
    return S_OK;
}

void CDSBaseFilter::LockAllPins()
{
        int i;
        for(i = 0; i < m_NumInputPins; ++i)
        {
            m_InputPins[i]->Lock();
        }
        for(i = 0; i < m_NumOutputPins; ++i)
        {
            m_OutputPins[i]->Lock();
        }
}

void CDSBaseFilter::UnlockAllPins()
{
        int i;
        for(i = 0; i < m_NumInputPins; ++i)
        {
            m_InputPins[i]->Unlock();
        }
        for(i = 0; i < m_NumOutputPins; ++i)
        {
            m_OutputPins[i]->Unlock();
        }
}

bool CDSBaseFilter::IsClockUpstream()
{
    return IsClockUpstreamFromFilter(this);
}

bool CDSBaseFilter::IsClockUpstreamFromFilter(IBaseFilter* Filter)
{
    SI(IEnumPins) EnumPins;
    HRESULT hr = Filter->EnumPins(EnumPins.GetReleasedInterfaceReference());
    if(hr == S_OK)
    {
        SI(IPin) Pin;
        hr = EnumPins->Next(1, Pin.GetReleasedInterfaceReference(), NULL);
        while(hr == S_OK)
        {
            PIN_DIRECTION PinDir;
            hr = Pin->QueryDirection(&PinDir);
            if(hr == S_OK && PinDir == PINDIR_INPUT)
            {
                SI(IPin) ConnectedTo;
                hr = Pin->ConnectedTo(ConnectedTo.GetReleasedInterfaceReference());
                if(hr == S_OK)
                {
                    PIN_INFO PinInfo;
                    hr = ConnectedTo->QueryPinInfo(&PinInfo);
                    if(hr == S_OK)
                    {
                        SI(IReferenceClock) RefClock = PinInfo.pFilter;
                        if(RefClock == m_RefClock)
                        {
                            return true;
                        }
                        else
                        {
                            return IsClockUpstreamFromFilter(PinInfo.pFilter);
                        }
                    }
                }
            }
            hr = EnumPins->Next(1, Pin.GetReleasedInterfaceReference(), NULL);
        }
    }
    return false;
}
