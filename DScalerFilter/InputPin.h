///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.h,v 1.1.1.1 2003-04-30 13:01:21 adcockj Exp $
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

#pragma once 

#include "EnumMediaTypes.h"

class CDScaler;
class COutputPin;

/////////////////////////////////////////////////////////////////////////////
// CInputPin
class ATL_NO_VTABLE CInputPin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IPin,
    public IMemInputPin,
    public IPinConnection,
    public IUpdateMediaTypes
{
public:
	CInputPin();
	~CInputPin();


BEGIN_COM_MAP(CInputPin)
	COM_INTERFACE_ENTRY(IPin)
    COM_INTERFACE_ENTRY(IMemInputPin)
    COM_INTERFACE_ENTRY(IPinConnection)
END_COM_MAP()

// IPin
public:
    STDMETHOD(Connect)(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(ReceiveConnection)(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(Disconnect)(void);
    STDMETHOD(ConnectedTo)(IPin **pPin);
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE *pmt);
    STDMETHOD(QueryPinInfo)(PIN_INFO *pInfo);
    STDMETHOD(QueryDirection)(PIN_DIRECTION *pPinDir);
    STDMETHOD(QueryId)(LPWSTR *Id);
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE *pmt);
    STDMETHOD(EnumMediaTypes)(IEnumMediaTypes **ppEnum);
    STDMETHOD(QueryInternalConnections)(IPin **apPin, ULONG *nPin);
    STDMETHOD(EndOfStream)(void);
    STDMETHOD(BeginFlush)(void);
    STDMETHOD(EndFlush)(void);
    STDMETHOD(NewSegment)(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

// IMemInputPin
public:
    STDMETHOD(GetAllocator)(IMemAllocator **ppAllocator);
    STDMETHOD(NotifyAllocator)(IMemAllocator *pAllocator, BOOL bReadOnly);
    STDMETHOD(GetAllocatorRequirements)(ALLOCATOR_PROPERTIES *pProps);
    STDMETHOD(Receive)(IMediaSample *pSample);
    STDMETHOD(ReceiveMultiple)(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
    STDMETHOD(ReceiveCanBlock)(void);

// IPinConnection
public:
    STDMETHOD(DynamicQueryAccept)(const AM_MEDIA_TYPE *pmt);
    STDMETHOD(NotifyEndOfStream)(HANDLE hNotifyEvent);
    STDMETHOD(IsEndPin)(void);
    STDMETHOD(DynamicDisconnect)(void);

public:
    BOOL HasChanged();
    void SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types);

public:
    CDScaler* m_Filter;
    COutputPin* m_OutputPin;
    CComPtr<IPin> m_ConnectedPin;
    CComPtr<IMemAllocator> m_Allocator;
    BOOL m_Flushing;
    BOOL m_bReadOnly;
    AM_MEDIA_TYPE m_InputMediaType;
    AM_MEDIA_TYPE m_ImpliedMediaType;
    HANDLE m_NotifyEvent;
    BOOL m_FormatChanged;
    DWORD m_VideoSampleFlag;
    DWORD m_VideoSampleMask;
private:
    void InternalDisconnect();
    void GuessInterlaceFlags();
    void GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
    HRESULT SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
    void WorkOutImpliedMediaType();

};

