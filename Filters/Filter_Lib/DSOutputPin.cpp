///////////////////////////////////////////////////////////////////////////////
// $Id: DSOutputPin.cpp,v 1.25 2008-03-28 18:07:58 adcockj Exp $
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
// Revision 1.24  2007/11/30 18:06:48  adcockj
// Initial go at h264 support
//
// Revision 1.23  2004/11/25 17:22:10  adcockj
// Fixed some more connection issues
//
// Revision 1.22  2004/11/07 09:12:05  adcockj
// fixed connection issue
//
// Revision 1.21  2004/11/06 14:07:01  adcockj
// Fixes for WM10 and seeking
//
// Revision 1.20  2004/11/02 17:59:56  adcockj
// fix for vmr9 issues
//
// Revision 1.19  2004/09/23 14:27:59  adcockj
// preliminary fixed for reconnection issues
//
// Revision 1.18  2004/09/10 15:35:57  adcockj
// Bug fixes for problems found in 0.0.2 with MPEG-1 & overlay
//
// Revision 1.17  2004/08/31 16:33:42  adcockj
// Minor improvements to quality control
// Preparation for next version
// Start on integrating film detect
//
// Revision 1.16  2004/07/26 17:08:13  adcockj
// Force use of fixed size output buffers to work around issues with Wave renderer
//
// Revision 1.15  2004/07/20 16:37:57  adcockj
// Fixes for main issues raised in testing of 0.0.1
//  - Improved parameter handling
//  - Fixed some overlay issues
//  - Auto aspect ratio with VMR
//  - Fixed some overlay stutters
//  - Fixed some push filter issues
//  - ffdshow and DirectVobSub connection issues
//
// Added
//  - Hardcode for PAL setting for ffdshow
//  - Added choice of IDCT for testing
//
// Revision 1.14  2004/07/07 14:09:01  adcockj
// removed tabs
//
// Revision 1.13  2004/07/01 16:12:47  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.12  2004/05/06 06:38:07  adcockj
// Interim fixes for connection and PES streams
//
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
            hr = SetType(&ProposedType);
            if(SUCCEEDED(hr))
            {
                hr = NegotiateAllocator(pReceivePin);    
                if(SUCCEEDED(hr))
                {
                    break;
                }
                else
                {
                    hr = pReceivePin->Disconnect();
                    CHECK(hr);
                }
            }
        }
        
        ++TypeNum;
        ClearMediaType(&ProposedType);
    }
    

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

STDMETHODIMP CDSOutputPin::GetLatency(REFERENCE_TIME *prtLatency)
{
    *prtLatency = 400000;
    return S_OK;
}


#define GETPUSHSOURCE \
    CDSBasePin* pPin = m_Filter->GetPin(0); \
    SI(IAMPushSource) PushSource; \
    if(pPin->m_Direction == PINDIR_INPUT) \
        PushSource = pPin->m_ConnectedPin;

STDMETHODIMP CDSOutputPin::GetPushSourceFlags(ULONG *pFlags)
{
    HRESULT hr =  S_OK;
    GETPUSHSOURCE
    if(PushSource)
    {
        if(m_Filter->IsClockUpstream())
        {
            *pFlags = 0;
        }
        else
        {
            *pFlags = AM_PUSHSOURCECAPS_NOT_LIVE;
        }
    }
    else
    {
        *pFlags = AM_PUSHSOURCECAPS_NOT_LIVE;
    }
    return hr;
}

STDMETHODIMP CDSOutputPin::SetPushSourceFlags(ULONG Flags)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDSOutputPin::SetStreamOffset(REFERENCE_TIME rtOffset)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDSOutputPin::GetStreamOffset(REFERENCE_TIME *prtOffset)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDSOutputPin::GetMaxStreamOffset(REFERENCE_TIME *prtMaxOffset)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDSOutputPin::SetMaxStreamOffset(REFERENCE_TIME rtMaxOffset)
{
    return E_NOTIMPL;
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
    AM_MEDIA_TYPE* pMediaType = NULL;

    // get a sample to output to
    HRESULT hr = m_Allocator->GetBuffer(OutSample, rtStart, rtStop, dwFlags);
	if(hr == VFW_E_SIZENOTSET)
	{
		hr = NegotiateAllocator(NULL);
        CHECK(hr);
	    hr = m_Allocator->GetBuffer(OutSample, rtStart, rtStop, dwFlags);
	}
    if(FAILED(hr))
    {
        if(m_Filter->m_State == State_Stopped)
        {
            return VFW_E_WRONG_STATE;
        }
        LOG(DBGLOG_FLOW, ("GetBuffer Failed %08x\n", hr));
        return hr;
    }

    if(*OutSample == NULL)
    {
        return VFW_E_WRONG_STATE;
    }

    // check for media type changes on the output side
    // a NULL means the type is the same as last time
    hr = (*OutSample)->GetMediaType(&pMediaType);

    CHECK(hr);

    if(hr == S_OK && pMediaType != NULL)
    {
        LogMediaType(pMediaType, "Media type from renderer", DBGLOG_ALL);

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
		if(FAILED(hr))
		{
            LogBadHRESULT(hr, __FILE__, __LINE__);
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

HRESULT CDSOutputPin::NegotiateAllocator(IPin *pReceivePin)
{
    HRESULT hr = S_OK;

    // if we haven't already choosen an allocator
    if(m_Allocator == NULL)
    {
        // ask the conencted filter for it's allocator
        // otherwise supply our own
        hr = m_MemInputPin->GetAllocator(m_Allocator.GetReleasedInterfaceReference());
        if(m_Allocator)
        {
            hr = NegotiateBufferSize(pReceivePin);
            if(FAILED(hr))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
                m_Allocator.Detach();
                hr = S_OK;
            }
        }

        if(!m_Allocator)
        {
            m_Allocator = m_MyMemAlloc;
            
            hr = NegotiateBufferSize(pReceivePin);
            if(FAILED(hr))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
		        m_Allocator.Detach();
                return VFW_E_NO_TRANSPORT;
            }
        }

        hr = m_MemInputPin->NotifyAllocator(m_Allocator.GetNonAddRefedInterface(), FALSE);
        if(FAILED(hr))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
		    m_Allocator.Detach();
            return VFW_E_NO_TRANSPORT;
        }

        if(m_Filter->m_State != State_Stopped)
        {
            hr = m_Allocator->Commit();
            CHECK(hr);
        }

        // If all is OK the save the Pin interface
        m_ConnectedPin = pReceivePin;

        // save the IPinConnection pointer
        // so that we can propagate dynamic reconnections
        m_PinConnection = pReceivePin;
    }
    else
    {
        hr = m_Allocator->Decommit();
        CHECK(hr);

        hr = NegotiateBufferSize(pReceivePin);
        if(FAILED(hr))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return VFW_E_NO_TRANSPORT;
        }

        hr = m_Allocator->Commit();
        CHECK(hr);
    }
    return hr;
}

HRESULT CDSOutputPin::NegotiateBufferSize(IPin *pReceivePin)
{
    HRESULT hr = S_OK;

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
            CHECK(hr);
        }
        else
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }
    }

    ALLOCATOR_PROPERTIES PropsWeWant;
    ZeroMemory(&PropsWeWant, sizeof(PropsWeWant));

    hr = m_Filter->GetAllocatorRequirements(&PropsWeWant, this);
    CHECK(hr);

    Props.cbAlign = max(Props.cbAlign, PropsWeWant.cbAlign);
    Props.cbBuffer = max(Props.cbBuffer, PropsWeWant.cbBuffer);
    Props.cbPrefix = max(Props.cbPrefix, PropsWeWant.cbPrefix);
    Props.cBuffers = max(Props.cBuffers, PropsWeWant.cBuffers);

    // handle fixed sized buffers
    if(GetMediaType()->bFixedSizeSamples)
    {
        Props.cbBuffer = max(Props.cbBuffer, (long)GetMediaType()->lSampleSize);
    }

    ZeroMemory(&PropsAct, sizeof(PropsAct));
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

    LOG(DBGLOG_FLOW, ("Allocator hr = %08x Negotiated Buffers - %d Size - %d Align - %d Prefix %d\n", hr, PropsAct.cBuffers, PropsAct.cbBuffer, PropsAct.cbAlign, PropsAct.cbPrefix));

    return hr;
}
