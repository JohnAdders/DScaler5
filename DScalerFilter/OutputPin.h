///////////////////////////////////////////////////////////////////////////////
// $Id: OutputPin.h,v 1.2 2003-05-01 16:22:24 adcockj Exp $
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
    public IQualityControl
{
public:
	COutputPin();
	~COutputPin();


BEGIN_COM_MAP(COutputPin)
	COM_INTERFACE_ENTRY(IPin)
    COM_INTERFACE_ENTRY(IQualityControl)
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

public:
    BOOL HasChanged();
    void SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types);
        
public:
    CDScaler* m_Filter;
    CInputPin* m_InputPin;
    CComPtr<IPin> m_ConnectedPin;
    AM_MEDIA_TYPE m_CurrentMediaType;
    AM_MEDIA_TYPE m_DesiredMediaType;
    CComQIPtr<IPinConnection> m_PinConnection;
    CComQIPtr<IMemInputPin> m_MemInputPin;
    CComPtr<IMemAllocator> m_Allocator;
    BOOL m_FormatChanged;
    void InternalDisconnect();

private:
    void CreateProposedMediaType(AM_MEDIA_TYPE* ProposedType);
    void CreateRGBMediaType(AM_MEDIA_TYPE* ProposedType, const GUID& RGBType);
};


