///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_DTS.cpp,v 1.2 2004-02-27 17:04:38 adcockj Exp $
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
// Revision 1.1  2004/02/25 17:14:02  adcockj
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
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "Convert.h"

using namespace libdts;

static struct scmap_t
{
	WORD nChannels;
	BYTE ch[6];
	DWORD dwChannelMask;
}
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


#if defined(LIBDTS_FIXED)

#define LEVEL (1<<26)
#define BIAS 1

CREATE_CONVERT_TO_32(31)
CREATE_CONVERT_TO_24(31)
CREATE_CONVERT_TO_16(31)
CREATE_CONVERT_TO_FLOAT(31)

#define ConvertToFloat Convert31ToFloat
#define ConvertTo16 Convert31To16
#define ConvertTo24 Convert31To24
#define ConvertTo32 Convert31To32

#elif defined(LIBDTS_DOUBLE)

#define LEVEL 1
#define BIAS 1

#define ConvertToFloat (float)
#define ConvertTo16 ConvertDoubleTo16
#define ConvertTo24 ConvertDoubleTo24
#define ConvertTo32 ConvertDoubleTo32

#else

#define LEVEL 1
#define BIAS 1

#define ConvertToFloat
#define ConvertTo16 ConvertFloatTo16
#define ConvertTo24 ConvertFloatTo24
#define ConvertTo32 ConvertFloatTo32

#endif

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
                    HRESULT hr = CreateInternalSPDIFMediaType(sample_rate, 16);
                    CHECK(hr);
                
                    DWORD len = 0x800; 

                    SI(IMediaSample) pOut;
                    WORD* pDataOut = NULL;

                    hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                    CHECK(hr);

				    pDataOut[0] = 0xf872;
				    pDataOut[1] = 0x4e1f;
				    pDataOut[2] = 0x000b;
				    pDataOut[3] = size*8;
				    _swab((char*)p, (char*)&pDataOut[4], size);

                    REFERENCE_TIME rtDur = 10000000i64 * size * 8 / bit_rate; // should be 106667 * 100 ns

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

                        HRESULT hr = CreateInternalIEEEMediaType(sample_rate, scmap.nChannels, scmap.dwChannelMask);
                        CHECK(hr);

                        SI(IMediaSample) pOut;
                        float* pDataOut = NULL;
                        DWORD len = blocks*256*scmap.nChannels*m_SampleSize;

                        hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
                        CHECK(hr);

						int i = 0;
                        BYTE*  pbDataOut = (BYTE*)pDataOut;

                        switch(m_OutputSampleType)
                        {
                        case OUTSAMPLE_FLOAT:
						    for(; i < blocks && dts_block(m_dts_state) == 0; i++)
						    {
							    sample_t* samples = dts_samples(m_dts_state);

							    for(int j = 0; j < 256; j++, samples++)
							    {
								    for(int ch = 0; ch < scmap.nChannels; ch++)
								    {
									    ASSERT(scmap.ch[ch] != -1);
									    *pDataOut++ = ConvertToFloat(*(samples + 256*scmap.ch[ch]));
								    }
							    }
						    }
                            break;
                        case OUTSAMPLE_32BIT:
						    for(; i < blocks && dts_block(m_dts_state) == 0; i++)
						    {
							    sample_t* samples = dts_samples(m_dts_state);

							    for(int j = 0; j < 256; j++, samples++)
							    {
								    for(int ch = 0; ch < scmap.nChannels; ch++)
								    {
									    ASSERT(scmap.ch[ch] != -1);
                                        long Sample = ConvertTo32(*(samples + 256*scmap.ch[ch]));
			                            *pbDataOut++ = (BYTE)(Sample);
			                            *pbDataOut++ = (BYTE)(Sample>>8);
			                            *pbDataOut++ = (BYTE)(Sample>>16);
			                            *pbDataOut++ = (BYTE)(Sample>>24);
								    }
							    }
						    }
                            break;
                        case OUTSAMPLE_24BIT:
						    for(; i < blocks && dts_block(m_dts_state) == 0; i++)
						    {
							    sample_t* samples = dts_samples(m_dts_state);

							    for(int j = 0; j < 256; j++, samples++)
							    {
								    for(int ch = 0; ch < scmap.nChannels; ch++)
								    {
									    ASSERT(scmap.ch[ch] != -1);
                                        long Sample = ConvertTo24(*(samples + 256*scmap.ch[ch]));
			                            *pbDataOut++ = (BYTE)(Sample);
			                            *pbDataOut++ = (BYTE)(Sample>>8);
			                            *pbDataOut++ = (BYTE)(Sample>>16);
								    }
							    }
                            }
                            break;
                        case OUTSAMPLE_16BIT:
						    for(; i < blocks && dts_block(m_dts_state) == 0; i++)
						    {
							    sample_t* samples = dts_samples(m_dts_state);

							    for(int j = 0; j < 256; j++, samples++)
							    {
								    for(int ch = 0; ch < scmap.nChannels; ch++)
								    {
									    ASSERT(scmap.ch[ch] != -1);
                                        short Sample = ConvertTo16(*(samples + 256*scmap.ch[ch]));
			                            *pbDataOut++ = (BYTE)(Sample);
			                            *pbDataOut++ = (BYTE)(Sample>>8);
								    }
							    }
                            }
                            break;
                        }

						if(i == blocks)
						{
					        REFERENCE_TIME rtDur = 10000000i64 * len / (m_SampleSize * sample_rate * scmap.nChannels);
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

