///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_A52.cpp,v 1.6 2004-07-01 16:12:47 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2004 John Adcock
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
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.5  2004/04/08 19:02:44  adcockj
// Zero out unused memory when using spdif
//
// Revision 1.4  2004/04/06 16:46:11  adcockj
// DVD Test Annex Compatability fixes
//
// Revision 1.3  2004/03/25 18:01:30  adcockj
// Fixed issues with downmixing
//
// Revision 1.2  2004/02/27 17:04:38  adcockj
// Added support for fixed point libraries
// Added dither to 16 conversions
// Changes to support library fixes
//
// Revision 1.1  2004/02/25 17:14:02  adcockj
// Fixed some timing bugs
// Tidy up of code
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "Convert.h"

using namespace liba52;

static struct scmap_t
{
	WORD nChannels;
	BYTE ch[6];
	DWORD dwChannelMask;
}
s_scmap_ac3[2*11] =
{
	{2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT},	// A52_CHANNEL
	{1, {0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER}, // A52_MONO
	{2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT}, // A52_STEREO
	{3, {0, 2, 1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // A52_3F
	{3, {0, 1, 2,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER}, // A52_2F1R
	{4, {0, 2, 1, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // A52_3F1R
	{4, {0, 1, 2, 3,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_2F2R
	{5, {0, 2, 1, 3, 4,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R
	{1, {0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER}, // A52_CHANNEL1
	{1, {0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER                  }, // A52_CHANNEL2
	{2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT}, // A52_DOLBY

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
};


static sample_t Silence[256] = {
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


#if defined(LIBA52_FIXED)

#define LEVEL (1<<26)
#define BIAS 1

CREATE_CONVERT_TO_32(31)
CREATE_CONVERT_TO_24(31)
CREATE_CONVERT_TO_16(31)
CREATE_CONVERT_TO_FLOAT(31)

typedef void (CONV_FUNC)(BYTE*&, long);

static CONV_FUNC* pConvFuncs[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    Convert31ToFloat,
    Convert31To32,
    Convert31To24,
    Convert31To16,
};

#elif defined(LIBA52_DOUBLE)

#define LEVEL 1
#define BIAS 1

typedef void (CONV_FUNC)(BYTE*&, double);

static CONV_FUNC* pConvFuncs[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    ConvertDoubleToFloat,
    ConvertDoubleTo32,
    ConvertDoubleTo24,
    ConvertDoubleTo16,
};

#else

#define LEVEL 1
#define BIAS 1

typedef void (CONV_FUNC)(BYTE*&, float);

static CONV_FUNC* pConvFuncs[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    ConvertFloatToFloat,
    ConvertFloatTo32,
    ConvertFloatTo24,
    ConvertFloatTo16,
};

#endif


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
			bool fEnoughData = p + size <= end;

			if(fEnoughData)
			{
    			LOG(DBGLOG_ALL, ("size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n", size, flags, sample_rate, bit_rate));

				if(GetParamBool(USESPDIF))
				{
					// go back to SPDIF if we need to
					if(m_CanReconnect && !m_ConnectedAsSpdif)
					{
						HRESULT hr = CreateInternalSPDIFMediaType(sample_rate, 16);
						m_NeedToAttachFormat = true;
						m_ConnectedAsSpdif = true;
						CHECK(hr);
					}

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

                    if((len - size - 8) > 0)
                    {
                        ZeroMemory(pDataOut + size/2 + 4, len - size - 8);
                    }

                    // each frame contains 6 * 256 samples
					REFERENCE_TIME rtDur = 10000000i64 * 6 * 256 / sample_rate;

                    hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur, rtDur);
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
                    case SPCFG_2F2R1S:
                        flags = A52_2F2R | A52_LFE;
                        break;
                    case SPCFG_3F2R:
                        flags = A52_3F2R;
                        break;
                    case SPCFG_3F2R1S:
                        flags = A52_3F2R | A52_LFE;
                        break;
                    }
				    
                    flags += A52_ADJUST_LEVEL;

					sample_t level = LEVEL, gain = 1, bias = 0;
					level *= gain;

					if(a52_frame(m_a52_state, p, &flags, &level, bias) == 0)
					{
						if(!GetParamBool(DYNAMICRANGECONTROL))
							a52_dynrng(m_a52_state, NULL, NULL);

						int scmapidx = min(flags&A52_CHANNEL_MASK, countof(s_scmap_ac3)/2);
                        scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(countof(s_scmap_ac3)/2):0)];

                        SI(IMediaSample) pOut;
                        DWORD len = 6*256*m_ChannelsRequested*m_SampleSize; 
                        BYTE* pDataOut = NULL;

                        HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len);
                        CHECK(hr);
                    
						int i = 0;
                        CONV_FUNC* pConvFunc = pConvFuncs[m_OutputSampleType];

						for(; i < 6 && a52_block(m_a52_state) == 0; i++)
						{
							sample_t* samples = a52_samples(m_a52_state);
                            sample_t* Channels[6] = { Silence, Silence, Silence, Silence, Silence, Silence, };
                            int ch = 0;
							int outch = 0;

                            for(int SpkFlag = 0; SpkFlag < 6; SpkFlag++)
                            {
                                if((scmap.dwChannelMask & (1 << SpkFlag) ) != 0)
                                {
                                    Channels[outch] = samples + 256*scmap.ch[ch];
                                    ch++;
                                }
								if((m_ChannelMask & (1 << SpkFlag) ) != 0)
								{
									outch++;
								}
                            }

                            ASSERT(outch == m_ChannelsRequested);
							ASSERT(ch == scmap.nChannels);
                        
							for(int j = 0; j < 256; j++, samples++)
							{
								for(int ch = 0; ch < m_ChannelsRequested; ch++)
								{
									pConvFunc(pDataOut, Channels[ch][j]);
								}
							}
						}

						if(i == 6)
						{
					        REFERENCE_TIME rtDur = 10000000i64 * len / (m_SampleSize * sample_rate * m_ChannelsRequested); // should be 320000 * 100ns
                            hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur, rtDur);
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
