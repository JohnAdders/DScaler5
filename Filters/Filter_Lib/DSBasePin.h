///////////////////////////////////////////////////////////////////////////////
// $Id: DSBasePin.h,v 1.4 2004-07-16 16:03:20 adcockj Exp $
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

#pragma once 

#include "EnumMediaTypes.h"
#include "InputMemAlloc.h"


class CDSBaseFilter;

/////////////////////////////////////////////////////////////////////////////
// CInputPin
class CDSBasePin : 
	public IPin,
    public IUpdateMediaTypes,
    public IKsPropertySet,
    public CCanLock
{

public:
	CDSBasePin(PIN_DIRECTION Direction);
	virtual ~CDSBasePin();
    void SetupObject(CDSBaseFilter* Filter, LPWSTR PinId);

public:
    // IPin
    STDMETHOD(Disconnect)(void);
    STDMETHOD(ConnectedTo)(IPin **pPin);
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE *pmt);
    STDMETHOD(QueryPinInfo)(PIN_INFO *pInfo);
    STDMETHOD(QueryDirection)(PIN_DIRECTION *pPinDir);
    STDMETHOD(QueryId)(LPWSTR *Id);
    STDMETHOD(EnumMediaTypes)(IEnumMediaTypes **ppEnum);
    STDMETHOD(QueryInternalConnections)(IPin **apPin, ULONG *nPin);

    // IUpdateMediaTypes
    ULONG FormatVersion();
    HRESULT GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type);

    // IKsPropertySet
    STDMETHOD(Set)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHOD(Get)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    STDMETHOD(QuerySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

    // Required virtual functions
    virtual HRESULT Activate() = 0;
    virtual HRESULT Deactivate() = 0;
    HRESULT SetType(const AM_MEDIA_TYPE* pmt);
    bool IsConnected() {return m_ConnectedPin?true:false;};
    const AM_MEDIA_TYPE* GetMediaType() {return &m_ConnectedMediaType;};
	HRESULT GetConnectedFilterCLSID(CLSID* pClsid);
	IBaseFilter* GetConnectedFilter();

public:
    CDSBaseFilter* m_Filter;
    SI(IPin) m_ConnectedPin;
    SI(IMemAllocator) m_Allocator;
    AM_MEDIA_TYPE m_ConnectedMediaType;
    ULONG m_FormatVersion;
    PIN_DIRECTION m_Direction;
    LPCWSTR m_PinId;
protected:
    virtual void InternalDisconnect() = 0;
protected:
	SI(IMemAllocator) m_MyMemAlloc;
};

