///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_MAD.cpp,v 1.5 2004-03-05 22:14:25 laurentg Exp $
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

#define	BLOCK_SIZE_LAYER1	1536
#define	BLOCK_SIZE_LAYER23	4608

using namespace libmad;

CREATE_CONVERT_TO_32(29)
CREATE_CONVERT_TO_24(29)
CREATE_CONVERT_TO_16(29)

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
		WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;

        if(GetParamBool(MPEGOVERSPDIF))
        {
            // Send the undecoded byte stream over spdif
            // Code was inspired by mpegspdif by Ilkka Karvinen <ik@iki.fi>
            // the source for mpegspdif can be found at 
            // http://www.cs.tut.fi/~ik/mpegspdif.html
		    if(wfein->nSamplesPerSec > m_frame.header.samplerate)
			    continue;

            HRESULT hr = CreateInternalSPDIFMediaType(m_frame.header.samplerate, 16);
            CHECK(hr);

            DWORD len = m_stream.next_frame - m_stream.this_frame;

	        SI(IMediaSample) pOut;
	        WORD* pDataOut = NULL;

			DWORD block_size = (m_frame.header.layer == MAD_LAYER_I) ? BLOCK_SIZE_LAYER1 : BLOCK_SIZE_LAYER23;

            hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, block_size);
            CHECK(hr);
            
			memset((char*)pDataOut, 0, block_size);
			pDataOut[0] = 0xf872;
			pDataOut[1] = 0x4e1f;
            if(m_frame.header.layer == MAD_LAYER_I)
            {
			    pDataOut[2] = 0x0004;
            }
            else
            {
			    pDataOut[2] = 0x0005;
            }
			pDataOut[3] = len*8;
			_swab((char*)m_stream.this_frame, (char*)&pDataOut[4], len);
            
            REFERENCE_TIME rtDur = 10000000i64 * m_frame.header.duration.fraction / MAD_TIMER_RESOLUTION;

            hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
			if(S_OK != hr)
				return hr;
        }
        else
        {
		    mad_synth_frame(&m_synth, &m_frame);

            // check that the incomming format is reasonable
            // sometimes we get told we have a lower format than we
            // actually get
		    if(wfein->nChannels > m_synth.pcm.channels || wfein->nSamplesPerSec > m_synth.pcm.samplerate)
			    continue;

		    const mad_fixed_t* left_ch   = m_synth.pcm.samples[0];
		    const mad_fixed_t* right_ch  = m_synth.pcm.samples[1];

            HRESULT hr = CreateInternalPCMMediaType(m_synth.pcm.samplerate, m_synth.pcm.channels, 0, 32);
            CHECK(hr);
        
            DWORD len = m_synth.pcm.length*m_synth.pcm.channels*m_SampleSize;


	        SI(IMediaSample) pOut;
	        BYTE* pDataOut = NULL;

            hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), (BYTE**)&pDataOut, len);
            CHECK(hr);

            unsigned short i;

            switch(m_OutputSampleType)
            {
            case OUTSAMPLE_32BIT:
		        for(i = 0; i < m_synth.pcm.length; i++)
		        {
                    int outvalue = Convert29To32(*left_ch++);
			        *pDataOut++ = (BYTE)(outvalue);
			        *pDataOut++ = (BYTE)(outvalue>>8);
			        *pDataOut++ = (BYTE)(outvalue>>16);
			        *pDataOut++ = (BYTE)(outvalue>>24);
			        if(m_synth.pcm.channels == 2)
                    {
                        int outvalue = Convert29To32(*right_ch++);
			            *pDataOut++ = (BYTE)(outvalue);
			            *pDataOut++ = (BYTE)(outvalue>>8);
			            *pDataOut++ = (BYTE)(outvalue>>16);
			            *pDataOut++ = (BYTE)(outvalue>>24);
                    }
		        }
                break;
            case OUTSAMPLE_24BIT:
		        for(i = 0; i < m_synth.pcm.length; i++)
		        {
                    int outvalue = Convert29To24(*left_ch++);
			        *pDataOut++ = (BYTE)(outvalue);
			        *pDataOut++ = (BYTE)(outvalue>>8);
			        *pDataOut++ = (BYTE)(outvalue>>16);
			        if(m_synth.pcm.channels == 2)
                    {
                        outvalue = Convert29To24(*right_ch++);
			            *pDataOut++ = (BYTE)(outvalue);
			            *pDataOut++ = (BYTE)(outvalue>>8);
			            *pDataOut++ = (BYTE)(outvalue>>16);
                    }
		        }
                break;
            case OUTSAMPLE_16BIT:
		        for(i = 0; i < m_synth.pcm.length; i++)
		        {
                    short outvalue = Convert29To16(*left_ch++);
			        *pDataOut++ = (BYTE)(outvalue);
			        *pDataOut++ = (BYTE)(outvalue>>8);
			        if(m_synth.pcm.channels == 2)
                    {
                        outvalue = Convert29To16(*right_ch++);
			            *pDataOut++ = (BYTE)(outvalue);
			            *pDataOut++ = (BYTE)(outvalue>>8);
                    }
		        }
                break;
            }
    
    	    REFERENCE_TIME rtDur = 10000000i64*len/(m_synth.pcm.samplerate*m_synth.pcm.channels*m_SampleSize);

            hr = Deliver(pOut.GetNonAddRefedInterface(), rtDur);
		    if(S_OK != hr)
			    return hr;
        }
	}

	return S_OK;
}

