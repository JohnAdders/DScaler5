///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
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

#include "stdafx.h"
#include "DSBasePin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"

CDSBasePin::CDSBasePin(PIN_DIRECTION Direction)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::CDSBasePin\n"));
    InitMediaType(&m_ConnectedMediaType);
    m_FormatVersion = 0;
    m_Direction = Direction;
    m_MyMemAlloc = new CInputMemAlloc;
}

void CDSBasePin::SetupObject(CDSBaseFilter* Filter, LPWSTR PinId)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::SetupObject\n"));
    m_Filter = Filter;
    m_PinId = PinId;
}


CDSBasePin::~CDSBasePin()
{
    LOG(DBGLOG_ALL, ("CDSBasePin::~CDSBasePin\n"));
    ClearMediaType(&m_ConnectedMediaType);
}

STDMETHODIMP CDSBasePin::Disconnect(void)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::Disconnect\n"));
    if(m_ConnectedPin == NULL)
    {
        return S_FALSE;
    }
    else
    {
        ClearMediaType(&m_ConnectedMediaType);
        m_ConnectedPin.Detach();

        m_Allocator->Decommit();
        m_Allocator.Detach();

        ++m_FormatVersion;

        InternalDisconnect();

        return S_OK;
    }
}

STDMETHODIMP CDSBasePin::ConnectedTo(IPin **pPin)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::ConnectedTo\n"));
    if(pPin == NULL)
    {
        return E_POINTER;
    }
    if(m_ConnectedPin == NULL)
    {
        *pPin = NULL;
        return VFW_E_NOT_CONNECTED;
    }
    else
    {
        *pPin = m_ConnectedPin.GetAddRefedInterface();
        return S_OK;
    }
}

STDMETHODIMP CDSBasePin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::ConnectionMediaType\n"));
    if(pmt == NULL)
    {
        return E_POINTER;
    }

    ZeroMemory(pmt, sizeof(AM_MEDIA_TYPE));

    if(m_ConnectedPin != NULL)
    {
        return CopyMediaType(pmt, &m_ConnectedMediaType);
    }
    else
    {
        return VFW_E_NOT_CONNECTED;
    }
    return S_OK;
}

STDMETHODIMP CDSBasePin::QueryPinInfo(PIN_INFO *pInfo)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::QueryPinInfo\n"));
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    pInfo->pFilter = m_Filter;
    pInfo->pFilter->AddRef();
    pInfo->dir = m_Direction;
    wcscpy(pInfo->achName, m_PinId);
    return S_OK;
}

STDMETHODIMP CDSBasePin::QueryDirection(PIN_DIRECTION *pPinDir)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::QueryDirection\n"));
    if(pPinDir == NULL)
    {
        return E_POINTER;
    }
    *pPinDir = m_Direction;
    return S_OK;
}

STDMETHODIMP CDSBasePin::QueryId(LPWSTR *Id)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::QueryId\n"));

    if(Id == NULL)
    {
        return E_POINTER;
    }

    *Id = (LPWSTR)CoTaskMemAlloc((wcslen(m_PinId) + 1) * sizeof(WCHAR));
    if(*Id == NULL)
    {
        return E_OUTOFMEMORY;
    }
    wcscpy(*Id, m_PinId);

    return S_OK;
}

STDMETHODIMP CDSBasePin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::EnumMediaTypes\n"));
    CEnumMediaTypes* NewEnum = new CEnumMediaTypes;
    HRESULT hr = NewEnum->SetUpdate(this);
    CHECK(hr);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    return S_OK;
}

STDMETHODIMP CDSBasePin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    LOG(DBGLOG_ALL, ("CDSBasePin::QueryInternalConnections\n"));
    // don't bother with this as we are a simple filter
    return E_NOTIMPL;
}

ULONG CDSBasePin::FormatVersion()
{
    return m_FormatVersion;
}

HRESULT CDSBasePin::GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type)
{
    HRESULT hr = S_OK;
    if(m_ConnectedPin != NULL)
    {
        if(TypeNum == 0)
        {
            hr = CopyMediaType(Type, &m_ConnectedMediaType);
            CHECK(hr);
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = m_Filter->CreateSuitableMediaType(Type, this, TypeNum);
        if(hr == VFW_S_NO_MORE_ITEMS)
        {
            hr = S_FALSE;
        }
    }
    return hr;
}

HRESULT CDSBasePin::SetType(const AM_MEDIA_TYPE *pmt)
{
    // save the media type to local variable
    HRESULT hr = CopyMediaType(&m_ConnectedMediaType, pmt);
    CHECK(hr);
    LogMediaType(&m_ConnectedMediaType, "Connected", DBGLOG_FLOW);
    ++m_FormatVersion;

    hr = m_Filter->NotifyFormatChange(pmt, this);
    CHECK(hr);

    return hr;
}

STDMETHODIMP CDSBasePin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    CProtectCode WhileVarInScope(this);
    return m_Filter->Set(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData, this);
}

STDMETHODIMP CDSBasePin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    CProtectCode WhileVarInScope(this);
    return m_Filter->Get(guidPropSet, dwPropID, pInstanceData, cbInstanceData, pPropData, cbPropData, pcbReturned, this);
}

STDMETHODIMP CDSBasePin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    CProtectCode WhileVarInScope(this);
    return m_Filter->QuerySupported(guidPropSet, dwPropID, pTypeSupport, this);
}

HRESULT CDSBasePin::GetConnectedFilterCLSID(CLSID* pClsid)
{
    PIN_INFO PinInfo;
    BOOL RetVal = TRUE;


    if(m_ConnectedPin == NULL)
    {
        return VFW_E_NOT_CONNECTED;
    }

    HRESULT hr = m_ConnectedPin->QueryPinInfo(&PinInfo);
    if(SUCCEEDED(hr))
    {
        if(PinInfo.pFilter != NULL)
        {
            hr = PinInfo.pFilter->GetClassID(pClsid);
            PinInfo.pFilter->Release();
        }
    }

    return hr;
}

IBaseFilter* CDSBasePin::GetConnectedFilter()
{
    PIN_INFO PinInfo;
    BOOL RetVal = TRUE;

    if(!m_ConnectedPin)
    {
        return NULL;
    }

    HRESULT hr = m_ConnectedPin->QueryPinInfo(&PinInfo);
    if(SUCCEEDED(hr))
    {
        if(PinInfo.pFilter != NULL)
        {
            PinInfo.pFilter->Release();
            return PinInfo.pFilter;
        }
    }

    return NULL;
}

STDMETHODIMP CDSBasePin::Notify(IBaseFilter *pSelf, Quality q)
{
    return m_Filter->Notify(pSelf, q, this);
}

STDMETHODIMP CDSBasePin::SetSink(IQualityControl *piqc)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::SetSink\n"));
    return E_NOTIMPL;
}

