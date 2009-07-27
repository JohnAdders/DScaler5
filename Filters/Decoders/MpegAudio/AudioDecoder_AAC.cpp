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

using namespace libfaad;

#if defined(LIBAAC_FIXED)

#define FAAD_FMT FAAD_FMT_32BIT
typedef long sample_t;

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

#else

typedef void (CONV_FUNC)(BYTE*&, float);

static CONV_FUNC* pConvFuncs[CAudioDecoder::OUTSAMPLE_LASTONE] =
{
    ConvertFloatToFloat,
    ConvertFloatTo32,
    ConvertFloatTo24,
    ConvertFloatTo16,
};


#define FAAD_FMT FAAD_FMT_FLOAT
typedef float sample_t;

#endif


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

// AAC order : C, L, R, L", R", LFE
// DShow order : L, R, C, LFE, L", R"

const int MAXCHANNELS = 6;
const int chmap[MAXCHANNELS][MAXCHANNELS+1] = {
    // first column tell us if we need to remap
    {  0, },                    // mono
    {  0, },                    // l, r
    {  1, 1, 2, 0, },            // c ,l, r -> l, r, c
    {  1, 1, 2, 0, 3, },        // c, l, r, bc -> l, r, c, bc
    {  1, 1, 2, 0, 3, 4, },        // c, l, r, bl, br -> l, r, c, bl, br
    {  1, 1, 2, 0, 5, 3, 4 }    // c, l, r, bl, br, lfe -> l, r, c, lfe, bl, br
};

HRESULT CAudioDecoder::ProcessAAC()
{
    BYTE* p = &m_buff[0];
    BYTE* base = &m_buff[0];
    size_t insize = m_buff.size();
    HRESULT hr = S_OK;

    faacDecFrameInfo frameInfo;

    while(m_aac_init == false && insize > 0)
    {
        unsigned long samplerate;
        unsigned char channels;

        long done = faacDecInit(m_aac_handle,
                        &m_buff[0],
                        m_buff.size(),
                        &samplerate,
                        &channels);
        if(done >= 0)
        {
            insize -= done;
            p += done;
            m_aac_init = true;
            if(channels > m_ChannelsRequested)
            {
                faacDecConfigurationPtr c = faacDecGetCurrentConfiguration(m_aac_handle);
                c->downMatrix = true;
                faacDecSetConfiguration(m_aac_handle, c);
            }
        }

    }
    while(insize > 0)
    {
        sample_t *samples = (sample_t *)faacDecDecode(m_aac_handle, &frameInfo, &m_buff[0], m_buff.size());

        if (frameInfo.error)
        {
            break;
        }
        p += frameInfo.bytesconsumed;
        insize -= frameInfo.bytesconsumed;

        if (!frameInfo.error && samples)
        {
            int Padding = (m_ChannelsRequested - 2) * m_SampleSize;
            CONV_FUNC* pConvFunc = pConvFuncs[m_OutputSampleType];

            for(unsigned long j = 0; j < frameInfo.samples / frameInfo.channels; j++)
            {
                if(m_BytesLeftInBuffer == 0)
                {
                    hr = GetOutputSampleAndPointer();
                    CHECK(hr);
                }

                if(frameInfo.channels >= 2)
                {
                    pConvFunc(m_pDataOut, *samples++);
                    pConvFunc(m_pDataOut, *samples++);
                    if(Padding > 0)
                    {
                        memset(m_pDataOut, 0, Padding);
                        m_pDataOut += Padding;
                    }
                }
                // else upconvert mono to stereo
                // TODO: note that the volume is halved in a very simplistic way
                // it may be worth improving this if people notice
                else if((m_ChannelMask & SPEAKER_FRONT_CENTER) == 0)
                {
                    pConvFunc(m_pDataOut, *samples / (sample_t)2);
                    pConvFunc(m_pDataOut, *samples++ / (sample_t)2);
                    if(Padding > 0)
                    {
                        memset(m_pDataOut, 0, Padding);
                        m_pDataOut += Padding;
                    }
                }
                // otherwise send mono to centre speaker always third
                else
                {
                    for(int j = 0; j < m_ChannelsRequested; ++j)
                    {
                        if(j == 2)
                        {
                            pConvFunc(m_pDataOut, *samples++);
                        }
                        else
                        {
                            pConvFunc(m_pDataOut, 0);
                        }
                    }
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
            int channelidx = frameInfo.channels-1;
            if(chmap[channelidx][0])
            {
            }
            else
            {
            }
        }
    }

    if(insize > 0)
    {
        memmove(base, p, insize);
    }
    m_buff.resize(insize);

    return S_OK;
}

BOOL CAudioDecoder::IsMediaTypeAAC(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_AAC ||
            pMediaType->subtype == MEDIASUBTYPE_MP4A ||
            pMediaType->subtype == MEDIASUBTYPE_mp4a);
}

void CAudioDecoder::InitAAC()
{
    m_aac_init = false;
    m_aac_handle = libfaad::faacDecOpen();

    faacDecConfigurationPtr c = faacDecGetCurrentConfiguration(m_aac_handle);
    c->outputFormat = FAAD_FMT;
    faacDecSetConfiguration(m_aac_handle, c);

    WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;

    if(wfein->cbSize > 0)
    {
        unsigned long freq;
        unsigned char channels;

        faacDecInit2(m_aac_handle, (BYTE*)(wfein+1), wfein->cbSize, &freq, &channels);

        faacDecConfigurationPtr c = faacDecGetCurrentConfiguration(m_aac_handle);

        c->downMatrix = false;

        switch(GetParamEnum(SPEAKERCONFIG))
        {
        case SPCFG_STEREO:
        case SPCFG_DOLBY:
        case SPCFG_2F2R:
        case SPCFG_2F2R1S:
            if(channels > 2)
            {
                c->downMatrix = true;
            }
            break;
        case SPCFG_3F2R:
            if(channels > 5)
            {
                c->downMatrix = true;
            }
        case SPCFG_3F2R1S:
            if(channels > 6)
            {
                c->downMatrix = true;
            }
            break;
        }

        faacDecSetConfiguration(m_aac_handle, c);

        m_aac_init = true;
    }
}

void CAudioDecoder::FinishAAC()
{
    if(m_aac_handle != NULL)
    {
        libfaad::faacDecClose(m_aac_handle);
        m_aac_handle = NULL;
        m_aac_init = false;
    }
}
