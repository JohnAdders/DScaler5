///////////////////////////////////////////////////////////////////////////////
// $Id: DSInputPin.cpp,v 1.14 2004-08-04 13:34:58 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.13  2004/08/03 08:55:57  adcockj
// Fixes for seeking issues
//
// Revision 1.12  2004/08/02 16:56:57  adcockj
// Fix for seeking issues with ReceiveMultiple
//
// Revision 1.11  2004/07/07 14:09:01  adcockj
// removed tabs
//
// Revision 1.10  2004/07/01 16:12:47  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.9  2004/05/25 16:59:30  adcockj
// fixed issues with new buffered pin
//
// Revision 1.8  2004/04/29 16:16:46  adcockj
// Yet more reconnection fixes
//
// Revision 1.7  2004/04/20 16:30:31  adcockj
// Improved Dynamic Connections
//
// Revision 1.6  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.5  2004/02/27 17:08:16  adcockj
// Improved locking at state changes
// Better error handling at state changes
//
// Revision 1.4  2004/02/25 17:14:03  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.2  2004/02/10 13:24:12  adcockj
// Lots of bug fixes + corrected interlaced YV12 upconversion
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSBasePin.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"

CDSInputPin::CDSInputPin() :
    CDSBasePin(PINDIR_INPUT)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::CDSInputPin\n"));
    m_Flushing = FALSE;
    m_bReadOnly = FALSE;
    m_NotifyEvent = NULL;
    m_Block = FALSE;
    m_BlockEvent = NULL;
    m_SourceType = SOURCE_DEFAULT;
}

CDSInputPin::~CDSInputPin()
{
    LOG(DBGLOG_ALL, ("CDSInputPin::~CDSInputPin\n"));
}


STDMETHODIMP CDSInputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSInputPin::Connect\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP CDSInputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::ReceiveConnection\n"));
    HRESULT hr = S_OK;
    BOOL TryToReconnectOnFail = FALSE;
    if(pConnector == NULL || pmt == NULL)
    {
        return E_POINTER;
    }

    // find out who the pin belongs to
    // we will process differently depending on 
    // the filter at the other end
    // if this function returns TRUE we are talking to 
    // ourself then error
    if(AreWeAreTalkingToOurself(pConnector) == TRUE)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    FixupMediaType((AM_MEDIA_TYPE *)pmt);

    if(QueryAccept(pmt) != S_OK)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    // prevents any processing while we're doing this
    // will wait for any pending processing to finish
    CProtectCode WhileVarInScope(this);

    hr = SetType(pmt);
    CHECK(hr);

    m_ConnectedPin = pConnector;

    hr = m_Filter->NotifyConnected(this);

    LOG(DBGLOG_ALL, ("CDSInputPin::ReceiveConnection Exit\n"));
    return hr;
}

STDMETHODIMP CDSInputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::QueryAccept\n"));

    LogMediaType(pmt, "CDSInputPin::QueryAccept", DBGLOG_ALL);

    if(m_Filter->IsThisATypeWeCanWorkWith(pmt, this) == false)
    {
        return S_FALSE;
    }

    return S_OK;
}

STDMETHODIMP CDSInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    return m_Filter->GetAllocatorRequirements(pProps, this);
}


STDMETHODIMP CDSInputPin::EndOfStream(void)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::EndOfStream\n"));
    HRESULT hr = S_OK;

    // synchronize with Recieve
    CProtectCode WhileVarInScope(this);

    bool Connected(false);
    for(int i(0); i < m_Filter->GetNumPins(); ++i)
    {
        CDSBasePin* pPin = m_Filter->GetPin(i);
        if(pPin->m_Direction == PINDIR_OUTPUT && pPin->m_ConnectedPin != NULL)
        {
            Connected = true;
        }
    }

    if(Connected)
    {
        // check if we need are in the middle of
        // a format renegoatiation
        if(m_NotifyEvent == NULL)
        {
            // finish off processing
            hr = m_Filter->SendOutLastSamples(this);
            CHECK(hr);

            // send the message downstream
            for(int i(0); i < m_Filter->GetNumPins(); ++i)
            {
                CDSBasePin* pPin = m_Filter->GetPin(i);
                if(pPin->m_Direction == PINDIR_OUTPUT && pPin->m_ConnectedPin != NULL)
                {
                    hr = pPin->m_ConnectedPin->EndOfStream();
                    CHECK(hr);
                }
            }
        }
        else
        {
            SetEvent(m_NotifyEvent);
        }
    }
    return hr;
}

STDMETHODIMP CDSInputPin::BeginFlush(void)
{
    HRESULT hr = S_OK;
    LOG(DBGLOG_FLOW, ("CDSInputPin::BeginFlush\n"));

    CProtectCode WhileVarInScope(m_Filter);

    // Sets an internal flag that causes all data-streaming methods to fail
    m_Flushing = TRUE;

    // pass the Flush downstream
    for(int i(0); i < m_Filter->GetNumPins(); ++i)
    {
        CDSBasePin* pPin = m_Filter->GetPin(i);
        if(pPin->m_Direction == PINDIR_OUTPUT && pPin->m_ConnectedPin != NULL)
        {
            hr = pPin->m_ConnectedPin->BeginFlush();
            if(hr == E_FAIL)
            {
                hr = S_OK;
            }
            CHECK(hr);
        }
    }

    // wait for processing to finish
    CProtectCode WhileVarInScope2(this);

    hr = m_Filter->Flush(this);
    CHECK(hr);

    return hr;
}

STDMETHODIMP CDSInputPin::EndFlush(void)
{
    LOG(DBGLOG_FLOW, ("CDSInputPin::EndFlush\n"));

    CProtectCode WhileVarInScope(m_Filter);

    // Make sure we've finished in the processing thread as well
    CProtectCode WhileVarInScope2(this);

    if(m_Block == TRUE)
    {
        if(m_BlockEvent != NULL)
        {
            ResetEvent(m_BlockEvent);
        }
    }

    HRESULT hr =  S_OK;

    // Calls EndFlush downstream. 
    for(int i(0); i < m_Filter->GetNumPins(); ++i)
    {
        CDSBasePin* pPin = m_Filter->GetPin(i);
        if(pPin->m_Direction == PINDIR_OUTPUT && pPin->m_ConnectedPin != NULL)
        {
            hr = pPin->m_ConnectedPin->EndFlush();
            if(hr == E_FAIL)
            {
                hr = S_OK;
            }
            CHECK(hr);
        }
    }

    m_Flushing = FALSE;
    return hr;
}

STDMETHODIMP CDSInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_FLOW, ("CDSInputPin::NewSegment %I64d %I64d %f\n", tStart, tStop, dRate));
    HRESULT hr = S_OK;

    // Will wait for all any streaming functions to finish
    // may block so be careful
    CProtectCode WhileVarInScope(this);

    hr = m_Filter->NewSegmentInternal(tStart, tStop, dRate, this);
    CHECK(hr);
    if(hr == S_OK)
    {
        for(int i(0); i < m_Filter->GetNumPins(); ++i)
        {
            CDSBasePin* pPin = m_Filter->GetPin(i);
            if(pPin->m_Direction == PINDIR_OUTPUT && pPin->m_ConnectedPin != NULL)
            {
                HRESULT hr = pPin->m_ConnectedPin->NewSegment(tStart, tStop, dRate);
                CHECK(hr);
            }
        }
    }
    return S_OK;
}

STDMETHODIMP CDSInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::GetAllocator\n"));
    if(ppAllocator == NULL)
    {
        return E_POINTER;
    }
    if(m_Allocator == NULL)
    {
        *ppAllocator = m_MyMemAlloc.GetAddRefedInterface();
        return S_OK;
    }
    else
    {
        *ppAllocator = m_Allocator.GetAddRefedInterface();
        return S_OK;
    }
}

STDMETHODIMP CDSInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::NotifyAllocator\n"));
    m_Allocator = pAllocator;
    m_bReadOnly = bReadOnly;

    // save the properties as we will need to check then later
    return m_Allocator->GetProperties(&m_AllocatorProperties);
}

STDMETHODIMP CDSInputPin::Receive(IMediaSample *InSample)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::Receive\n"));

    if(InSample == NULL)
    {
        return E_POINTER;
    }
    if(m_Flushing == TRUE)
    {
        LOG(DBGLOG_FLOW, ("CDSInputPin::Receive flushing\n"));
        return S_FALSE;
    }
    if(m_Filter->m_State == State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }

    // all code below here is protected
    // from runnning at the same time as other 
    // functions with this line
    CProtectCode WhileVarInScope(this);
    
    HRESULT hr = S_OK;
    AM_SAMPLE2_PROPERTIES InSampleProperties;
    ZeroMemory(&InSampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));

    //LogSample(InSample, "New Input Sample");

    hr = GetSampleProperties(InSample, &InSampleProperties);
    CHECK(hr);

    // check for media type changes on the input side
    // a NULL means the type is the same as last time
    if(InSampleProperties.pMediaType != NULL)
    {

        LOG(DBGLOG_FLOW, ("Got new media type\n"));

        FixupMediaType(InSampleProperties.pMediaType);

        // this shouldn't ever fail as a good filter will have
        // called this already but I've seen a filter ignore the
        // results of a queryaccept
        hr = QueryAccept(InSampleProperties.pMediaType);
        if(hr != S_OK)
        {
            FreeMediaType(InSampleProperties.pMediaType);
            return VFW_E_INVALIDMEDIATYPE;
        }
        SetType(InSampleProperties.pMediaType);
    }

    // check to see if we are blocked
    // need to check this before we get each sample
    CheckForBlocking();

    // make sure we don't bother sending nothing down
    if(InSampleProperties.lActual > 0 && InSampleProperties.pbBuffer != NULL) 
    {
        // Send the sample to the filter for processing 
        hr = m_Filter->ProcessSample(InSample, &InSampleProperties, this);
    }

    // make sure that anything that needs to be cleaned up
    // is actually cleaned up
    if(InSampleProperties.pMediaType != NULL)
    {
        FreeMediaType(InSampleProperties.pMediaType);
    }
    return hr;
}

STDMETHODIMP CDSInputPin::ReceiveMultiple(IMediaSample **InSamples, long nSamples, long *nSamplesProcessed)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::ReceiveMultiple\n"));
    HRESULT hr;
    if(nSamplesProcessed == NULL)
    {
        return E_POINTER;
    }

    CProtectCode WhileVarInScope(this);

    *nSamplesProcessed = 0;
    for(int i(0); i < nSamples; ++i)
    {
        hr = Receive(InSamples[i]);
        if(hr != S_OK)
        {
            return hr;
        }
        ++(*nSamplesProcessed);
    }
    return hr;
}

STDMETHODIMP CDSInputPin::ReceiveCanBlock(void)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::ReceiveCanBlock\n"));
    return S_OK;
}

STDMETHODIMP CDSInputPin::DynamicQueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::DynamicQueryAccept\n"));
    LogMediaType(pmt, "CDSInputPin::DynamicQueryAccept", DBGLOG_ALL);

    HRESULT hr = S_OK;
    if(m_Filter->IsThisATypeWeCanWorkWith(pmt, this) == false)
    {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return hr;
}

STDMETHODIMP CDSInputPin::NotifyEndOfStream(HANDLE hNotifyEvent)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::NotifyEndOfStream\n"));
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

STDMETHODIMP CDSInputPin::IsEndPin(void)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::IsEndPin\n"));
    return S_FALSE;
}

STDMETHODIMP CDSInputPin::DynamicDisconnect(void)
{
    LOG(DBGLOG_ALL, ("CDSInputPin::DynamicDisconnect\n"));
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

HRESULT CDSInputPin::GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    SI(IMediaSample2) Sample2 = Sample;
    if(Sample2)
    {
        hr = Sample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)SampleProperties);
        // for consistency with the old stuff
        // copy the Buffer's internal media type so that we
        // can alway free the media type is one is returned
        // this is a bit of a hit but shouldn't happen too often
        if(SampleProperties->pMediaType != NULL)
        {
            AM_MEDIA_TYPE* NewType = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
            if(NewType == NULL)
            {
                return E_OUTOFMEMORY;
            }
            InitMediaType(NewType);
            hr = CopyMediaType(NewType, SampleProperties->pMediaType);
            SampleProperties->pMediaType = NewType;
        }
    }
    else
    {
        ZeroMemory(SampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));
        SampleProperties->cbData =  sizeof(AM_SAMPLE2_PROPERTIES);
        SampleProperties->lActual =  Sample->GetActualDataLength();
        if(Sample->GetTime(&SampleProperties->tStart, &SampleProperties->tStop) == S_OK)
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        }
        hr = Sample->GetMediaType(&SampleProperties->pMediaType);
        CHECK(hr);
        hr = Sample->GetPointer(&SampleProperties->pbBuffer);
        CHECK(hr);
        SampleProperties->cbBuffer = Sample->GetSize();
        if(Sample->IsDiscontinuity())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
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
    return hr;
}

HRESULT CDSInputPin::SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    SI(IMediaSample2) Sample2 = Sample;
    if(Sample2)
    {
        // miss out all the stuff at the end of the structure so don't update
        // dwStreamId pMediaType pbBuffer cbBuffer
        hr = Sample2->SetProperties(FIELD_OFFSET(AM_SAMPLE2_PROPERTIES, dwStreamId), (BYTE*)SampleProperties);
    }
    else
    {
        if(SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
        {
            hr = Sample->SetTime(&SampleProperties->tStart, &SampleProperties->tStop);
            CHECK(hr);
        }
      
        hr = Sample->SetDiscontinuity((SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY) > 0);
        CHECK(hr);

        hr = Sample->SetPreroll((SampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL) > 0);
        CHECK(hr);

        hr = Sample->SetSyncPoint((SampleProperties->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) > 0);
        CHECK(hr);
    }
    return hr;
}

void CDSInputPin::FixupMediaType(AM_MEDIA_TYPE *pmt)
{
    BITMAPINFOHEADER* BitmapInfo = NULL;
    LPRECT pSource;

    if(pmt->formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
        BitmapInfo = &Format->bmiHeader;
        pSource = &Format->rcSource; 
    }
    else if(pmt->formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* Format = (VIDEOINFOHEADER*)pmt->pbFormat;
        BitmapInfo = &Format->bmiHeader;
        pSource = &Format->rcSource; 
    }
    else
    {
        return;
    }

    if(pSource->right == 0)
    {
        pSource->right = BitmapInfo->biWidth;
        pSource->bottom = abs(BitmapInfo->biHeight);
    }

    if(BitmapInfo->biSizeImage > BitmapInfo->biHeight * BitmapInfo->biWidth * BitmapInfo->biBitCount / 8U)
    {
        BitmapInfo->biWidth = BitmapInfo->biSizeImage / (BitmapInfo->biHeight * BitmapInfo->biBitCount / 8);
    }
}

void CDSInputPin::CheckForBlocking()
{
    if(m_Block == TRUE)
    {
        if(m_BlockEvent != NULL)
        {
            ResetEvent(m_BlockEvent);
            m_BlockEvent = NULL;
        }
        // do a crude spin on the Block variable
        // this should allow other threads in 
        while(m_Block)
        {
            Sleep(1);
        }
    }
}


BOOL CDSInputPin::AreWeAreTalkingToOurself(IPin* pConnector)
{
    PIN_INFO PinInfo;
    BOOL RetVal = TRUE;

    m_SourceType = SOURCE_DEFAULT;

    CLSID MyClassId;
    HRESULT hr = m_Filter->GetClassID(&MyClassId);
    CHECK(hr);

    hr = pConnector->QueryPinInfo(&PinInfo);
    if(SUCCEEDED(hr))
    {
        if(PinInfo.pFilter != NULL)
        {
            CLSID ClassId;
            hr = PinInfo.pFilter->GetClassID(&ClassId);
            if(SUCCEEDED(hr))
            {
                if(ClassId == MyClassId)
                {   
                    // if we're talking to ourselves
                    // return TRUE
                    RetVal = TRUE;
                }
                else
                {
                    RetVal = FALSE;
                }
            }
            PinInfo.pFilter->Release();
        }
    }

    return RetVal;
}

HRESULT CDSInputPin::Block(DWORD dwBlockFlags, HANDLE hEvent)
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::Block\n"));
    if(dwBlockFlags == AM_PIN_FLOW_CONTROL_BLOCK)
    {
        if(m_Block == FALSE)
        {
            if(hEvent != NULL)
            {
                m_BlockEvent = hEvent;
                m_Block = TRUE;
                return S_OK;
            }
            else
            {
                m_BlockEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                if(m_BlockEvent != NULL)
                {
                    m_Block = TRUE;
                    WaitForSingleObject(m_BlockEvent, INFINITE);
                    return S_OK;
                }
                else
                {
                    return E_UNEXPECTED;
                }
            }
        }
        else
        {
            return VFW_E_PIN_ALREADY_BLOCKED;
        }
    }
    else if(dwBlockFlags == 0 && hEvent == NULL)
    {
        m_Block = FALSE;
        m_BlockEvent = NULL;
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT CDSInputPin::Activate()
{
    return S_OK;
}

HRESULT CDSInputPin::Deactivate()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::Deactivate\n"));

    if(m_Block == TRUE)
    {
        if(m_BlockEvent != NULL)
        {
            ResetEvent(m_BlockEvent);
        }
    }

    return S_OK;
}

