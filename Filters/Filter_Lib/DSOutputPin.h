///////////////////////////////////////////////////////////////////////////////
// $Id: DSOutputPin.h,v 1.13 2004-10-05 19:27:08 adcockj Exp $
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
#include "control.h"


class CDSBaseFilter;

/////////////////////////////////////////////////////////////////////////////
// CInputPin
class CDSOutputPin : 
    public CDSBasePin,
    public IPinFlowControl,
    public IMediaSeeking,
    public IAMPushSource
{
public:

IMPLEMENT_UNKNOWN(CDSOutputPin)

BEGIN_INTERFACE_TABLE(CDSOutputPin)
    IMPLEMENTS_INTERFACE(IPin)
    IMPLEMENTS_INTERFACE(IPinFlowControl)
    IMPLEMENTS_INTERFACE(IQualityControl)
    IMPLEMENTS_INTERFACE(IMediaSeeking)
    IMPLEMENTS_INTERFACE(IKsPropertySet)
    IMPLEMENTS_INTERFACE(IAMLatency)
END_INTERFACE_TABLE()

public:
    CDSOutputPin();
    ~CDSOutputPin();

// IPin
public:
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE *pmt);
    STDMETHOD(Connect)(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(ReceiveConnection)(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(EndOfStream)(void);
    STDMETHOD(BeginFlush)(void);
    STDMETHOD(EndFlush)(void);
    STDMETHOD(NewSegment)(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
    STDMETHOD(Disconnect)(void);

    HRESULT Activate();
    HRESULT Deactivate();
    HRESULT CanWeWorkWithThisInputType(const AM_MEDIA_TYPE *pmt);

// IPinFlowControl
public:
    STDMETHOD(Block)(DWORD dwBlockFlags, HANDLE hEvent);

// IMediaSeeking
public:
    STDMETHOD(GetCapabilities)(DWORD *pCapabilities);
    STDMETHOD(CheckCapabilities)(DWORD *pCapabilities);
    STDMETHOD(IsFormatSupported)(const GUID *pFormat);
    STDMETHOD(QueryPreferredFormat)(GUID *pFormat);
    STDMETHOD(GetTimeFormat)(GUID *pFormat);
    STDMETHOD(IsUsingTimeFormat)(const GUID *pFormat);
    STDMETHOD(SetTimeFormat)(const GUID *pFormat);
    STDMETHOD(GetDuration)(LONGLONG *pDuration);
    STDMETHOD(GetStopPosition)(LONGLONG *pStop);
    STDMETHOD(GetCurrentPosition)(LONGLONG *pCurrent);
    STDMETHOD(ConvertTimeFormat)(
                                    LONGLONG *pTarget,
                                    const GUID *pTargetFormat,
                                    LONGLONG Source,
                                    const GUID *pSourceFormat
                                );
    STDMETHOD(SetPositions)( 
                            LONGLONG *pCurrent,
                            DWORD dwCurrentFlags,
                            LONGLONG *pStop,
                            DWORD dwStopFlags
                           );
    STDMETHOD(GetPositions)(LONGLONG *pCurrent, LONGLONG *pStop);
    STDMETHOD(GetAvailable)(LONGLONG *pEarliest, LONGLONG *pLatest);
    STDMETHOD(SetRate)(double dRate);
    STDMETHOD(GetRate)(double *pdRate);
    STDMETHOD(GetPreroll)(LONGLONG *pllPreroll);

// IAMLatency
public:
    STDMETHOD(GetLatency)(REFERENCE_TIME *prtLatency);

// IAMPushSource
public:
    STDMETHOD(GetPushSourceFlags)(ULONG *pFlags);
    STDMETHOD(SetPushSourceFlags)(ULONG Flags);
    STDMETHOD(SetStreamOffset)(REFERENCE_TIME rtOffset);
    STDMETHOD(GetStreamOffset)(REFERENCE_TIME *prtOffset);
    STDMETHOD(GetMaxStreamOffset)(REFERENCE_TIME *prtMaxOffset);
    STDMETHOD(SetMaxStreamOffset)(REFERENCE_TIME rtMaxOffset);

public:
    BITMAPINFOHEADER* GetBitmapInfo();
    HRESULT NegotiateAllocator(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    /// returns S_FALSE if the format has changed
    HRESULT GetOutputSample(IMediaSample** OutSample, REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop, bool PrevFrameSkipped);
    HRESULT SendSample(IMediaSample* OutSample);
        
public:
    SI(IPinConnection) m_PinConnection;
    SI(IMemInputPin) m_MemInputPin;

protected:
    void InternalDisconnect();
};

class CDSOutputPushPin : public CDSOutputPin
{
public:

IMPLEMENT_UNKNOWN(CDSOutputPushPin)

BEGIN_INTERFACE_TABLE(CDSOutputPushPin)
    IMPLEMENTS_INTERFACE(IPin)
    IMPLEMENTS_INTERFACE(IPinFlowControl)
    IMPLEMENTS_INTERFACE(IQualityControl)
    IMPLEMENTS_INTERFACE(IMediaSeeking)
    IMPLEMENTS_INTERFACE(IKsPropertySet)
    IMPLEMENTS_INTERFACE(IAMPushSource)
    IMPLEMENTS_INTERFACE(IAMLatency)
END_INTERFACE_TABLE()

    CDSOutputPushPin() : CDSOutputPin(){};
};
