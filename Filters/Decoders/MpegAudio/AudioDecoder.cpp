///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder.cpp,v 1.6 2004-02-29 13:47:47 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2003 Gabest
//	http://www.gabest.org
//
///////////////////////////////////////////////////////////////////////////////
//
//  This Program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//   
//  This Program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//   
//  You should have received a copy of the GNU General Public License
//  along with GNU Make; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
//  http://www.gnu.org/copyleft/gpl.html
//
///////////////////////////////////////////////////////////////////////////////
//
// This file was based on the mpadec filter which is part of the MPC
// program see http://sf.net/projects/guliverkli/ for more details
//
// Changes made to files by John Adcock 12/02/04
//  - Removed use of MFC
//  - Replaced use of ATL with YACL
//  - Replaced Baseclasses with FilterLib
//  - Removed DeCSS
//  - Removed conversion to float for MPEG audio
//  - Attempted to add support for different PCM types
//
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2004/02/27 17:04:38  adcockj
// Added support for fixed point libraries
// Added dither to 16 conversions
// Changes to support library fixes
//
// Revision 1.4  2004/02/25 17:14:01  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/17 16:39:59  adcockj
// Added dts analog support (based on dtsdec-0.0.1 and Gabest's patch to mpadecfilter)
//
// Revision 1.2  2004/02/16 17:25:00  adcockj
// Fix build errors, locking problems and DVD compatability
//
// Revision 1.1  2004/02/13 12:22:17  adcockj
// Initial check-in of audio decoder (based on mpadecfilter from guliverkli, libmad and liba52)
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"
#include "MoreUuids.h"
#include "CPUID.h"

extern HINSTANCE g_hInstance;


CAudioDecoder::CAudioDecoder() :
    CDSBaseFilter(L"Mpeg Audio Filter", 1, 1)
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::CreatePins\n"));
    
    m_AudioInPin = new CDSInputPin;
    if(m_AudioInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_AudioInPin->AddRef();
    m_AudioInPin->SetupObject(this, L"Audio In");

    m_AudioOutPin = new CDSOutputPin;
    if(m_AudioOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_AudioOutPin->AddRef();
    m_AudioOutPin->SetupObject(this, L"Audio Out");

    m_rtNextFrameStart = _I64_MIN;
    m_rtOutputStart = 0;
    m_sample_max = 0.1;
    m_NeedToAttachFormat = false;
    m_OutputSampleType = OUTSAMPLE_16BIT;
    m_SampleSize = 2;
    
    InitMediaType(&m_InternalMT);
    ZeroMemory(&m_InternalWFE, sizeof(WAVEFORMATEXTENSIBLE));

    // request buffer large enough for 100ms of 6 channel 32 bit audio
	m_OutputBufferSize = 48000*6*(32/8)/10; 

}

CAudioDecoder::~CAudioDecoder()
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::~CAudioDecoder\n"));
}

STDMETHODIMP CAudioDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_ALL, ("CAudioDecoder::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CAudioDecoder;
    return S_OK;
}


STDMETHODIMP CAudioDecoder::get_Name(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_NAME, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Name = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CAudioDecoder::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CAudioDecoder::get_Authors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_AUTHORS, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Authors = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CAudioDecoder::ParamChanged(DWORD dwParamIndex)
{
    return S_OK;
}

HRESULT CAudioDecoder::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}


HRESULT CAudioDecoder::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, CDSBasePin* pPin)
{
	return E_NOTIMPL;
}

HRESULT CAudioDecoder::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
	return E_NOTIMPL;
}


HRESULT CAudioDecoder::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}


HRESULT CAudioDecoder::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProperties, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        return E_NOTIMPL;
    }
    else if(pPin == m_AudioOutPin)
    {
	    pProperties->cBuffers = 6;
	    pProperties->cbBuffer = m_OutputBufferSize; 
	    pProperties->cbAlign = 1;
	    pProperties->cbPrefix = 0;
    }
    else
    {
        return E_UNEXPECTED;
    }
	return S_OK;
}

bool CAudioDecoder::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool Result = false;
    if(pPin == m_AudioInPin)
    {
	    Result = (pmt->majortype == MEDIATYPE_Audio && 
                  pmt->subtype == MEDIASUBTYPE_MP3) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                  pmt->subtype == MEDIASUBTYPE_MPEG1AudioPayload) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG1Payload) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG1Packet) ||
			    (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3) ||
			    (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_WAVE_DTS) ||
			    (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
			    (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO);
    }
    else if(pPin == m_AudioOutPin)
    {
    	Result = (pmt->majortype == MEDIATYPE_Audio) && 
                  (pmt->subtype == MEDIASUBTYPE_PCM ||
				    pmt->subtype == MEDIASUBTYPE_IEEE_FLOAT);
    }
	return Result;
}

HRESULT CAudioDecoder::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    if(pPin != m_AudioInPin)
    {
        return E_UNEXPECTED;
    }

    // \todo use pSampleProperties properly
	HRESULT hr;

    if(pSampleProperties->dwStreamId != AM_STREAM_MEDIA)
		return m_AudioOutPin->SendSample(InSample);

	BYTE* pDataIn = pSampleProperties->pbBuffer;

	long len = pSampleProperties->lActual;

	if(*(DWORD*)pDataIn == 0xBA010000) // MEDIATYPE_*_PACK
	{
		len -= 14; 
        pDataIn += 14;
		if(int stuffing = (pDataIn[-1]&7))
        {
            len -= stuffing; 
            pDataIn += stuffing;
        }
	}

	if(len <= 0) return S_OK;

	const GUID& subtype = m_AudioInPin->GetMediaType()->subtype;

	if(*(DWORD*)pDataIn == 0xBB010000)
	{
		len -= 4; pDataIn += 4;
		int hdrlen = ((pDataIn[0]<<8)|pDataIn[1]) + 2;
		len -= hdrlen; pDataIn += hdrlen;
	}

	if(len <= 0) return S_OK;

	if((*(DWORD*)pDataIn&0xE0FFFFFF) == 0xC0010000 || *(DWORD*)pDataIn == 0xBD010000)
	{
		if(subtype == MEDIASUBTYPE_MPEG1Packet)
		{
			len -= 4+2+7; pDataIn += 4+2+7; // is it always ..+7 ?
		}
		else if(subtype == MEDIASUBTYPE_MPEG2_AUDIO) // can this be after 0x000001BD too?
		{
			len -= 8; pDataIn += 8;
			len -= *pDataIn+1; pDataIn += *pDataIn+1;
		}
		else if(subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO)
		{
			len -= 8; pDataIn += 8;
			len -= *pDataIn+1; pDataIn += *pDataIn+1;
			len -= 7; pDataIn += 7;
		}
		else if(subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3
			|| subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)
		{
			len -= 8; pDataIn += 8;
			len -= *pDataIn+1; pDataIn += *pDataIn+1;
			len -= 4; pDataIn += 4;
		}
	}

	if(len <= 0) return S_OK;

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY || 
        pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY)
	{
        m_IsDiscontinuity = true;
		m_buff.resize(0);
        m_sample_max = 0.1f;
		m_rtOutputStart = 0;
	}

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID && m_rtNextFrameStart == _I64_MIN)
    {
		m_rtNextFrameStart = pSampleProperties->tStart;
	}
    else
    {
        m_rtNextFrameStart = _I64_MIN;
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        LOG(DBGLOG_ALL, ("Receive: %I64d - %I64d\n", pSampleProperties->tStart, m_rtNextFrameStart));
    }

	int tmp = m_buff.size();
	m_buff.resize(m_buff.size() + len);
	memcpy(&m_buff[0] + tmp, pDataIn, len);
	len += tmp;

	if(subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO)
		hr = ProcessLPCM();
	else if(subtype == MEDIASUBTYPE_DOLBY_AC3 || subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3)
		hr = ProcessAC3();
	else if(subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)
		hr = ProcessDTS();
	else // if(.. the rest ..)
		hr = ProcessMPA();

	return hr;
}


HRESULT CAudioDecoder::CreateInternalSPDIFMediaType(DWORD nSamplesPerSec, WORD wBitsPerSample)
{
	m_InternalMT.majortype = MEDIATYPE_Audio;
   	m_InternalMT.subtype = MEDIASUBTYPE_PCM;
	m_InternalMT.formattype = FORMAT_WaveFormatEx;

	m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
	m_InternalWFE.Format.nChannels = 2;
	m_InternalWFE.Format.wBitsPerSample = wBitsPerSample;
	m_InternalWFE.Format.nBlockAlign = 2 * wBitsPerSample / 8;
	m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

    m_InternalWFE.Format.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	m_InternalWFE.Format.cbSize = 0;

    m_InternalMT.cbFormat = sizeof(m_InternalWFE.Format) + m_InternalWFE.Format.cbSize;
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 

    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
    if(hr == S_OK)
    {
        return hr;
    }
    else
    {
        return E_UNEXPECTED;
    }
}


HRESULT CAudioDecoder::CreateInternalPCMMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask, WORD wBitsPerSample)
{
	m_InternalMT.majortype = MEDIATYPE_Audio;
   	m_InternalMT.subtype = MEDIASUBTYPE_PCM;
	m_InternalMT.formattype = FORMAT_WaveFormatEx;

	m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
	m_InternalWFE.Format.nChannels = nChannels;
	m_InternalWFE.Format.wBitsPerSample = wBitsPerSample;
	m_InternalWFE.Format.nBlockAlign = nChannels * wBitsPerSample / 8;
	m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

    if(wBitsPerSample > 16 || nChannels > 2)
    {
		m_InternalWFE.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		m_InternalWFE.Format.cbSize = sizeof(m_InternalWFE) - sizeof(m_InternalWFE.Format);
		m_InternalWFE.SubFormat = m_InternalMT.subtype;
        m_InternalWFE.dwChannelMask = dwChannelMask;
        m_InternalWFE.Samples.wValidBitsPerSample = wBitsPerSample;
    }
    else
    {
    	m_InternalWFE.Format.wFormatTag = (WORD)m_InternalMT.subtype.Data1;
	    m_InternalWFE.Format.cbSize = 0;
    }

    m_InternalMT.cbFormat = sizeof(m_InternalWFE.Format) + m_InternalWFE.Format.cbSize;
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 

    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);

    switch(wBitsPerSample)
    {
    case 16:
        m_OutputSampleType = OUTSAMPLE_16BIT;
        m_SampleSize = 2;
        if(hr == S_OK) return hr;
        return E_UNEXPECTED;
        break;
    case 24:
        m_OutputSampleType = OUTSAMPLE_24BIT;
        m_SampleSize = 3;
        if(hr == S_OK) return hr;
        return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 16);
        break;
    case 32:
        m_OutputSampleType = OUTSAMPLE_32BIT;
        m_SampleSize = 4;
        if(hr == S_OK) return hr;
        return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 24);
        break;
    }
    return E_UNEXPECTED;
}

HRESULT CAudioDecoder::CreateInternalIEEEMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	m_InternalMT.majortype = MEDIATYPE_Audio;
   	m_InternalMT.subtype = MEDIASUBTYPE_IEEE_FLOAT;
	m_InternalMT.formattype = FORMAT_WaveFormatEx;

	m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
	m_InternalWFE.Format.nChannels = nChannels;
	m_InternalWFE.Format.wBitsPerSample = 32;
	m_InternalWFE.Format.nBlockAlign = nChannels * 4;
	m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

	m_InternalWFE.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	m_InternalWFE.Format.cbSize = sizeof(m_InternalWFE) - sizeof(m_InternalWFE.Format);
	m_InternalWFE.SubFormat = m_InternalMT.subtype;
    m_InternalWFE.dwChannelMask = dwChannelMask;
    m_InternalWFE.Samples.wValidBitsPerSample = 32;

    m_InternalMT.cbFormat = sizeof(m_InternalWFE);
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 
    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
    if(hr == S_OK)
    {
        m_OutputSampleType = OUTSAMPLE_FLOAT;
        m_SampleSize = sizeof(float);
        return hr;
    }
    
    return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 32);

}

HRESULT CAudioDecoder::Deliver(IMediaSample* pOut, REFERENCE_TIME rtDur)
{
	HRESULT hr = S_OK;

    if(m_rtNextFrameStart != _I64_MIN)
    {
        if(!GetParamBool(JITTERREMOVER) || abs((long)(m_rtNextFrameStart - m_rtOutputStart)) > rtDur || m_IsDiscontinuity)
        {
            if(abs((long)(m_rtNextFrameStart - m_rtOutputStart)) > 0)
            {
                LOG(DBGLOG_FLOW, ("Time Diff: %I64d - %I64d\n", m_rtOutputStart, m_rtNextFrameStart));
            }
            m_rtOutputStart = m_rtNextFrameStart;
        }
    }
    
    m_rtNextFrameStart = _I64_MIN;

    LOG(DBGLOG_FLOW, ("Deliver: %I64d - %I64d\n", m_rtOutputStart, m_rtOutputStart + rtDur));

	if(m_NeedToAttachFormat)
	{
		m_AudioOutPin->SetType(&m_InternalMT);
		pOut->SetMediaType(&m_InternalMT);
	}

	pOut->SetTime(&m_rtOutputStart, NULL);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetSyncPoint(TRUE);

	if(m_rtOutputStart >= 0)
    {
	    pOut->SetDiscontinuity(m_IsDiscontinuity); 
        m_IsDiscontinuity = false;

    	hr = m_AudioOutPin->SendSample(pOut);
    }

    m_rtOutputStart += rtDur;
    return hr;
}


HRESULT CAudioDecoder::ReconnectOutput(DWORD Len)
{
	HRESULT hr;

	if(!AreMediaTypesIdentical(&m_InternalMT, m_AudioOutPin->GetMediaType()) || Len > m_OutputBufferSize)
	{
        // should have already done a QueryAccept when deciding types
        // so no need to do again here

        if((DWORD)Len > m_OutputBufferSize)
        {
            m_OutputBufferSize = Len * 3 / 2;
            
            hr = m_AudioOutPin->NegotiateAllocator(NULL, &m_InternalMT);
            CHECK(hr);
        }
        m_NeedToAttachFormat = true;
	}
    else
    {
        m_NeedToAttachFormat = false;
    }
    return S_OK;
}

HRESULT CAudioDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{

    if(pPin == m_AudioOutPin)
    {
        if(!m_AudioInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

	    if(TypeNum < 0) return E_INVALIDARG;
	    if(TypeNum > 0) return VFW_S_NO_MORE_ITEMS;

	    pmt->majortype = MEDIATYPE_Audio;
	    pmt->subtype = MEDIASUBTYPE_PCM;
	    pmt->formattype = FORMAT_WaveFormatEx;
        pmt->cbFormat = sizeof(WAVEFORMATEX);
	    WAVEFORMATEX* wfe = (WAVEFORMATEX*)CoTaskMemAlloc(pmt->cbFormat);
	    memset(wfe, 0, sizeof(WAVEFORMATEX));
	    wfe->cbSize = 0;
        if(GetParamBool(USESPDIF))
        {
	        wfe->wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
        }
        else
        {
	        wfe->wFormatTag = WAVE_FORMAT_PCM;

        }
	    wfe->nChannels = 2;
	    wfe->wBitsPerSample = 16;
	    wfe->nSamplesPerSec = 48000;
	    wfe->nBlockAlign = wfe->nChannels*wfe->wBitsPerSample/8;
	    wfe->nAvgBytesPerSec = wfe->nSamplesPerSec*wfe->nBlockAlign;
        pmt->pbFormat = (BYTE*)wfe;
        return S_OK;
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }
}


HRESULT CAudioDecoder::Activate()
{
	m_a52_state = liba52::a52_init(MM_ACCEL_DJBFFT);

    m_dts_state = libdts::dts_init(0);

	libmad::mad_stream_init(&m_stream);
	libmad::mad_frame_init(&m_frame);
	libmad::mad_synth_init(&m_synth);
	mad_stream_options(&m_stream, 0);

    m_rtNextFrameStart = _I64_MIN;

	return S_OK;
}

HRESULT CAudioDecoder::Deactivate()
{
	liba52::a52_free(m_a52_state);

    libdts::dts_free(m_dts_state);

	mad_synth_finish(&m_synth);
	libmad::mad_frame_finish(&m_frame);
	libmad::mad_stream_finish(&m_stream);

    m_IsDiscontinuity = false; 

    m_sample_max = 0.1f; 
    return S_OK;
}

HRESULT CAudioDecoder::Flush(CDSBasePin* pPin)
{
    m_buff.resize(0);
    return S_OK;
}

HRESULT CAudioDecoder::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
	    LOG(DBGLOG_FLOW, ("New Segment %010I64d - %010I64d  @ %f\n", tStart, tStop, dRate));
	    m_buff.resize(0);
        m_sample_max = 0.1f;
        return S_OK;
	}
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CAudioDecoder::SendOutLastSamples(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CAudioDecoder::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CAudioDecoder::GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText)
{
    if(dwParamIndex == SPEAKERCONFIG)
    {
        return GetEnumTextSpeakerConfig(ppwchText);
    }
    else
    {
        return E_NOTIMPL;
    }
}


HRESULT CAudioDecoder::GetEnumTextSpeakerConfig(WCHAR **ppwchText)
{
    wchar_t SpeakerText[] = L"Speaker Config\0" L"None\0" L"Stereo\0" L"Dolby Stereo\0" L"2 Front 2 Rear\0" L"3 Front 2 Rear\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(SpeakerText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, SpeakerText, sizeof(SpeakerText));
    return S_OK;
}


HRESULT CAudioDecoder::GetOutputSampleAndPointer(IMediaSample** pOut, BYTE** ppDataOut, DWORD Len)
{
    HRESULT hr = ReconnectOutput(Len);
    CHECK(hr);

    *ppDataOut = NULL;

    hr = m_AudioOutPin->GetOutputSample(pOut, NULL, NULL, m_IsDiscontinuity);
    CHECK(hr);

    hr = (*pOut)->GetPointer(ppDataOut);
    CHECK(hr);

    hr = (*pOut)->SetActualDataLength(Len);
    CHECK(hr);
    
    return hr;
}
