///////////////////////////////////////////////////////////////////////////////
// $Id: DSOutputPin.cpp,v 1.12 2004-05-06 06:38:07 adcockj Exp $
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
// Revision 1.11  2004/04/29 16:16:46  adcockj
// Yet more reconnection fixes
//
// Revision 1.10  2004/04/20 16:30:31  adcockj
// Improved Dynamic Connections
//
// Revision 1.9  2004/04/16 16:19:44  adcockj
// Better reconnection and improved AFD support
//
// Revision 1.8  2004/03/08 17:20:05  adcockj
// Minor bug fixes
//
// Revision 1.7  2004/03/05 15:56:29  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.6  2004/02/29 13:47:49  adcockj
// Format change fixes
// Minor library updates
//
// Revision 1.5  2004/02/27 17:08:16  adcockj
// Improved locking at state changes
// Better error handling at state changes
//
// Revision 1.4  2004/02/25 17:14:03  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/16 17:25:02  adcockj
// Fix build errors, locking problems and DVD compatability
//
// Revision 1.2  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSOutputPin.h"
#include "DSInputPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"

CDSOutputPin::CDSOutputPin() :
    CDSBasePin(PINDIR_OUTPUT)
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::CDSOutputPin\n"));
}

CDSOutputPin::~CDSOutputPin()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::~CDSOutputPin\n"));
}

STDMETHODIMP CDSOutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("CDSOutputPin::Connect\n"));
    HRESULT hr;

    if(m_ConnectedPin != NULL)
    {
        return VFW_E_ALREADY_CONNECTED;
    }

    // check that we can pass stuff to this pin
    m_MemInputPin = pReceivePin;
    if(m_MemInputPin == NULL)
    {
        return VFW_E_NO_TRANSPORT;
    }

    AM_MEDIA_TYPE ProposedType;
    InitMediaType(&ProposedType);

    int TypeNum = 0;
    while(1)
    {
        // find out which type we really want to send out
        hr = m_Filter->CreateSuitableMediaType(&ProposedType, this, TypeNum);
        if(hr == VFW_E_NOT_CONNECTED)
        {
            ClearMediaType(&ProposedType);
            return VFW_E_NO_ACCEPTABLE_TYPES;
        } 

        // we got to the end of the list
        if(hr == VFW_S_NO_MORE_ITEMS)
        {
            ClearMediaType(&ProposedType);
            return VFW_E_NO_ACCEPTABLE_TYPES;
        }

        CHECK(hr);

        // check that any format we've been passed in is OK
        // otherwise just go to next type
        if(pmt != NULL)
        {
            if((pmt->majortype != GUID_NULL && pmt->majortype != ProposedType.majortype) ||
                (pmt->subtype != GUID_NULL && pmt->subtype != ProposedType.subtype) || 
                (pmt->formattype != GUID_NULL && pmt->formattype != ProposedType.formattype))
            {
                ++TypeNum;
                continue;
            }
        }

        // See if the format we want is supported
        hr = pReceivePin->ReceiveConnection(this, &ProposedType);
        if(SUCCEEDED(hr))
        {
            break;
        }
        
        ++TypeNum;
        ClearMediaType(&ProposedType);
    }
    
    hr = NegotiateAllocator(pReceivePin, &ProposedType);    
    CHECK(hr);

    ClearMediaType(&ProposedType);

	hr = m_Filter->NotifyConnected(this);

    if(FAILED(hr))
    {
        InternalDisconnect();
    }

    return hr;
}

HRESULT CDSOutputPin::SendSample(IMediaSample* OutSample)
{
    if(!m_MemInputPin) return VFW_E_NOT_CONNECTED;
    return m_MemInputPin->Receive(OutSample);
}

STDMETHODIMP CDSOutputPin::ReceiveConnection(IPin *pConnector, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::ReceiveConnection\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP CDSOutputPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::QueryAccept\n"));
    LogMediaType(pmt, "CDSOutputPin::QueryAccept", DBGLOG_FLOW);

    if(m_Filter->IsThisATypeWeCanWorkWith(pmt, this) == false)
    {
        return S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP CDSOutputPin::Disconnect(void)
{
	HRESULT hr = CDSBasePin::Disconnect();
	CHECK(hr);
	m_MemInputPin.Detach();
	m_PinConnection.Detach();
	return hr;
}


STDMETHODIMP CDSOutputPin::EndOfStream(void)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::EndOfStream\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP CDSOutputPin::BeginFlush(void)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::BeginFlush\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP CDSOutputPin::EndFlush(void)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::EndFlush\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP CDSOutputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::NewSegment\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP CDSOutputPin::Notify(IBaseFilter *pSelf, Quality q)
{
    return m_Filter->Notify(pSelf, q, this);
}

STDMETHODIMP CDSOutputPin::SetSink(IQualityControl *piqc)
{
    LOG(DBGLOG_ALL, ("*Unexpected Call* - CDSOutputPin::SetSink\n"));
    return E_NOTIMPL;
}

STDMETHODIMP CDSOutputPin::Block(DWORD dwBlockFlags, HANDLE hEvent)
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::Block\n"));
    CDSBasePin* pPin = m_Filter->GetPin(0);
    if(pPin->m_Direction == PINDIR_INPUT)
    {
        CDSInputPin* pInPin = (CDSInputPin*)pPin;
        return pInPin->Block(dwBlockFlags, hEvent);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

void CDSOutputPin::InternalDisconnect()
{
    m_PinConnection.Detach();
    m_MemInputPin.Detach();
}

// we just pass all seeking calls downsteam
// if they accept but then we fall back to asking the filter
// for support
#define GETMEDIASEEKING \
    CDSBasePin* pPin = m_Filter->GetPin(0); \
    SI(IMediaSeeking) MediaSeeking; \
    if(pPin->m_Direction == PINDIR_INPUT) \
        MediaSeeking = pPin->m_ConnectedPin; \
    if(!MediaSeeking) MediaSeeking = (IBaseFilter*)m_Filter; \
    if(!MediaSeeking) return E_NOTIMPL;

STDMETHODIMP CDSOutputPin::GetCapabilities(DWORD *pCapabilities)
{
    GETMEDIASEEKING
    return MediaSeeking->GetCapabilities(pCapabilities);
}

STDMETHODIMP CDSOutputPin::CheckCapabilities(DWORD *pCapabilities)
{
    GETMEDIASEEKING
    return MediaSeeking->CheckCapabilities(pCapabilities);
}

STDMETHODIMP CDSOutputPin::IsFormatSupported(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->IsFormatSupported(pFormat);
}

STDMETHODIMP CDSOutputPin::QueryPreferredFormat(GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->QueryPreferredFormat(pFormat);
}

STDMETHODIMP CDSOutputPin::GetTimeFormat(GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->GetTimeFormat(pFormat);
}

STDMETHODIMP CDSOutputPin::IsUsingTimeFormat(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->IsUsingTimeFormat(pFormat);
}

STDMETHODIMP CDSOutputPin::SetTimeFormat(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->SetTimeFormat(pFormat);
}

STDMETHODIMP CDSOutputPin::GetDuration(LONGLONG *pDuration)
{
    GETMEDIASEEKING
    return MediaSeeking->GetDuration(pDuration);
}

STDMETHODIMP CDSOutputPin::GetStopPosition(LONGLONG *pStop)
{
    GETMEDIASEEKING
    return MediaSeeking->GetStopPosition(pStop);
}

STDMETHODIMP CDSOutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    GETMEDIASEEKING
    return MediaSeeking->GetCurrentPosition(pCurrent);
}

STDMETHODIMP CDSOutputPin::ConvertTimeFormat(
                                LONGLONG *pTarget,
                                const GUID *pTargetFormat,
                                LONGLONG Source,
                                const GUID *pSourceFormat
                            )
{
    GETMEDIASEEKING
    return MediaSeeking->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);
}

STDMETHODIMP CDSOutputPin::SetPositions( 
                        LONGLONG *pCurrent,
                        DWORD dwCurrentFlags,
                        LONGLONG *pStop,
                        DWORD dwStopFlags
                       )
{
    GETMEDIASEEKING
    return MediaSeeking->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

STDMETHODIMP CDSOutputPin::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
    GETMEDIASEEKING
    return MediaSeeking->GetPositions(pCurrent, pStop);
}

STDMETHODIMP CDSOutputPin::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    GETMEDIASEEKING
    return MediaSeeking->GetAvailable(pEarliest, pLatest);
}

STDMETHODIMP CDSOutputPin::SetRate(double dRate)
{
    GETMEDIASEEKING
    return MediaSeeking->SetRate(dRate);
}

STDMETHODIMP CDSOutputPin::GetRate(double *pdRate)
{
    GETMEDIASEEKING
    return MediaSeeking->GetRate(pdRate);
}

STDMETHODIMP CDSOutputPin::GetPreroll(LONGLONG *pllPreroll)
{
    GETMEDIASEEKING
    return MediaSeeking->GetPreroll(pllPreroll);
}


HRESULT CDSOutputPin::GetOutputSample(IMediaSample** OutSample, REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop, bool PrevFrameSkipped)
{
    *OutSample = NULL;

    DWORD dwFlags = PrevFrameSkipped ? AM_GBF_PREVFRAMESKIPPED : 0;

	// get a sample to output to
	HRESULT hr = m_Allocator->GetBuffer(OutSample, rtStart, rtStop, dwFlags);
	if(FAILED(hr))
	{
		return hr;
	}

	if(*OutSample == NULL)
	{
		return VFW_E_WRONG_STATE;
	}

	// check for media type changes on the output side
	// a NULL means the type is the same as last time
    AM_MEDIA_TYPE* pMediaType = NULL;

    hr = (*OutSample)->GetMediaType(&pMediaType);

	CHECK(hr);

	if(hr == S_OK && pMediaType != NULL)
	{
	    LOG(DBGLOG_ALL, ("Got new media type from renderer\n"));

		hr = SetType(pMediaType);
		CHECK(hr);

        FreeMediaType(pMediaType);
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}

HRESULT CDSOutputPin::Activate()
{
    HRESULT hr = S_OK;
    if(m_Allocator != NULL)
    {
        hr = m_Allocator->Commit();
		if(hr == E_OUTOFMEMORY)
		{
		    LOG(DBGLOG_FLOW, ("Out of memory - will try and reduce buffer count\n"));

			ALLOCATOR_PROPERTIES Properties;
			ZeroMemory(&Properties, sizeof(ALLOCATOR_PROPERTIES));
			m_Allocator->GetProperties(&Properties);
			while(hr == E_OUTOFMEMORY && Properties.cBuffers > 1)
			{
				Properties.cBuffers--;
				ALLOCATOR_PROPERTIES ActualProperties;
				hr = m_Allocator->SetProperties(&Properties, &ActualProperties);
				CHECK(hr)

				hr = m_Allocator->Commit();
			}
		}
    }
    return hr;
}

HRESULT CDSOutputPin::Deactivate()
{
    HRESULT hr = S_OK;
    if(m_Allocator != NULL)
    {
        hr = m_Allocator->Decommit();
        CHECK(hr);
    }
    return hr;
}

HRESULT CDSOutputPin::NegotiateAllocator(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr = S_OK;
	bool NeedToCommit = false;

	// if we haven't already choosen an allocator
    if(m_Allocator == NULL)
    {
        // ask the conencted filter for it's allocator
        // otherwise supply our own
        hr = m_MemInputPin->GetAllocator(m_Allocator.GetReleasedInterfaceReference());
        if(!m_Allocator)
        {
            // see what we get but don't return
            if(FAILED(hr))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
            }
            m_Allocator = m_MyMemAlloc;
        }
		if(m_Filter->m_State != State_Stopped)
		{
			NeedToCommit = true;
		}
    }
	else
	{
        hr = m_Allocator->Decommit();
        CHECK(hr);

		NeedToCommit = true;
	}

    // lets try and negotiate a sensible set of requirements
    // try to allocate the settings the renderer has asked for
    // subject to some sensible minumums.
    ALLOCATOR_PROPERTIES Props;
    ALLOCATOR_PROPERTIES PropsAct;
    ZeroMemory(&Props, sizeof(ALLOCATOR_PROPERTIES));
    hr = m_MemInputPin->GetAllocatorRequirements(&Props);
    if(FAILED(hr))
    {
        // if the pin doen't want to tell up what it wants 
        // then see what the current allocator setup is
        if(hr == E_NOTIMPL)
        {
            hr = m_Allocator->GetProperties(&Props);
            if(FAILED(hr))
            {
                return VFW_E_NO_TRANSPORT;
            }
        }
        else
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return VFW_E_NO_TRANSPORT;
        }
    }
    hr = SetType(pmt);
    CHECK(hr);

    ALLOCATOR_PROPERTIES PropsWeWant;
    ZeroMemory(&PropsWeWant, sizeof(PropsWeWant));

    hr = m_Filter->GetAllocatorRequirements(&PropsWeWant, this);
    CHECK(hr);

    Props.cbAlign = max(Props.cbAlign, PropsWeWant.cbAlign);
    Props.cbBuffer = max(Props.cbBuffer, PropsWeWant.cbBuffer);
    Props.cbPrefix = max(Props.cbPrefix, PropsWeWant.cbPrefix);
    Props.cBuffers = max(Props.cBuffers, PropsWeWant.cBuffers);
    
    hr = m_Allocator->SetProperties(&Props, &PropsAct);

    // \todo see if we need this rubbish
    // the old video renderer seems to set things up with
    // an alignment of 16 
    // that it's own allocator doesn't in fact support so
    // we loop down until we find an alignment it does like
    while(hr == VFW_E_BADALIGN && Props.cbAlign > 1)
    {
        Props.cbAlign /= 2;
        hr = m_Allocator->SetProperties(&Props, &PropsAct);
    }

    if(FAILED(hr))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return VFW_E_NO_TRANSPORT;
    }
    
    LOG(DBGLOG_FLOW, ("Allocator Negotiated Buffers - %d Size - %d Align - %d Prefix %d\n", PropsAct.cBuffers, PropsAct.cbBuffer, PropsAct.cbAlign, PropsAct.cbPrefix));

    hr = m_MemInputPin->NotifyAllocator(m_Allocator.GetNonAddRefedInterface(), FALSE);
    if(FAILED(hr))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return VFW_E_NO_TRANSPORT;
    }

    if(pReceivePin != NULL)
    {
        // If all is OK the save the Pin interface
        m_ConnectedPin = pReceivePin;
    
        // save the IPinConnection pointer
        // so that we can propagate dynamic reconnections
        m_PinConnection = pReceivePin;
    }

	if(NeedToCommit)
	{
        hr = m_Allocator->Commit();
        CHECK(hr);
	}

    return S_OK;
}
