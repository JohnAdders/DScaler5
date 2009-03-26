///////////////////////////////////////////////////////////////////////////////
// $Id$
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
class CInputPin;

/////////////////////////////////////////////////////////////////////////////
// COutputPin
class ATL_NO_VTABLE COutputPin :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IPin,
    public IUpdateMediaTypes,
    public IPinFlowControl,
    public IMediaSeeking,
    public IQualityControl
{
public:
    COutputPin();
    ~COutputPin();


BEGIN_COM_MAP(COutputPin)
    COM_INTERFACE_ENTRY(IPin)
    COM_INTERFACE_ENTRY(IPinFlowControl)
    COM_INTERFACE_ENTRY(IQualityControl)
    COM_INTERFACE_ENTRY(IMediaSeeking)
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


// IQualityControl
public:
    STDMETHOD(Notify)(IBaseFilter *pSelf, Quality q);
    STDMETHOD(SetSink)(IQualityControl *piqc);

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

public:
    ULONG FormatVersion();
    HRESULT GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type);
    BITMAPINFOHEADER* GetBitmapInfo();
    HRESULT ChangeOutputFormat(const AM_MEDIA_TYPE* InputType);
    HRESULT SetMediaType(const AM_MEDIA_TYPE* NewType);


public:
    CDScaler* m_Filter;
    CInputPin* m_InputPin;
    CComPtr<IPin> m_ConnectedPin;
    AM_MEDIA_TYPE m_CurrentMediaType;
    CComQIPtr<IPinConnection> m_PinConnection;
    CComQIPtr<IMemInputPin> m_MemInputPin;
    CComPtr<IMemAllocator> m_Allocator;
    ULONG m_FormatVersion;

    HRESULT CreateOutputMediaType(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType);
    HRESULT CreateOutputMediaTypeBasedOnExisting(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType, const AM_MEDIA_TYPE* OldType);

private:
    BOOL AreTypesCloseEnough(const AM_MEDIA_TYPE* CurrentType, const AM_MEDIA_TYPE* ProposedType);
    HRESULT InternalConnect(IPin *pReceivePin, const AM_MEDIA_TYPE* InputType);
    void InternalDisconnect();
};


