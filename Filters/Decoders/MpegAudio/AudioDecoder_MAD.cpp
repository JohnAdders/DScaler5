///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_MAD.cpp,v 1.19 2004-08-04 15:19:25 adcockj Exp $
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
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.18  2004/08/04 13:14:42  adcockj
// Fix for Torjorn's issues with dynamic sample rate changes
//
// Revision 1.17  2004/08/03 08:55:56  adcockj
// Fixes for seeking issues
//
// Revision 1.16  2004/07/29 10:26:54  adcockj
// Fixes for audio issues reported by Laurent
//
// Revision 1.15  2004/07/28 13:59:30  adcockj
// spdif fixes
//
// Revision 1.14  2004/07/27 16:53:21  adcockj
// Some spdif fixes
//
// Revision 1.13  2004/07/26 17:08:13  adcockj
// Force use of fixed size output buffers to work around issues with Wave renderer
//
// Revision 1.12  2004/07/16 15:45:19  adcockj
// Fixed compilation issues under .NET
// Improved (hopefully) handling of negative times and preroll
// Changed name of filter
//
// Revision 1.11  2004/07/07 14:08:10  adcockj
// Improved format change handling to cope with more situations
// Removed tabs
//
// Revision 1.10  2004/07/01 21:16:55  adcockj
// Another load of fixes to recent changes
//
// Revision 1.9  2004/07/01 20:03:08  adcockj
// Silly bugs fixed
//
// Revision 1.8  2004/07/01 16:12:47  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.7  2004/05/10 06:40:26  adcockj
// Fixes for better compatability with PES streams
//
// Revision 1.6  2004/03/25 18:01:30  adcockj
// Fixed issues with downmixing
//
// Revision 1.5  2004/03/05 22:14:25  laurentg
// Padding to make MP3 over S/PDIF working - work only with MP3 48 KHz
//
// Revision 1.4  2004/03/01 15:50:24  adcockj
// Added MPEG over spdif as a option
//
// Revision 1.3  2004/02/29 13:47:47  adcockj
// Format change fixes
// Minor library updates
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

#define BLOCK_SIZE_LAYER1   1536
#define BLOCK_SIZE_LAYER23  4608

using namespace libmad;

CREATE_CONVERT_TO_32(29)
CREATE_CONVERT_TO_24(29)
CREATE_CONVERT_TO_16(29)
CREATE_CONVERT_TO_FLOAT(29)

typedef void (CONV_FUNC)(BYTE*&, long);

static CONV_FUNC* pConvFuncs[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    Convert29ToFloat,
    Convert29To32,
    Convert29To24,
    Convert29To16,
};


HRESULT CAudioDecoder::ProcessMPA()
{
    mad_stream_buffer(&m_stream, &m_buff[0], m_buff.size());
    HRESULT hr = S_OK;

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

        if(m_BufferSizeAtFrameStart <= 0)
        {
            hr = UpdateStartTime();
            CHECK(hr)
        }
        
        short len = m_stream.next_frame - m_stream.this_frame;
        m_BufferSizeAtFrameStart -= len;

        if(GetParamBool(MPEGOVERSPDIF) && m_ConnectedAsSpdif)
        {
            // Send the undecoded byte stream over spdif
            // Code was inspired by mpegspdif by Ilkka Karvinen <ik@iki.fi>
            // the source for mpegspdif can be found at 
            // http://www.cs.tut.fi/~ik/mpegspdif.html
            if(m_InputSampleRate > m_frame.header.samplerate)
                continue;

            long FinalLen = MulDiv(m_frame.header.duration.fraction, m_frame.header.samplerate * 4, MAD_TIMER_RESOLUTION);

            if(m_frame.header.layer == MAD_LAYER_I)
            {
                hr = SendDigitalData(0x0004, len, FinalLen, (char*)m_stream.this_frame);
            }
            else
            {
                hr = SendDigitalData(0x0005, len, FinalLen, (char*)m_stream.this_frame);
            }
            CHECK(hr);
        }
        else
        {
            mad_synth_frame(&m_synth, &m_frame);

            // check that the incoming format is reasonable
            // sometimes we get told we have a lower format than we
            // actually get
            if(m_InputSampleRate > m_synth.pcm.samplerate)
                continue;

            const mad_fixed_t* left_ch   = m_synth.pcm.samples[0];
            const mad_fixed_t* right_ch  = m_synth.pcm.samples[1];

            if(m_CanReconnect && m_synth.pcm.samplerate != m_OutputSampleRate)
            {
                hr = SendOutLastSamples(m_AudioInPin);
                CHECK(hr);
                if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                {
                    hr = CreateInternalIEEEMediaType(m_synth.pcm.samplerate, m_ChannelsRequested, m_ChannelMask);
                }
                else
                {
                    hr = CreateInternalPCMMediaType(m_synth.pcm.samplerate, m_ChannelsRequested, m_ChannelMask, 32);
                }
                m_NeedToAttachFormat = true;
                m_OutputSampleRate = m_synth.pcm.samplerate;
                CHECK(hr);
            }

            CONV_FUNC* pConvFunc = pConvFuncs[m_OutputSampleType];
            int Padding = (m_ChannelsRequested - 2) * m_SampleSize;

            for(unsigned short i = 0; i < m_synth.pcm.length; i++)
            {
                if(m_BytesLeftInBuffer == 0)
                {
                    hr = GetOutputSampleAndPointer();
                    CHECK(hr);
                }

                if(m_synth.pcm.channels == 2)
                {
                    pConvFunc(m_pDataOut, *left_ch++);
                    pConvFunc(m_pDataOut, *right_ch++);
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
                    pConvFunc(m_pDataOut, *left_ch>>1);
                    pConvFunc(m_pDataOut, *left_ch++>>1);
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
                            pConvFunc(m_pDataOut, *left_ch++);
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
                    hr = Deliver();
                    if(hr != S_OK)
                    {
                        return hr;
                    }
                }
            }
        }
    }

    return S_OK;
}

