///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.cpp,v 1.2 2004-03-06 20:51:10 adcockj Exp $
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
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.30  2003/12/09 11:45:57  adcockj
// Improved implementation of EnumPins
//
// Revision 1.29  2003/11/13 21:39:50  adcockj
// fix for diag to work properly
//
// Revision 1.28  2003/11/13 17:27:44  adcockj
// Minor improvements
//
// Revision 1.27  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
// Revision 1.26  2003/09/30 16:59:26  adcockj
// Improved handling of small format changes
//
// Revision 1.25  2003/09/28 15:08:07  adcockj
// optimization fix and minor changes
//
// Revision 1.24  2003/09/24 16:33:00  adcockj
// Bug fixes - starting to work now..
//
// Revision 1.23  2003/09/19 16:12:14  adcockj
// Further improvements
//
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
	m_ExpectedStartIn = 0;
	m_LastStartEnd = 0;
	m_Counter = 0;
    m_FieldsInBuffer = 0;
    m_NextFieldNumber = 0;
    m_SourceType = SOURCE_DEFAULT;
    m_FieldTiming = 0;
    m_DetectedPulldownIndex = 0;
    m_DetectedPulldownType = FULLRATEVIDEO;
}

CInputPin::~CInputPin()
{
    ClearAll();
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
	
    // find out who the pin belongs to
    // we will process differently depending on 
    // the filter at the other end
    // if this fumction returns TRUE we are talking to 
    // ourself then error
	if(WorkOutWhoWeAreTalkingTo(pConnector) == TRUE)
	{
        return VFW_E_TYPE_NOT_ACCEPTED;
	}

	FixupMediaType((AM_MEDIA_TYPE *)pmt);

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
			hr = InternalProcessOutput(FALSE);
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
		m_ExpectedStartIn = tStart;
		m_LastStartEnd = tStart;
        hr = m_OutputPin->m_ConnectedPin->NewSegment(tStart, tStop, dRate);
        CHECK(hr);
        m_FieldTiming = 0;
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
    
    return InternalReceive(InSample);
}

HRESULT CInputPin::InternalReceive(IMediaSample *InSample)
{
    HRESULT hr = S_OK;
    AM_SAMPLE2_PROPERTIES InSampleProperties;
    ZeroMemory(&InSampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));
    IMediaSample* OutSample = NULL;

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
			FixupMediaType(InSampleProperties.pMediaType);

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

        // reconfigure the filters if the input format has changed
        hr = m_Filter->CheckProcessingLine();
        CHECK(hr);
        

        // if there was a discontinuity then we need to ask for the buffer
        // differently 
        if(InSampleProperties.dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
		{
			if(m_ExpectedStartIn != 0)
			{
				LOG(DBGLOG_FLOW, ("Discontinuity\n"));
				hr = ClearAll();
				CHECK(hr);
				m_ExpectedStartIn = 0;
			}
			else
			{
				LOG(DBGLOG_FLOW, ("Ignored First Discontinuity\n"));
			}
			m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
		}
		// if we get a later sample then flush the buffers
		else if(m_ExpectedStartIn != 0 && m_ExpectedStartIn + 100000 < InSampleProperties.tStart)
        {
			LOG(DBGLOG_FLOW, ("Skipped Frames expected %d got %d\n", (long)m_ExpectedStartIn, (long)InSampleProperties.tStart));

			GuessInterlaceFlags(&InSampleProperties);
			hr = ClearAll();
			CHECK(hr);

            m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
        }

		//LOG(DBGLOG_FLOW, ("Incoming expected %d Start %d end %d addr %08x\n", (long)m_ExpectedStartIn, (long)InSampleProperties.tStart, (long)InSampleProperties.tStop, InSampleProperties.dwTypeSpecificFlags));
		
		m_ExpectedStartIn = InSampleProperties.tStop;

        hr = PushSample(InSample, &InSampleProperties);
        CHECK(hr);

		hr = InternalProcessOutput(FALSE);

		if(FAILED(hr) || hr == S_FALSE)
		{
			hr = ClearAll();
			CHECK(hr);

            m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
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
		//LOG(DBGLOG_FLOW, ("Frame Skipped\n"));
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

HRESULT CInputPin::InternalProcessOutput(BOOL HurryUp)
{
    REFERENCE_TIME FrameEndTime = 0;
	HRESULT hr = S_OK;
	
	while(hr == S_OK && m_FieldsInBuffer > m_Filter->m_NumberOfFieldsToBuffer)
	{
        switch(WorkOutHowToProcess(FrameEndTime))
        {
        case PROCESS_IGNORE:
            break;
        case PROCESS_WEAVE:
            hr = WeaveOutput(FrameEndTime);
            break;
        case PROCESS_DEINTERLACE:
            hr = DeinterlaceOutput(FrameEndTime);
            break;
        }

        PopStack();
	}
	return hr;
}

HRESULT CInputPin::WeaveOutput(REFERENCE_TIME& FrameEndTime)
{
	HRESULT hr = S_OK;
    IMediaSample* OutSample = NULL;

    hr = GetOutputSample(&OutSample);
    if(OutSample == NULL)
    {
        return S_FALSE;
    }
    CHECK(hr);


	IMediaBuffer* OutBuffer = CMediaBufferWrapper::CreateBuffer(OutSample);

	DWORD Status(0);

    hr = Weave((IInterlacedBufferStack*)this, OutBuffer);

	if(hr == S_OK && m_Flushing == FALSE)
	{
        // just in case things go a bit funny
        if(m_LastStartEnd >= FrameEndTime)
        {
            m_LastStartEnd = FrameEndTime - 1;
        }
		hr = OutSample->SetTime(&m_LastStartEnd, &FrameEndTime);
		CHECK(hr);
        
        //LOG(DBGLOG_FLOW, ("Output Start %d end %d\n", (long)m_LastStartEnd, (long)FrameEndTime));
		
        // finally send the processed sample on it's way
		hr = m_OutputPin->m_MemInputPin->Receive(OutSample);
	}
	// if we are at the start then we might need to send 
	// a couple of frames in before getting a response
	else if(hr == S_FALSE)
	{
		LOG(DBGLOG_FLOW, ("Skipped\n"));
		hr = S_OK;
	}

	OutBuffer->Release();
	OutSample->Release();

    m_LastStartEnd = FrameEndTime;

    return hr;
}


HRESULT CInputPin::DeinterlaceOutput(REFERENCE_TIME& FrameEndTime)
{
	HRESULT hr = S_OK;
    IMediaSample* OutSample = NULL;

    hr = GetOutputSample(&OutSample);
    if(OutSample == NULL)
    {
        return S_FALSE;
    }
    CHECK(hr);


	IMediaBuffer* OutBuffer = CMediaBufferWrapper::CreateBuffer(OutSample);

	DWORD Status(0);

    hr = m_Filter->m_CurrentDeinterlacingMethod->Process((IInterlacedBufferStack*)this, OutBuffer);

	if(hr == S_OK && m_Flushing == FALSE)
	{
		hr = OutSample->SetTime(&m_LastStartEnd, &FrameEndTime);
		CHECK(hr);
		
        // finally send the processed sample on it's way
		hr = m_OutputPin->m_MemInputPin->Receive(OutSample);
	}
	// if we are at the start then we might need to send 
	// a couple of frames in before getting a response
	else if(hr == S_FALSE)
	{
		LOG(DBGLOG_FLOW, ("Skipped\n"));
		hr = S_OK;
	}

	OutBuffer->Release();
	OutSample->Release();

    m_LastStartEnd = FrameEndTime;

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

HRESULT CInputPin::GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type)
{   
    HRESULT hr = S_OK;
    if(m_ConnectedPin != NULL)
    {
        if(TypeNum == 0)
        {
            hr = CopyMediaType(Type, &m_InputMediaType);
            CHECK(hr);
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        hr = S_FALSE;
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

void CInputPin::FixupMediaType(AM_MEDIA_TYPE *pmt)
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
	ClearAll();
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

class DECLSPEC_UUID("F50B3F13-19C4-11CF-AA9A-02608C9BABA2") ElecardVideoDecoder;

BOOL CInputPin::WorkOutWhoWeAreTalkingTo(IPin* pConnector)
{
	PIN_INFO PinInfo;
	BOOL RetVal = TRUE;

    m_SourceType = SOURCE_DEFAULT;

	HRESULT hr = pConnector->QueryPinInfo(&PinInfo);
	if(SUCCEEDED(hr))
	{
		CLSID ClassId;
		hr = PinInfo.pFilter->GetClassID(&ClassId);
		if(SUCCEEDED(hr))
		{
			RetVal = FALSE;
		}
		if(PinInfo.pFilter != NULL)
		{
			PinInfo.pFilter->Release();
		}
	}

	return RetVal;
}

HRESULT CInputPin::PushSample(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    HRESULT hr = S_OK;
    switch(m_SourceType)
    {
    case SOURCE_ELECARD:
        hr = PushSampleElecard(InputSample, InSampleProperties);
        break;
    case SOURCE_SONIC:
        hr = PushSampleSonic(InputSample, InSampleProperties);
        break;
    case SOURCE_NVDVD:
        hr = PushSampleNVDVD(InputSample, InSampleProperties);
        break;
    case SOURCE_WINDVD:
        hr = PushSampleWinDVD(InputSample, InSampleProperties);
        break;
    case SOURCE_DV:
        hr = PushSampleDefault(InputSample, InSampleProperties);
        break;
    case SOURCE_DEFAULT:
    default:
        hr = PushSampleDefault(InputSample, InSampleProperties);
        break;
    }
    return hr;
}

void CInputPin::ShiftUpSamples(int NumberToShift, IMediaSample* InputSample)
{
    if(m_FieldsInBuffer > 0)
    {
        for(int i(m_FieldsInBuffer - 1); i >= 0; --i)
		{
			m_IncomingFields[i + NumberToShift] = m_IncomingFields[i];
		}
    }
    m_FieldsInBuffer += NumberToShift;
    while(NumberToShift--)
    {
        m_IncomingFields[NumberToShift].Clear();
        m_IncomingFields[NumberToShift].m_Sample = InputSample;
        m_IncomingFields[NumberToShift].m_FieldNumber = m_NextFieldNumber++;
    }
}

HRESULT CInputPin::PushSampleDefault(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    ShiftUpSamples(2, InputSample);

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMediaType.pbFormat);
    if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
    {
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    else
    {
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    return S_OK;
}

HRESULT CInputPin::PushSampleElecard(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    if(m_FieldTiming > 0 && (InSampleProperties->tStop - InSampleProperties->tStart) > m_FieldTiming * 4)
    {
        ClearAll();
    }

    switch(InSampleProperties->dwTypeSpecificFlags & (AM_VIDEO_FLAG_FIELD1FIRST | AM_VIDEO_FLAG_REPEAT_FIELD))
    {
    // 2 fields field 2 first
    case 0:
        ShiftUpSamples(2, InputSample);
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
        SupplyHint(PULLDOWN_32, 1, FALSE);
        break;
    // 2 fields field 2 first
    case AM_VIDEO_FLAG_FIELD1FIRST:
        ShiftUpSamples(2, InputSample);
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
        SupplyHint(PULLDOWN_32, 1, FALSE);
        break;
    // 3 fields field 2 first
    case AM_VIDEO_FLAG_REPEAT_FIELD:
        ShiftUpSamples(3, InputSample);
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + 2 * (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
        m_IncomingFields[2].m_IsTopLine = FALSE;
        m_IncomingFields[2].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
        SupplyHint(PULLDOWN_32, 4, TRUE);
        break;
    // 3 fields field 1 first
    case (AM_VIDEO_FLAG_REPEAT_FIELD | AM_VIDEO_FLAG_FIELD1FIRST):
        ShiftUpSamples(3, InputSample);
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + 2 * (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
        m_IncomingFields[2].m_IsTopLine = TRUE;
        m_IncomingFields[2].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
        SupplyHint(PULLDOWN_32, 4, TRUE);
        break;
    }

    if(m_FieldTiming == 0)
    {
        m_FieldTiming = (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    return S_OK;
}

HRESULT CInputPin::PushSampleSonic(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    ShiftUpSamples(2, InputSample);

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMediaType.pbFormat);
    if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
    {
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    else
    {
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    // \todo work out best way of getting this 
    if(m_FieldTiming == 0)
    {
        m_FieldTiming = (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    return S_OK;
}

HRESULT CInputPin::PushSampleWinDVD(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    ShiftUpSamples(2, InputSample);

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMediaType.pbFormat);
    if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
    {
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    else
    {
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    // \todo work out best way of getting this 
    if(m_FieldTiming == 0)
    {
        m_FieldTiming = (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    return S_OK;
}

HRESULT CInputPin::PushSampleNVDVD(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    ShiftUpSamples(2, InputSample);

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMediaType.pbFormat);
    if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
    {
        m_IncomingFields[0].m_IsTopLine = FALSE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = TRUE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }
    else
    {
        m_IncomingFields[0].m_IsTopLine = TRUE;
        m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
        m_IncomingFields[1].m_IsTopLine = FALSE;
        m_IncomingFields[1].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    // \todo work out best way of getting this 
    if(m_FieldTiming == 0)
    {
        m_FieldTiming = (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
    }

    return S_OK;
}

STDMETHODIMP CInputPin::get_NumFields(DWORD* Count)
{
    *Count = m_FieldsInBuffer;
    return S_OK;
}

STDMETHODIMP CInputPin::GetField(DWORD Index, IInterlacedField** Field)
{
    if(*Field != NULL)
    {
        (*Field)->Release();
    }
    if(Index < m_FieldsInBuffer)
    {
        *Field = &(m_IncomingFields[m_FieldsInBuffer - Index - 1]);
        (*Field)->AddRef();
    }
    else
    {
        *Field = NULL;
    }
    return S_OK;
}

STDMETHODIMP CInputPin::PopStack()
{
    ATLASSERT(m_FieldsInBuffer > 0);
    m_IncomingFields[m_FieldsInBuffer - 1].Clear();
    --m_FieldsInBuffer;
    return S_OK;
}

STDMETHODIMP CInputPin::ClearAll()
{
    while(m_FieldsInBuffer)
    {
        PopStack();
    }
    m_NextFieldNumber = 0;
    m_DetectedPulldownIndex = 0;
    m_DetectedPulldownType = FULLRATEVIDEO;
    return S_OK;
}

STDMETHODIMP CInputPin::CField::GetBufferAndLength(BYTE** ppBuffer, DWORD* pcbLength)
{
    *pcbLength = m_Sample->GetActualDataLength();
    return  m_Sample->GetPointer(ppBuffer);
}

STDMETHODIMP CInputPin::CField::GetMaxLength(DWORD* pcbMaxLength)
{
    *pcbMaxLength = m_Sample->GetSize();
    return S_OK;
}

STDMETHODIMP CInputPin::CField::SetLength(DWORD cbLength)
{
    return m_Sample->SetActualDataLength(cbLength);
}

STDMETHODIMP CInputPin::CField::get_TopFieldFirst(BOOLEAN* TopFieldFirst)
{
    if(m_IsTopLine)
    {
        *TopFieldFirst = TRUE;
    }
    else
    {
        *TopFieldFirst = FALSE;
    }
    return S_OK;
}

STDMETHODIMP CInputPin::CField::get_Hint(eDetectionHint* HintValue)
{
    *HintValue = m_Hint;
    return S_OK;
}

CInputPin::eHowToProcess CInputPin::WorkOutHowToProcess(REFERENCE_TIME& FrameEndTime)
{
    eHowToProcess HowToProcess = PROCESS_DEINTERLACE;

    //\todo make delay adjustable
    DWORD Delay = 1;

    if(m_FieldsInBuffer > Delay)
    {
        int FrameNum = m_IncomingFields[m_FieldsInBuffer - Delay - 1].m_FieldNumber;
        FrameEndTime = m_IncomingFields[m_FieldsInBuffer - Delay - 1].m_EndTime;

        DWORD Index;
        eDeinterlaceType CurrentType;

        if(m_Filter->GetParamBool(CDScaler::MANUALPULLDOWN) == FALSE)
        {
            // use the detected mode to deinterlace
            CurrentType = m_DetectedPulldownType;
            Index = m_DetectedPulldownIndex;
        }
        else
        {
            // force the manually selected mode
            CurrentType = (eDeinterlaceType)m_Filter->GetParamEnum(CDScaler::PULLDOWNMODE);
            Index = m_Filter->GetParamInt(CDScaler::PULLDOWNINDEX);
        }

        switch(m_Filter->GetParamEnum(CDScaler::PULLDOWNMODE))
        {
        case PULLDOWN_32:
            switch((FrameNum + Index) % 5)
            {
            case 1:
                HowToProcess = PROCESS_WEAVE;
                FrameEndTime += m_FieldTiming / 2;
                break;
            case 4:
                HowToProcess = PROCESS_WEAVE;
                break;
            default:
                HowToProcess = PROCESS_IGNORE;
                break;
            }
            break;
        case PULLDOWN_22:
            if(((FrameNum + Index) & 1) == 1)
            {
                HowToProcess = PROCESS_WEAVE;
            }
            else
            {
                HowToProcess = PROCESS_IGNORE;
            }
            break;
        case HALFRATEVIDEO:
            if((FrameNum & 1) == 0)
            {
                HowToProcess = PROCESS_DEINTERLACE;
            }
            else
            {
                HowToProcess = PROCESS_IGNORE;
            }
            break;
        default:
        case FULLRATEVIDEO:
            break;
        }
    }
    else
    {
        HowToProcess = PROCESS_IGNORE;
    }

    return HowToProcess;
}

void CInputPin::SupplyHint(eDeinterlaceType HintMode, DWORD HintIndex, BOOL StrongHint)
{
    switch(HintMode)
    {
    case PULLDOWN_32:
        if(StrongHint == FALSE && HintMode != m_DetectedPulldownType)
        {
            return;
    }
    else
    {
            if(StrongHint == TRUE && HintMode != m_DetectedPulldownType)
            {
                m_DetectedPulldownType = HintMode;
                m_DetectedPulldownIndex = (m_NextFieldNumber - HintIndex + 9) % 5;
            }
            else if(m_DetectedPulldownIndex != ((m_NextFieldNumber - HintIndex + 9) % 5))
            {
                m_DetectedPulldownType = FULLRATEVIDEO;
            }
        }
    }
}
