///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder_UserData.cpp,v 1.1 2004-02-25 17:14:02 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MpegDecoder.h"



HRESULT CMpegDecoder::ProcessUserData(mpeg2_state_t State, const BYTE* const UserData, int UserDataLen)
{
    HRESULT hr = S_OK;

	if(State == STATE_GOP && UserDataLen > 4 && *(DWORD*)UserData == 0xf8014343
		&& m_CCOutPin->IsConnected())
	{
		SI(IMediaSample) pSample;
		hr = m_CCOutPin->GetOutputSample(pSample.GetReleasedInterfaceReference(), false);
        CHECK(hr);
		BYTE* pData = NULL;
		
        hr = pSample->GetPointer(&pData);
        CHECK(hr);
        if(pData == NULL)
        {
            return E_UNEXPECTED;
        }

		*(DWORD*)pData = 0xb2010000;
		memcpy(pData + 4, UserData, UserDataLen);
		hr = pSample->SetActualDataLength(UserDataLen + 4);
        CHECK(hr);

		hr = m_CCOutPin->SendSample(pSample.GetNonAddRefedInterface());
        CHECK(hr);
	}
    else
    {
	    if(mpeg2_info(m_dec)->user_data_len > 4)
	    {
		    LOG(DBGLOG_ALL, ("User Data State %d First DWORD %08x Length %d\n", State, *(DWORD*)UserData, UserDataLen));
	    }
        else
        {
		    LOG(DBGLOG_ALL, ("User Data State %d Length %d\n", State, UserDataLen));
        }
    }
    return hr;
}
