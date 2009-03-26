///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 John Adcock
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

#include "stdafx.h"
#include "AudioDecoder.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "Convert.h"
#include "MoreUuids.h"

using namespace liba52;

static struct scmap_t
{
    WORD nChannels;
    BYTE ch[6];
    DWORD dwChannelMask;
}
s_scmap_ac3[2*11] =
{
    {2, {0, 1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT},    // A52_CHANNEL
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

    {3, {1, 2, 0,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},  // A52_CHANNEL|A52_LFE
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

typedef void (CONV_FUNC16)(BYTE*&, short);

static CONV_FUNC16* pConvFuncs16[CAudioDecoder::OUTSAMPLE_LASTONE] =
{
    Convert16ToFloat,
    Convert16To32,
    Convert16To24,
    Convert16To16,
};

typedef void (CONV_FUNC_FLOAT)(BYTE*&, float);

static CONV_FUNC_FLOAT* pConvFuncsFloat[CAudioDecoder::OUTSAMPLE_LASTONE] =
{
    ConvertFloatToFloat,
    ConvertFloatTo32,
    ConvertFloatTo24,
    ConvertFloatTo16,
};

int ea52_syncinfo (uint8_t* buf, int* flags, int* sample_rate, int* bit_rate, bool* isEAC3)
{
    static int rate[] = { 32,  40,  48,  56,  64,  80,  96, 112,
             128, 160, 192, 224, 256, 320, 384, 448,
             512, 576, 640};
    static uint8_t lfeon[8] = {0x10, 0x10, 0x04, 0x04, 0x04, 0x01, 0x04, 0x01};
    int frmsizecod;
    int bitrate;
    int half;
    int acmod;

    if ((buf[0] != 0x0b) || (buf[1] != 0x77))    /* syncword */
    return 0;

    int bsid = buf[5] >> 3;

    if (bsid > 10)
    {
        *isEAC3 = true;
        int fscod = buf[4] >> 6;
        int numblkscod = 3;
        if(fscod == 0x03)
        {
            int fscod2 = (buf[4] >> 4) & 0x03;
            switch(fscod2)
            {
            case 0x00:
                *sample_rate = 24000;
                break;
            case 0x01:
                *sample_rate = 22050;
                break;
            case 0x02:
                *sample_rate = 16000;
                break;
            case 0x03:
                return 0;
            }
        }
        else
        {
            switch(fscod)
            {
            case 0x00:
                *sample_rate = 48000;
                break;
            case 0x01:
                *sample_rate = 44100;
                break;
            case 0x02:
                *sample_rate = 32000;
                break;
            case 0x03:
                return 0;
            }
            numblkscod = (buf[4] >> 4) & 0x03;
        }

        int numblks;
        switch(numblkscod)
        {
        case 0:
            numblks = 1;
            break;
        case 1:
            numblks = 2;
            break;
        case 2:
            numblks = 3;
            break;
        case 3:
            numblks = 6;
            break;
        }
        acmod = (buf[4] >> 1) & 0x07;
        *flags = acmod | ((buf[4] & 0x01) ? A52_LFE : 0);

        return (((buf[2] & 0x07) << 8) + buf[3] + 1) * 2;
    }
    else
    {
        *isEAC3 = false;

        int bsmod = (buf[5] & 0x07);
        // ignore other audio modes
        if(bsmod != 0)
        {
            return 0;
        }


        // support bsid as half modes
        // note this is not descriobed in standard
        if(bsid > 8)
        {
            half = bsid - 8;
        }
        else
        {
            half = 0;
        }

        /* acmod, dsurmod and lfeon */
        acmod = buf[6] >> 5;
        *flags = ((((buf[6] & 0xf8) == 0x50) ? A52_DOLBY : acmod) |
              ((buf[6] & lfeon[acmod]) ? A52_LFE : 0));

        frmsizecod = buf[4] & 63;
        if (frmsizecod >= 38)
        return 0;
        bitrate = rate [frmsizecod >> 1];
        *bit_rate = (bitrate * 1000) >> half;

        switch (buf[4] & 0xc0)
        {
        case 0:
            *sample_rate = 48000 >> half;
            return 4 * bitrate;
        case 0x40:
            *sample_rate = 44100 >> half;
            return 2 * (320 * bitrate / 147 + (frmsizecod & 1));
        case 0x80:
            *sample_rate = 32000 >> half;
            return 6 * bitrate;
        default:
            return 0;
        }
    }
}


HRESULT CAudioDecoder::ProcessAC3()
{
    BYTE* p = &m_buff[0];
    BYTE* base = p;
    BYTE* end = p + m_buff.size();
    HRESULT hr = S_OK;

    while(end - p >= 7)
    {
        int size = 0, sample_rate, bit_rate;

        int flags(0);
        bool isEAC3(false);

        if((size = ea52_syncinfo(p, &flags, &sample_rate, &bit_rate, &isEAC3)) > 0)
        {
            bool fEnoughData = p + size <= end;

            if(fEnoughData)
            {
                LOG(DBGLOG_ALL, ("size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n", size, flags, sample_rate, bit_rate));

                if(m_BufferSizeAtFrameStart <= 0)
                {
                    hr = UpdateStartTime();
                    CHECK(hr)
                }

                if(isEAC3)
                {
                    if(m_CodecContext == NULL)
                    {
                        m_Codec = ffmpeg::avcodec_find_decoder(ffmpeg::CODEC_ID_AC3);
                        if(m_Codec == NULL)
                        {
                            return E_UNEXPECTED;
                        }
                        m_CodecContext = ffmpeg::avcodec_alloc_context();
                        m_CodecContext->sample_rate = sample_rate;
                        if(GetParamBool(DYNAMICRANGECONTROL))
                        {
                            //m_CodecContext->drc_scale = 1.0;
                        }
                        else
                        {
                            //m_CodecContext->drc_scale = 0.0;
                        }
                        int scmapidx = min(flags&A52_CHANNEL_MASK, countof(s_scmap_ac3)/2);
                        scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(countof(s_scmap_ac3)/2):0)];
                        switch(GetParamEnum(SPEAKERCONFIG))
                        {
                        case SPCFG_STEREO:
                        case SPCFG_DOLBY:
                            m_CodecContext->request_channels = min(scmap.nChannels, 2);
                            break;
                        case SPCFG_2F2R:
                            m_CodecContext->request_channels = min(scmap.nChannels, 4);
                            break;
                        case SPCFG_2F2R1S:
                            m_CodecContext->request_channels = min(scmap.nChannels, 5);
                            break;
                        case SPCFG_3F2R:
                            m_CodecContext->request_channels = min(scmap.nChannels, 5);
                            break;
                        case SPCFG_3F2R1S:
                            m_CodecContext->request_channels = min(scmap.nChannels, 6);
                            break;
                        }

                        if (ffmpeg::avcodec_open(m_CodecContext, m_Codec) < 0)
                        {
                            av_free(m_CodecContext);
                            m_CodecContext = NULL;
                            m_Codec = NULL;
                            return E_UNEXPECTED;
                        }
                    }
                    int frameSize(AVCODEC_MAX_AUDIO_FRAME_SIZE);
                    static std::vector<int16_t> samples(AVCODEC_MAX_AUDIO_FRAME_SIZE / 2 + 16);
                    int16_t* pSamples = (int16_t*)(((DWORD)&samples[0] + 15) & 0xfffffFF0);

                    int decodedBytes =  ffmpeg::avcodec_decode_audio2(m_CodecContext, pSamples, &frameSize, p, size);
                    if(decodedBytes > 0)
                    {
                        if(m_CodecContext->sample_fmt == ffmpeg::SAMPLE_FMT_FLT)
                        {
                            CONV_FUNC_FLOAT* pConvFunc = pConvFuncsFloat[m_OutputSampleType];
                            float* pfSamples = (float*)pSamples;
                            for(int j = 0; j < frameSize / 4 / m_CodecContext->request_channels; j++)
                            {
                                if(m_BytesLeftInBuffer == 0)
                                {
                                    hr = GetOutputSampleAndPointer();
                                    CHECK(hr);
                                }

                                for(int ch = 0; ch < m_CodecContext->request_channels; ch++)
                                {
                                    pConvFunc(m_pDataOut, *pfSamples++);
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
                        else
                        {
                            CONV_FUNC16* pConvFunc = pConvFuncs16[m_OutputSampleType];
                            for(int j = 0; j < frameSize / 2 / m_CodecContext->request_channels; j++)
                            {
                                if(m_BytesLeftInBuffer == 0)
                                {
                                    hr = GetOutputSampleAndPointer();
                                    CHECK(hr);
                                }

                                for(int ch = 0; ch < m_CodecContext->request_channels; ch++)
                                {
                                    pConvFunc(m_pDataOut, *pSamples++);
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
                else
                {
                    if(m_a52_state == NULL)
                    {
                        m_a52_state = liba52::a52_init(MM_ACCEL_DJBFFT);
                    }

                    if(m_ConnectedAsSpdif)
                    {
                        hr = SendDigitalData(0x0001, size, 0x1800, (char*)p);
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

BOOL CAudioDecoder::IsMediaTypeAC3(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_DOLBY_AC3 ||
            pMediaType->subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3);
}

void CAudioDecoder::InitAC3()
{
}

void CAudioDecoder::FinishAC3()
{
    if(m_a52_state != NULL)
    {
        liba52::a52_free(m_a52_state);
        m_a52_state = NULL;
    }

    if(m_CodecContext != NULL)
    {
        avcodec_close(m_CodecContext);
        av_free(m_CodecContext);
        m_Codec = NULL;
        m_CodecContext = NULL;
    }
}
