///////////////////////////////////////////////////////////////////////////////
// $Id: OutputPin.cpp,v 1.1.1.1 2003-04-30 13:01:22 adcockj Exp $
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
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutputPin.h"
#include "DScaler.h"
#include "InputPin.h"
#include "EnumMediaTypes.h"

COutputPin::COutputPin()
{
    LOG(DBGLOG_FLOW, "COutputPin::COutputPin\n");
    InitMediaType(&m_CurrentMediaType);
    InitMediaType(&m_DesiredMediaType);
    m_FormatChanged = FALSE;
}

COutputPin::~COutputPin()
{
    LOG(DBGLOG_FLOW, "COutputPin::~COutputPin\n");
    ClearMediaType(&m_CurrentMediaType);
    ClearMediaType(&m_DesiredMediaType);
}


STDMETHODIMP COutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "COutputPin::Connect\n");
    HRESULT hr;
    

    if(m_InputPin->m_ConnectedPin == NULL)
    {
        return VFW_E_NO_ACCEPTABLE_TYPES;
    }

    if(m_ConnectedPin != NULL)
    {
        hr = m_ConnectedPin->Disconnect();
        hr = m_Allocator->Decommit();
        InternalDisconnect();
    }

    // check that we can pass stuff to this pin
    m_MemInputPin = pReceivePin;
    if(m_MemInputPin == NULL)
    {
        return VFW_E_NO_TRANSPORT;
    }
    
    AM_MEDIA_TYPE ProposedType;
    InitMediaType(&ProposedType);

    // say which type we really want to send out
    CreateProposedMediaType(&ProposedType);

    CopyMediaType(&m_DesiredMediaType, &ProposedType);
    // check that any format we've been passed in is OK
    // otherwise just tell the caller that it isn't
    if(pmt != NULL)
    {
        if((pmt->majortype != GUID_NULL && pmt->majortype != ProposedType.majortype) ||
            (pmt->subtype != GUID_NULL && pmt->subtype != ProposedType.subtype) || 
            (pmt->formattype != GUID_NULL && pmt->formattype != ProposedType.formattype))
        {
            InternalDisconnect();
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    // See if the format we want is supported
    // at the moment
    hr = pReceivePin->ReceiveConnection(this, &ProposedType);
    if(hr != S_OK)
    {
        // OK so they don't want to us with 
        // our prefered type at the moment so
        // see if we need to fake RGB output
        // Which we will do if we're connecting to
        // the old style video renderer
        // which has IOverlay but isn't a VMR
        CComQIPtr<IOverlay> Overlay = pReceivePin;
        CComQIPtr<IVMRVideoStreamControl> VMR7 = pReceivePin;
        CComQIPtr<IVMRVideoStreamControl9> VMR9 = pReceivePin;
        if(Overlay != NULL && (VMR7 == NULL && VMR9 == NULL))
        {
            // try and connect
            const GUID* RGBTypes[] = 
            {
                &MEDIASUBTYPE_RGB32,
                &MEDIASUBTYPE_RGB24,
                &MEDIASUBTYPE_RGB555,
                &MEDIASUBTYPE_RGB565,
                &MEDIASUBTYPE_RGB8,
            };
            int Count(0);
            while(hr != S_OK && Count < sizeof(RGBTypes) / sizeof(GUID))
            {   
                CreateRGBMediaType(&ProposedType, *RGBTypes[Count]);
                hr = pReceivePin->ReceiveConnection(this, &ProposedType);
                ++Count;
            }
        }
        if(hr != S_OK)
        {
            InternalDisconnect();
            return VFW_E_NO_ACCEPTABLE_TYPES;
        }
    }

    CopyMediaType(&m_CurrentMediaType, &ProposedType);
    LogMediaType(&m_CurrentMediaType, "Output Connected");
    m_FormatChanged = TRUE;
    
    // ask the conencted filter for it's allocator
    // if it hasn't got one then use the one from downstream
    // \todo we should probably have our own allocator
    // here but it doesn't seem worth it yet
    m_MemInputPin->GetAllocator(&m_Allocator);
    if(m_Allocator == NULL)
    {
        m_Allocator = m_InputPin->m_Allocator;
    }

    // lets try and negotiate a sensible set of requirements
    // try to allocate the settings the renderer has asked for
    // subject to some sensible minumums.
    // he old video renderer seems to ask for an alignment
    // that it's own allocator doesn't in fact support
    // we loop down until we find an alignment it does like
    ALLOCATOR_PROPERTIES Props;
    ALLOCATOR_PROPERTIES PropsAct;
    hr = m_InputPin->GetAllocatorRequirements(&Props);
    
    Props.cBuffers = max(3, Props.cBuffers);
    Props.cbBuffer = max(max(ProposedType.lSampleSize, m_CurrentMediaType.lSampleSize), (ULONG)Props.cbBuffer);
    Props.cbAlign = max(1, Props.cbAlign);

    hr = m_Allocator->SetProperties(&Props, &PropsAct);
    while(hr == VFW_E_BADALIGN && Props.cbAlign > 1)
    {
        Props.cbAlign /= 2;
        hr = m_Allocator->SetProperties(&Props, &PropsAct);
    }
    if(FAILED(hr))
    {
        InternalDisconnect();
        return VFW_E_NO_TRANSPORT;
    }

    LOG(DBGLOG_ALL, " Allocator Negotiated %d Buffers %d Size, %d Align\n", PropsAct.cBuffers, PropsAct.cbBuffer, PropsAct.cbAlign);

    hr = m_MemInputPin->NotifyAllocator(m_Allocator, FALSE);
    if(FAILED(hr))
    {
        InternalDisconnect();
        return VFW_E_NO_TRANSPORT;
    }

    // If all is OK the save the Pin interface
    m_ConnectedPin = pReceivePin;
    
    // save the IPinConnection pointer
    // so that we can propagate dynamic reconnections
    m_PinConnection = pReceivePin;
    return S_OK;
}

STDMETHODIMP COutputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::ReceiveConnection\n");
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::Disconnect(void)
{
    LOG(DBGLOG_FLOW, "COutputPin::Disconnect\n");
    if(m_ConnectedPin == NULL)
    {
        return S_FALSE;
    }
    else if(m_Filter->m_State == State_Stopped)
    {
        InternalDisconnect();
        return S_OK;
    }
    else
    {
        return VFW_E_NOT_STOPPED;
    }
    return S_OK;
}

STDMETHODIMP COutputPin::ConnectedTo(IPin **pPin)
{
    LOG(DBGLOG_FLOW, "COutputPin::ConnectedTo\n");
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
        m_ConnectedPin.CopyTo(pPin);
        return S_OK;
    }
}

STDMETHODIMP COutputPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "COutputPin::ConnectionMediaType\n");
    if(pmt == NULL)
    {
        return E_POINTER;
    }

    if(m_ConnectedPin != NULL)
    {
        return CopyMediaType(pmt, &m_CurrentMediaType);
    }
    else
    {
        return VFW_E_NOT_CONNECTED;
    }
}

STDMETHODIMP COutputPin::QueryPinInfo(PIN_INFO *pInfo)
{
    LOG(DBGLOG_FLOW, "COutputPin::QueryPinInfo\n");
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    pInfo->pFilter = m_Filter;
    pInfo->pFilter->AddRef();
    pInfo->dir = PINDIR_OUTPUT;
    wcscpy(pInfo->achName, L"Output");
    return S_OK;
}

STDMETHODIMP COutputPin::QueryDirection(PIN_DIRECTION *pPinDir)
{
    LOG(DBGLOG_FLOW, "COutputPin::QueryDirection\n");
    if(pPinDir == NULL)
    {
        return E_POINTER;
    }
    *pPinDir = PINDIR_OUTPUT;
    return S_OK;
}

STDMETHODIMP COutputPin::QueryId(LPWSTR *Id)
{
    LOG(DBGLOG_FLOW, "COutputPin::QueryId\n");
    if(Id == NULL)
    {
        return E_POINTER;
    }

    *Id = (LPWSTR)CoTaskMemAlloc(7 * sizeof(WCHAR));
    if(*Id == NULL)
    {
        return E_OUTOFMEMORY;
    }
    wcscpy(*Id, L"Output");

    return S_OK;
}

STDMETHODIMP COutputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "COutputPin::QueryAccept\n");
    if(pmt->majortype != MEDIATYPE_Video)
    {
        return S_FALSE;
    }

    if(pmt->formattype != FORMAT_VIDEOINFO2 && 
        pmt->formattype != FORMAT_VideoInfo)
    {
        return S_FALSE;
    }

    if(pmt->subtype != MEDIASUBTYPE_YUY2 && 
       pmt->subtype != MEDIASUBTYPE_YV12)
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP COutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_FLOW, "COutputPin::EnumMediaTypes\n");
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    NewEnum->SetUpdate(this);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    return S_OK;
}

STDMETHODIMP COutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    LOG(DBGLOG_FLOW, "COutputPin::QueryInternalConnections\n");
    // don't bother with this as we are a simple filter
    return E_NOTIMPL;
}

STDMETHODIMP COutputPin::EndOfStream(void)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::EndOfStream\n");
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::BeginFlush(void)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::BeginFlush\n");
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::EndFlush(void)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::EndFlush\n");
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::NewSegment\n");
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::Notify(IBaseFilter *pSelf, Quality q)
{
    LOG(DBGLOG_FLOW, "COutputPin::Notify \n Type %d Proportion %d Late %d", q.Type, q.Proportion, (long)q.Late);
    // \todo see what we get sent here and work out how to handle some
    // of ethe more common messages
    // in the mean time just pass the messages downstream
    CComQIPtr<IQualityControl> QualityControl = m_InputPin->m_ConnectedPin;
    if(QualityControl != NULL)
    {
        return QualityControl->Notify(pSelf, q);
    }
    else
    {
        return E_NOTIMPL;
    }
}

STDMETHODIMP COutputPin::SetSink(IQualityControl *piqc)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - COutputPin::SetSink\n");
    return E_NOTIMPL;
}

void COutputPin::InternalDisconnect()
{
    ClearMediaType(&m_CurrentMediaType);
    m_FormatChanged = TRUE;
    ClearMediaType(&m_DesiredMediaType);
    m_ConnectedPin.Release();
    m_PinConnection.Release();
    m_MemInputPin.Release();
    m_Allocator.Release();
}

void COutputPin::CreateProposedMediaType(AM_MEDIA_TYPE* ProposedType)
{
    CopyMediaType(ProposedType, &m_InputPin->m_ImpliedMediaType);
}

void COutputPin::CreateRGBMediaType(AM_MEDIA_TYPE* ProposedType, const GUID& RGBType)
{
    ProposedType->majortype = MEDIATYPE_Video;
    ProposedType->subtype = RGBType;
    ProposedType->bFixedSizeSamples = TRUE;
    ProposedType->bTemporalCompression = 0;
    ProposedType->formattype = FORMAT_VideoInfo;
    ProposedType->cbFormat = sizeof(VIDEOINFO);
    VIDEOINFO* NewFormat = (VIDEOINFO*)CoTaskMemAlloc(ProposedType->cbFormat);
    if(NewFormat == NULL)
    {
        ClearMediaType(ProposedType);
        return;
    }
    VIDEOINFOHEADER2* OldFormat = (VIDEOINFOHEADER2*)ProposedType->pbFormat;
    ZeroMemory(NewFormat, ProposedType->cbFormat);
    memcpy(&NewFormat->bmiHeader, &OldFormat->bmiHeader, sizeof(BITMAPINFOHEADER));
    CoTaskMemFree(ProposedType->pbFormat);
    ProposedType->pbFormat = (BYTE*)NewFormat;
    if(RGBType == MEDIASUBTYPE_RGB32)
    {
        NewFormat->bmiHeader.biBitCount = 32;
        NewFormat->bmiHeader.biCompression = BI_RGB;
        NewFormat->bmiHeader.biXPelsPerMeter = 0;
        NewFormat->bmiHeader.biYPelsPerMeter = 0;
        NewFormat->bmiHeader.biSizeImage = NewFormat->bmiHeader.biHeight * NewFormat->bmiHeader.biWidth * 4;
    }
    else if(RGBType == MEDIASUBTYPE_RGB24)
    {
        NewFormat->bmiHeader.biBitCount = 24;
        NewFormat->bmiHeader.biCompression = BI_RGB;
        NewFormat->bmiHeader.biXPelsPerMeter = 0;
        NewFormat->bmiHeader.biYPelsPerMeter = 0;
        NewFormat->bmiHeader.biSizeImage = NewFormat->bmiHeader.biHeight * NewFormat->bmiHeader.biWidth * 3;
    }
    else if(RGBType == MEDIASUBTYPE_RGB555)
    {
        NewFormat->bmiHeader.biBitCount = 16;
        NewFormat->bmiHeader.biCompression = BI_BITFIELDS;
        NewFormat->bmiHeader.biXPelsPerMeter = 0;
        NewFormat->bmiHeader.biYPelsPerMeter = 0;
        NewFormat->bmiHeader.biSizeImage = NewFormat->bmiHeader.biHeight * NewFormat->bmiHeader.biWidth * 2;
        NewFormat->TrueColorInfo.dwBitMasks[0] = 0x00007c00;
        NewFormat->TrueColorInfo.dwBitMasks[1] = 0x000003e0;
        NewFormat->TrueColorInfo.dwBitMasks[2] = 0x0000001f;
    }
    else if(RGBType == MEDIASUBTYPE_RGB565)
    {
        NewFormat->bmiHeader.biBitCount = 16;
        NewFormat->bmiHeader.biCompression = BI_BITFIELDS;
        NewFormat->bmiHeader.biXPelsPerMeter = 0;
        NewFormat->bmiHeader.biYPelsPerMeter = 0;
        NewFormat->bmiHeader.biSizeImage = NewFormat->bmiHeader.biHeight * NewFormat->bmiHeader.biWidth * 2;
        NewFormat->TrueColorInfo.dwBitMasks[0] = 0x0000f800;
        NewFormat->TrueColorInfo.dwBitMasks[1] = 0x000007e0;
        NewFormat->TrueColorInfo.dwBitMasks[2] = 0x0000001f;
    }
    else
    {
        NewFormat->bmiHeader.biBitCount = 8;
        NewFormat->bmiHeader.biCompression = iPALETTE_COLORS;
        NewFormat->bmiHeader.biXPelsPerMeter = 0;
        NewFormat->bmiHeader.biYPelsPerMeter = 0;
        PALETTEENTRY CurrentPalette[iPALETTE_COLORS];

        // \todo may be something odd to do for multi monitor
        // but nobody is actually watch video like this so really who cares
        HDC hDC = GetDC(NULL);  
        GetSystemPaletteEntries(hDC, 0, iPALETTE_COLORS, CurrentPalette);
        ReleaseDC(NULL, hDC);
        for(int i(0); i < iPALETTE_COLORS;++i)
        {
            NewFormat->TrueColorInfo.bmiColors[i].rgbRed = CurrentPalette[i].peRed;
            NewFormat->TrueColorInfo.bmiColors[i].rgbGreen = CurrentPalette[i].peGreen;
            NewFormat->TrueColorInfo.bmiColors[i].rgbBlue = CurrentPalette[i].peBlue;
        }
    }

    ProposedType->lSampleSize = NewFormat->bmiHeader.biSizeImage;
}

BOOL COutputPin::HasChanged()
{
    return m_FormatChanged;
}

void COutputPin::SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types)
{   
    if(m_ConnectedPin != NULL)
    {
        NumTypes = 2;
        CopyMediaType(Types, &m_DesiredMediaType);
        ++Types;
        CopyMediaType(Types, &m_CurrentMediaType);
    }
    else if(m_InputPin->m_ConnectedPin != NULL)
    {
        NumTypes = 0;
        CreateProposedMediaType(Types);
        ++Types;
        ClearMediaType(Types);
    }
    else
    {
        NumTypes = 0;
        ClearMediaType(Types);
        ++Types;
        ClearMediaType(Types);
    }
}

