///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_PCM.cpp,v 1.2 2004-03-11 16:51:23 adcockj Exp $
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
// Revision 1.1  2004/02/25 17:14:02  adcockj
// Fixed some timing bugs
// Tidy up of code
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"

HRESULT CAudioDecoder::ProcessLPCM()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_AudioInPin->GetMediaType()->pbFormat;

    HRESULT hr = CreateInternalPCMMediaType(wfein->nSamplesPerSec, wfein->nChannels, 0, wfein->wBitsPerSample);
    CHECK(hr);


	int ThreeFrames;
	DWORD len;
    REFERENCE_TIME rtDur;

    // Each frame in a PCM packet is 1/600th of a second long
    // since that would lead to fractional times we buch them up three at a time
    // to get 1/200th of a second or 5ms units
	ThreeFrames = m_buff.size() / ((wfein->nSamplesPerSec/ 600) * wfein->nChannels*(wfein->wBitsPerSample/8)) / 3;
	if(ThreeFrames == 0)
	{
		return S_OK;
	}
	len = ThreeFrames * 3 *  ((wfein->nSamplesPerSec / 600) * wfein->nChannels * (wfein->wBitsPerSample/8));
    
    // each unit of three frames is 5ms long
	rtDur = 50000 * ThreeFrames;

    SI(IMediaSample) pOut;
    BYTE* pDataOut = NULL;


	if(wfein->wBitsPerSample == 16 && m_SampleSize == 2)
	{
        // if it's 16 bit audio 
        // all we need to do is byte swap
		hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len);
		CHECK(hr);

		_swab((char*)&m_buff[0], (char*)pDataOut, len);
	}
	else if(wfein->wBitsPerSample == 24 && m_SampleSize == 3)
	{
        // if it's 24 bit audio 
        // we need to do unmangle the data
        // and convert to little endian
        // the packing method was discussed in conversations on the ogle list
        // see http://lists.berlios.de/pipermail/ogle-devel/2003-April/000408.html

		ASSERT((len % 12) == 0)
		hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len);
		CHECK(hr);
		for(int i = 0; i < len; i += 12)
		{
			*pDataOut++ = m_buff[i + 8];
			*pDataOut++ = m_buff[i + 1];
			*pDataOut++ = m_buff[i];

			*pDataOut++ = m_buff[i + 9];
			*pDataOut++ = m_buff[i + 3];
			*pDataOut++ = m_buff[i + 2];

			*pDataOut++ = m_buff[i + 10];
			*pDataOut++ = m_buff[i + 5];
			*pDataOut++ = m_buff[i + 4];

			*pDataOut++ = m_buff[i + 11];
			*pDataOut++ = m_buff[i + 7];
			*pDataOut++ = m_buff[i + 6];
		}
	}
	else if(wfein->wBitsPerSample == 24 && m_SampleSize == 2)
	{
        // if it's 24 bit audio 
        // and we're outputting 16 bit audio
        // we need to do unmangle the data
        // and convert to little endian
        // \todo add dither

		hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len * 2  / 3);
		CHECK(hr);

		for(int i = 0; i < len; i += 12)
		{
			*pDataOut++ = m_buff[i + 1];
			*pDataOut++ = m_buff[i];

			*pDataOut++ = m_buff[i + 3];
			*pDataOut++ = m_buff[i + 2];

			*pDataOut++ = m_buff[i + 5];
			*pDataOut++ = m_buff[i + 4];

			*pDataOut++ = m_buff[i + 7];
			*pDataOut++ = m_buff[i + 6];
		}
	}
	else
	{
		return E_UNEXPECTED;
	}

    if(len < m_buff.size())
    {
    	memmove(&m_buff[0], &m_buff[len], m_buff.size() - len);
	    m_buff.resize(m_buff.size() - len);
    }
    else
    {
        m_buff.resize(0);
    }

    return Deliver(pOut.GetNonAddRefedInterface(), rtDur);
}
