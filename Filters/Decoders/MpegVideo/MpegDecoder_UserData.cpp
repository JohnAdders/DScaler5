///////////////////////////////////////////////////////////////////////////////
// $Id$
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
        hr = m_CCOutPin->GetOutputSample(pSample.GetReleasedInterfaceReference(), NULL, NULL, false);
        CHECK(hr);
        BYTE* pData = NULL;
        
        hr = pSample->GetPointer(&pData);
        CHECK(hr);
        if(pData == NULL)
        {
            return E_UNEXPECTED;
        }


        hr = pSample->SetActualDataLength(UserDataLen + 4);
        if(SUCCEEDED(hr))
        {
            *(DWORD*)pData = 0xb2010000;
            memcpy(pData + 4, UserData, UserDataLen);

            hr = m_CCOutPin->SendSample(pSample.GetNonAddRefedInterface());
            CHECK(hr);
        }            
        else
        {
            LOG(DBGLOG_FLOW, ("Too much CC data got %d to fit in %d \n", UserDataLen + 4, pSample->GetSize()));
        }
    }
    // process Active Format Description
    // see http://www.dtg.org.uk/publications/books/afd.pdf
    // and ETSI TR 101 154
    if(UserDataLen >=6 && *(DWORD*)UserData == 0x31475444)
    {
        // check that the active format flag and the reserved bytes
        // are what we expect
        if(UserData[4] == 65 && !m_fWaitForKeyFrame)
        {
            if((UserData[5] & 0x0F) != m_AFD)
            {
                m_AFD = UserData[5] & 0x0F;
                LOG(DBGLOG_FLOW, ("Got AFD %d\n", m_AFD));
                CorrectOutputSize();
            }
        }
    }
    // process ATSC User data
    // as described in http://www.atsc.org/standards/a_53c.pdf
    else if(UserDataLen >= 6 && *(DWORD*)UserData == 0x34394147)
    {
        // process ATSC sub title data
        // what I'm trying to do here is convert the ATSC data into the 
        // DVD CC format so that we can keep a simple output pin
        // Not sure if this works properly or not due to limited test material
        //
        // The DVD GOP format is as described by the xine file
        // src/libspudec/cc_decoder.c
        if(UserData[4] == 0x03 && m_CCOutPin->IsConnected())
        {
            // check that the CC data is to be processed
            if((UserData[5] & 0x40) == 0x40)
            {
                const BYTE* pInData = UserData + 5;
                int Count = *pInData++ & 0x1F;
            
                // skip emergency data
                ++pInData;

                SI(IMediaSample) pSample;
                hr = m_CCOutPin->GetOutputSample(pSample.GetReleasedInterfaceReference(), NULL, NULL, false);
                CHECK(hr);
                BYTE* pOutData = NULL;
                
                hr = pSample->GetPointer(&pOutData);
                CHECK(hr);
                if(pOutData == NULL)
                {
                    return E_UNEXPECTED;
                }


                // create the start of the DVD GOP subtitle format
                *(DWORD*)pOutData = 0xb2010000;
                pOutData += 4;
                *(DWORD*)pOutData = 0xf8014343;
                pOutData += 4;

                BYTE OutCount = 0;
                BYTE * pCount = pOutData++;

                for(int i = 0; i < Count; ++i)
                {
                    // Only process the type 0 subtitles, these seem
                    // to be the ones we want
                    if(*pInData == 0xFC)
                    {
                        // for the DVD format we seem to need to pass
                        // pairs of CC triplets
                        // this format is similar to what appears on
                        // some DVD's I've tested with and so appears to be 
                        // valid
                        *pOutData++ = 0xFF;
                        pInData++;
                        *pOutData++ = *pInData++;
                        *pOutData++ = *pInData++;
                        *pOutData++ = 0xFE;
                        *pOutData++ = 0x00;
                        *pOutData++ = 0x00;
                        OutCount += 2;
                    }
                    else
                    {
                        pInData += 3;
                    }
                }

                if(OutCount > 0)
                {
                    // Set the triplet count and also set the flag to say
                    // the first triplet is a normal CC one;
                    *pCount = (BYTE)OutCount | 0x80;

                    hr = pSample->SetActualDataLength(OutCount * 3 + 9);
                    CHECK(hr);

                    hr = m_CCOutPin->SendSample(pSample.GetNonAddRefedInterface());
                    CHECK(hr);
                }
            }
        }
        // Process bar information
        // this should help with optimal display on all taget aspect ratios
        // along with the AFD but I'v yet to see a TS with it in
        else if(UserData[4] == 0x06)
        {

            LOG(DBGLOG_FLOW, ("*** Not yet implemeneted *** Bar Data %02x\n", UserData[5]));
            LOG(DBGLOG_FLOW, ("In you see this please inform the mailing list\n"));
        }

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