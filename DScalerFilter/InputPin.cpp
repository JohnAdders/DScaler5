///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.cpp,v 1.1.1.1 2003-04-30 13:01:21 adcockj Exp $
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
#include "InputPin.h"
#include "DScaler.h"
#include "OutputPin.h"
#include "EnumMediaTypes.h"

CInputPin::CInputPin()
{
    LOG(DBGLOG_FLOW, "CInputPin::CInputPin\n");
    m_Flushing = FALSE;
    m_bReadOnly = FALSE;
    m_NotifyEvent = NULL;
    InitMediaType(&m_InputMediaType);
    InitMediaType(&m_ImpliedMediaType);
    m_FormatChanged = FALSE;
}

CInputPin::~CInputPin()
{
    LOG(DBGLOG_FLOW, "CInputPin::~CInputPin\n");
    ClearMediaType(&m_InputMediaType);
    ClearMediaType(&m_ImpliedMediaType);
    m_FormatChanged = FALSE;
}


STDMETHODIMP CInputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "*Unexpected Call* - CInputPin::Connect\n");
    return E_UNEXPECTED;
}

STDMETHODIMP CInputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "CInputPin::ReceiveConnection\n");
    HRESULT hr = S_OK;
    if(pConnector == NULL || pmt == NULL)
    {
        return E_POINTER;

    }
    if(QueryAccept(pmt) != S_OK)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    m_ConnectedPin = pConnector;
    CopyMediaType(&m_InputMediaType, pmt);
    WorkOutImpliedMediaType();
    LogMediaType(&m_InputMediaType, "Input Connected");
    m_FormatChanged = TRUE;
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        hr = m_Filter->m_Graph->ReconnectEx(m_OutputPin, &m_ImpliedMediaType);
    }
    return hr;
}

STDMETHODIMP CInputPin::Disconnect(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::Disconnect\n");
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
}

STDMETHODIMP CInputPin::ConnectedTo(IPin **pPin)
{
    LOG(DBGLOG_FLOW, "CInputPin::ConnectedTo\n");
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

STDMETHODIMP CInputPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "CInputPin::ConnectionMediaType\n");
    if(pmt == NULL)
    {
        return E_POINTER;
    }

    if(m_ConnectedPin != NULL)
    {
        return CopyMediaType(pmt, &m_InputMediaType);
    }
    else
    {
        ClearMediaType(pmt);
        return VFW_E_NOT_CONNECTED;
    }
    return S_OK;
}

STDMETHODIMP CInputPin::QueryPinInfo(PIN_INFO *pInfo)
{
    LOG(DBGLOG_FLOW, "CInputPin::QueryPinInfo\n");
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    pInfo->pFilter = m_Filter;
    pInfo->pFilter->AddRef();
    pInfo->dir = PINDIR_INPUT;
    wcscpy(pInfo->achName, L"Input");
    return S_OK;
}

STDMETHODIMP CInputPin::QueryDirection(PIN_DIRECTION *pPinDir)
{
    LOG(DBGLOG_FLOW, "CInputPin::QueryDirection\n");
    if(pPinDir == NULL)
    {
        return E_POINTER;
    }
    *pPinDir = PINDIR_INPUT;
    return S_OK;
}

STDMETHODIMP CInputPin::QueryId(LPWSTR *Id)
{
    LOG(DBGLOG_FLOW, "CInputPin::QueryId\n");

    if(Id == NULL)
    {
        return E_POINTER;
    }

    *Id = (LPWSTR)CoTaskMemAlloc(6 * sizeof(WCHAR));
    if(*Id == NULL)
    {
        return E_OUTOFMEMORY;
    }
    wcscpy(*Id, L"Input");

    return S_OK;
}

STDMETHODIMP CInputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "CInputPin::QueryAccept\n");
    
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

STDMETHODIMP CInputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_FLOW, "CInputPin::EnumMediaTypes\n");
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    NewEnum->SetUpdate(this);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    return S_OK;
}

STDMETHODIMP CInputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    LOG(DBGLOG_FLOW, "CInputPin::QueryInternalConnections\n");
    // don't bother with this as we are a simple filter
    return E_NOTIMPL;
}

STDMETHODIMP CInputPin::EndOfStream(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::EndOfStream\n");
    // synchronize with Recieve
    Lock();
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        // check if we need are in the middle of
        // a format renegoatiation
        if(m_NotifyEvent == NULL)
        {
            m_OutputPin->m_ConnectedPin->EndOfStream();
        }
        else
        {
            SetEvent(m_NotifyEvent);
        }
    }
    Unlock();
    return S_OK;
}

STDMETHODIMP CInputPin::BeginFlush(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::BeginFlush\n");
    // pass the Flush downstream
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        m_OutputPin->m_ConnectedPin->BeginFlush();
    }
    // Sets an internal flag that causes all data-streaming methods to fail
    m_Flushing = TRUE;
    return S_OK;
}

STDMETHODIMP CInputPin::EndFlush(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::EndFlush\n");

    // \todo Waits for all queued samples to be discarded. 
    // \todo Frees any buffered data, including any pending end-of-stream notifications. 
    // \todo Clears any pending EC_COMPLETE notifications. 

    // Calls EndFlush downstream. 
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        m_OutputPin->m_ConnectedPin->EndFlush();
    }
    m_Flushing = FALSE;
    return S_OK;
}

STDMETHODIMP CInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_FLOW, "CInputPin::NewSegment\n");
    HRESULT hr = S_OK;
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        hr = m_OutputPin->m_ConnectedPin->NewSegment(tStart, tStop, dRate);
    }
    return hr;
}

STDMETHODIMP CInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    LOG(DBGLOG_FLOW, "CInputPin::GetAllocator\n");
    if(ppAllocator == NULL)
    {
        return E_POINTER;
    }
    if(m_Allocator == NULL)
    {
        *ppAllocator = NULL;
        return VFW_E_NO_ALLOCATOR;
    }
    else
    {
        m_Allocator.CopyTo(ppAllocator);
        return S_OK;
    }
}

STDMETHODIMP CInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    LOG(DBGLOG_FLOW, "CInputPin::NotifyAllocator\n");
    m_Allocator = pAllocator;
    m_bReadOnly = bReadOnly;
    return S_OK;
}

STDMETHODIMP CInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    LOG(DBGLOG_FLOW, "CInputPin::GetAllocatorRequirements\n");
    if(pProps == NULL)
    {
        return E_POINTER;
    }
    // shoot for the moon ....
    pProps->cbAlign = 16;
    pProps->cbBuffer = 1024*576*2;
    pProps->cbPrefix = 0;
    pProps->cBuffers = 8;

    return S_OK;
}

STDMETHODIMP CInputPin::Receive(IMediaSample *pSample)
{
    //LOG(DBGLOG_FLOW, "CInputPin::Receive\n");
    HRESULT hr = S_OK;

    static DWORD dwFlags = AM_GBF_PREVFRAMESKIPPED;

    if(pSample == NULL)
    {
        return E_POINTER;
    }
    if(m_Flushing == TRUE)
    {
        return S_FALSE;
    }
    if(m_Filter->m_State == State_Stopped || m_OutputPin->m_ConnectedPin == NULL)
    {
        return VFW_E_WRONG_STATE;
    }

    LogSample(pSample, "New Input Sample");
    // chack for media type changes on the input side
    // a NULL means the type is the same as last time
    AM_MEDIA_TYPE* InputType = NULL;
    hr = pSample->GetMediaType(&InputType);
    if(InputType != NULL)
    {
        CopyMediaType(&m_InputMediaType, InputType);
        LogMediaType(&m_InputMediaType, "Input Format Change");
        WorkOutImpliedMediaType();
        m_FormatChanged = TRUE;
    }

    AM_SAMPLE2_PROPERTIES SampleProperties;
    GetSampleProperties(pSample, &SampleProperties);

    if(SampleProperties.dwSampleFlags | AM_SAMPLE_DATADISCONTINUITY)
    {
        dwFlags |= AM_GBF_PREVFRAMESKIPPED;
    }

    // get a sample to output to
    IMediaSample* OutSample;
    hr = m_OutputPin->m_Allocator->GetBuffer(&OutSample, NULL, NULL, dwFlags);
    if(FAILED(hr) || OutSample == NULL)
    {
        LOG(DBGLOG_FLOW, "Frame Skipped\n");
        dwFlags |= AM_GBF_PREVFRAMESKIPPED;
        return S_FALSE;
    }
    else
    {
        dwFlags = 0;
    }

    // check for media type changes on the output side
    // a NULL means the type is the same as last time
    AM_MEDIA_TYPE* OutputType = NULL;
    hr = OutSample->GetMediaType(&OutputType);
    if(OutputType != NULL)
    {
        CopyMediaType(&m_OutputPin->m_CurrentMediaType, OutputType);
        LogMediaType(OutputType, "Output Format Change");
        m_OutputPin->m_FormatChanged = TRUE;
    }

    BYTE* pInBuffer = NULL;
    BYTE* pOutBuffer = NULL;
    DWORD Size = pSample->GetSize();
    DWORD Size2 = OutSample->GetSize();
    if(Size2 < Size)
    {
        Size = Size2;
    }
    hr = pSample->GetPointer(&pInBuffer);
    hr = OutSample->GetPointer(&pOutBuffer);
    memcpy(pOutBuffer, pInBuffer, Size);

    SampleProperties.dwSampleFlags &= m_VideoSampleMask;
    SampleProperties.dwSampleFlags |= m_VideoSampleFlag;
    SetSampleProperties(OutSample, &SampleProperties);

    hr = m_OutputPin->m_MemInputPin->Receive(OutSample);

    OutSample->Release();

    return S_OK;
}

STDMETHODIMP CInputPin::ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed)
{
    LOG(DBGLOG_FLOW, "CInputPin::ReceiveMultiple\n");
    HRESULT hr;
    if(nSamplesProcessed == NULL)
    {
        return E_POINTER;
    }

    *nSamplesProcessed = 0;
    for(int i(0); i < nSamples; ++i)
    {
        hr = Receive(pSamples[i]);
        if(FAILED(hr))
        {
            return hr;
        }
        ++(*nSamplesProcessed);
    }
    return hr;
}

STDMETHODIMP CInputPin::ReceiveCanBlock(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::ReceiveCanBlock\n");
    return S_OK;
}

STDMETHODIMP CInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, "CInputPin::DynamicQueryAccept\n");
    // first see if we can propogate this downstream
    if(m_OutputPin->m_PinConnection != NULL)
    {
        // then see if we understand this media type
        HRESULT hr = QueryAccept(pmt);
        if(hr != S_OK)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        // \todo translate the media type in to the one that we'll be outputting
        AM_MEDIA_TYPE NewType;
        
        return m_OutputPin->m_PinConnection->DynamicQueryAccept(&NewType);

    }
    else
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
}

STDMETHODIMP CInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    LOG(DBGLOG_FLOW, "CInputPin::NotifyEndOfStream\n");
    // generate required return code
    // see docs
    // we don't really ever expect to get called on
    // this though because we are not an end pin
    HRESULT hr = S_OK;
    if(hNotifyEvent == NULL && m_NotifyEvent == NULL)
    {
        hr = S_FALSE;
    }
    // all we do here is store the event
    m_NotifyEvent = hNotifyEvent;
    return hr;
}

STDMETHODIMP CInputPin::IsEndPin(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::IsEndPin\n");
    return S_FALSE;
}

STDMETHODIMP CInputPin::DynamicDisconnect(void)
{
    LOG(DBGLOG_FLOW, "CInputPin::DynamicDisconnect\n");
    if(m_ConnectedPin == NULL)
    {
        return S_FALSE;
    }
    else
    {
        InternalDisconnect();
        return S_OK;
    }
}

void CInputPin::InternalDisconnect()
{
    ClearMediaType(&m_InputMediaType);
    ClearMediaType(&m_ImpliedMediaType);
    m_FormatChanged = TRUE;
    m_ConnectedPin.Release();
    m_Allocator.Release();
}

BOOL CInputPin::HasChanged()
{
    return m_FormatChanged;
}

void CInputPin::SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types)
{   
    if(m_ConnectedPin != NULL)
    {
        NumTypes = 1;
        CopyMediaType(Types, &m_InputMediaType);
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

void CInputPin::GuessInterlaceFlags()
{
    VIDEOINFOHEADER2* VideoInfo = (VIDEOINFOHEADER2*)m_ImpliedMediaType.pbFormat;
    switch(VideoInfo->AvgTimePerFrame)
    {
    // corresponds to 25Hz
    case 400000:
        m_VideoSampleFlag = AM_VIDEO_FLAG_FIELD1FIRST;
        VideoInfo->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;
        break;
    // corresponds to 30Hz or 29.97Hz
    case 333333:
    case 333334:
    case 333366:
    case 333367:
        VideoInfo->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;
        break;
    // corresponds to 50Hz
    case 200000:
        VideoInfo->dwInterlaceFlags = AMINTERLACE_DisplayModeWeaveOnly;
        break;
    // corresponds to 60Hz or 59.94Hz
    case 166666:
    case 166667:
    case 166683:
    case 166684:
        VideoInfo->dwInterlaceFlags = AMINTERLACE_DisplayModeWeaveOnly;
        break;
    default:
        VideoInfo->dwInterlaceFlags = AMINTERLACE_DisplayModeWeaveOnly;
        break;
    }
}

void CInputPin::GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
    {
        Sample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)SampleProperties);
    }
    else
    {
        ZeroMemory(SampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));
        SampleProperties->cbData =  sizeof(AM_SAMPLE2_PROPERTIES);
        SampleProperties->lActual =  Sample->GetSize();
        if(Sample->GetTime(&SampleProperties->tStart, &SampleProperties->tStop) == S_OK)
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        }
        Sample->GetMediaType(&SampleProperties->pMediaType);
        Sample->GetPointer(&SampleProperties->pbBuffer);
        SampleProperties->cbBuffer = Sample->GetActualDataLength();
        if(Sample->IsDiscontinuity())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEDISCONTINUITY;
        }
        if(Sample->IsPreroll())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_PREROLL;
        }
        if(Sample->IsSyncPoint())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        }

        SampleProperties->dwStreamId = AM_STREAM_MEDIA;
    }
}

HRESULT CInputPin::SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
    {
        hr = Sample2->SetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&SampleProperties);
    }
    else
    {
        if(SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
        {
            hr = Sample->SetTime(&SampleProperties->tStart, &SampleProperties->tStop);
            if(FAILED(hr)) return hr;
        }

        hr = Sample->SetMediaType(SampleProperties->pMediaType);
        if(FAILED(hr)) return hr;
        
        hr = Sample->SetDiscontinuity((SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY) > 0);
        if(FAILED(hr)) return hr;

        hr = Sample->SetPreroll((SampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL) > 0);
        if(FAILED(hr)) return hr;

        hr = Sample->SetSyncPoint((SampleProperties->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) > 0);
        if(FAILED(hr)) return hr;
    }
    return hr;
}

void CInputPin::WorkOutImpliedMediaType()
{
    m_VideoSampleFlag = 0;

    if(m_InputMediaType.formattype == FORMAT_VIDEOINFO2)
    {
        CopyMediaType(&m_ImpliedMediaType, &m_InputMediaType);
        VIDEOINFOHEADER2* VideoInfo = (VIDEOINFOHEADER2*)m_ImpliedMediaType.pbFormat;
        if(VideoInfo->dwInterlaceFlags & AMINTERLACE_IsInterlaced)
        {
            if(VideoInfo->dwInterlaceFlags == (AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeWeaveOnly))
            {
                // undo forced weave flags
                // we need to guess which field is first
                VideoInfo->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;
                GuessInterlaceFlags();
                m_VideoSampleMask = 0;
            }
            else if(VideoInfo->dwInterlaceFlags == (AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly))
            {
                // undo forced bob flags
                // we should be told which field is first in the samples
                VideoInfo->dwInterlaceFlags &= ~AMINTERLACE_DisplayModeBobOnly;
                VideoInfo->dwInterlaceFlags |= AMINTERLACE_DisplayModeBobOrWeave;
                m_VideoSampleMask = AM_VIDEO_FLAG_FIELD1FIRST;
            }
            else
            {
                // otherwise leave the flags alone
                m_VideoSampleMask = 0xffffffff;
            }
        }
        else
        {
            GuessInterlaceFlags();
            m_VideoSampleMask = 0;
        }
    }
    else if(m_InputMediaType.formattype == FORMAT_VideoInfo)
    {
        m_ImpliedMediaType.majortype = m_InputMediaType.majortype;
        m_ImpliedMediaType.subtype = m_InputMediaType.subtype;
        m_ImpliedMediaType.bFixedSizeSamples = m_InputMediaType.bFixedSizeSamples;
        m_ImpliedMediaType.bTemporalCompression = m_InputMediaType.bTemporalCompression;
        m_ImpliedMediaType.lSampleSize = m_InputMediaType.lSampleSize;
        m_ImpliedMediaType.formattype = FORMAT_VIDEOINFO2;
        m_ImpliedMediaType.cbFormat = sizeof(VIDEOINFOHEADER2);
        VIDEOINFOHEADER2* NewFormat = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
        ZeroMemory(NewFormat, sizeof(VIDEOINFOHEADER2));
        if(NewFormat == NULL)
        {
            ClearMediaType(&m_ImpliedMediaType);
            return;
        }
        m_ImpliedMediaType.pbFormat =(BYTE*)NewFormat;
        VIDEOINFOHEADER* OldFormat = (VIDEOINFOHEADER*)m_InputMediaType.pbFormat;

        NewFormat->rcSource = OldFormat->rcSource;
        if(NewFormat->rcSource.bottom == 0)
        {
            NewFormat->rcSource.bottom = OldFormat->bmiHeader.biHeight;
        }
        if(NewFormat->rcSource.right == 0)
        {
            NewFormat->rcSource.right = OldFormat->bmiHeader.biWidth;
        }
        NewFormat->rcTarget = OldFormat->rcTarget;
        if(NewFormat->rcTarget.bottom == 0)
        {
            NewFormat->rcTarget.bottom = OldFormat->bmiHeader.biHeight;
        }
        if(NewFormat->rcTarget.right == 0)
        {
            NewFormat->rcTarget.right = OldFormat->bmiHeader.biWidth;
        }
        NewFormat->dwBitRate = OldFormat->dwBitRate;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame;

        NewFormat->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;
        NewFormat->dwPictAspectRatioX = OldFormat->bmiHeader.biWidth;
        NewFormat->dwPictAspectRatioY = OldFormat->bmiHeader.biHeight;
        memcpy(&NewFormat->bmiHeader, &OldFormat->bmiHeader, sizeof(BITMAPINFOHEADER));
        GuessInterlaceFlags();
        m_VideoSampleMask = 0;
    }
    else
    {
        ClearMediaType(&m_ImpliedMediaType);
    }
}
