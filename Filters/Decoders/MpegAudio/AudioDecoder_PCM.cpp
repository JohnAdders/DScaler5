///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_PCM.cpp,v 1.5 2004-07-07 14:08:10 adcockj Exp $
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
// Revision 1.4  2004/07/01 21:16:55  adcockj
// Another load of fixes to recent changes
//
// Revision 1.3  2004/07/01 16:12:47  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.2  2004/03/11 16:51:23  adcockj
// Improve LPCM support
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

typedef void (CONV_FUNC16)(BYTE*&, short);



static CONV_FUNC16* pConvFuncs16[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    Convert16ToFloat,
    Convert16To32,
    Convert16To24,
    Convert16To16,
};


CREATE_CONVERT_TO_16(24);

typedef void (CONV_FUNC24)(BYTE*&, long);


static CONV_FUNC24* pConvFuncs24[CAudioDecoder::OUTSAMPLE_LASTONE] = 
{
    Convert24ToFloat,
    Convert24To32,
    Convert24To24,
    Convert24To16,
};


HRESULT CAudioDecoder::ProcessLPCM()
{
    WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;

    // Each frame in a PCM packet is 1/600th of a second long
    // since that would lead to fractional times we bunch them up three at a time
    // to get 1/200th of a second or 5ms units
    int ThreeFrames = m_buff.size() / ((wfein->nSamplesPerSec / 600) * wfein->nChannels * (wfein->wBitsPerSample / 8)) / 3;
    if(ThreeFrames == 0)
    {
        return S_OK;
    }
    DWORD lenOut = ThreeFrames * 3 *  (m_OutputSampleRate/600) * m_ChannelsRequested * m_SampleSize;
    DWORD lenIn = ThreeFrames * 3 *  (wfein->nSamplesPerSec/600) * wfein->nChannels * (wfein->wBitsPerSample / 8);
    
    // each unit of three frames is 5ms long
    REFERENCE_TIME rtDur = 50000 * ThreeFrames;

    SI(IMediaSample) pOut;
    BYTE* pDataOut = NULL;

    HRESULT hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, lenOut);
    CHECK(hr);

    int Padding = (m_ChannelsRequested - 2) * m_SampleSize;

    if(m_DownSample && m_OutputSampleRate != wfein->nSamplesPerSec)
    {
        if(wfein->wBitsPerSample == 16)
        {
            CONV_FUNC16* pConvFunc = pConvFuncs16[m_OutputSampleType];

            for(int i = 0; i < lenIn; i+=8)
            {
                static short lastLeft = 0;
                static short lastRight = 0;
                short left1 = (m_buff[i] << 8) + m_buff[i + 1];
                short right1 = (m_buff[i + 2] << 8) + m_buff[i + 3];
                short left2 = (m_buff[i] << 8) + m_buff[i + 1];
                short right2 = (m_buff[i + 2] << 8) + m_buff[i + 3];

                pConvFunc(pDataOut, (lastLeft + 2 * left1 + left2) >> 2);
                pConvFunc(pDataOut, (lastRight + 2 * right1 + right2) >> 2);
                lastLeft = left2;
                lastRight = right2;
                
                if(Padding > 0)
                {
                    memset(pDataOut, 0, Padding);
                    pDataOut += Padding;
                }
            }
        }
        else if(wfein->wBitsPerSample == 24)
        {
            CONV_FUNC24* pConvFunc = pConvFuncs24[m_OutputSampleType];
            for(int i = 0; i < lenIn; i += 12)
            {
                // if it's 24 bit audio 
                // we need to do unmangle the data
                // the packing method was discussed in conversations on the ogle list
                // see http://lists.berlios.de/pipermail/ogle-devel/2003-April/000408.html
                static long lastLeft = 0;
                static long lastRight = 0;

                long left1 = (m_buff[i] << 16) + (m_buff[i + 1] << 8) + m_buff[i + 8];
                long right1 = (m_buff[i + 2] << 16) + (m_buff[i + 3] << 8) + m_buff[i + 9];
                long left2 = (m_buff[i + 4] << 16) + (m_buff[i + 5] << 8) + m_buff[i + 10];
                long right2 = (m_buff[i + 6] << 16) + (m_buff[i + 7] << 8) + m_buff[i + 11];
            
                pConvFunc(pDataOut, (lastLeft + 2 * left1 + left2) >> 2);
                pConvFunc(pDataOut, (lastRight + 2 * right1 + right2) >> 2);

                if(Padding > 0)
                {
                    memset(pDataOut, 0, Padding);
                    pDataOut += Padding;
                }
            }
        }
        else
        {
            return E_UNEXPECTED;
        }
    }
    else
    {
        if(wfein->wBitsPerSample == 16)
        {
            CONV_FUNC16* pConvFunc = pConvFuncs16[m_OutputSampleType];
            for(int i = 0; i < lenIn; i+=4)
            {
                short left = (m_buff[i] << 8) + m_buff[i + 1];
                short right = (m_buff[i + 2] << 8) + m_buff[i + 3];
                pConvFunc(pDataOut, left);
                pConvFunc(pDataOut, right);
                
                if(Padding > 0)
                {
                    memset(pDataOut, 0, Padding);
                    pDataOut += Padding;
                }
            }
        }
        else if(wfein->wBitsPerSample == 24)
        {
            CONV_FUNC24* pConvFunc = pConvFuncs24[m_OutputSampleType];
            for(int i = 0; i < lenIn; i += 12)
            {
                // if it's 24 bit audio 
                // we need to do unmangle the data
                // and convert to little endian
                // the packing method was discussed in conversations on the ogle list
                // see http://lists.berlios.de/pipermail/ogle-devel/2003-April/000408.html

                long left1 = (m_buff[i] << 24) + (m_buff[i + 1] << 16) + (m_buff[i + 8] << 8);
                long right1 = (m_buff[i + 2] << 24) + (m_buff[i + 3] << 16) + (m_buff[i + 9] << 8);
                long left2 = (m_buff[i + 4] << 24) + (m_buff[i + 5] << 16) + (m_buff[i + 10] << 8);
                long right2 = (m_buff[i + 6] << 24) + (m_buff[i + 7] << 16) + (m_buff[i + 11] << 8);
            
                pConvFunc(pDataOut, left1 >> 8);
                pConvFunc(pDataOut, right1 >> 8);

                if(Padding > 0)
                {
                    memset(pDataOut, 0, Padding);
                    pDataOut += Padding;
                }


                pConvFunc(pDataOut, left2 >> 8);
                pConvFunc(pDataOut, right2 >> 8);

                if(Padding > 0)
                {
                    memset(pDataOut, 0, Padding);
                    pDataOut += Padding;
                }
            }
        }
        else
        {
            return E_UNEXPECTED;
        }
    }

    if(lenIn < m_buff.size())
    {
        memmove(&m_buff[0], &m_buff[lenIn], m_buff.size() - lenIn);
        m_buff.resize(m_buff.size() - lenIn);
    }
    else
    {
        m_buff.resize(0);
    }

    return Deliver(pOut.GetNonAddRefedInterface(), rtDur, rtDur);
}
