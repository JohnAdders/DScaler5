///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.cpp,v 1.8 2003-05-07 16:27:41 adcockj Exp $
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
// Revision 1.7  2003/05/07 07:03:56  adcockj
// Some bug fixes
//
// Revision 1.6  2003/05/06 16:38:00  adcockj
// Changed to fixed size output buffer and changed connection handling
//
// Revision 1.5  2003/05/06 07:00:29  adcockj
// Some cahnges from Torbjorn also some other attempted fixes
//
// Revision 1.4  2003/05/02 16:05:23  adcockj
// Logging with file and line numbers
//
// Revision 1.3  2003/05/02 07:03:13  adcockj
// Some minor changes most not really improvements
//
// Revision 1.2  2003/05/01 16:22:24  adcockj
// Dynamic connection test code
//
// Revision 1.1.1.1  2003/04/30 13:01:21  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputPin.h"
#include "DScaler.h"
#include "OutputPin.h"
#include "EnumMediaTypes.h"

CInputPin::CInputPin()
{
    LOG(DBGLOG_FLOW, ("CInputPin::CInputPin\n"));
    m_Flushing = FALSE;
    m_bReadOnly = FALSE;
    m_NotifyEvent = NULL;
    InitMediaType(&m_InputMediaType);
    m_FormatVersion = 0;
}

CInputPin::~CInputPin()
{
    LOG(DBGLOG_FLOW, ("CInputPin::~CInputPin\n"));
    ClearMediaType(&m_InputMediaType);
}


STDMETHODIMP CInputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - CInputPin::Connect\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP CInputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("CInputPin::ReceiveConnection\n"));
    HRESULT hr = S_OK;
    BOOL TryToReconnectOnFail = FALSE;
    if(pConnector == NULL || pmt == NULL)
    {
        return E_POINTER;

    }
    if(QueryAccept(pmt) != S_OK)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    // see if we need to check with the output
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        AM_MEDIA_TYPE TestType;
        InitMediaType(&TestType);
        m_OutputPin->WorkOutNewMediaType(pmt, &TestType);
        
        // can the input pin of the filter downstream accept
        // our version of this format
        // \todo maybe we need to use DynamicQueryAccept if it's there
        if(m_OutputPin->m_ConnectedPin->QueryAccept(&TestType) != S_OK)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
        CopyMediaType(&m_OutputPin->m_CurrentMediaType, &TestType);
        m_OutputPin->m_FormatChanged = TRUE;    
        ++m_OutputPin->m_FormatVersion;
    }

    m_ConnectedPin = pConnector;
    CopyMediaType(&m_InputMediaType, pmt);
    LogMediaType(&m_InputMediaType, "Input Connected");
    ++m_FormatVersion;

    LOG(DBGLOG_FLOW, ("CInputPin::ReceiveConnection Exit\n"));
    return hr;
}

STDMETHODIMP CInputPin::Disconnect(void)
{
    LOG(DBGLOG_FLOW, ("CInputPin::Disconnect\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::ConnectedTo\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::ConnectionMediaType\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::QueryPinInfo\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::QueryDirection\n"));
    if(pPinDir == NULL)
    {
        return E_POINTER;
    }
    *pPinDir = PINDIR_INPUT;
    return S_OK;
}

STDMETHODIMP CInputPin::QueryId(LPWSTR *Id)
{
    LOG(DBGLOG_FLOW, ("CInputPin::QueryId\n"));

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
    LOG(DBGLOG_FLOW, ("CInputPin::QueryAccept\n"));

    LogMediaType(pmt, "CInputPin::QueryAccept");
    
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
    BITMAPINFOHEADER* BitmapInfo;
    if(pmt->formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
        BitmapInfo = &Format->bmiHeader;
    }
    else
    {
        VIDEOINFOHEADER* Format = (VIDEOINFOHEADER*)pmt->pbFormat;
        BitmapInfo = &Format->bmiHeader;
    }

    // check that the incoming format is SDTV    
    if(BitmapInfo->biHeight < -576 || BitmapInfo->biHeight > 576)
    {
        return S_FALSE;
    }

    if(BitmapInfo->biWidth > 768)
    {
        return S_FALSE;
    }
    
    // check that the incoming type is the same as
    // we're outputting if connected
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        if(pmt->subtype != m_OutputPin->m_CurrentMediaType.subtype)
        {
            return S_FALSE;
        }
    }
    return S_OK;
}

STDMETHODIMP CInputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_FLOW, ("CInputPin::EnumMediaTypes\n"));
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    NewEnum->SetUpdate(this);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    return S_OK;
}

STDMETHODIMP CInputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    LOG(DBGLOG_FLOW, ("CInputPin::QueryInternalConnections\n"));
    // don't bother with this as we are a simple filter
    return E_NOTIMPL;
}

STDMETHODIMP CInputPin::EndOfStream(void)
{
    LOG(DBGLOG_FLOW, ("CInputPin::EndOfStream\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::BeginFlush\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::EndFlush\n"));

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
    LOG(DBGLOG_FLOW, ("CInputPin::NewSegment\n"));
    HRESULT hr = S_OK;
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        hr = m_OutputPin->m_ConnectedPin->NewSegment(tStart, tStop, dRate);
    }
    return hr;
}

STDMETHODIMP CInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    LOG(DBGLOG_FLOW, ("CInputPin::GetAllocator\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::NotifyAllocator\n"));
    m_Allocator = pAllocator;
    m_bReadOnly = bReadOnly;
    return S_OK;
}

STDMETHODIMP CInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    LOG(DBGLOG_FLOW, ("CInputPin::GetAllocatorRequirements\n"));
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
    //LOG(DBGLOG_FLOW, ("CInputPin::Receive\n"));
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
    // check for media type changes on the input side
    // a NULL means the type is the same as last time
    AM_MEDIA_TYPE* InputType = NULL;
    hr = pSample->GetMediaType(&InputType);

    //if the input and output pins are using the same allocator, there is 
    //no way to tell if this is from the output or input pin ( i think )
    if(InputType != NULL)
    {
        CopyMediaType(&m_InputMediaType, InputType);
        LogMediaType(&m_InputMediaType, "Input Format Change");
        m_OutputPin->WorkOutNewMediaType(&m_InputMediaType, &m_OutputPin->m_CurrentMediaType);
        m_OutputPin->m_FormatChanged = TRUE;
        ++m_OutputPin->m_FormatVersion;
        ++m_FormatVersion;
        FreeMediaType(InputType);
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
        LOG(DBGLOG_FLOW, ("Frame Skipped\n"));
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
        CopyMediaType(&m_OutputPin->m_ConnectedMediaType, OutputType);
        LogMediaType(OutputType, "Output Format Change");
        m_OutputPin->WorkOutNewMediaType(&m_InputMediaType, &m_OutputPin->m_CurrentMediaType);
        m_OutputPin->m_FormatChanged = TRUE;
        FreeMediaType(OutputType);
    }

    CComQIPtr<IDirectDrawMediaSample> pTest = OutSample;
    BITMAPINFOHEADER* InputBMI = GetBitmapInfo();
    BITMAPINFOHEADER* OutputBMI = m_OutputPin->GetBitmapInfo();

    ATLASSERT(InputBMI->biWidth <= OutputBMI->biWidth);
    ATLASSERT(abs(InputBMI->biHeight) <= abs(OutputBMI->biHeight));
    int Lines = abs(InputBMI->biHeight);
    
    AM_SAMPLE2_PROPERTIES OutSampleProperties;
    GetSampleProperties(OutSample, &OutSampleProperties);

    BYTE* pInBuffer = SampleProperties.pbBuffer;
    BYTE* pOutBuffer = OutSampleProperties.pbBuffer;
    
    for(int i(0); i < Lines; ++i)
    {
        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth * 2);
        pOutBuffer += OutputBMI->biWidth * 2;
        pInBuffer += InputBMI->biWidth * 2;
    }
    
    OutSampleProperties.dwSampleFlags = SampleProperties.dwSampleFlags;
    OutSampleProperties.dwTypeSpecificFlags = 0;
    OutSampleProperties.tStart = SampleProperties.tStart;
    OutSampleProperties.tStop = SampleProperties.tStop;
    OutSampleProperties.dwSampleFlags = SampleProperties.dwSampleFlags;

    if(m_OutputPin->m_FormatChanged == TRUE)
    {
        OutSampleProperties.pMediaType = (AM_MEDIA_TYPE*)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
        if(OutSampleProperties.pMediaType != NULL)
        {
            InitMediaType(OutSampleProperties.pMediaType);
            m_OutputPin->WorkOutNewMediaType(&m_InputMediaType, OutSampleProperties.pMediaType);
            CopyMediaType(OutSampleProperties.pMediaType, &m_OutputPin->m_CurrentMediaType);
            ++m_OutputPin->m_FormatVersion;
            m_OutputPin->m_FormatChanged = FALSE;
        }
    }

    hr = SetSampleProperties(OutSample, &OutSampleProperties);

    hr = m_OutputPin->m_MemInputPin->Receive(OutSample);

    if(OutSampleProperties.pMediaType != NULL)
    {
        FreeMediaType(OutSampleProperties.pMediaType);
    }
    OutSample->Release();
    return hr;
}

STDMETHODIMP CInputPin::ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed)
{
    LOG(DBGLOG_FLOW, ("CInputPin::ReceiveMultiple\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::ReceiveCanBlock\n"));
    return S_OK;
}

STDMETHODIMP CInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("CInputPin::DynamicQueryAccept\n"));

    LogMediaType(pmt, "CInputPin::DynamicQueryAccept");
    
    // first see if we can propogate this downstream
    if(m_OutputPin->m_PinConnection != NULL)
    {
        // then see if we understand this media type
        HRESULT hr = QueryAccept(pmt);
        if(hr != S_OK)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }

        // \todo do some extra checks if we are connected

        AM_MEDIA_TYPE TestType;
        InitMediaType(&TestType);
        m_OutputPin->WorkOutNewMediaType(pmt, &TestType);
        
        return m_OutputPin->m_PinConnection->DynamicQueryAccept(&TestType);
    }
    else
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
}

STDMETHODIMP CInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    LOG(DBGLOG_FLOW, ("CInputPin::NotifyEndOfStream\n"));
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
    LOG(DBGLOG_FLOW, ("CInputPin::IsEndPin\n"));
    return S_FALSE;
}

STDMETHODIMP CInputPin::DynamicDisconnect(void)
{
    LOG(DBGLOG_FLOW, ("CInputPin::DynamicDisconnect\n"));
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
    m_ConnectedPin.Release();
    m_Allocator.Release();
    ++m_FormatVersion;
}

ULONG CInputPin::FormatVersion()
{
    return m_FormatVersion;
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

void CInputPin::GuessInterlaceFlags(AM_SAMPLE2_PROPERTIES* Props)
{
    switch((long)(Props->tStop - Props->tStart))
    {
    // corresponds to 25Hz
    case 400000:
        m_VideoSampleFlag = AM_VIDEO_FLAG_FIELD1FIRST;
        break;
    // corresponds to 30Hz or 29.97Hz
    case 333333:
    case 333334:
    case 333366:
    case 333367:
        m_VideoSampleFlag = 0;
        break;
    // corresponds to 50Hz
    case 200000:
        m_VideoSampleFlag = AM_VIDEO_FLAG_WEAVE;
        break;
    // corresponds to 60Hz or 59.94Hz
    case 166666:
    case 166667:
    case 166683:
    case 166684:
        m_VideoSampleFlag = AM_VIDEO_FLAG_WEAVE;
        break;
    default:
        m_VideoSampleFlag = 0;
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

BITMAPINFOHEADER* CInputPin::GetBitmapInfo()
{
    if(m_InputMediaType.formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)m_InputMediaType.pbFormat;
        return &Format->bmiHeader;
    }
    else
    {
        VIDEOINFOHEADER* Format = (VIDEOINFOHEADER*)m_InputMediaType.pbFormat;
        return &Format->bmiHeader;
    }
}
