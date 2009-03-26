///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2003 Gabest
//  http://www.gabest.org
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

#include "stdafx.h"
#include "AudioDecoder.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "Convert.h"
#include "MoreUuids.h"

using namespace libdts;

static struct scmap_t
{
    WORD nChannels;
    BYTE ch[6];
    DWORD dwChannelMask;
}
s_scmap_dts[2*10] =
{
           {1, {0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER}, // DTS_MONO
           {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT},     // DTS_CHANNEL
           {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT}, // DTS_STEREO
           {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT}, // DTS_STEREO_SUMDIFF
           {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT}, // DTS_STEREO_TOTAL
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

static const sample_t Silence[256] = {
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

#if defined(LIBDTS_FIXED)

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

#elif defined(LIBDTS_DOUBLE)

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

HRESULT CAudioDecoder::ProcessDTS()
{
    BYTE* p = &m_buff[0];
    BYTE* base = p;
    BYTE* end = p + m_buff.size();
    HRESULT hr = S_OK;

    while(end - p >= 14)
    {
       int size = 0, flags, sample_rate, bit_rate, frame_length;

        if((size = dts_syncinfo(m_dts_state, p, &flags, &sample_rate, &bit_rate, &frame_length)) > 0)
        {
            bool fEnoughData = p + size <= end;

            if(fEnoughData)
            {
                LOG(DBGLOG_ALL, ("dts: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d, frame_length=%d\n", size, flags, sample_rate, bit_rate, frame_length));

                if(m_BufferSizeAtFrameStart <= 0)
                {
                    hr = UpdateStartTime();
                    CHECK(hr)
                }

                if(m_ConnectedAsSpdif)
                {
                    int len = frame_length * 4;

                    if(frame_length == 512)
                    {
                        hr = SendDigitalData(0x000b, size, len, (char*)p);
                    }
                    else if(frame_length == 1024)
                    {
                        hr = SendDigitalData(0x000c, size, len, (char*)p);
                    }
                    else
                    {
                        hr = SendDigitalData(0x000d, size, len, (char*)p);
                    }
                    if(hr != S_OK)
                    {
                        return hr;
                    }
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
                    case SPCFG_2F2R1S:
                        flags = DTS_2F2R | DTS_LFE;
                        break;
                    case SPCFG_3F2R:
                        flags = DTS_3F2R;
                        break;
                    case SPCFG_3F2R1S:
                        flags = DTS_3F2R | DTS_LFE;
                        break;
                    }

                    flags |= DTS_ADJUST_LEVEL;

                    sample_t level = 1, gain = 1, bias = 0;

                    if(dts_frame(m_dts_state, p, &flags, &level, bias) == 0)
                    {
                        if(!GetParamBool(DYNAMICRANGECONTROL))
                            dts_dynrng(m_dts_state, NULL, NULL);

                        int scmapidx = min(flags&DTS_CHANNEL_MASK, countof(s_scmap_dts)/2);
                        scmap_t& scmap = s_scmap_dts[scmapidx + ((flags&DTS_LFE)?(countof(s_scmap_dts)/2):0)];

                        int blocks = dts_blocks_num(m_dts_state);

                        int i = 0;
                        CONV_FUNC* pConvFunc = pConvFuncs[m_OutputSampleType];

                        for(; i < blocks && dts_block(m_dts_state) == 0; i++)
                        {
                            sample_t* samples = dts_samples(m_dts_state);
                            const sample_t* Channels[6] = { Silence, Silence, Silence, Silence, Silence, Silence, };
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

                            for(int j = 0; j < 256; j++)
                            {
                                if(m_BytesLeftInBuffer == 0)
                                {
                                    hr = GetOutputSampleAndPointer();
                                    CHECK(hr);
                                }

                                for(int ch = 0; ch < m_ChannelsRequested; ch++)
                                {
                                    pConvFunc(m_pDataOut, Channels[ch][j]);
                                }

                                m_BytesLeftInBuffer -= m_ChannelsRequested * m_SampleSize;
                                ASSERT(m_BytesLeftInBuffer >=0);
                                if(m_BytesLeftInBuffer == 0)
                                {
                                    hr = Deliver(false);
                                    if(hr != S_OK)
                                    {
                                        return hr;
                                    }
                                }
                            }
                        }
                    }
                }
                p += size;
                m_BufferSizeAtFrameStart -= size;
            }

            if(!fEnoughData)
                break;
        }
        else
        {
            p++;
        }
    }

    if(end - p > 0)
    {
        memmove(base, p, end - p);
    }
    m_buff.resize(end - p);

    return hr;
}

BOOL CAudioDecoder::IsMediaTypeDTS(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_DTS ||
            pMediaType->subtype == MEDIASUBTYPE_WAVE_DTS);
}

void CAudioDecoder::InitDTS()
{
    m_dts_state = libdts::dts_init(0);
}

void CAudioDecoder::FinishDTS()
{
    if(m_dts_state != NULL)
    {
        libdts::dts_free(m_dts_state);
        m_dts_state = NULL;
    }
}
