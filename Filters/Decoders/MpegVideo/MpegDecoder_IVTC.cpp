///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder_UserData.cpp 4553 2009-03-26 17:07:03Z adcockj $
///////////////////////////////////////////////////////////////////////////////
// MpegVideo.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

// this file is based on the DScaler 5 IVTC mod by kzeuh

#include "stdafx.h"
#include "resource.h"
#include "MpegDecoder.h"

// plan for IVTC
//
// allow support of interlaced fields in VideoData structure
// move across detection loop
// move across similar field detection
// create history of similarity and only detect if pattern is there after no being there
// so under movement only

// so detection gives a IVTC state
// if 1 skip
// if 2 blend with previous

// open questions
// 720p removal of 3:2 judder requires 2 frame history

// design of ivtc object
// reset() start again
// newField(field/frame, isTop)
// newFrame(frame)
// getVideoData

// sequence of 3:2 is
// AA AB BC CC DD
// 0, 1, 2, 3, 4
// where first is top second is bottom
// number is index


#ifdef NEVER
// Inverse Telecine Processor on "hard" 3:2 film content added by Kzeuh
DWORD CMpegDecoder::AreSameFields(CFrameBuffer *fr1, CFrameBuffer *fr2, bool bTop, signed long dwCycleNo)
{
	DWORD dwErrors = 0;
	DWORD dwRowCount = 0;
	DWORD dwSumErrors =0;
	DWORD dwOverflow = 0;
	DWORD dwSumPeaks = 0;
	BYTE *prow1 = NULL;
	BYTE *prow2 = NULL;
	DWORD dwYStep = 0;
	DWORD dwXStep = 0;
	DWORD dwMaxPeaks =0;
	DWORD dwPeak=0;
	float RV_MUL = .40f;
	float RH_MUL = .30f;



	if(dwCycleNo < 0)
	{
		dwXStep = 1;
		dwYStep = 2;

		dwMaxPeaks = 250; 
		dwPeak = 30;
		RV_MUL = .10f;
		RH_MUL = .10f;
	}
	else
	{
		if(dwCycleNo == 0)
		{
			// as long as no sync has been established expect a very accurate match
			dwXStep = 1;
			dwYStep = 2;
			dwMaxPeaks = (int)(100*(1-RV_MUL)*(1-RH_MUL)); // 500;
			dwPeak = 30;
		}
		else
		{
			if(dwCycleNo <=12)
			{
				// during the next two seconds following the sync, expect still a high level
				// of matching to ensure the sync was not made by mystake 
				// (eg. not established on non moving interlaced pictures)
				dwXStep = 4;
				dwYStep = 4;
				dwMaxPeaks = (int)(500*(1-RV_MUL)*(1-RH_MUL)); //500
				dwPeak = 30;
			}
			else
			{
				// after that be tolerant enough to ignore errors made by the encoder, 
				// but not too tolerant to be able to detect obvious video content

				dwXStep = 4;
				dwYStep = 16;
				dwMaxPeaks = (int)(600*(1-RV_MUL)*(1-RH_MUL)); //600
				dwPeak = 30;
			}
		}
		if(m_OutputHeight!=1080 || m_InternalPitch!=1920)
		{
			dwMaxPeaks = (DWORD) ((double) dwMaxPeaks * ((double) m_OutputHeight * (double) m_InternalPitch)/ ((double) 1080 * (double) 1920));
		}
	}


	int rw_v_start=2+((int)(m_OutputHeight*RV_MUL)/2)&0xfffffffe;
	int rw_v_end=m_OutputHeight-rw_v_start;
	int rw_h_start=((int)(m_InternalPitch*RH_MUL)/2)&0xfffffff0;
	int rw_h_end=m_InternalPitch-rw_h_start;



	//for (int i = 2; i < m_OutputHeight; i+= dwYStep)
	for (int i = rw_v_start; i < rw_v_end; i+= dwYStep)
	{

		if(bTop)
		{
			prow1 = fr1->m_Buf[0] + i*m_InternalPitch; 
			prow2 = fr2->m_Buf[0] + i*m_InternalPitch; 
		}
		else
		{
			prow1 = fr1->m_Buf[0] + (i+1)*m_InternalPitch; 
			prow2 = fr2->m_Buf[0] + (i+1)*m_InternalPitch; 
		}
		if(dwXStep >=2)
		{
			if (i % (2 * dwYStep) == 0)
			{
				prow1 =prow1 + dwXStep/2;
				prow2 =prow2 + dwXStep/2;
			}
		}
		

		//for(int j = 0; j< m_InternalPitch; j+=dwXStep)
		for(int j = rw_h_start; j<rw_h_end; j+=dwXStep)
		{
			dwErrors =   abs(*(prow1+j) -  *(prow2+j));
			dwSumErrors += dwErrors;
			if(dwErrors >dwPeak) 
			{
				if(++dwOverflow>dwMaxPeaks) return 2;
				dwSumPeaks+=dwErrors;
			}
		}
		dwRowCount++;
	}

	if(( dwOverflow >10 && ( (dwSumPeaks / dwOverflow)>(dwPeak + 15) ) ) )
	{
	if(dwCycleNo < 0)
		return 1;
	}
	if( ( (double) dwSumErrors / ((double) dwRowCount * (double) m_InternalPitch / (double) dwXStep)) > 10)
		return 2;

	return 0;
}

#endif