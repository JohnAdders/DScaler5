///////////////////////////////////////////////////////////////////////////////
// $Id: OutputPin.cpp,v 1.1 2004-02-06 12:17:17 adcockj Exp $
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
// Revision 1.23  2003/12/09 11:45:57  adcockj
// Improved implementation of EnumPins
//
// Revision 1.22  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
// Revision 1.21  2003/09/30 16:59:26  adcockj
// Improved handling of small format changes
//
// Revision 1.20  2003/09/28 15:08:07  adcockj
// optimization fix and minor changes
//
// Revision 1.19  2003/09/19 16:12:14  adcockj
// Further improvements
//
// Revision 1.17  2003/08/21 16:17:58  adcockj
// Changed filter to wrap the deinterlacing DMO, fixed many bugs
//
// Revision 1.16  2003/07/25 16:00:55  adcockj
// Remove 704 stuff
//
// Revision 1.15  2003/05/20 16:50:59  adcockj
// Interim checkin, preparation for DMO processing path
//
// Revision 1.14  2003/05/19 07:02:24  adcockj
// Patches from Torbjorn to extend logging
//
// Revision 1.13  2003/05/17 11:29:35  adcockj
// Fixed crashing
//
// Revision 1.12  2003/05/10 13:21:31  adcockj
// Bug fixes
//
// Revision 1.11  2003/05/09 15:51:05  adcockj
// Code tidy up
// Added aspect ratio parameters
//
// Revision 1.10  2003/05/09 07:03:26  adcockj
// Bug fixes for new format code
//
// Revision 1.9  2003/05/08 15:58:38  adcockj
// Better error handling, threading and format support
//
// Revision 1.8  2003/05/06 16:38:01  adcockj
// Changed to fixed size output buffer and changed connection handling
//
// Revision 1.7  2003/05/06 07:00:30  adcockj
// Some cahnges from Torbjorn also some other attempted fixes
//
// Revision 1.6  2003/05/02 16:05:23  adcockj
// Logging with file and line numbers
//
// Revision 1.5  2003/05/02 10:51:49  adcockj
// Improved Allocator negotiation and added stub for Block
//
// Revision 1.4  2003/05/02 07:03:13  adcockj
// Some minor changes most not really improvements
//
// Revision 1.3  2003/05/01 18:15:17  adcockj
// Moved IMedaiSeeking to output pin
//
// Revision 1.2  2003/05/01 16:22:24  adcockj
// Dynamic connection test code
//
// Revision 1.1.1.1  2003/04/30 13:01:22  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OutputPin.h"
#include "DScaler.h"
#include "InputPin.h"
#include "EnumMediaTypes.h"

COutputPin::COutputPin()
{
    LOG(DBGLOG_FLOW, ("COutputPin::COutputPin\n"));
    InitMediaType(&m_CurrentMediaType);
    m_FormatVersion = 0;
}

COutputPin::~COutputPin()
{
    LOG(DBGLOG_FLOW, ("COutputPin::~COutputPin\n"));
    ClearMediaType(&m_CurrentMediaType);
}

HRESULT COutputPin::ChangeOutputFormat(const AM_MEDIA_TYPE* InputType)
{
    AM_MEDIA_TYPE ProposedType;
    HRESULT hr = S_OK;
    InitMediaType(&ProposedType);

    // if we are running then behave slightly differently
    if(m_ConnectedPin != NULL && m_Filter->m_State != State_Stopped && m_PinConnection != NULL)
    {
        // say which type we really want to send out
        hr = CreateOutputMediaTypeBasedOnExisting(InputType, &ProposedType, &m_CurrentMediaType);
        CHECK(hr);
    }
    else
    {
        // say which type we really want to send out
        hr = CreateOutputMediaType(InputType, &ProposedType);
        CHECK(hr);
    }
    
    hr = m_Allocator->Decommit();
    CHECK(hr);

    hr = InternalConnect(m_ConnectedPin, &ProposedType);
    ClearMediaType(&ProposedType);
    CHECK(hr);
    return hr;
}


HRESULT COutputPin::InternalConnect(IPin *pReceivePin, const AM_MEDIA_TYPE* ProposedType)
{
    // See if the format we want is supported
    // at the moment
    HRESULT hr = pReceivePin->ReceiveConnection(this, ProposedType);
    if(hr != S_OK)
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return VFW_E_NO_ACCEPTABLE_TYPES;
    }

    if(m_Allocator == NULL)
    {
        // ask the conencted filter for it's allocator
        // if it hasn't got one then use the one from upstream
        // \todo we should probably have our own allocator
        // here but it doesn't seem worth it yet
        hr = m_MemInputPin->GetAllocator(&m_Allocator);
        if(m_Allocator == NULL)
        {
            // see what we get but don't return
            if(FAILED(hr))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
            }
            m_Allocator = m_InputPin->m_Allocator;
        }
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
    
    Props.cBuffers = max(1, Props.cBuffers);
    Props.cbBuffer = max(768*576*2, (ULONG)Props.cbBuffer);
    Props.cbAlign = max(1, Props.cbAlign);

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

    if(FAILED(hr) && FALSE)
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return VFW_E_NO_TRANSPORT;
    }
    
    LOG(DBGLOG_ALL, ("Allocator Negotiated Buffers - %d Size - %d Align - %d Prefix %d\n", PropsAct.cBuffers, PropsAct.cbBuffer, PropsAct.cbAlign, PropsAct.cbPrefix));

    hr = m_MemInputPin->NotifyAllocator(m_Allocator, FALSE);
    if(FAILED(hr))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return VFW_E_NO_TRANSPORT;
    }

    hr = SetMediaType(ProposedType);
    return S_OK;
}



STDMETHODIMP COutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("COutputPin::Connect\n"));
    HRESULT hr;

    if(m_InputPin->m_ConnectedPin == NULL)
    {
        return VFW_E_NO_ACCEPTABLE_TYPES;
    }

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

    // say which type we really want to send out
    hr = CreateOutputMediaType(&m_InputPin->m_InputMediaType, &ProposedType);
    CHECK(hr);

    // check that any format we've been passed in is OK
    // otherwise just tell the caller that it isn't
    if(pmt != NULL)
    {
        if((pmt->majortype != GUID_NULL && pmt->majortype != ProposedType.majortype) ||
            (pmt->subtype != GUID_NULL && pmt->subtype != ProposedType.subtype) || 
            (pmt->formattype != GUID_NULL && pmt->formattype != ProposedType.formattype))
        {
            ClearMediaType(&ProposedType);
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    hr = InternalConnect(pReceivePin, &ProposedType);
    ClearMediaType(&ProposedType);
    if(FAILED(hr))
    {
        return hr;
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
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::ReceiveConnection\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::Disconnect(void)
{
    LOG(DBGLOG_FLOW, ("COutputPin::Disconnect\n"));
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
    LOG(DBGLOG_FLOW, ("COutputPin::ConnectedTo\n"));
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
        return m_ConnectedPin.CopyTo(pPin);
    }
}

STDMETHODIMP COutputPin::ConnectionMediaType(AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_FLOW, ("COutputPin::ConnectionMediaType\n"));
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
    LOG(DBGLOG_FLOW, ("COutputPin::QueryPinInfo\n"));
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
    LOG(DBGLOG_FLOW, ("COutputPin::QueryDirection\n"));
    if(pPinDir == NULL)
    {
        return E_POINTER;
    }
    *pPinDir = PINDIR_OUTPUT;
    return S_OK;
}

STDMETHODIMP COutputPin::QueryId(LPWSTR *Id)
{
    LOG(DBGLOG_FLOW, ("COutputPin::QueryId\n"));
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
    LOG(DBGLOG_FLOW, ("COutputPin::QueryAccept\n"));
    LogMediaType(pmt, "COutputPin::QueryAccept");
    HRESULT hr = S_OK;
    if(m_InputPin->m_ConnectedPin != NULL)
    {
        if(m_ConnectedPin != NULL)
        {
            if(!AreTypesCloseEnough(&m_CurrentMediaType, pmt))
            {
                hr = S_FALSE;
            }
        }
        else
        {
            AM_MEDIA_TYPE TestType;
            InitMediaType(&TestType);
            hr = CreateOutputMediaType(&m_InputPin->m_InputMediaType, &TestType);
            CHECK(hr);
            if(!AreTypesCloseEnough(&TestType, pmt))
            {
                hr = S_FALSE;
            }
            ClearMediaType(&TestType);
        }
    }
    else
    {
        hr = S_FALSE;
    }
    return S_OK;
}

STDMETHODIMP COutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    LOG(DBGLOG_FLOW, ("COutputPin::EnumMediaTypes\n"));
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    HRESULT hr = NewEnum->SetUpdate(this);
    CHECK(hr);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    return S_OK;
}

STDMETHODIMP COutputPin::QueryInternalConnections(IPin **apPin, ULONG *nPin)
{
    LOG(DBGLOG_FLOW, ("COutputPin::QueryInternalConnections\n"));
    // don't bother with this as we are a simple filter
    return E_NOTIMPL;
}

STDMETHODIMP COutputPin::EndOfStream(void)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::EndOfStream\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::BeginFlush(void)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::BeginFlush\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::EndFlush(void)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::EndFlush\n"));
    // shouldn't be called on output pin
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::NewSegment\n"));
    return E_UNEXPECTED;
}

STDMETHODIMP COutputPin::Notify(IBaseFilter *pSelf, Quality q)
{
    // \todo see what we get sent here and work out how to handle some
    // of the more common messages
    // in the mean time just pass the messages upstream
	if(q.Type == Famine)
	{
		//if(q.Late > 400000)
		{
			CComQIPtr<IQualityControl> QualityControl = m_InputPin->m_ConnectedPin;
			if(QualityControl != NULL)
			{
                LOG(DBGLOG_ALL, ("Coped With Famine - %d\n", q.Late));
				return QualityControl->Notify(pSelf, q);
			}
			else
			{
                LOG(DBGLOG_ALL, ("Ignored Famine - %d\n", q.Late));
				return E_NOTIMPL;
			}
		}
		//else
		{
			return S_OK;
		}
	}
	if(q.Type == Flood)
	{
		//if(q.Late > 400000)
		{
			CComQIPtr<IQualityControl> QualityControl = m_InputPin->m_ConnectedPin;
			if(QualityControl != NULL)
			{
                LOG(DBGLOG_ALL, ("Coped With Flood - %d\n", q.Late));
				return QualityControl->Notify(pSelf, q);
			}
			else
			{
                LOG(DBGLOG_ALL, ("Ignored Flood - %d\n", q.Late));
				return E_NOTIMPL;
			}
		}
		//else
		{
			return S_OK;
		}
	}
	else
	{
		return S_OK;
	}
}

STDMETHODIMP COutputPin::SetSink(IQualityControl *piqc)
{
    LOG(DBGLOG_FLOW, ("*Unexpected Call* - COutputPin::SetSink\n"));
    return E_NOTIMPL;
}

STDMETHODIMP COutputPin::Block(DWORD dwBlockFlags, HANDLE hEvent)
{
    LOG(DBGLOG_FLOW, ("COutputPin::Block\n"));
    if(dwBlockFlags == AM_PIN_FLOW_CONTROL_BLOCK)
    {
        if(m_InputPin->m_Block == FALSE)
        {
            if(hEvent != NULL)
            {
                m_InputPin->m_BlockEvent = hEvent;
                m_InputPin->m_Block = TRUE;
                return S_OK;
            }
            else
            {
                m_InputPin->m_BlockEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                if(m_InputPin->m_BlockEvent != NULL)
                {
                    m_InputPin->m_Block = TRUE;
                    WaitForSingleObject(m_InputPin->m_BlockEvent, INFINITE);
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
        m_InputPin->m_Block = FALSE;
        m_InputPin->m_BlockEvent = NULL;
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}


void COutputPin::InternalDisconnect()
{
    ClearMediaType(&m_CurrentMediaType);
    ++m_FormatVersion;
    m_ConnectedPin.Release();
    m_PinConnection.Release();
    m_MemInputPin.Release();
    m_Allocator.Release();
}

ULONG COutputPin::FormatVersion()
{
    return m_FormatVersion;
}

HRESULT COutputPin::GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type)
{   
    HRESULT hr = S_OK;
    if(m_CurrentMediaType.majortype != GUID_NULL)
    {
        if(TypeNum == 0)
        {
            hr = CopyMediaType(Type, &m_CurrentMediaType);
            CHECK(hr);
        }
        else
        {
            hr = S_FALSE;
        }
    }
    else
    {
        if(m_InputPin->m_ConnectedPin != NULL)
        {
            if(TypeNum == 0)
            {
                hr = CreateOutputMediaType(&m_InputPin->m_InputMediaType, Type);
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
    }
    return hr;
}

// we just pass all seeking calls downsteam

#define GETMEDIASEEKING \
    if(m_InputPin->m_ConnectedPin == NULL) return E_NOTIMPL; \
    PIN_INFO PinInfo; \
    if(FAILED(m_InputPin->m_ConnectedPin->QueryPinInfo(&PinInfo))) return E_NOTIMPL; \
    CComQIPtr<IMediaSeeking> MediaSeeking = PinInfo.pFilter; \
    PinInfo.pFilter->Release(); \
    if(MediaSeeking == NULL) return E_NOTIMPL;

STDMETHODIMP COutputPin::GetCapabilities(DWORD *pCapabilities)
{
    GETMEDIASEEKING
    return MediaSeeking->GetCapabilities(pCapabilities);
}

STDMETHODIMP COutputPin::CheckCapabilities(DWORD *pCapabilities)
{
    GETMEDIASEEKING
    return MediaSeeking->CheckCapabilities(pCapabilities);
}

STDMETHODIMP COutputPin::IsFormatSupported(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->IsFormatSupported(pFormat);
}

STDMETHODIMP COutputPin::QueryPreferredFormat(GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->QueryPreferredFormat(pFormat);
}

STDMETHODIMP COutputPin::GetTimeFormat(GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->GetTimeFormat(pFormat);
}

STDMETHODIMP COutputPin::IsUsingTimeFormat(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->IsUsingTimeFormat(pFormat);
}

STDMETHODIMP COutputPin::SetTimeFormat(const GUID *pFormat)
{
    GETMEDIASEEKING
    return MediaSeeking->SetTimeFormat(pFormat);
}

STDMETHODIMP COutputPin::GetDuration(LONGLONG *pDuration)
{
    GETMEDIASEEKING
    return MediaSeeking->GetDuration(pDuration);
}

STDMETHODIMP COutputPin::GetStopPosition(LONGLONG *pStop)
{
    GETMEDIASEEKING
    return MediaSeeking->GetStopPosition(pStop);
}

STDMETHODIMP COutputPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    GETMEDIASEEKING
    return MediaSeeking->GetCurrentPosition(pCurrent);
}

STDMETHODIMP COutputPin::ConvertTimeFormat(
                                LONGLONG *pTarget,
                                const GUID *pTargetFormat,
                                LONGLONG Source,
                                const GUID *pSourceFormat
                            )
{
    GETMEDIASEEKING
    return MediaSeeking->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);
}

STDMETHODIMP COutputPin::SetPositions( 
                        LONGLONG *pCurrent,
                        DWORD dwCurrentFlags,
                        LONGLONG *pStop,
                        DWORD dwStopFlags
                       )
{
    GETMEDIASEEKING
    return MediaSeeking->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);
}

STDMETHODIMP COutputPin::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
    GETMEDIASEEKING
    return MediaSeeking->GetPositions(pCurrent, pStop);
}

STDMETHODIMP COutputPin::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    GETMEDIASEEKING
    return MediaSeeking->GetAvailable(pEarliest, pLatest);
}

STDMETHODIMP COutputPin::SetRate(double dRate)
{
    GETMEDIASEEKING
    return MediaSeeking->SetRate(dRate);
}

STDMETHODIMP COutputPin::GetRate(double *pdRate)
{
    GETMEDIASEEKING
    return MediaSeeking->GetRate(pdRate);
}

STDMETHODIMP COutputPin::GetPreroll(LONGLONG *pllPreroll)
{
    GETMEDIASEEKING
    return MediaSeeking->GetPreroll(pllPreroll);
}

HRESULT COutputPin::CreateOutputMediaType(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType)
{
    BITMAPINFOHEADER* BitmapInfo = NULL;
	LPRECT pSourceRect;
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

    NewFormat->dwInterlaceFlags = 0;

    if(InputType->formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* OldFormat = (VIDEOINFOHEADER2*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;
		pSourceRect = &OldFormat->rcSource;
        NewFormat->dwPictAspectRatioX = OldFormat->dwPictAspectRatioX;
        NewFormat->dwPictAspectRatioY = OldFormat->dwPictAspectRatioY;
        NewFormat->dwBitRate = OldFormat->dwBitRate * 2;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame / 2;
    }
    else if(InputType->formattype == FORMAT_VideoInfo)
    {
        NewType->pbFormat =(BYTE*)NewFormat;
        VIDEOINFOHEADER* OldFormat = (VIDEOINFOHEADER*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;
		pSourceRect = &OldFormat->rcSource;

        // if the input format is a known TV style one then
        // it should be assumed to be 4:3
		if((BitmapInfo->biWidth == 704 || BitmapInfo->biWidth == 720 || BitmapInfo->biWidth == 768) &&
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

        NewFormat->dwBitRate = OldFormat->dwBitRate * 2;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame / 2;
    }
    else
    {
        ClearMediaType(NewType);
    }


    NewFormat->dwPictAspectRatioX *= m_Filter->GetParamInt(CDScaler::ASPECTINCREASEX);
    NewFormat->dwPictAspectRatioY *= m_Filter->GetParamInt(CDScaler::ASPECTINCREASEY);

    // we want to use the new height but we'll work with a normal stride
    // if it's within 32 of the new width
    long Height = BitmapInfo->biHeight; 
    long Width;

	if(Height == 576)
	{
		NewFormat->AvgTimePerFrame = 200000;
	}

	if(pSourceRect->right > 0)
	{
		Width = pSourceRect->right;
	}
	else
	{
		Width = BitmapInfo->biWidth;
	}

    if((Width == 704) && (Height == 480 || Height == 576))
    {
        // adjustment to make 704 video the same shape as 720
        NewFormat->dwPictAspectRatioX = 4 * 44;
        NewFormat->dwPictAspectRatioY = 3 * 45;
    }

    if(m_Filter->GetParamBool(CDScaler::INPUTISANAMORPHIC) != 0)
    {
        NewFormat->dwPictAspectRatioX *= 4;
        NewFormat->dwPictAspectRatioY *= 3;
    }


    DWORD Size = Height * Width * BitmapInfo->biBitCount / 8;

    NewFormat->rcSource.top = 0;
    NewFormat->rcSource.bottom = Height;
    NewFormat->rcSource.left = 0;
    NewFormat->rcSource.right = Width;
    NewFormat->rcTarget.top = 0;
    NewFormat->rcTarget.bottom = Height;
    NewFormat->rcTarget.left = 0;
    NewFormat->rcTarget.right = Width;
    
    NewFormat->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    NewFormat->bmiHeader.biHeight = Height;
    NewFormat->bmiHeader.biWidth = Width;
    NewFormat->bmiHeader.biPlanes = 1;
    NewFormat->bmiHeader.biBitCount = BitmapInfo->biBitCount;
    NewFormat->bmiHeader.biCompression = BitmapInfo->biCompression;
    NewFormat->bmiHeader.biSizeImage = Size;
    NewFormat->bmiHeader.biXPelsPerMeter = 0;
    NewFormat->bmiHeader.biYPelsPerMeter = 0;
    NewFormat->bmiHeader.biClrUsed = 0;
    NewFormat->bmiHeader.biClrImportant = 0;

    NewType->lSampleSize = Size;
    return S_OK;
}

HRESULT COutputPin::CreateOutputMediaTypeBasedOnExisting(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType, const AM_MEDIA_TYPE* OldType)
{
    HRESULT hr = COutputPin::CreateOutputMediaType(InputType, NewType);
    CHECK(hr);
    if(NewType->lSampleSize <= OldType->lSampleSize)
    {
        VIDEOINFOHEADER2* NewFormat = (VIDEOINFOHEADER2*)NewType->pbFormat;
        VIDEOINFOHEADER2* OldFormat = (VIDEOINFOHEADER2*)OldType->pbFormat;
        DWORD Height = NewFormat->bmiHeader.biHeight * ((OldFormat->bmiHeader.biHeight < 0)?-1:1);
        DWORD Width = max(NewFormat->bmiHeader.biWidth, OldFormat->bmiHeader.biWidth);

        if(abs(Height) * Width * NewFormat->bmiHeader.biBitCount / 8 < OldType->lSampleSize)
        {
            NewFormat->bmiHeader.biHeight = Height;

            NewFormat->bmiHeader.biWidth = Width;
            NewFormat->bmiHeader.biSizeImage = abs(Height) * Width * NewFormat->bmiHeader.biBitCount / 8;
            NewType->lSampleSize = OldType->lSampleSize;
        }
    }
    return hr;
}


BITMAPINFOHEADER* COutputPin::GetBitmapInfo()
{
    VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)m_CurrentMediaType.pbFormat;
    return &Format->bmiHeader;
}

BOOL COutputPin::AreTypesCloseEnough(const AM_MEDIA_TYPE* CurrentType, const AM_MEDIA_TYPE* ProposedType)
{
    if(CurrentType->majortype != ProposedType->majortype)
    {
        return FALSE;
    }
    if(CurrentType->subtype != ProposedType->subtype)
    {
        return FALSE;
    }
    if(CurrentType->formattype != ProposedType->formattype)
    {
        return FALSE;
    }
    if(CurrentType->formattype != FORMAT_VIDEOINFO2)
    {
        return FALSE;
    }
    VIDEOINFOHEADER2* CurrentFormat = (VIDEOINFOHEADER2*)CurrentType->pbFormat;
    VIDEOINFOHEADER2* ProposedFormat = (VIDEOINFOHEADER2*)ProposedType->pbFormat;

    if(CurrentFormat->bmiHeader.biWidth < ProposedFormat->bmiHeader.biWidth)
    {
        return FALSE;
    }
    if(abs(CurrentFormat->bmiHeader.biHeight) < abs(ProposedFormat->bmiHeader.biHeight))
    {
        return FALSE;
    }
    return TRUE;
}

HRESULT COutputPin::SetMediaType(const AM_MEDIA_TYPE* NewType)
{
    LogMediaType(NewType, "Output Format Change");
    HRESULT hr = CopyMediaType(&m_CurrentMediaType, NewType);
    ++m_FormatVersion;
    // tell the filter that we need to rebuild the processing path
    m_Filter->SetTypesChangedFlag();
    CHECK(hr);
    return hr;
}
