///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder.cpp,v 1.1 2004-02-13 12:22:17 adcockj Exp $
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
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"
#include "MoreUuids.h"
#include <mmreg.h>
#include "CPUID.h"

extern HINSTANCE g_hInstance;

static struct scmap_t
{
	WORD nChannels;
	BYTE ch[6];
	DWORD dwChannelMask;
    LPCWSTR pwszDesc;
}
s_scmap[2*11] =
{
	// A52_CHANNEL
    {
        2, 
        {0, 1,-1,-1,-1,-1}, 
        0,
        L"Dual Mono",
    },
	// A52_MONO
    {
        1, 
        {0,-1,-1,-1,-1,-1}, 
        0,
        L"Mono",
    },
	// A52_STEREO
    {
        2, 
        {0, 1,-1,-1,-1,-1}, 
        0,
        L"Stereo",
    },
	// A52_3F
    {
        3, 
        {0, 2, 1,-1,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER,
        L"Left, Right and Center",
    },
	// A52_2F1R
    {
        3, 
        {0, 1, 2,-1,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER,
        L"Left, Right and 1 Rear",
    },
	// A52_3F1R
    {
        4, 
        {0, 2, 1, 3,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER,
        L"Left, Right, Center and 1 Rear",
    },
	// A52_2F2R
    {
        4, 
        {0, 1, 2, 3,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT,
        L"Left, Right and 2 Rears",
    },
	// A52_3F2R
    {
        5, 
        {0, 2, 1, 3, 4,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT,
        L"3 Fronts and 2 Rears",
    },
	// A52_CHANNEL1
    {
        1, 
        {0,-1,-1,-1,-1,-1}, 
        0,
        L"Channel 1",
    },
	// A52_CHANNEL2
    {
        1, 
        {0,-1,-1,-1,-1,-1}, 
        0,
        L"Channel 2",
    },
	// A52_DOLBY
    {
        2, 
        {0, 1,-1,-1,-1,-1}, 
        0,
        L"Dolby Pro-logic Stereo",
    },

	// A52_CHANNEL|A52_LFE
    {
        3, 
        {1, 2, 0,-1,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY
    },	
	// A52_MONO|A52_LFE
    {
        2, 
        {1, 0,-1,-1,-1,-1}, 
        SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY
    }, 
	// A52_STEREO|A52_LFE
    {
        3, 
        {1, 2, 0,-1,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY
    }, 
	// A52_3F|A52_LFE
    {
        4, 
        {1, 3, 2, 0,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY
    }, 
	// A52_2F1R|A52_LFE
    {
        4, 
        {1, 2, 0, 3,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER
    }, 
	// A52_3F1R|A52_LFE
    {
        5, 
        {1, 3, 2, 0, 4,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER
    }, 
	// A52_2F2R|A52_LFE
    {
        5, 
        {1, 2, 0, 3, 4,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT
    }, 
	// A52_3F2R|A52_LFE
    {
        6, 
        {1, 3, 2, 0, 4, 5}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT
    }, 
	// A52_CHANNEL1|A52_LFE
    {
        2, 
        {1, 0,-1,-1,-1,-1}, 
        SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY
    }, 
	// A52_CHANNEL2|A52_LFE
    {
        2, 
        {1, 0,-1,-1,-1,-1}, 
        SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY
    }, 
	// A52_DOLBY|A52_LFE
    {
        3, 
        {1, 2, 0,-1,-1,-1}, 
        SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY
    }, 
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
    m_rtStart = 0;
    m_sample_max = 0.1;
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
	    pProperties->cBuffers = 4;
	    pProperties->cbBuffer = 0x1800;
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

	REFERENCE_TIME rtStart = _I64_MIN, rtStop = _I64_MIN;
	hr = InSample->GetTime(&rtStart, &rtStop);


	if(InSample->IsDiscontinuity() == S_OK)
	{
        m_fDiscontinuity = true;
		m_buff.resize(0);
        m_sample_max = 0.1f;
		ASSERT(SUCCEEDED(hr)); // what to do if not?
		if(FAILED(hr)) return S_OK; // lets wait then...
		m_rtStart = rtStart;
	}

	if(SUCCEEDED(hr) && abs(m_rtStart - rtStart) > 1000000) // +-100ms jitter is allowed for now
	{
		m_buff.resize(0);
		m_rtStart = rtStart;
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

	BYTE* pDataIn = &m_buff[0];
	
    int len = m_buff.size() & ~(wfein->nChannels*wfein->wBitsPerSample/8-1);

	HRESULT hr = Deliver(&m_buff[0], len, wfein);
    
    if(len < m_buff.size())
    {
    	memmove(&m_buff[0], &m_buff[len], m_buff.size() - len);
	    m_buff.resize(m_buff.size() - len);
    }
    else
    {
        m_buff.resize(0);
    }
    return hr;
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
        if(CpuFeatureFlags & FEATURE_MMXEXT)
        {
            flags |= MM_ACCEL_X86_MMXEXT;
        }
        if(CpuFeatureFlags & FEATURE_3DNOW)
        {
            flags |= MM_ACCEL_X86_3DNOW;
        }
        if(CpuFeatureFlags & FEATURE_MMX)
        {
            flags |= MM_ACCEL_X86_MMX;
        }

		if((size = a52_syncinfo(p, &flags, &sample_rate, &bit_rate)) > 0)
		{
			LOG(DBGLOG_FLOW, ("size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n", size, flags, sample_rate, bit_rate));

			bool fEnoughData = p + size <= end;

			if(fEnoughData)
			{
				int iSpeakerConfig = GetParamEnum(SPEAKERCONFIG) + GetParamBool(SPEAKERCONFIG)?A52_DOLBY:0;

				if(GetParamBool(USESPDIF))
				{
					BYTE pBuff[0x1800];

					WORD* pDataOut = (WORD*)pBuff;
					pDataOut[0] = 0xf872;
					pDataOut[1] = 0x4e1f;
					pDataOut[2] = 0x0001;
					pDataOut[3] = size*8;
					_swab((char*)p, (char*)&pDataOut[4], size);

					REFERENCE_TIME rtDur = 10000000i64 * size*8 / bit_rate; // should be 320000 * 100ns

					HRESULT hr;
					if(S_OK != (hr = Deliver(pBuff, 0x1800, sample_rate, rtDur)))
						return hr;
				}
				else
				{
					flags = iSpeakerConfig&(A52_CHANNEL_MASK|A52_LFE);
					flags |= A52_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if(a52_frame(m_a52_state, p, &flags, &level, bias) == 0)
					{
						if(!GetParamBool(DYNAMICRANGECONTROL))
							a52_dynrng(m_a52_state, NULL, NULL);

						int scmapidx = min(flags&A52_CHANNEL_MASK, sizeof(s_scmap)/sizeof(s_scmap[0])/2);
                        scmap_t& scmap = s_scmap[scmapidx + ((flags&A52_LFE)?(sizeof(s_scmap)/sizeof(s_scmap[0])/2):0)];

                        // allocate enough space for 6 channels
						float pBuff[6*256*6];
                        float* p = pBuff;

						int i = 0;

						for(; i < 6 && a52_block(m_a52_state) == 0; i++)
						{
							sample_t* samples = a52_samples(m_a52_state);

							for(int j = 0; j < 256; j++, samples++)
							{
								for(int ch = 0; ch < scmap.nChannels; ch++)
								{
									ASSERT(scmap.ch[ch] != -1);
									*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if(i == 6)
						{
							HRESULT hr;
							if(S_OK != (hr = Deliver(pBuff, 6*256*scmap.nChannels, sample_rate, scmap.nChannels, scmap.dwChannelMask)))
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

static int dts_syncinfo(BYTE* p, int* sample_rate, int* bit_rate)
{
	if(*(DWORD*)p != 0x0180FE7F)
		return 0;

	p += 4;

	int frametype = (p[0]>>7); // 1
	int deficitsamplecount = (p[0]>>2)&31; // 5
	int crcpresent = (p[0]>>1)&1; // 1
	int npcmsampleblocks = ((p[0]&1)<<6)|(p[1]>>2); // 7
	int framebytes = (((p[1]&3)<<12)|(p[2]<<4)|(p[3]>>4)) + 1; // 14
	int audiochannelarrangement = (p[3]&15)<<2|(p[4]>>6); // 6
	int freq = (p[4]>>2)&15; // 4
	int transbitrate = ((p[4]&3)<<3)|(p[5]>>5); // 5

	int freqtbl[] = 
	{
		0,8000,16000,32000,
		0,0,
		11025,22050,44100,
		0,0,
		12000,24000,48000,
		0,0
	};

	int bitratetbl[] = 
	{
		32000,56000,64000,96000,112000,128000,192000,224000,
		256000,320000,384000,448000,512000,576000,640000,754500,
		960000,1024000,1152000,1280000,1344000,1408000,1411200,1472000,
		1509750,1920000,2048000,3072000,3840000,0,0,0
	};

	*sample_rate = freqtbl[freq];
	*bit_rate = bitratetbl[transbitrate];

	return framebytes;
}

HRESULT CAudioDecoder::ProcessDTS()
{
	BYTE* p = &m_buff[0];
	BYTE* base = p;
	BYTE* end = p + m_buff.size();

	while(end - p >= 10) // ?
	{
		int size = 0, sample_rate, bit_rate;

		if((size = dts_syncinfo(p, &sample_rate, &bit_rate)) > 0)
		{
			LOG(DBGLOG_FLOW, ("size=%d, sample_rate=%d, bit_rate=%d\n", size, sample_rate, bit_rate));

			bool fEnoughData = p + size <= end;

			if(fEnoughData)
			{
				BYTE pBuff[0x800];

				WORD* pDataOut = (WORD*)pBuff;
				pDataOut[0] = 0xf872;
				pDataOut[1] = 0x4e1f;
				pDataOut[2] = 0x000b;
				pDataOut[3] = size*8;
				_swab((char*)p, (char*)&pDataOut[4], size);

				REFERENCE_TIME rtDur = 10000000i64 * size*8 / bit_rate; // should be 106667 * 100 ns

				HRESULT hr;
				if(S_OK != (hr = Deliver(pBuff, 0x800, sample_rate, rtDur)))
					return hr;

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
		}

		mad_synth_frame(&m_synth, &m_frame);

		WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;
		if(/*wfein->nChannels != m_synth.pcm.channels ||*/ wfein->nSamplesPerSec != m_synth.pcm.samplerate)
			continue;

		const mad_fixed_t* left_ch   = m_synth.pcm.samples[0];
		const mad_fixed_t* right_ch  = m_synth.pcm.samples[1];

		BYTE* pBuff = new BYTE[m_synth.pcm.length*m_synth.pcm.channels*2];
        BYTE* pDataOut = pBuff;

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

        WAVEFORMATEX wfe;
        wfe.wBitsPerSample = 16;
        wfe.nChannels = m_synth.pcm.channels;
        wfe.nSamplesPerSec = m_synth.pcm.samplerate;
	    wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
	    wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;

		HRESULT hr = Deliver(pBuff, m_synth.pcm.length*m_synth.pcm.channels*2, &wfe);

        delete [] pBuff;

		if(S_OK != hr)
			return hr;
	}

	return S_OK;
}

HRESULT CAudioDecoder::Deliver(float* pBuff, DWORD len, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	HRESULT hr;

	AM_MEDIA_TYPE mt;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = MEDIASUBTYPE_IEEE_FLOAT;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEXTENSIBLE wfex;
	memset(&wfex, 0, sizeof(wfex));
	wfex.Format.nChannels = nChannels;
	wfex.Format.nSamplesPerSec = nSamplesPerSec;
    wfex.Format.wBitsPerSample = 32;
	wfex.Format.nBlockAlign = wfex.Format.nChannels*wfex.Format.wBitsPerSample/8;
	wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec*wfex.Format.nBlockAlign;

	wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
	wfex.dwChannelMask = dwChannelMask;
	wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
	wfex.SubFormat = mt.subtype;

    mt.cbFormat = sizeof(wfex) + wfex.Format.cbSize;
    mt.pbFormat = (BYTE*)&wfex; 

	int nSamples = len/wfex.Format.nChannels;

	if(FAILED(hr = ReconnectOutput(nSamples, mt)))
		return hr;

	SI(IMediaSample) pOut;
	BYTE* pDataOut = NULL;

    // need to preserve hr
    if(FAILED(m_AudioOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), false)) || !pOut)
        return E_FAIL;

    if(FAILED(pOut->GetPointer(&pDataOut)))
		return E_FAIL;

	REFERENCE_TIME rtDur = 10000000i64*nSamples/wfex.Format.nSamplesPerSec;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
    LOG(DBGLOG_FLOW, ("CMpaDecFilter: %I64d - %I64d\n", rtStart/10000, rtStop/10000));
	if(rtStart < 200000 /* < 0, FIXME: 0 makes strange noises */)
		return S_OK;

	if(hr == S_OK)
	{
		m_AudioOutPin->SetType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
    pOut->SetDiscontinuity(m_fDiscontinuity); 
    m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(len * wfex.Format.wBitsPerSample/8);

	float* pDataIn = pBuff;

    // TODO: move this into the audio switcher 
    float sample_mul = 1; 
    BOOL bNorm = GetParamBool(NORMALIZE);
    if(bNorm) 
    { 
        for(int i = 0; i < len; i++) 
        { 
            float f = *pDataIn++; 
            if(f < 0) f = -f; 
            if(m_sample_max < f) m_sample_max = f; 
        } 
        sample_mul = 1.0f / m_sample_max; 
        pDataIn = pBuff; 
    } 

	for(int i = 0; i < len; i++)
	{
		float f = *pDataIn++;

		ASSERT(f >= -1 && f <= 1);

        // TODO: move this into the audio switcher 
        if(bNorm) 
            f *= sample_mul; 

        if(f < -1) f = -1; 
        else if(f > 1) f = 1; 

		*(float*)pDataOut = f;
		pDataOut += sizeof(float);
		break;
	}

	hr = m_AudioOutPin->SendSample(pOut.GetNonAddRefedInterface());
    return hr;
}

HRESULT CAudioDecoder::Deliver(BYTE* pBuff, DWORD len, DWORD nSamplesPerSec, REFERENCE_TIME rtDur)
{
	HRESULT hr;

	AM_MEDIA_TYPE mt;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = MEDIASUBTYPE_PCM;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEX wfe;
	memset(&wfe, 0, sizeof(wfe));
	wfe.wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	wfe.nSamplesPerSec = nSamplesPerSec;
	wfe.nChannels = 2;
	wfe.wBitsPerSample = 16;
	wfe.nBlockAlign = wfe.nChannels * wfe.wBitsPerSample / 8;
	wfe.nAvgBytesPerSec = wfe.nSamplesPerSec * wfe.nBlockAlign;
	wfe.cbSize = 0;

    mt.cbFormat = sizeof(wfe) + wfe.cbSize;
    mt.pbFormat = (BYTE*)&wfe; 

	if(FAILED(hr = ReconnectOutput(len / wfe.nBlockAlign, mt)))
		return hr;

	SI(IMediaSample) pOut;
	BYTE* pDataOut = NULL;

    // need to preserve hr
    if(FAILED(m_AudioOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), false)) || !pOut)
        return E_FAIL;

    if(FAILED(pOut->GetPointer(&pDataOut)))
		return E_FAIL;

	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
    
    //TRACE(_T("CMpaDecFilter: %I64d - %I64d\n"), rtStart/10000, rtStop/10000);
	if(rtStart < 0)
		return S_OK;

	if(hr == S_OK)
	{
		m_AudioOutPin->SetType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity); 
    m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(len);
	memcpy(pDataOut, pBuff, len);

	hr = m_AudioOutPin->SendSample(pOut.GetNonAddRefedInterface());
    return hr;
}

HRESULT CAudioDecoder::Deliver(BYTE* pBuff, DWORD len, WAVEFORMATEX* wfein)
{
	HRESULT hr;

	AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
	mt.majortype = MEDIATYPE_Audio;
    if(wfein->wBitsPerSample == 32)
    {
        mt.subtype = MEDIASUBTYPE_IEEE_FLOAT;
    }
    else
    {
    	mt.subtype = MEDIASUBTYPE_PCM;
    }
	mt.formattype = FORMAT_WaveFormatEx;

    WAVEFORMATEXTENSIBLE wfex;
	memset(&wfex, 0, sizeof(wfex));

	wfex.Format.nSamplesPerSec = wfein->nSamplesPerSec;
	wfex.Format.nChannels = wfein->nChannels;
	wfex.Format.wBitsPerSample = wfein->wBitsPerSample;
	wfex.Format.nBlockAlign = wfein->nChannels * wfein->wBitsPerSample / 8;
	wfex.Format.nAvgBytesPerSec = wfein->nSamplesPerSec * wfein->nBlockAlign;

    if(wfein->wBitsPerSample > 16 || wfein->nChannels > 2)
    {
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
		wfex.SubFormat = mt.subtype;
    }
    else
    {
    	wfex.Format.wFormatTag = (WORD)mt.subtype.Data1;
	    wfex.Format.cbSize = 0;
    }

    mt.cbFormat = sizeof(wfex) + wfex.Format.cbSize;
    mt.pbFormat = (BYTE*)&wfex; 

	if(FAILED(hr = ReconnectOutput(len / wfex.Format.nBlockAlign, mt)))
		return hr;

	SI(IMediaSample) pOut;
	BYTE* pDataOut = NULL;

    // need to preserve hr
    if(FAILED(m_AudioOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), false)) || !pOut)
        return E_FAIL;

    if(FAILED(pOut->GetPointer(&pDataOut)))
		return E_FAIL;

	REFERENCE_TIME rtDur = 10000000i64*len/wfex.Format.nSamplesPerSec/wfex.Format.nBlockAlign;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
    
    //TRACE(_T("CMpaDecFilter: %I64d - %I64d\n"), rtStart/10000, rtStop/10000);
	if(rtStart < 0)
		return S_OK;

	if(hr == S_OK)
	{
		m_AudioOutPin->SetType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity); 
    m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(len);
	memcpy(pDataOut, pBuff, len);

	hr = m_AudioOutPin->SendSample(pOut.GetNonAddRefedInterface());
    return hr;
}

HRESULT CAudioDecoder::ReconnectOutput(int nSamples, AM_MEDIA_TYPE& mt)
{
	HRESULT hr;

	if(!m_AudioOutPin->m_MemInputPin) return E_NOINTERFACE;

	SI(IMemAllocator) pAllocator;
	if(FAILED(hr = m_AudioOutPin->m_MemInputPin->GetAllocator(pAllocator.GetReleasedInterfaceReference())) || !pAllocator) 
		return hr;

	ALLOCATOR_PROPERTIES props, actual;
	if(FAILED(hr = pAllocator->GetProperties(&props)))
		return hr;

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.pbFormat;
	long cbBuffer = nSamples * wfe->nBlockAlign;

	if(!AreMediaTypesIdentical(&mt,m_AudioOutPin->GetMediaType()) || cbBuffer > props.cbBuffer)
	{
        if(cbBuffer > props.cbBuffer) 
        { 
		    props.cBuffers = 4;
		    props.cbBuffer = cbBuffer * 3 / 2;

		    if(FAILED(hr = m_AudioOutPin->m_ConnectedPin->BeginFlush())
		    || FAILED(hr = m_AudioOutPin->m_ConnectedPin->EndFlush())
		    || FAILED(hr = pAllocator->Decommit())
		    || FAILED(hr = pAllocator->SetProperties(&props, &actual))
		    || FAILED(hr = pAllocator->Commit()))
			    return hr;
        }

		return S_OK;
	}

	return S_FALSE;
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
	m_a52_state = a52_init(0);

	mad_stream_init(&m_stream);
	mad_frame_init(&m_frame);
	mad_synth_init(&m_synth);
	mad_stream_options(&m_stream, 0);
	return S_OK;
}

HRESULT CAudioDecoder::Deactivate()
{
	a52_free(m_a52_state);

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
    int i;
    int ExtraTextLen(0);

    for(i = 0; i <= A52_DOLBY; ++i)
    {
        ExtraTextLen += wcslen(s_scmap[i].pwszDesc) + 1;
    }
    *ppwchText = (WCHAR*)CoTaskMemAlloc(2 * ExtraTextLen + 2 + 20 * 2);
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
	memcpy(*ppwchText, L"Speaker Config\0None\0", 20 * 2);

    LPWSTR pNext = *ppwchText + 20;
    for(i = 0; i <= A52_DOLBY; ++i)
    {
        wcscpy(pNext, s_scmap[i].pwszDesc);
        pNext += wcslen(s_scmap[i].pwszDesc);
        pNext[0] = '\0';
        ++pNext;
    }
    pNext[0] = '\0';
    return S_OK;
}
