///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder.cpp,v 1.3 2004-02-17 16:39:59 adcockj Exp $
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

static struct scmap_t
{
	WORD nChannels;
	BYTE ch[6];
	DWORD dwChannelMask;
}
s_scmap_ac3[2*11] =
{
	{2, {0, 1,-1,-1,-1,-1}, 0},	// A52_CHANNEL
	{1, {0,-1,-1,-1,-1,-1}, 0}, // A52_MONO
	{2, {0, 1,-1,-1,-1,-1}, 0}, // A52_STEREO
	{3, {0, 2, 1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // A52_3F
	{3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER}, // A52_2F1R
	{4, {0, 2, 1, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // A52_3F1R
	{4, {0, 1, 2, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_2F2R
	{5, {0, 2, 1, 3, 4,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R
	{1, {0,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL1
	{1, {0,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL2
	{2, {0, 1,-1,-1,-1,-1}, 0}, // A52_DOLBY

	{3, {1, 2, 0,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},	// A52_CHANNEL|A52_LFE
	{2, {1, 0,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_MONO|A52_LFE
	{3, {1, 2, 0,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_STEREO|A52_LFE
	{4, {1, 3, 2, 0,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_3F|A52_LFE
	{4, {1, 2, 0, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // A52_2F1R|A52_LFE
	{5, {1, 3, 2, 0, 4,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // A52_3F1R|A52_LFE
	{5, {1, 2, 0, 3, 4,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_2F2R|A52_LFE
	{6, {1, 3, 2, 0, 4, 5}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R|A52_LFE
	{2, {1, 0,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL1|A52_LFE
	{2, {1, 0,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL2|A52_LFE
	{3, {1, 2, 0,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_DOLBY|A52_LFE
},
s_scmap_dts[2*10] = 
{ 
           {1, {0,-1,-1,-1,-1,-1}, 0}, // DTS_MONO 
           {2, {0, 1,-1,-1,-1,-1}, 0},     // DTS_CHANNEL 
           {2, {0, 1,-1,-1,-1,-1}, 0}, // DTS_STEREO 
           {2, {0, 1,-1,-1,-1,-1}, 0}, // DTS_STEREO_SUMDIFF 
           {2, {0, 1,-1,-1,-1,-1}, 0}, // DTS_STEREO_TOTAL 
           {3, {1, 2, 0,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // DTS_3F 
           {3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER}, // DTS_2F1R 
           {4, {1, 2, 0, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // DTS_3F1R 
           {4, {0, 1, 2, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_2F2R 
           {5, {1, 2, 0, 3, 4,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R 
    
           {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_MONO|DTS_LFE 
           {3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},  // DTS_CHANNEL|DTS_LFE 
           {3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO|DTS_LFE 
           {3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_SUMDIFF|DTS_LFE 
           {3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_TOTAL|DTS_LFE 
           {4, {1, 2, 0, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_3F|DTS_LFE 
           {4, {0, 1, 3, 2,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // DTS_2F1R|DTS_LFE 
           {5, {1, 2, 0, 4, 3,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // DTS_3F1R|DTS_LFE 
           {5, {0, 1, 4, 2, 3,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_2F2R|DTS_LFE 
           {6, {1, 2, 0, 5, 3, 4}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R|DTS_LFE 
 };


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

    m_fDiscontinuity = true;
    m_rtNextFrameStart = _I64_MIN;
    m_rtOutputStart = 0;
    m_sample_max = 0.1;
    m_NeedToAttachFormat = false;

    ZeroMemory(&m_InternalMT, sizeof(AM_MEDIA_TYPE));
    ZeroMemory(&m_InternalWFE, sizeof(WAVEFORMATEXTENSIBLE));
}

CAudioDecoder::~CAudioDecoder()
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::~CAudioDecoder\n"));
}

STDMETHODIMP CAudioDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::GetClassID\n"));
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
        // request buffer large enough for 100ms of 6 channel 32 bit audio
	    pProperties->cbBuffer = 48000*6*(32/8)/10; 
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

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
	{
        m_fDiscontinuity = true;
		m_buff.resize(0);
        m_sample_max = 0.1f;
		m_rtOutputStart = 0;
	}

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
		m_rtNextFrameStart = pSampleProperties->tStart;
	}
    else
    {
        m_rtNextFrameStart = _I64_MIN;
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

HRESULT CAudioDecoder::ProcessLPCM()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;

    CreateInternalPCMMediaType(wfein->nSamplesPerSec, wfein->nChannels, 0, wfein->wBitsPerSample);

    DWORD len = m_buff.size() & ~(wfein->nChannels*wfein->wBitsPerSample/8-1);

    SI(IMediaSample) pOut;
    BYTE* pDataOut = NULL;

    HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len);
    CHECK(hr);

    memcpy(pDataOut, &m_buff[0], len);

    if(len < m_buff.size())
    {
    	memmove(&m_buff[0], &m_buff[len], m_buff.size() - len);
	    m_buff.resize(m_buff.size() - len);
    }
    else
    {
        m_buff.resize(0);
    }

    REFERENCE_TIME rtDur = 10000000i64*len / (wfein->nSamplesPerSec * wfein->nBlockAlign);

    return Deliver(pOut.GetNonAddRefedInterface(), rtDur);
}

HRESULT CAudioDecoder::ProcessAC3()
{
	BYTE* p = &m_buff[0];
	BYTE* base = p;
	BYTE* end = p + m_buff.size();

	while(end - p >= 7)
	{
		int size = 0, sample_rate, bit_rate;

        int flags(0);

		if((size = a52_syncinfo(p, &flags, &sample_rate, &bit_rate)) > 0)
		{
			LOG(DBGLOG_FLOW, ("size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n", size, flags, sample_rate, bit_rate));

			bool fEnoughData = p + size <= end;

			if(fEnoughData)
			{
				if(GetParamBool(USESPDIF))
				{
                    CreateInternalSPDIFMediaType(sample_rate, 16);
                    
                    DWORD len = 0x1800; 

                    SI(IMediaSample) pOut;
                    WORD* pDataOut = NULL;

                    HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                    CHECK(hr);

					pDataOut[0] = 0xf872;
					pDataOut[1] = 0x4e1f;
					pDataOut[2] = 0x0001;
					pDataOut[3] = size*8;
					_swab((char*)p, (char*)&pDataOut[4], size);

					REFERENCE_TIME rtDur = 10000000i64 * size*8 / bit_rate; // should be 320000 * 100ns

                    hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
					if(S_OK != hr)
						return hr;
				}
				else
				{
                    switch(GetParamEnum(SPEAKERCONFIG))
                    {
                    case SPCFG_STEREO:
                        flags = A52_STEREO;
                        break;
                    case SPCFG_DOLBY:
                        flags = A52_DOLBY;
                        break;
                    case SPCFG_2F2R:
                        flags = A52_2F2R;
                        break;
                    case SPCFG_3F2R:
                        flags = A52_3F2R;
                        break;
                    }
				    
                    flags += GetParamBool(DECODE_LFE)?A52_LFE:0;
                    flags += A52_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if(a52_frame(m_a52_state, p, &flags, &level, bias) == 0)
					{
						if(!GetParamBool(DYNAMICRANGECONTROL))
							a52_dynrng(m_a52_state, NULL, NULL);

						int scmapidx = min(flags&A52_CHANNEL_MASK, countof(s_scmap_ac3)/2);
                        scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(countof(s_scmap_ac3)/2):0)];

                        CreateInternalIEEEMediaType(sample_rate, scmap.nChannels, scmap.dwChannelMask);

                        SI(IMediaSample) pOut;
                        float* pDataOut = NULL;
                        DWORD len = 6*256*scmap.nChannels*sizeof(float); 

                        HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                        CHECK(hr);
                    

						int i = 0;

						for(; i < 6 && a52_block(m_a52_state) == 0; i++)
						{
							sample_t* samples = a52_samples(m_a52_state);

							for(int j = 0; j < 256; j++, samples++)
							{
								for(int ch = 0; ch < scmap.nChannels; ch++)
								{
									ASSERT(scmap.ch[ch] != -1);
									*pDataOut++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if(i == 6)
						{
					        REFERENCE_TIME rtDur = 10000000i64 * len / (sizeof(float) * sample_rate * scmap.nChannels); // should be 320000 * 100ns
                            hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
						    if(S_OK != hr)
							    return hr;
    					}
    				}
				}

				p += size;
			}

			memmove(base, p, end - p);
			end = base + (end - p);
			p = base;

			if(!fEnoughData)
				break;
		}
		else
		{
			p++;
		}
	}

	m_buff.resize(end - p);

	return S_OK;
}

HRESULT CAudioDecoder::ProcessDTS()
{
	BYTE* p = &m_buff[0];
	BYTE* base = p;
	BYTE* end = p + m_buff.size();

	while(end - p >= 14)
	{
       int size = 0, flags, sample_rate, bit_rate, frame_length; 

		if((size = dts_syncinfo(m_dts_state, p, &flags, &sample_rate, &bit_rate, &frame_length)) > 0) 
		{
			LOG(DBGLOG_FLOW, ("dts: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d, frame_length=%d\n", size, flags, sample_rate, bit_rate, frame_length)); 
    
			bool fEnoughData = p + size <= end;

			if(fEnoughData)
			{
				if(GetParamBool(USESPDIF))
				{
                    CreateInternalSPDIFMediaType(sample_rate, 16);
                
                    DWORD len = 0x800; 

                    SI(IMediaSample) pOut;
                    WORD* pDataOut = NULL;

                    HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                    CHECK(hr);

				    pDataOut[0] = 0xf872;
				    pDataOut[1] = 0x4e1f;
				    pDataOut[2] = 0x000b;
				    pDataOut[3] = size*8;
				    _swab((char*)p, (char*)&pDataOut[4], size);

				    REFERENCE_TIME rtDur = 10000000i64 * size*8 / bit_rate; // should be 106667 * 100 ns

                    hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
				    if(S_OK != hr)
					    return hr;
                }
                else
                {
                    switch(GetParamEnum(SPEAKERCONFIG))
                    {
                    case SPCFG_STEREO:
                        flags = DTS_STEREO;
                        break;
                    case SPCFG_DOLBY:
                        flags = DTS_STEREO;
                        break;
                    case SPCFG_2F2R:
                        flags = DTS_2F2R;
                        break;
                    case SPCFG_3F2R:
                        flags = DTS_3F2R;
                        break;
                    }
				    
                    flags += GetParamBool(DECODE_LFE)?DTS_LFE:0;
                    flags += DTS_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if(dts_frame(m_dts_state, p, &flags, &level, bias) == 0)
					{
						if(!GetParamBool(DYNAMICRANGECONTROL))
							dts_dynrng(m_dts_state, NULL, NULL);

                        int scmapidx = min(flags&DTS_CHANNEL_MASK, countof(s_scmap_dts)/2); 
                        scmap_t& scmap = s_scmap_dts[scmapidx + ((flags&DTS_LFE)?(countof(s_scmap_dts)/2):0)]; 
    
                        int blocks = dts_blocks_num(m_dts_state); 

                        CreateInternalIEEEMediaType(sample_rate, scmap.nChannels, scmap.dwChannelMask);

                        SI(IMediaSample) pOut;
                        float* pDataOut = NULL;
                        DWORD len = blocks*256*scmap.nChannels*sizeof(float);

                        HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                        CHECK(hr);
                    

						int i = 0;

						for(; i < blocks && dts_block(m_dts_state) == 0; i++)
						{
							sample_t* samples = dts_samples(m_dts_state);

							for(int j = 0; j < 256; j++, samples++)
							{
								for(int ch = 0; ch < scmap.nChannels; ch++)
								{
									ASSERT(scmap.ch[ch] != -1);
									*pDataOut++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if(i == blocks)
						{
					        REFERENCE_TIME rtDur = 10000000i64 * len / (sizeof(float) * sample_rate * scmap.nChannels); // should be 320000 * 100ns
                            hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
						    if(S_OK != hr)
							    return hr;
    					}
    				}
                }
				p += size;
			}

			memmove(base, p, end - p);
			end = base + (end - p);
			p = base;

			if(!fEnoughData)
				break;
		}
		else
		{
			p++;
		}
	}

	m_buff.resize(end - p);

	return S_OK;
}

static inline int scaleto24(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 24));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 24);
}

static inline int scaleto16(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

HRESULT CAudioDecoder::ProcessMPA()
{
	mad_stream_buffer(&m_stream, &m_buff[0], m_buff.size());

	while(1)
	{
		if(mad_frame_decode(&m_frame, &m_stream) == -1)
		{
			if(m_stream.error == MAD_ERROR_BUFLEN)
			{
				memmove(&m_buff[0], m_stream.this_frame, m_stream.bufend - m_stream.this_frame);
				m_buff.resize(m_stream.bufend - m_stream.this_frame);
				break;
			}

			LOG(DBGLOG_FLOW, ("*m_stream.error == %d\n", m_stream.error));

			if(!MAD_RECOVERABLE(m_stream.error))
				return E_FAIL;
            
            continue;
		}

		mad_synth_frame(&m_synth, &m_frame);

        // check that the incomming format is reasonable
        // sometimes we get told we have a lower format than we
        // actually get
		WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;
		if(wfein->nChannels > m_synth.pcm.channels || wfein->nSamplesPerSec > m_synth.pcm.samplerate)
			continue;

		const mad_fixed_t* left_ch   = m_synth.pcm.samples[0];
		const mad_fixed_t* right_ch  = m_synth.pcm.samples[1];

        CreateInternalPCMMediaType(m_synth.pcm.samplerate, m_synth.pcm.channels, 0, 16);
        
        DWORD len = m_synth.pcm.length*m_synth.pcm.channels*2;

        HRESULT hrReconnect = ReconnectOutput(len);
	    if(FAILED(hrReconnect))
		    return hrReconnect;

	    SI(IMediaSample) pOut;
	    BYTE* pDataOut = NULL;

        HRESULT hr = m_AudioOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), false);
        if(FAILED(hr) || !pOut)
            return E_FAIL;

        hr = pOut->GetPointer(&pDataOut);
        CHECK(hr);
        
    	hr = pOut->SetActualDataLength(len);
        CHECK(hr);

		for(unsigned short i = 0; i < m_synth.pcm.length; i++)
		{
            int outvalue = scaleto16(*left_ch++);
			*pDataOut++ = (BYTE)(outvalue);
			*pDataOut++ = (BYTE)(outvalue>>8);
			//*pDataOut++ = (BYTE)(outvalue>>16);
			if(m_synth.pcm.channels == 2)
            {
                outvalue = scaleto16(*right_ch++);
			    *pDataOut++ = (BYTE)(outvalue);
			    *pDataOut++ = (BYTE)(outvalue>>8);
			    //*pDataOut++ = (BYTE)(outvalue>>16);
            }
		}
    
    	REFERENCE_TIME rtDur = 10000000i64*len/(m_synth.pcm.samplerate*m_synth.pcm.channels*2);

        hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
		if(S_OK != hr)
			return hr;
	}

	return S_OK;
}


void CAudioDecoder::CreateInternalSPDIFMediaType(DWORD nSamplesPerSec, WORD wBitsPerSample)
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
}


void CAudioDecoder::CreateInternalPCMMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask, WORD wBitsPerSample)
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
}

void CAudioDecoder::CreateInternalIEEEMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
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
}

HRESULT CAudioDecoder::Deliver(IMediaSample* pOut, REFERENCE_TIME rtDur)
{
	HRESULT hr = S_OK;

    if(m_rtNextFrameStart != _I64_MIN)
    {
        if(abs((long)(m_rtNextFrameStart - m_rtOutputStart)) > 20000)
        {
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
	    pOut->SetDiscontinuity(m_fDiscontinuity); 
        m_fDiscontinuity = false;

    	hr = m_AudioOutPin->SendSample(pOut);
    }

    m_rtOutputStart += rtDur;
    return hr;
}


HRESULT CAudioDecoder::ReconnectOutput(DWORD Len)
{
	HRESULT hr;

	if(!m_AudioOutPin->m_MemInputPin) return E_NOINTERFACE;

	SI(IMemAllocator) pAllocator;
	if(FAILED(hr = m_AudioOutPin->m_MemInputPin->GetAllocator(pAllocator.GetReleasedInterfaceReference())) || !pAllocator) 
		return hr;

	ALLOCATOR_PROPERTIES props, actual;
	if(FAILED(hr = pAllocator->GetProperties(&props)))
		return hr;


	if(!AreMediaTypesIdentical(&m_InternalMT, m_AudioOutPin->GetMediaType()) || Len > (DWORD)props.cbBuffer)
	{
        if(Len > (DWORD)props.cbBuffer) 
        { 
		    props.cBuffers = 4;
		    props.cbBuffer = Len * 3 / 2;

		    if(FAILED(hr = m_AudioOutPin->m_ConnectedPin->BeginFlush())
		    || FAILED(hr = m_AudioOutPin->m_ConnectedPin->EndFlush())
		    || FAILED(hr = pAllocator->Decommit())
		    || FAILED(hr = pAllocator->SetProperties(&props, &actual))
		    || FAILED(hr = pAllocator->Commit()))
			    return hr;
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
	    wfe->wFormatTag = WAVE_FORMAT_PCM;
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
	m_a52_state = a52_init(MM_ACCEL_DJBFFT);

    m_dts_state = dts_init(0);

	mad_stream_init(&m_stream);
	mad_frame_init(&m_frame);
	mad_synth_init(&m_synth);
	mad_stream_options(&m_stream, 0);
	return S_OK;
}

HRESULT CAudioDecoder::Deactivate()
{
	a52_free(m_a52_state);

    dts_free(m_dts_state);

	mad_synth_finish(&m_synth);
	mad_frame_finish(&m_frame);
	mad_stream_finish(&m_stream);

    m_fDiscontinuity = false; 

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
    if(FAILED(hr))
        return hr;

    *ppDataOut = NULL;

    hr = m_AudioOutPin->GetOutputSample(pOut, false);
    if(FAILED(hr || *pOut == NULL))
        return E_FAIL;

    hr = (*pOut)->GetPointer(ppDataOut);
    CHECK(hr);

    hr = (*pOut)->SetActualDataLength(Len);
    CHECK(hr);
    
    return hr;
}
