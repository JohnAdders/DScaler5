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

#pragma once 

#include "EnumMediaTypes.h"
#include "InputMemAlloc.h"
#include "DSBasePin.h"


/////////////////////////////////////////////////////////////////////////////
// CDSInputPin
class CDSInputPin : 
    public CDSBasePin,
    public IMemInputPin,
    public IPinConnection
{
public:

IMPLEMENT_UNKNOWN(CDSInputPin)

BEGIN_INTERFACE_TABLE(CDSInputPin)
    IMPLEMENTS_INTERFACE(IPin)
    IMPLEMENTS_INTERFACE(IMemInputPin)
    IMPLEMENTS_INTERFACE(IQualityControl)
    IMPLEMENTS_INTERFACE(IPinConnection)
    IMPLEMENTS_INTERFACE(IKsPropertySet)
END_INTERFACE_TABLE()

public:
    CDSInputPin();
    ~CDSInputPin();

// IPin
public:
    STDMETHOD(Connect)(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(ReceiveConnection)(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE *pmt);
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
    HRESULT Activate();
    HRESULT Deactivate();
    HRESULT Block(DWORD dwBlockFlags, HANDLE hEvent);
    BOOL IsFlushing() {return m_Flushing;};
    virtual HRESULT GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
    HRESULT SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);

protected:
    void InternalDisconnect() {};

    enum eSourceType
    {
        SOURCE_DEFAULT,
        SOURCE_ELECARD,
        SOURCE_SONIC,
        SOURCE_NVDVD,
        SOURCE_WINDVD,
        SOURCE_DV,
    };

    BOOL AreWeAreTalkingToOurself(IPin* pConnector);
    void FixupMediaType(AM_MEDIA_TYPE *pmt);
    void CheckForBlocking();

protected:
    BOOL m_Flushing;
    BOOL m_bReadOnly;
    HANDLE m_NotifyEvent;
    BOOL m_Block;
    HANDLE m_BlockEvent;
    ALLOCATOR_PROPERTIES m_AllocatorProperties;
    eSourceType m_SourceType;
};

