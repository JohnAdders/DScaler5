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

#include "stdafx.h"
#include "DSBasePin.h"
#include "DSBufferedInputPin.h"
#include "DSOutputPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"

CDSBufferedInputPin::CDSBufferedInputPin() :
    CDSInputPin()
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::CDSBufferedInputPin\n"));

    m_SamplesReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_ThreadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_WorkerThread = NULL;
    m_ThreadRetCode = S_OK;

}

CDSBufferedInputPin::~CDSBufferedInputPin()
{
    CloseHandle(m_SamplesReadyEvent);
    CloseHandle(m_ThreadStopEvent);

    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::~CDSBufferedInputPin\n"));
}


STDMETHODIMP CDSBufferedInputPin::BeginFlush(void)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::BeginFlush\n"));

    HRESULT hr = CDSInputPin::BeginFlush();

    CProtectCode WhileVarInScope2(&m_SamplesLock);

    while(!m_Samples.empty())
    {
        m_Samples.pop();
    }

    return hr;
}

STDMETHODIMP CDSBufferedInputPin::EndFlush(void)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::EndFlush\n"));

    // make sure we are not in ProcessBufferedSamples
    CProtectCode WhileVarInScope(&m_WorkerThreadLock);

    HRESULT hr = CDSInputPin::EndFlush();

    return hr;
}

STDMETHODIMP CDSBufferedInputPin::EndOfStream(void)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::EndOfStream\n"));

    // all code below here is protected
    // from runnning at the same time as other
    // functions with this line
    CProtectCode WhileVarInScope(this);

    CProtectCode WhileVarInScope2(&m_SamplesLock);

    m_Samples.push((IMediaSample*)NULL);

    SetEvent(m_SamplesReadyEvent);

    return S_OK;
}

STDMETHODIMP CDSBufferedInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::NewSegment\n"));

    // make sure we are not in ProcessBufferedSamples
    CProtectCode WhileVarInScope(&m_WorkerThreadLock);

    return CDSInputPin::NewSegment(tStart, tStop, dRate);
}


STDMETHODIMP CDSBufferedInputPin::Receive(IMediaSample *InSample)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::Receive\n"));

    if(InSample == NULL)
    {
        return E_POINTER;
    }
    if(m_Flushing == TRUE)
    {
        LOG(DBGLOG_FLOW, ("CDSBufferedInputPin::Receive flushing\n"));
        return S_FALSE;
    }
    if(m_Filter->m_State == State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }

    HRESULT hr = S_OK;

    // all code below here is protected
    // from runnning at the same time as other
    // functions with this line
    CProtectCode WhileVarInScope(this);

    CProtectCode WhileVarInScope2(&m_SamplesLock);

    if(m_ThreadRetCode != S_OK)
    {
        LOG(DBGLOG_FLOW, ("CDSBufferedInputPin::Receive error %08x\n", m_ThreadRetCode));
        return m_ThreadRetCode;
    }

    m_Samples.push(InSample);

    SetEvent(m_SamplesReadyEvent);

    return hr;
}

void CDSBufferedInputPin::ProcessingThread(void* pParam)
{
    CDSBufferedInputPin* pThis = (CDSBufferedInputPin*)pParam;

    //SetThreadPriority(pThis->m_WorkerThread, THREAD_PRIORITY_HIGHEST);

    while(1)
    {
        HRESULT hr = S_OK;
        DWORD dwWaitResult;
        HANDLE hEvents[2];

        hEvents[0] = pThis->m_ThreadStopEvent;  // thread's read event
        hEvents[1] = pThis->m_SamplesReadyEvent;

        dwWaitResult = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);

        switch (dwWaitResult)
        {
            case WAIT_OBJECT_0:
                {
                    CProtectCode WhileVarInScope2(&pThis->m_SamplesLock);
                    pThis->m_ThreadRetCode = S_FALSE;
                    while(!pThis->m_Samples.empty())
                    {
                        pThis->m_Samples.pop();
                    }
                }
                LOG(DBGLOG_FLOW, ("ProcessingThread Exit by event\n"));
                ExitThread(0);
                break;
            case WAIT_OBJECT_0 + 1:
                hr = pThis->ProcessBufferedSamples();
                if(FAILED(hr))
                {
                    if(hr == VFW_E_NOT_COMMITTED)
                    {
                        {
                            CProtectCode WhileVarInScope2(&pThis->m_SamplesLock);
                            pThis->m_ThreadRetCode = hr;
                        }
                        ExitThread(0);
                    }
                }
                break;
            default:
                {
                    CProtectCode WhileVarInScope2(&pThis->m_SamplesLock);
                    pThis->m_ThreadRetCode = E_UNEXPECTED;
                }
                LOG(DBGLOG_FLOW, ("ProcessingThread Exit by unexpected\n"));
                ExitThread(0);
                break;
        }
    }
}

HRESULT CDSBufferedInputPin::ProcessBufferedSamples()
{
    size_t size = 1;
    HRESULT hr = S_OK;

    CProtectCode WhileVarInScope(&m_WorkerThreadLock);
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::ProcessBufferedSamples\n"));

    while(size > 0 && SUCCEEDED(hr))
    {
        SI(IMediaSample) InSample;

        {
            CProtectCode WhileVarInScope(&m_SamplesLock);
            size = m_Samples.size();
            if(size > 0)
            {
                InSample = m_Samples.front();
                m_Samples.pop();
            }
        }

        if(InSample)
        {
            hr = ProcessBufferedSample(InSample.GetNonAddRefedInterface());
        }
        else
        {
            hr = CDSInputPin::EndOfStream();
        }

        if(WaitForSingleObject(m_ThreadStopEvent, 0) != WAIT_TIMEOUT)
        {
            hr = E_UNEXPECTED;
        }

        {
            CProtectCode WhileVarInScope(&m_SamplesLock);
            size = m_Samples.size();
            if(size == 0)
            {
                ResetEvent(m_SamplesReadyEvent);
                return hr;

            }
        }
    }

    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        CProtectCode WhileVarInScope(&m_SamplesLock);
        while(!m_Samples.empty())
        {
            m_Samples.pop();
        }
        ResetEvent(m_SamplesReadyEvent);
    }

    return hr;
}

HRESULT CDSBufferedInputPin::ProcessBufferedSample(IMediaSample* InSample)
{
    if(m_Flushing == TRUE)
    {
        return S_FALSE;
    }

    if(m_Filter->m_State == State_Stopped)
    {
        return VFW_E_WRONG_STATE;
    }


    AM_SAMPLE2_PROPERTIES InSampleProperties;
    ZeroMemory(&InSampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));

    //LogSample(InSample, "New Input Sample");

    HRESULT hr = GetSampleProperties(InSample, &InSampleProperties);
    CHECK(hr);

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


STDMETHODIMP CDSBufferedInputPin::ReceiveCanBlock(void)
{
    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::ReceiveCanBlock\n"));
    return S_FALSE;
}

HRESULT CDSBufferedInputPin::Activate()
{
    HRESULT hr = CDSInputPin::Activate();
    CHECK(hr);

    LOG(DBGLOG_ALL, ("CDSBufferedInputPin::Activate\n"));

    ResetEvent(m_ThreadStopEvent);
    ResetEvent(m_SamplesReadyEvent);
    m_ThreadRetCode = S_OK;

    m_WorkerThread = CreateThread(NULL, 0,
        (LPTHREAD_START_ROUTINE) ProcessingThread,
        this,  // pass event handle
        0, &m_ThreadId);

    return hr;
}

HRESULT CDSBufferedInputPin::Deactivate()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::Deactivate\n"));

    SetEvent(m_ThreadStopEvent);

    HRESULT hr = CDSInputPin::Deactivate();
    CHECK(hr);

    DWORD dwWaitResult = WaitForSingleObject(m_WorkerThread, 100);

    if(dwWaitResult == WAIT_TIMEOUT)
    {
        TerminateThread(m_WorkerThread, 0);
    }

    CloseHandle(m_WorkerThread);
    m_WorkerThread = NULL;

    return hr;
}

