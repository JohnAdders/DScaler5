///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder_PCM.cpp,v 1.1 2004-02-25 17:14:02 adcockj Exp $
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

    DWORD len = m_buff.size() & ~(wfein->nChannels*wfein->wBitsPerSample/8-1);

    SI(IMediaSample) pOut;
    BYTE* pDataOut = NULL;

    hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, len);
    CHECK(hr);

    memcpy(pDataOut, &m_buff[0], len);

    if(len < m_buff.size())
    {
    	memmove(&m_buff[0], &m_buff[len], m_buff.size() - len);
	    m_buff.resize(m_buff.size() - len);
    }
    else
    {
        m_buff.resize(0);
    }

    REFERENCE_TIME rtDur = 10000000i64*len / (wfein->nSamplesPerSec * wfein->nBlockAlign);

    return Deliver(pOut.GetNonAddRefedInterface(), rtDur);
}
