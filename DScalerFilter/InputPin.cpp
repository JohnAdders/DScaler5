///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.cpp,v 1.22 2003-08-22 16:48:24 adcockj Exp $
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
// Revision 1.21  2003/08/21 16:17:58  adcockj
// Changed filter to wrap the deinterlacing DMO, fixed many bugs
//
// Revision 1.20  2003/07/30 06:58:42  adcockj
// Uncommented temporary non-processing of YUY2
//
// Revision 1.19  2003/07/29 07:01:54  adcockj
// Fixed some issues with YV12 and general chroma positioning
//
// Revision 1.18  2003/07/25 16:00:55  adcockj
// Remove 704 stuff
//
// Revision 1.17  2003/05/20 16:50:59  adcockj
// Interim checkin, preparation for DMO processing path
//
// Revision 1.16  2003/05/17 11:29:35  adcockj
// Fixed crashing
//
// Revision 1.15  2003/05/16 15:18:36  adcockj
// Interim check inas we move to supporting DMO plug-ins
//
// Revision 1.14  2003/05/10 13:21:31  adcockj
// Bug fixes
//
// Revision 1.13  2003/05/09 15:51:05  adcockj
// Code tidy up
// Added aspect ratio parameters
//
// Revision 1.12  2003/05/09 07:03:25  adcockj
// Bug fixes for new format code
//
// Revision 1.11  2003/05/08 16:20:25  adcockj
// Tidy up
//
// Revision 1.10  2003/05/08 15:58:38  adcockj
// Better error handling, threading and format support
//
// Revision 1.9  2003/05/08 07:00:59  adcockj
// Couple of minor fixes
//
// Revision 1.8  2003/05/07 16:27:41  adcockj
// Slightly better properties implementation
//
// Revision 1.7  2003/05/07 07:03:56  adcockj
// Some bug fixes
//
// Revision 1.6  2003/05/06 16:38:00  adcockj
// Changed to fixed size output buffer and changed connection handling
//
// Revision 1.5  2003/05/06 07:00:29  adcockj
// Some changes from Torbjorn also some other attempted fixes
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
#include "MediaBufferWrapper.h"
#include "Process.h"

CInputPin::CInputPin()
{
    LOG(DBGLOG_FLOW, ("CInputPin::CInputPin\n"));
    m_Flushing = FALSE;
    m_bReadOnly = FALSE;
    m_NotifyEvent = NULL;
    InitMediaType(&m_InputMediaType);
    InitMediaType(&m_InternalMediaType);
    m_FormatVersion = 0;
    m_Block = FALSE;
    m_BlockEvent = NULL;
	m_dwFlags = AM_GBF_PREVFRAMESKIPPED;
	m_MyMemAlloc = new CComObject<CInputMemAlloc>;
	m_ExpectedStart = 0;
}

CInputPin::~CInputPin()
{
    LOG(DBGLOG_FLOW, ("CInputPin::~CInputPin\n"));
    ClearMediaType(&m_InputMediaType);
    ClearMediaType(&m_InternalMediaType);
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

    // Chcek that the format coming in is OK
    if(m_OutputPin->m_ConnectedPin != NULL && m_Filter->m_State != State_Stopped && m_OutputPin->m_PinConnection != NULL)
    {
        if(DynamicQueryAccept(pmt) != S_OK)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else
    {
        if(QueryAccept(pmt) != S_OK)
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    // prevents any processing while we're doing this
    // will wait for any pending processing to finish
    CProtectCode WhileVarInScope(this);

    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        hr = m_OutputPin->ChangeOutputFormat(pmt);
        CHECK(hr);
    }

    m_ConnectedPin = pConnector;
    hr = SetInputType(pmt);
    CHECK(hr);

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
        return m_ConnectedPin.CopyTo(pPin);;
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

    if(!IsThisATypeWeWorkWith(pmt))
    {
        return S_FALSE;
    }

    // see if we need to check with the output
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        HRESULT hr = S_OK;
        AM_MEDIA_TYPE TestType;
        InitMediaType(&TestType);
        hr = m_OutputPin->CreateOutputMediaType(pmt, &TestType);
        CHECK(hr);

        if(m_Filter->m_State != State_Stopped && m_OutputPin->m_PinConnection != NULL)
        {
            // can the input pin of the filter downstream accept
            // our version of this format dynamically
            hr = m_OutputPin->m_PinConnection->DynamicQueryAccept(&TestType);
        }
        else
        {
            // can the input pin of the filter downstream accept
            // our version of this format
            hr = m_OutputPin->m_ConnectedPin->QueryAccept(&TestType);
        }
        if(hr != S_OK)
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return S_FALSE;
        }
        ClearMediaType(&TestType);
    }
    return S_OK;
}

STDMETHODIMP CInputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_FLOW, ("CInputPin::EnumMediaTypes\n"));
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    HRESULT hr = NewEnum->SetUpdate(this);
    CHECK(hr);
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
    HRESULT hr = S_OK;
    // synchronize with Recieve
    CProtectCode WhileVarInScope(this);

    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        // check if we need are in the middle of
        // a format renegoatiation
        if(m_NotifyEvent == NULL)
        {
			hr = m_Filter->m_CurrentDeinterlacingMethod->Discontinuity(0);
            CHECK(hr);

            IMediaSample* OutSample = NULL;
            hr = GetOutputSample(&OutSample);
            CHECK(hr);

			hr = InternalProcessOutput(&OutSample);

            if(OutSample != NULL)
            {
                OutSample->Release();
            }

            CHECK(hr);

            hr = m_OutputPin->m_ConnectedPin->EndOfStream();
            CHECK(hr);
        }
        else
        {
            SetEvent(m_NotifyEvent);
        }
    }
    return hr;
}

STDMETHODIMP CInputPin::BeginFlush(void)
{
    LOG(DBGLOG_FLOW, ("CInputPin::BeginFlush\n"));

	CProtectCode WhileVarInScope(m_Filter);

	// Sets an internal flag that causes all data-streaming methods to fail
    m_Flushing = TRUE;

    // pass the Flush downstream
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        HRESULT hr = m_OutputPin->m_ConnectedPin->BeginFlush();
        CHECK(hr);
    }

    return S_OK;
}

STDMETHODIMP CInputPin::EndFlush(void)
{
    LOG(DBGLOG_FLOW, ("CInputPin::EndFlush\n"));

	CProtectCode WhileVarInScope(m_Filter);

    FinishProcessing();

    // Calls EndFlush downstream. 
    if(m_OutputPin->m_ConnectedPin != NULL)
    {
        HRESULT hr = m_OutputPin->m_ConnectedPin->EndFlush();
        CHECK(hr);
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
        // Will wait for all any streaming functions to finish
        // may block so be careful
        CProtectCode WhileVarInScope(this);
		m_ExpectedStart = tStart;
        hr = m_OutputPin->m_ConnectedPin->NewSegment(tStart, tStop, dRate);
        CHECK(hr);
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
        return m_MyMemAlloc.CopyTo(ppAllocator);
    }
    else
    {
        return m_Allocator.CopyTo(ppAllocator);
    }
}

STDMETHODIMP CInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    LOG(DBGLOG_FLOW, ("CInputPin::NotifyAllocator\n"));
    m_Allocator = pAllocator;
    m_bReadOnly = bReadOnly;

    // save the properties as we will need to check then later
    return m_Allocator->GetProperties(&m_AllocatorProperties);
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

STDMETHODIMP CInputPin::Receive(IMediaSample *InSample)
{
    //LOG(DBGLOG_FLOW, ("CInputPin::Receive\n"));

    if(InSample == NULL)
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

    // all code below here is protected
    // from runnning at the same time as other 
    // functions with this line
    CProtectCode WhileVarInScope(this);
    
    //HRESULT hr = m_Filter->CheckProcessingLine();
    //CHECK(hr);

    return InternalReceive(InSample);
}

HRESULT CInputPin::InternalReceive(IMediaSample *InSample)
{
    HRESULT hr = S_OK;
    AM_SAMPLE2_PROPERTIES InSampleProperties;
    ZeroMemory(&InSampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));
    IMediaSample* OutSample;

    __try
    {
        //LogSample(InSample, "New Input Sample");

        hr = GetSampleProperties(InSample, &InSampleProperties);
        CHECK(hr);

        // just pass through any non-media messages
        if(InSampleProperties.dwStreamId != AM_STREAM_MEDIA) 
        {
            LOG(DBGLOG_ERROR, ("Passed through non-media sample\n"));
            return m_OutputPin->m_MemInputPin->Receive(InSample);
        }

        // check for media type changes on the input side
        // a NULL means the type is the same as last time
        if(InSampleProperties.pMediaType != NULL)
        {
            // this shouldn't ever fail as a good filter will have
            // called this already but I've seen a filter ignore the
            // results of a queryaccept
            hr = QueryAccept(InSampleProperties.pMediaType);
            if(hr != S_OK)
            {
                return VFW_E_INVALIDMEDIATYPE;
            }
            // note that doing this here prevents us from 
            // sending data while the reconnection is going on
            // be careful if we move to threads here
            hr = m_OutputPin->ChangeOutputFormat(InSampleProperties.pMediaType);
            if(FAILED(hr))
            {
                // if the dynamic connection didn't work
                // then try and reconnect with the previous
                // format, this should work unless there is 
                // something seriously wrong
                hr = m_OutputPin->ChangeOutputFormat(&m_InputMediaType);
                CHECK(hr);
                return VFW_E_INVALIDMEDIATYPE;
            }
            SetInputType(InSampleProperties.pMediaType);
        }

		// check to see if we are blocked
        // need to check this before we get each sample
        CheckForBlocking();

        hr = GetOutputSample(&OutSample);
        CHECK(hr);

		if(OutSample == NULL)
		{
			return S_FALSE;
		}

        hr = m_Filter->CheckProcessingLine();
        CHECK(hr);

        // if there was a discontinuity then we need to ask for the buffer
        // differently 
        if(InSampleProperties.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY ||
			(m_ExpectedStart != 0 && m_ExpectedStart != InSampleProperties.tStart))
        {
            GuessInterlaceFlags(&InSampleProperties);
			hr = m_Filter->m_CurrentDeinterlacingMethod->Discontinuity(0);
			CHECK(hr);

			hr = InternalProcessOutput(&OutSample);
            m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
        }
		
		m_ExpectedStart = InSampleProperties.tStop;

		// pass input buffer to DMO
        IMediaBuffer* InBuffer = CMediaBufferWrapper::CreateBuffer(InSample);

		hr = m_Filter->m_CurrentDeinterlacingMethod->ProcessInput(
																	0, 
																	InBuffer, 
																	DMO_INPUT_DATA_BUFFERF_TIME | DMO_INPUT_DATA_BUFFERF_TIMELENGTH, 
																	InSampleProperties.tStart,
																	InSampleProperties.tStop - InSampleProperties.tStart
   																 );
        InSample->Release();


		if(hr == DMO_E_NOTACCEPTING)
		{
			return S_FALSE;
		}
		CHECK(hr);

		hr = InternalProcessOutput(&OutSample);

		if(FAILED(hr) || hr == S_FALSE)
		{
			return S_FALSE;
		}
    }
    // since there are loads of ways out of this function
    // make sure that anything that needs to be cleaned up
    // is actually cleaned up
    __finally
    {
        if(InSampleProperties.pMediaType != NULL)
        {
            FreeMediaType(InSampleProperties.pMediaType);
        }
        if(OutSample != NULL)
        {
            OutSample->Release();
        }
    }
    return hr;
}

HRESULT CInputPin::GetOutputSample(IMediaSample** OutSample)
{
    *OutSample = NULL;

	// get a sample to output to
	HRESULT hr = m_OutputPin->m_Allocator->GetBuffer(OutSample, NULL, NULL, m_dwFlags);
	if(FAILED(hr) || *OutSample == NULL)
	{
		LOG(DBGLOG_FLOW, ("Frame Skipped\n"));
		m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
		return S_FALSE;
	}
	else
	{
		m_dwFlags = 0;
	}

	// check for media type changes on the output side
	// a NULL means the type is the same as last time
    AM_MEDIA_TYPE* pMediaType = NULL;

    hr = (*OutSample)->GetMediaType(&pMediaType);

	CHECK(hr);

	if(hr == S_OK && pMediaType != NULL)
	{
		hr = m_OutputPin->SetMediaType(pMediaType);
		CHECK(hr);
        FreeMediaType(pMediaType);
	}
    return S_OK;
}

HRESULT CInputPin::InternalProcessOutput(IMediaSample** OutSample)
{
	DMO_OUTPUT_DATA_BUFFER OutDataBuffer;
	HRESULT hr = S_OK;
	
	OutDataBuffer.dwStatus = DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;

	while(hr == S_OK && OutDataBuffer.dwStatus == DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
	{
        IMediaBuffer* OutBuffer = CMediaBufferWrapper::CreateBuffer(*OutSample);

		OutDataBuffer.pBuffer = OutBuffer;
		OutDataBuffer.rtTimelength = 0;
		OutDataBuffer.rtTimestamp = 0;

		DWORD Status(0);

		hr = m_Filter->m_CurrentDeinterlacingMethod->ProcessOutput(
																	0, 
																	1,
																	&OutDataBuffer, 
																	&Status
																);
		
		if(hr == S_OK && m_Flushing == FALSE)
		{
			// finally send the processed sample on it's way
			hr = m_OutputPin->m_MemInputPin->Receive(*OutSample);
		}
		// if we are at the start then we might need to send 
		// a couple of frames in before getting a response
		else if(hr == S_FALSE)
		{
			hr = S_OK;
		}

		OutBuffer->Release();

		if(hr == S_OK && OutDataBuffer.dwStatus == DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE)
		{
			if(*OutSample != NULL)
			{
				(*OutSample)->Release();
				*OutSample = NULL;
			}

			hr = GetOutputSample(OutSample);
            CHECK(hr);
		}
	}
	return hr;
}

STDMETHODIMP CInputPin::ReceiveMultiple(IMediaSample **InSamples, long nSamples, long *nSamplesProcessed)
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
        hr = Receive(InSamples[i]);
        CHECK(hr);
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

    HRESULT hr = S_OK;
    // see if we need to check with the output
    if(m_OutputPin->m_ConnectedPin != NULL && m_OutputPin->m_PinConnection != NULL)
    {
        AM_MEDIA_TYPE TestType;
        InitMediaType(&TestType);
        hr = m_OutputPin->CreateOutputMediaType(pmt, &TestType);
        CHECK(hr);

        // can the input pin of the filter downstream accept
        // our version of this format dynamically
        hr = m_OutputPin->m_PinConnection->DynamicQueryAccept(&TestType);
        ClearMediaType(&TestType);
    }
    else
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
    }
    return hr;
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

HRESULT CInputPin::SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types)
{   
    HRESULT hr = S_OK;
    if(m_ConnectedPin != NULL)
    {
        NumTypes = 1;
        hr = CopyMediaType(Types, &m_InputMediaType);
        CHECK(hr);
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
    return hr;
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

HRESULT CInputPin::GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
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
        SampleProperties->lActual =  Sample->GetSize();
        if(Sample->GetTime(&SampleProperties->tStart, &SampleProperties->tStop) == S_OK)
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        }
        hr = Sample->GetMediaType(&SampleProperties->pMediaType);
        CHECK(hr);
        hr = Sample->GetPointer(&SampleProperties->pbBuffer);
        CHECK(hr);
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
    return hr;
}

HRESULT CInputPin::SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
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

BOOL CInputPin::IsThisATypeWeWorkWith(const AM_MEDIA_TYPE *pmt)
{
    if(pmt->majortype != MEDIATYPE_Video)
    {
        return FALSE;
    }

    if(pmt->formattype != FORMAT_VIDEOINFO2 && 
        pmt->formattype != FORMAT_VideoInfo)
    {
        return FALSE;
    }

    if(pmt->subtype != MEDIASUBTYPE_YUY2 && 
       pmt->subtype != MEDIASUBTYPE_YV12 &&
       pmt->subtype != MEDIASUBTYPE_NV12)
    {
        return FALSE;
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
        return FALSE;
    }

    if(BitmapInfo->biWidth > 768)
    {
        return FALSE;
    }
    return TRUE;

}

HRESULT CInputPin::FinishProcessing()
{
    // Will wait for all any streaming functions to finish
    // may block so be careful
	
    CProtectCode WhileVarInScope(this);

    if(m_Block == TRUE)
    {
        if(m_BlockEvent != NULL)
        {
            ResetEvent(m_BlockEvent);
        }
    }

    // \todo free any buffers
	m_Filter->m_CurrentDeinterlacingMethod->Flush();
    return S_OK;   
}

void CInputPin::CheckForBlocking()
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

HRESULT CInputPin::SetInputType(const AM_MEDIA_TYPE *pmt)
{
    // save the media type to local variable
    HRESULT hr = CopyMediaType(&m_InputMediaType, pmt);
    CHECK(hr);
    LogMediaType(&m_InputMediaType, "Input Connected");
    ++m_FormatVersion;
    // create a new internal type for use by the DMOs
    hr = CreateInternalMediaType(&m_InputMediaType, &m_InternalMediaType);
    CHECK(hr);

	// tell the filter that we need to rebuild the processing path
    m_Filter->SetTypesChangedFlag();
    return hr;
}


HRESULT CInputPin::CreateInternalMediaType(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType)
{
    BITMAPINFOHEADER* BitmapInfo = NULL;
    NewType->majortype = MEDIATYPE_Video;
    NewType->subtype = InputType->subtype;
    NewType->bFixedSizeSamples = TRUE;
    NewType->bTemporalCompression = FALSE;
    NewType->formattype = FORMAT_VIDEOINFO2;
    NewType->cbFormat = sizeof(VIDEOINFOHEADER2);
    VIDEOINFOHEADER2* NewFormat = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
    if(NewFormat == NULL)
    {
        ClearMediaType(NewType);
        return E_OUTOFMEMORY;
    }
    ZeroMemory(NewFormat, sizeof(VIDEOINFOHEADER2));
    NewType->pbFormat = (BYTE*)NewFormat;

    NewFormat->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;

    if(InputType->formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* OldFormat = (VIDEOINFOHEADER2*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;
        NewFormat->dwPictAspectRatioX = OldFormat->dwPictAspectRatioX;
        NewFormat->dwPictAspectRatioY = OldFormat->dwPictAspectRatioY;
        NewFormat->dwBitRate = OldFormat->dwBitRate;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame;
        if(OldFormat->dwInterlaceFlags & AMINTERLACE_Field1First)
        {
            NewFormat->dwInterlaceFlags |= AMINTERLACE_Field1First;
        }
		else if(OldFormat->dwInterlaceFlags & AMINTERLACE_DisplayModeWeaveOnly && BitmapInfo->biHeight == 576)
        {
            NewFormat->dwInterlaceFlags |= AMINTERLACE_Field1First;
        }

    }
    else if(InputType->formattype == FORMAT_VideoInfo)
    {
        NewType->pbFormat =(BYTE*)NewFormat;
        VIDEOINFOHEADER* OldFormat = (VIDEOINFOHEADER*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;

        // if the input format is a known TV style one then
        // it should be assumed to be 4:3
        if((BitmapInfo->biWidth == 704)  &&
           (BitmapInfo->biHeight == 480 || BitmapInfo->biHeight == 576))
        {
            // adjustment to make 704 video the same shape as 720
            NewFormat->dwPictAspectRatioX = 4 * 44;
            NewFormat->dwPictAspectRatioY = 3 * 45;
        }
        else if((BitmapInfo->biWidth == 704 || BitmapInfo->biWidth == 720 || BitmapInfo->biWidth == 768) &&
            (BitmapInfo->biHeight == 480 || BitmapInfo->biHeight == 576))
        {
            NewFormat->dwPictAspectRatioX = 4;
            NewFormat->dwPictAspectRatioY = 3;
        }
        else
        {
            // first guess is square pixels
            NewFormat->dwPictAspectRatioX = BitmapInfo->biWidth;
            NewFormat->dwPictAspectRatioY = BitmapInfo->biHeight;

            // the update the aspect ratio with the pels per meter info if both
            // are present, this was the old way of handling aspect ratio
            if(BitmapInfo->biXPelsPerMeter > 0 && BitmapInfo->biYPelsPerMeter > 0)
            {
                NewFormat->dwPictAspectRatioX *= BitmapInfo->biYPelsPerMeter;
                NewFormat->dwPictAspectRatioY *= BitmapInfo->biXPelsPerMeter;
            }
        }

        if(BitmapInfo->biHeight == 576)
        {
            NewFormat->dwInterlaceFlags |= AMINTERLACE_Field1First;
        }

        NewFormat->dwBitRate = OldFormat->dwBitRate;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame;
    }
    else
    {
        ClearMediaType(NewType);
    }

    if(m_Filter->GetParamBool(CDScaler::INPUTISANAMORPHIC) != 0)
    {
        NewFormat->dwPictAspectRatioX *= 4;
        NewFormat->dwPictAspectRatioY *= 3;
    }

	memcpy(&NewFormat->bmiHeader, BitmapInfo, sizeof(BITMAPINFOHEADER));

    NewType->lSampleSize = BitmapInfo->biSizeImage;
    return S_OK;
}
