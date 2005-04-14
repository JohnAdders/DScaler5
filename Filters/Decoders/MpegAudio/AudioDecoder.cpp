///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder.cpp,v 1.52 2005-04-14 11:21:05 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2003 Gabest
//  http://www.gabest.org
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
// Changes made to files by John Adcock 12/02/04
//  - Removed use of MFC
//  - Replaced use of ATL with YACL
//  - Replaced Baseclasses with FilterLib
//  - Removed DeCSS
//  - Removed conversion to float for MPEG audio
//  - Attempted to add support for different PCM types
//
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.51  2005/03/20 14:17:34  adcockj
// Better debug logging
//
// Revision 1.50  2005/03/08 13:22:02  adcockj
// Added connection type option
//
// Revision 1.49  2005/02/17 09:28:09  adcockj
// Added extra type to work aroung bug in MPC with dvr-ms files
//
// Revision 1.48  2005/02/08 15:32:34  adcockj
// Added CSS handling
//
// Revision 1.47  2005/02/05 13:39:33  adcockj
// fixed rate addition issues
//
// Revision 1.46  2005/02/03 13:36:49  adcockj
// added rate change interface
//
// Revision 1.45  2005/01/21 13:52:42  adcockj
// Discontinuity and preroll changes
//
// Revision 1.44  2004/10/31 14:19:10  adcockj
// Comments clean
//
// Revision 1.43  2004/10/29 09:11:58  adcockj
// Fixed spdif connection issue
//
// Revision 1.42  2004/10/27 12:10:55  adcockj
// checked over Laurent's changes
//
// Revision 1.41  2004/10/24 16:24:28  laurentg
// Negative delay added when doing S/PDIF passthrough
//
// Revision 1.40  2004/10/22 07:34:40  adcockj
// fix for some connection issues
//
// Revision 1.39  2004/10/21 18:52:09  adcockj
// Sync fixes
//
// Revision 1.38  2004/10/05 19:27:08  adcockj
// Correct some stuttering/timing issues
//
// Revision 1.37  2004/09/23 14:27:57  adcockj
// preliminary fixed for reconnection issues
//
// Revision 1.36  2004/08/31 16:33:40  adcockj
// Minor improvements to quality control
// Preparation for next version
// Start on integrating film detect
//
// Revision 1.35  2004/08/16 16:08:44  adcockj
// timestamp fixes
//
// Revision 1.34  2004/08/03 18:47:40  adcockj
// spdif fixes
//
// Revision 1.33  2004/08/03 08:55:56  adcockj
// Fixes for seeking issues
//
// Revision 1.32  2004/07/31 18:52:09  adcockj
// Test seeking fix
//
// Revision 1.31  2004/07/30 08:13:53  adcockj
// Fix for spdif delay
//
// Revision 1.30  2004/07/29 13:44:59  adcockj
// More fixes for Laurent's issues
//
// Revision 1.29  2004/07/29 10:26:54  adcockj
// Fixes for audio issues reported by Laurent
//
// Revision 1.28  2004/07/28 13:59:29  adcockj
// spdif fixes
//
// Revision 1.27  2004/07/27 16:53:20  adcockj
// Some spdif fixes
//
// Revision 1.26  2004/07/26 17:24:43  adcockj
// Fixed crashing
//
// Revision 1.25  2004/07/26 17:08:00  adcockj
// Force use of fixed size output buffers to work around issues with Wave renderer
//
// Revision 1.24  2004/07/23 16:25:07  adcockj
// Fix issues with changing buffer size with wave renderer to hopefully fix DTS problem
//
// Revision 1.23  2004/07/20 20:01:59  adcockj
// Fixed accidental check in of test code
//
// Revision 1.22  2004/07/20 20:00:27  adcockj
// Fixes for compilation under VS 6
//
// Revision 1.21  2004/07/20 16:37:48  adcockj
// Fixes for main issues raised in testing of 0.0.1
//  - Improved parameter handling
//  - Fixed some overlay issues
//  - Auto aspect ratio with VMR
//  - Fixed some overlay stutters
//  - Fixed some push filter issues
//  - ffdshow and DirectVobSub connection issues
//
// Added
//  - Hardcode for PAL setting for ffdshow
//  - Added choice of IDCT for testing
//
// Revision 1.20  2004/07/16 15:45:19  adcockj
// Fixed compilation issues under .NET
// Improved (hopefully) handling of negative times and preroll
// Changed name of filter
//
// Revision 1.19  2004/07/11 15:07:04  adcockj
// Better connection logic with spdif
//
// Revision 1.18  2004/07/11 14:35:25  adcockj
// Fixed spdif connections and some dts issues
//
// Revision 1.17  2004/07/07 14:08:10  adcockj
// Improved format change handling to cope with more situations
// Removed tabs
//
// Revision 1.16  2004/07/01 21:16:55  adcockj
// Another load of fixes to recent changes
//
// Revision 1.15  2004/07/01 20:03:08  adcockj
// Silly bugs fixed
//
// Revision 1.14  2004/07/01 16:12:47  adcockj
// First attempt at better handling of audio when the output is connected to a
// filter that can't cope with dynamic changes.
//
// Revision 1.13  2004/05/12 15:55:06  adcockj
// Fixed issue with format changes during preroll
//
// Revision 1.12  2004/05/06 06:38:06  adcockj
// Interim fixes for connection and PES streams
//
// Revision 1.11  2004/04/20 16:30:06  adcockj
// Improved Dynamic Connections
//
// Revision 1.10  2004/04/06 16:46:11  adcockj
// DVD Test Annex Compatability fixes
//
// Revision 1.9  2004/03/25 18:01:30  adcockj
// Fixed issues with downmixing
//
// Revision 1.8  2004/03/15 17:15:37  adcockj
// Better PES header handling - Inspired by Gabest's latest MPC patch
//
// Revision 1.7  2004/03/11 16:51:20  adcockj
// Improve LPCM support
//
// Revision 1.6  2004/02/29 13:47:47  adcockj
// Format change fixes
// Minor library updates
//
// Revision 1.5  2004/02/27 17:04:38  adcockj
// Added support for fixed point libraries
// Added dither to 16 conversions
// Changes to support library fixes
//
// Revision 1.4  2004/02/25 17:14:01  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.3  2004/02/17 16:39:59  adcockj
// Added dts analog support (based on dtsdec-0.0.1 and Gabest's patch to mpadecfilter)
//
// Revision 1.2  2004/02/16 17:25:00  adcockj
// Fix build errors, locking problems and DVD compatability
//
// Revision 1.1  2004/02/13 12:22:17  adcockj
// Initial check-in of audio decoder (based on mpadecfilter from guliverkli, libmad and liba52)
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "EnumPins.h"
#include "DSCSSInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"
#include "MoreUuids.h"
#include "CPUID.h"
#include "mmreg.h"

extern HINSTANCE g_hInstance;


CAudioDecoder::CAudioDecoder() :
    CDSBaseFilter(L"Mpeg Audio Filter", 1, 1)
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::CreatePins\n"));
    
    m_AudioInPin = new CDSCSSInputPin();
    if(m_AudioInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_AudioInPin->AddRef();
    m_AudioInPin->SetupObject(this, L"Audio In");

    m_AudioOutPin = new CDSOutputPin();
    if(m_AudioOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_AudioOutPin->AddRef();
    m_AudioOutPin->SetupObject(this, L"Audio Out");

    m_rtNextFrameStart = _I64_MIN;
    m_rtOutputStart = 0;
    m_OutputSampleType = OUTSAMPLE_16BIT;
    m_SampleSize = 2;
    m_NeedToAttachFormat = false;
    m_InputSampleRate = 48000;
    m_OutputSampleRate = 0;
    m_ConnectedAsSpdif = false;
    m_ChannelMask = 0;
    m_ChannelsRequested = 2;
    m_CanReconnect = false;
    m_DownSample = false;
	m_Preroll = false;
    m_BytesLeftInBuffer = 0;
    m_pDataOut = NULL;
    m_BufferSizeAtFrameStart = 0;
    m_AC3SilenceFrames = 0;
    m_ProcessingType = PROCESS_AC3;
    
    InitMediaType(&m_InternalMT);
    ZeroMemory(&m_InternalWFE, sizeof(WAVEFORMATEXTENSIBLE));

    m_a52_state = NULL;
    m_dts_state = NULL;
    m_madinit = false;

    m_rate.Rate = 10000;
    m_rate.StartTime = 0;

	m_ratechange.Rate = 10000;
    m_ratechange.StartTime = -1;

}

CAudioDecoder::~CAudioDecoder()
{
    LOG(DBGLOG_FLOW, ("CAudioDecoder::~CAudioDecoder\n"));
}

STDMETHODIMP CAudioDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_ALL, ("CAudioDecoder::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CAudioDecoder;
    return S_OK;
}


STDMETHODIMP CAudioDecoder::get_Name(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_NAME, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Name = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CAudioDecoder::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CAudioDecoder::get_Authors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    TCHAR Result[MAX_PATH];
    if(LoadString(g_hInstance, IDS_AUTHORS, Result, MAX_PATH))
    {
        wchar_t wResult[MAX_PATH];
        ustrcpy(wResult, Result);
        *Authors = SysAllocString(wResult);
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CAudioDecoder::ParamChanged(DWORD dwParamIndex)
{
    switch(dwParamIndex)
    {
    case SPEAKERCONFIG:
    case USESPDIF:
    case MPEGOVERSPDIF:
    case CONNECTTYPE:
        if(m_AudioOutPin->m_ConnectedPin)
        {
            return S_FALSE;
        }
    default:
        break;
    }
    return S_OK;
}

HRESULT CAudioDecoder::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_TSRateChange)
    {
        return SupportPropSetRate(dwPropID, pTypeSupport);
    }
	else
	{
        return E_NOTIMPL;
	}
}


HRESULT CAudioDecoder::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_TSRateChange)
    {
        return SetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT CAudioDecoder::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_TSRateChange)
    {
        return GetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData, pcbReturned);
    }
    else
    {
        return E_NOTIMPL;
    }
}


HRESULT CAudioDecoder::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        if(q.Type == Famine)
        {
            SI(IQualityControl) QualityControl = m_AudioOutPin->m_ConnectedPin;
            if(QualityControl)
            {
                LOG(DBGLOG_FLOW, ("Coped With Famine - %d\n", q.Late));
                return QualityControl->Notify(this, q);
            }
        }
        if(q.Type == Flood)
        {
            SI(IQualityControl) QualityControl = m_AudioOutPin->m_ConnectedPin;
            if(QualityControl)
            {
                LOG(DBGLOG_FLOW, ("Coped With Flood - %d\n", q.Late));
                return QualityControl->Notify(this, q);
            }
        }
    }
    LOG(DBGLOG_FLOW, ("Coped With Flood - %d\n", q.Late));
	return VFW_E_NOT_FOUND;
}


HRESULT CAudioDecoder::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProperties, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        pProperties->cBuffers = 6;
        pProperties->cbBuffer = 8192; 
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else if(pPin == m_AudioOutPin)
    {
        // we specify a fixed size buffer and so size is already
        // taken care of
        pProperties->cBuffers = 3;
        pProperties->cbBuffer = 0; 
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else
    {
        return E_UNEXPECTED;
    }
    return S_OK;
}

bool CAudioDecoder::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool Result = false;
    if(pPin == m_AudioInPin)
    {
        Result = (pmt->majortype == MEDIATYPE_Audio && 
                  pmt->subtype == MEDIASUBTYPE_MP3) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                  pmt->subtype == MEDIASUBTYPE_MPEG1AudioPayload) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG1Payload) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG1Packet) ||
                (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_MPEG2_AUDIO_MPCBUG) ||
                (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DOLBY_AC3) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3) ||
                (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DTS) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_WAVE_DTS) ||
                (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
                (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) ||
                (pmt->majortype == MEDIATYPE_Audio && 
                 pmt->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO);
        if(pmt->formattype == FORMAT_WaveFormatEx)
        {
            MPEG1WAVEFORMAT* wfe = (MPEG1WAVEFORMAT*)pmt->pbFormat;
            int i= 0;
        }
    }
    else if(pPin == m_AudioOutPin)
    {
        Result = (pmt->majortype == MEDIATYPE_Audio) && 
                  (pmt->subtype == MEDIASUBTYPE_PCM ||
                    pmt->subtype == MEDIASUBTYPE_IEEE_FLOAT ||
                    pmt->subtype == MEDIASUBTYPE_DOLBY_AC3_SPDIF);
    }
    return Result;
}

HRESULT CAudioDecoder::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    if(pPin != m_AudioInPin)
    {
        return E_UNEXPECTED;
    }

    // \todo use pSampleProperties properly
    HRESULT hr;

    if(pSampleProperties->dwStreamId != AM_STREAM_MEDIA)
        return m_AudioOutPin->SendSample(InSample);

    BYTE* pDataIn = pSampleProperties->pbBuffer;

    long len = pSampleProperties->lActual;

    if(*(DWORD*)pDataIn == 0xBA010000) // MEDIATYPE_*_PACK
    {
        len -= 14; 
        pDataIn += 14;
        if(int stuffing = (pDataIn[-1]&7))
        {
            len -= stuffing; 
            pDataIn += stuffing;
        }
    }

    if(len <= 0) return S_OK;

    const GUID& subtype = m_AudioInPin->GetMediaType()->subtype;

    if(*(DWORD*)pDataIn == 0xBB010000)
    {
        len -= 4; pDataIn += 4;
        int hdrlen = ((pDataIn[0]<<8)|pDataIn[1]) + 2;
        len -= hdrlen; pDataIn += hdrlen;
    }

    if(len <= 0) return S_OK;

    if((*(DWORD*)pDataIn&0xE0FFFFFF) == 0xC0010000 || *(DWORD*)pDataIn == 0xBD010000)
    {
        BYTE StreamId = pDataIn[3];

        // skip past the PES header annd Id
        len -= 4;
        pDataIn += 4;

        int ExpectedLength = (pDataIn[0] << 8)+ pDataIn[1];
        
        // Skip past the packet length
        len -= 2;
        pDataIn += 2;

        BYTE* EndOfPacketLength = pDataIn;

        // skip past MPEG1 PES Packet stuffing
        for(int i(0); i < 16 && *pDataIn == 0xff; ++i)
        {
            len--;
            pDataIn++;
        }

        if((*pDataIn & 0xC0) == 0x80)
        {
            // MPEG2 PES Format

            // Skip to the header data length
            len -= 2;
            pDataIn += 2;
    
            // skip past all the optional headers and the llength byte itself
            len -= *pDataIn + 1;
            pDataIn += *pDataIn + 1;
        }
        else
        {
            // MPEG1 PES format

            // skip STD bits if present
            if((*pDataIn & 0xC0) == 0x40)
            {
                len -= 2;
                pDataIn += 2;
            }

            if((*pDataIn  & 0xF0) == 0x30)
            {
                // Skip DTS and PTS
                len -= 10;
                pDataIn += 10;
            }
            else if((*pDataIn & 0xF0) == 0x20)
            {
                // Skip PTS
                len -= 5;
                pDataIn += 5;
            }
            else
            {
                // Skip Non DTS PTS byte
                len -= 1;
                pDataIn += 1;
            }
        }

        if(StreamId == 0xBD)
        {
            if(m_ProcessingType == PROCESS_PCM)
            {
                len -= 7; 
                pDataIn += 7;

                // LPCM packets seem to sometimes have variable 
                // padding on the end
                // just need to strip this off
                // see http://members.freemail.absa.co.za/ginggs/dvd/mpeg2_lpcm.txt
                // for a description of the format of the packets
                if(pDataIn[len - 1] == 0xFF)
                {
                    int PadCount = 1;
                    while(pDataIn[len - 1 - PadCount] == 0xFF)
                    {
                        ++PadCount;
                    }
                    if(PadCount == ((pDataIn[len - 1 - PadCount - 1] << 8) + pDataIn[len - 1 - PadCount]) &&
                        pDataIn[len - 1 - PadCount - 2] == 0xbe &&
                        pDataIn[len - 1 - PadCount - 3] == 0x01 && 
                        pDataIn[len - 1 - PadCount - 4] == 0x00 && 
                        pDataIn[len - 1 - PadCount - 5] == 0x00)
                    {
                        len -= PadCount + 6;
                    }
                }
            }
            else if(m_ProcessingType == PROCESS_AC3 || m_ProcessingType == PROCESS_DTS)
            {
                len -= 4; 
                pDataIn += 4;
            }
            else
            {
                len -= 1; 
                pDataIn += 1;
            }
        }
        if(ExpectedLength > 0)
        {
            ExpectedLength -= pDataIn - EndOfPacketLength;
            len = min(len, ExpectedLength); 
        }
    }

    if(len <= 0) return S_OK;

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY)
    {
        LOG(DBGLOG_FLOW, ("Got Time Discontinuity\n"));
        SendOutLastSamples(m_AudioInPin);
    }
    if(InSample->IsDiscontinuity() == S_OK)
    {
        LOG(DBGLOG_FLOW, ("Got Discontinuity\n"));
        m_IsDiscontinuity = true;
        m_BytesLeftInBuffer = 0;
        m_CurrentOutputSample.Detach();
        m_buff.resize(0);
        m_pDataOut = NULL;
        m_rtNextFrameStart = _I64_MIN;
        m_BufferSizeAtFrameStart = 0;
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        if(m_rtNextFrameStart == _I64_MIN)
        {
            m_rtNextFrameStart = pSampleProperties->tStart;
            m_BufferSizeAtFrameStart = m_buff.size();
        }
    }
    else
    {
        m_rtNextFrameStart = _I64_MIN;
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        LOG(DBGLOG_FLOW, ("Receive: %I64d - %I64d\n", pSampleProperties->tStart, m_rtNextFrameStart));
    }

	m_Preroll = ((pSampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL) == AM_SAMPLE_PREROLL);

    int tmp = m_buff.size();
    m_buff.resize(m_buff.size() + len);
    memcpy(&m_buff[0] + tmp, pDataIn, len);
    len += tmp;

    switch(m_ProcessingType)
    {
    case PROCESS_PCM:
        hr = ProcessLPCM();
        break;
    case PROCESS_AC3:
        hr = ProcessAC3();
        break;
    case PROCESS_DTS:
        hr = ProcessDTS();
        break;
    case PROCESS_MPA:
        hr = ProcessMPA();
        break;
    }

    return hr;
}


HRESULT CAudioDecoder::CreateInternalSPDIFMediaType(DWORD nSamplesPerSec, WORD wBitsPerSample)
{
    m_InternalMT.majortype = MEDIATYPE_Audio;
    m_InternalMT.subtype = MEDIASUBTYPE_DOLBY_AC3_SPDIF;
    m_InternalMT.formattype = FORMAT_WaveFormatEx;

    m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
    m_InternalWFE.Format.nChannels = 2;
    m_InternalWFE.Format.wBitsPerSample = wBitsPerSample;
    m_InternalWFE.Format.nBlockAlign = 2 * wBitsPerSample / 8;
    m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

    m_InternalWFE.Format.wFormatTag = (WORD)MEDIASUBTYPE_DOLBY_AC3_SPDIF.Data1;
    m_InternalWFE.Format.cbSize = 0;

    m_InternalMT.cbFormat = sizeof(m_InternalWFE.Format) + m_InternalWFE.Format.cbSize;
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 

    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
    if(hr == S_OK)
    {
        return hr;
    }
    else
    {
        return E_UNEXPECTED;
    }
}


HRESULT CAudioDecoder::CreateInternalPCMMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask, WORD wBitsPerSample)
{
    m_InternalMT.majortype = MEDIATYPE_Audio;
    m_InternalMT.subtype = MEDIASUBTYPE_PCM;
    m_InternalMT.formattype = FORMAT_WaveFormatEx;

    m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
    m_InternalWFE.Format.nChannels = nChannels;
    m_InternalWFE.Format.wBitsPerSample = wBitsPerSample;
    m_InternalWFE.Format.nBlockAlign = nChannels * wBitsPerSample / 8;
    m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

    if(wBitsPerSample > 16 || nChannels > 2)
    {
        m_InternalWFE.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        m_InternalWFE.Format.cbSize = sizeof(m_InternalWFE) - sizeof(m_InternalWFE.Format);
        m_InternalWFE.SubFormat = m_InternalMT.subtype;
        m_InternalWFE.dwChannelMask = dwChannelMask;
        m_InternalWFE.Samples.wValidBitsPerSample = wBitsPerSample;
    }
    else
    {
        m_InternalWFE.Format.wFormatTag = (WORD)m_InternalMT.subtype.Data1;
        m_InternalWFE.Format.cbSize = 0;
    }

    m_InternalMT.cbFormat = sizeof(m_InternalWFE.Format) + m_InternalWFE.Format.cbSize;
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 

    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);

    switch(wBitsPerSample)
    {
    case 16:
        m_OutputSampleType = OUTSAMPLE_16BIT;
        m_SampleSize = 2;
        if(hr == S_OK) return hr;
        return E_UNEXPECTED;
        break;
    case 24:
        m_OutputSampleType = OUTSAMPLE_24BIT;
        m_SampleSize = 3;
        if(hr == S_OK) return hr;
        return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 16);
        break;
    case 32:
        m_OutputSampleType = OUTSAMPLE_32BIT;
        m_SampleSize = 4;
        if(hr == S_OK) return hr;
        return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 24);
        break;
    }
    return E_UNEXPECTED;
}

HRESULT CAudioDecoder::CreateInternalIEEEMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
    m_InternalMT.majortype = MEDIATYPE_Audio;
    m_InternalMT.subtype = MEDIASUBTYPE_IEEE_FLOAT;
    m_InternalMT.formattype = FORMAT_WaveFormatEx;

    m_InternalWFE.Format.nSamplesPerSec = nSamplesPerSec;
    m_InternalWFE.Format.nChannels = nChannels;
    m_InternalWFE.Format.wBitsPerSample = 32;
    m_InternalWFE.Format.nBlockAlign = nChannels * 4;
    m_InternalWFE.Format.nAvgBytesPerSec = nSamplesPerSec * m_InternalWFE.Format.nBlockAlign;

    m_InternalWFE.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    m_InternalWFE.Format.cbSize = sizeof(m_InternalWFE) - sizeof(m_InternalWFE.Format);
    m_InternalWFE.SubFormat = m_InternalMT.subtype;
    m_InternalWFE.dwChannelMask = dwChannelMask;
    m_InternalWFE.Samples.wValidBitsPerSample = 32;

    m_InternalMT.cbFormat = sizeof(m_InternalWFE);
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWFE; 
    HRESULT hr = m_AudioOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
    if(hr == S_OK)
    {
        m_OutputSampleType = OUTSAMPLE_FLOAT;
        m_SampleSize = sizeof(float);
        return hr;
    }
    
    return CreateInternalPCMMediaType(nSamplesPerSec, nChannels, dwChannelMask, 32);

}

HRESULT CAudioDecoder::Deliver(bool IsSpdif)
{
    HRESULT hr = S_OK;
    REFERENCE_TIME rtDur = 10000000i64 * m_InternalMT.lSampleSize / (m_OutputSampleRate * m_ChannelsRequested * m_SampleSize);
	REFERENCE_TIME rtStop = m_rtOutputStart + rtDur;

	REFERENCE_TIME rtStart = m_rtOutputStart;
    if(IsSpdif)
    {
        int DelayMs = GetParamInt(AUDIOTIMEOFFSET);
		rtStart += DelayMs * 10000;
		rtStop += DelayMs * 10000;
	}

// blocked out code for monitoring current graph clock.
#ifndef _NOT_DEFINED_
	REFERENCE_TIME Now = 0;
    static REFERENCE_TIME Last = 0;
    LARGE_INTEGER Freq;
    LARGE_INTEGER Now2;
    static LARGE_INTEGER Last2;
    QueryPerformanceFrequency(&Freq);
    QueryPerformanceCounter(&Now2);

    m_RefClock->GetTime(&Now);

    LOG(DBGLOG_FLOW, ("Times - %010I64d - %010I64d - %010I64d\n", Now - m_rtStartTime, rtStart, rtStop));
    Last = Now;
    Last2 = Now2;
#endif

	if(!m_Preroll && rtStop > 0)
	{
        m_CurrentOutputSample->SetTime(&rtStart, &rtStop);
	    m_CurrentOutputSample->SetMediaTime(NULL, NULL);

	    LOG(DBGLOG_FLOW, ("Deliver: %I64d - %I64d\n", m_rtOutputStart, rtDur));

		m_CurrentOutputSample->SetPreroll(FALSE);
		m_CurrentOutputSample->SetSyncPoint(TRUE);

		if(m_NeedToAttachFormat)
		{
			m_AudioOutPin->SetType(&m_InternalMT);
			m_CurrentOutputSample->SetMediaType(&m_InternalMT);
			m_CurrentOutputSample->SetDiscontinuity(TRUE); 
		}
		else
		{
			m_CurrentOutputSample->SetDiscontinuity(m_IsDiscontinuity); 
		}
	    
		m_IsDiscontinuity = false;

		hr = m_AudioOutPin->SendSample(m_CurrentOutputSample.GetNonAddRefedInterface());

        m_CurrentOutputSample.Detach();

		m_NeedToAttachFormat = false;
	}
	else
	{
		LOG(DBGLOG_FLOW, ("Preroll: %I64d - %I64d\n", m_rtOutputStart, rtDur));
	}

    if(hr == S_OK)
    {
        m_rtOutputStart += rtDur;
    }
    return hr;
}


HRESULT CAudioDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{

    if(pPin == m_AudioOutPin)
    {
        if(!m_AudioInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

        if(TypeNum < 0) return E_INVALIDARG;
        AM_MEDIA_TYPE* pMediaType = &m_AudioInPin->m_ConnectedMediaType;

        BOOL UseSpdif = (GetParamBool(USESPDIF) &&
                        (IsMediaTypeAC3(pMediaType) || IsMediaTypeDTS(pMediaType))) ||
                        (GetParamBool(MPEGOVERSPDIF) && IsMediaTypeMP3(pMediaType));

        if(UseSpdif)
        {
            // if we want spdif then we should always be able to connect
            // at 48000 but at other frequencies we probably will have to fall back to
            // PCM
            if(m_InputSampleRate == 480000)
            {
                if(TypeNum > 0) return VFW_S_NO_MORE_ITEMS;
            }
            else
            {
                if(TypeNum > 1) return VFW_S_NO_MORE_ITEMS;
                if(TypeNum == 1)
                {
                    UseSpdif = 0;
                }
            }
            TypeNum = 2;
        }
        else
        {
            // we need to cope with the compilation option for integer
            // calculation 
            // so after this -1 will be used to indeicate that we 
            // want floating point
            // then 0 = 32 bit 1 = 24bit and 2 = 16 bit
            if(TypeNum > GetParamEnum(CONNECTTYPE)) return VFW_S_NO_MORE_ITEMS;
            TypeNum += 2 - GetParamEnum(CONNECTTYPE);
        }

        pmt->majortype = MEDIATYPE_Audio;
        pmt->formattype = FORMAT_WaveFormatEx;
        pmt->cbFormat = sizeof(WAVEFORMATEXTENSIBLE);
        WAVEFORMATEXTENSIBLE* wfe = (WAVEFORMATEXTENSIBLE*)CoTaskMemAlloc(pmt->cbFormat);
        memset(wfe, 0, sizeof(WAVEFORMATEXTENSIBLE));
        if(TypeNum > -1)
        {
            pmt->subtype = MEDIASUBTYPE_PCM;
            wfe->SubFormat = MEDIASUBTYPE_PCM;
        }
        else
        {
            pmt->subtype = MEDIASUBTYPE_IEEE_FLOAT;
            wfe->SubFormat = MEDIASUBTYPE_IEEE_FLOAT;
        }

        int ChannelsRequested = 2;
        DWORD ChannelMask = 0;

        if(!UseSpdif)
        {
            switch(GetParamEnum(SPEAKERCONFIG))
            {
            case SPCFG_STEREO:
                break;
            case SPCFG_DOLBY:
                break;
            case SPCFG_2F2R:
                ChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
                ChannelsRequested = 4;
                break;
            case SPCFG_2F2R1S:
                ChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
                ChannelsRequested = 5;
                break;
            case SPCFG_3F2R:
                ChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
                ChannelsRequested = 5;
                break;
            case SPCFG_3F2R1S:
                ChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT;
                ChannelsRequested = 6;
                break;
            }
        }
        wfe->Format.nChannels = ChannelsRequested;
        wfe->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
        wfe->dwChannelMask = ChannelMask;

        switch(TypeNum)
        {
        case -1:
            wfe->Format.wBitsPerSample = 32;
            wfe->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            break;
        case 0:
            wfe->Format.wBitsPerSample = 32;
            wfe->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            break;
        case 1:
            wfe->Format.wBitsPerSample = 24;
            wfe->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            break;
        case 2:
            wfe->Format.wBitsPerSample = 16;
            if(UseSpdif)
            {
                pmt->subtype = MEDIASUBTYPE_DOLBY_AC3_SPDIF;
                wfe->Format.wFormatTag = (WORD)MEDIASUBTYPE_DOLBY_AC3_SPDIF.Data1;
                wfe->Format.cbSize = 0;
                pmt->cbFormat = sizeof(WAVEFORMATEX);
            }
            else if(ChannelsRequested != 2)
            {
                wfe->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
            }
            else
            {
                wfe->Format.wFormatTag = WAVE_FORMAT_PCM;
                wfe->Format.cbSize = 0;
                pmt->cbFormat = sizeof(WAVEFORMATEX);
            }
            break;
        }
        wfe->Format.nSamplesPerSec = m_InputSampleRate;
        wfe->Format.nBlockAlign = wfe->Format.nChannels*wfe->Format.wBitsPerSample/8;
        wfe->Format.nAvgBytesPerSec = wfe->Format.nSamplesPerSec*wfe->Format.nBlockAlign;
        wfe->Samples.wValidBitsPerSample = wfe->Format.wBitsPerSample;

        pmt->pbFormat = (BYTE*)wfe;
        pmt->bFixedSizeSamples = TRUE;
        // make the sample buffer a quarter of a secong long
        pmt->lSampleSize = 1 * 6 * 256 * ChannelsRequested * wfe->Format.wBitsPerSample / 8;
        return S_OK;
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }
}


HRESULT CAudioDecoder::Activate()
{
    m_rtNextFrameStart = _I64_MIN;

    if(m_ConnectedAsSpdif && IsMediaTypeAC3(m_AudioInPin->GetMediaType()))
    {
        m_AC3SilenceFrames = 3;
    }
    else
    {
		if(m_OutputSampleType == OUTSAMPLE_FLOAT && IsClockUpstream())
		{
			HRESULT hr = CreateInternalPCMMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask, 32);
			CHECK(hr);
		}
        m_AC3SilenceFrames = 0;
    }

    InitLibraries();

    return S_OK;
}

HRESULT CAudioDecoder::Deactivate()
{
    FinishLibraries();

    m_IsDiscontinuity = false; 

    m_BytesLeftInBuffer = 0;
    m_CurrentOutputSample.Detach();
    m_buff.resize(0);
    m_pDataOut = NULL;
    m_rtNextFrameStart = _I64_MIN;
    m_BufferSizeAtFrameStart = 0;

    return S_OK;
}


void CAudioDecoder::InitLibraries()
{
    FinishLibraries();

    switch(m_ProcessingType)
    {
    case PROCESS_AC3:
        m_a52_state = liba52::a52_init(MM_ACCEL_DJBFFT);
        break;
    case PROCESS_MPA:
        libmad::mad_stream_init(&m_stream);
        libmad::mad_frame_init(&m_frame);
        libmad::mad_synth_init(&m_synth);
        mad_stream_options(&m_stream, 0);
        m_madinit = true;
        break;
    case PROCESS_DTS:
        m_dts_state = libdts::dts_init(0);
        break;
    default:
    case PROCESS_PCM:
        break;
    }
}

void CAudioDecoder::FinishLibraries()
{
    if(m_a52_state != NULL)
    {
        liba52::a52_free(m_a52_state);
        m_a52_state = NULL;
    }

    if(m_dts_state != NULL)
    {
        libdts::dts_free(m_dts_state);
        m_dts_state = NULL;
    }

    if(m_madinit == true)
    {
        mad_synth_finish(&m_synth);
        libmad::mad_frame_finish(&m_frame);
        libmad::mad_stream_finish(&m_stream);
        m_madinit = false;
    }
}

HRESULT CAudioDecoder::Flush(CDSBasePin* pPin)
{
    m_BytesLeftInBuffer = 0;
    m_CurrentOutputSample.Detach();
    m_buff.resize(0);
    m_pDataOut = NULL;
    m_rtNextFrameStart = _I64_MIN;
    m_BufferSizeAtFrameStart = 0;
  
    if(m_ConnectedAsSpdif && m_ProcessingType == PROCESS_AC3)
    {
        m_AC3SilenceFrames = 3;
    }
    else
    {
        m_AC3SilenceFrames = 0;
    }
    
    InitLibraries();

    return S_OK;
}

HRESULT CAudioDecoder::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        LOG(DBGLOG_FLOW, ("New Segment %010I64d - %010I64d  @ %f\n", tStart, tStop, dRate));
        m_BytesLeftInBuffer = 0;
        m_CurrentOutputSample.Detach();
        m_buff.resize(0);
        m_pDataOut = NULL;
        m_BufferSizeAtFrameStart = 0;
        m_rtOutputStart = 0;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CAudioDecoder::SendOutLastSamples(CDSBasePin* pPin)
{
    HRESULT hr = S_OK;
    if(pPin == m_AudioInPin && m_BytesLeftInBuffer > 0)
    {
        memset(m_pDataOut, 0, m_BytesLeftInBuffer);
        hr = Deliver(false);
        m_CurrentOutputSample.Detach();
        m_BytesLeftInBuffer = 0;
        m_pDataOut = NULL;

    }
    return S_OK;
}

HRESULT CAudioDecoder::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        if(pMediaType->formattype != FORMAT_WaveFormatEx)
        {
            return E_UNEXPECTED;
        }

        WAVEFORMATEX* wfe = (WAVEFORMATEX*)(pMediaType->pbFormat);
        m_InputSampleRate = wfe->nSamplesPerSec;
        m_IsDiscontinuity = true;
        m_NeedToAttachFormat = true;
        m_AC3SilenceFrames = 0;
        
        // cope with dynamic format changes
        // these can happen during DVD playback
        // as the DVD navigator always connectes with AC3 first
        // and then switches as required
        // We try and cope with as many situations as possible
        //  - We're connected direct to a "good" filter and we can change
        //    to a suitable format
        //  - We're connected to Sp-dif and a good filter and we need to change
        //    back and forward between to an spdif format
        //  - We're conncted to a bad filter that doesn't allow dynamic reconnection 
        //    so we try and do the best we can
        // also we need to cope with getting 96000 kHz pcm and the renderer not
        // being able to support that
        HRESULT hr = S_OK;
        if(IsMediaTypeMP3(pMediaType))
        {
            LOG(DBGLOG_FLOW, ("Got change to MP3\n"));
            m_ProcessingType = PROCESS_MPA;

            if(m_AudioOutPin->m_ConnectedPin && m_CanReconnect)
            {
                if(GetParamBool(USESPDIF) && !m_ConnectedAsSpdif && GetParamBool(MPEGOVERSPDIF))
                {
                    hr = CreateInternalSPDIFMediaType(m_InputSampleRate, 16);
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_ConnectedAsSpdif = TRUE;
                    m_NeedToAttachFormat = true;
                }
                else if((m_InputSampleRate != m_OutputSampleRate) ||
                    (GetParamBool(USESPDIF) && m_ConnectedAsSpdif && !GetParamBool(MPEGOVERSPDIF)))
                {
                    if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                    {
                        hr = CreateInternalIEEEMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask);
                    }
                    else
                    {
                        hr = CreateInternalPCMMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask, 32);
                    }
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_NeedToAttachFormat = true;
                    m_ConnectedAsSpdif = false;
                    CHECK(hr);
                }
            }
        }
        else if(IsMediaTypeAC3(pMediaType))
        {
            LOG(DBGLOG_FLOW, ("Got change to AC3\n"));
            m_ProcessingType = PROCESS_AC3;

            if(m_ConnectedAsSpdif)
            {
                m_AC3SilenceFrames = 3;
            }
            
            if(m_AudioOutPin->m_ConnectedPin && m_CanReconnect)
            {
                if(GetParamBool(USESPDIF) && !m_ConnectedAsSpdif)
                {
                    hr = CreateInternalSPDIFMediaType(m_InputSampleRate, 16);
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_ConnectedAsSpdif = TRUE;
                    m_NeedToAttachFormat = true;
                }
                else if(m_InputSampleRate != m_OutputSampleRate)
                {
                    if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                    {
                        hr = CreateInternalIEEEMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask);
                    }
                    else
                    {
                        hr = CreateInternalPCMMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask, 32);
                    }
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_NeedToAttachFormat = true;
                    m_ConnectedAsSpdif = false;
                    CHECK(hr);
                }
            }
        }
        else if(IsMediaTypeDTS(pMediaType))
        {
            LOG(DBGLOG_FLOW, ("Got change to DTS\n"));
            m_ProcessingType = PROCESS_DTS;

            if(m_AudioOutPin->m_ConnectedPin && m_CanReconnect)
            {
                if(GetParamBool(USESPDIF) && !m_ConnectedAsSpdif)
                {
                    hr = CreateInternalSPDIFMediaType(m_InputSampleRate, 16);
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_ConnectedAsSpdif = TRUE;
                    m_NeedToAttachFormat = true;
                }
                else if(m_InputSampleRate != m_OutputSampleRate)
                {
                    if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                    {
                        hr = CreateInternalIEEEMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask);
                    }
                    else
                    {
                        hr = CreateInternalPCMMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask, 32);
                    }
                    CHECK(hr);
                    m_OutputSampleRate = m_InputSampleRate;
                    m_NeedToAttachFormat = true;
                    m_ConnectedAsSpdif = false;
                    CHECK(hr);
                }
            }
        }
        else if(IsMediaTypePCM(pMediaType))
        {
            LOG(DBGLOG_FLOW, ("Got change to PCM\n"));
            m_ProcessingType = PROCESS_PCM;

            if(m_AudioOutPin->m_ConnectedPin && m_CanReconnect)
            {
                if(m_ConnectedAsSpdif || m_InputSampleRate != m_OutputSampleRate)
                {
                    if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                    {
                        hr = CreateInternalIEEEMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask);
                    }
                    else
                    {
                        hr = CreateInternalPCMMediaType(m_InputSampleRate, m_ChannelsRequested, m_ChannelMask, 32);
                    }
                    if(m_InputSampleRate == 96000)
                    {
                        if(FAILED(hr))
                        {
                            if(m_OutputSampleType == OUTSAMPLE_FLOAT)
                            {
                                hr = CreateInternalIEEEMediaType(48000, m_ChannelsRequested, m_ChannelMask);
                            }
                            else
                            {
                                hr = CreateInternalPCMMediaType(48000, m_ChannelsRequested, m_ChannelMask, 32);
                            }
                            m_OutputSampleRate = 48000;
                            CHECK(hr);
                            m_DownSample = true;
                        }
                        else
                        {
                            m_OutputSampleRate = m_InputSampleRate;
                            m_DownSample = false;
                        }
                    }
                    else
                    {
                        m_OutputSampleRate = m_InputSampleRate;
                    }
                    CHECK(hr);
                    m_NeedToAttachFormat = true;
                    m_ConnectedAsSpdif = false;
                }
            }
            else
            {
                m_DownSample = true;
            }
        }
        else
        {
            LOG(DBGLOG_FLOW, ("Got unexpected change format\n"));
        }
        InitLibraries();
    }
    else if(pPin == m_AudioOutPin)
    {
        if(pMediaType->formattype == FORMAT_WaveFormatEx)
        {
            WAVEFORMATEXTENSIBLE* wfe = (WAVEFORMATEXTENSIBLE*)(pMediaType->pbFormat);
            m_OutputSampleRate = wfe->Format.nSamplesPerSec;
            if(pMediaType->subtype == MEDIASUBTYPE_IEEE_FLOAT)
            {
                m_OutputSampleType = OUTSAMPLE_FLOAT;
                m_SampleSize = 4;
            }
            else
            {
                switch(wfe->Format.wBitsPerSample)
                {
                case 32:
                    m_OutputSampleType = OUTSAMPLE_32BIT;
                    m_SampleSize = 4;
                    break;
                case 24:
                    m_OutputSampleType = OUTSAMPLE_24BIT;
                    m_SampleSize = 3;
                    break;
                default:
                    m_OutputSampleType = OUTSAMPLE_16BIT;
                    m_SampleSize = 2;
                    break;
                }
            }
            m_ChannelsRequested = wfe->Format.nChannels;
            m_ChannelMask = 0;
            if(wfe->Format.cbSize != 0)
            {
                m_ChannelMask = wfe->dwChannelMask;
            }
            if(m_ChannelMask == 0)
            {
                m_ChannelMask = SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT;
            }

            m_ConnectedAsSpdif = (wfe->Format.wFormatTag == MEDIASUBTYPE_DOLBY_AC3_SPDIF.Data1);
            CopyMediaType(&m_InternalMT, pMediaType);
        }
    }
    return S_OK;
}

HRESULT CAudioDecoder::NotifyConnected(CDSBasePin* pPin)
{
    if(pPin == m_AudioOutPin)
    {
        SI(IPinConnection) PinConnection = pPin->m_ConnectedPin;
        if(PinConnection)
        {
            m_CanReconnect = true;
        }
        else
        {
            CLSID ConnectedClsid;
            HRESULT hr = pPin->GetConnectedFilterCLSID(&ConnectedClsid);
            CHECK(hr);
            if(ConnectedClsid == CLSID_AudioRender)
            {
                m_CanReconnect = true;
            }
            else
            {
                m_CanReconnect = false;
            }
        }

    }

    return S_OK;
}

HRESULT CAudioDecoder::GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText)
{
    if(dwParamIndex == SPEAKERCONFIG)
    {
        return GetEnumTextSpeakerConfig(ppwchText);
    }
    else if(dwParamIndex == CONNECTTYPE)
    {
        return GetEnumTextConnectType(ppwchText);
    }
    else
    {
        return E_NOTIMPL;
    }
}


HRESULT CAudioDecoder::GetEnumTextSpeakerConfig(WCHAR **ppwchText)
{
    wchar_t SpeakerText[] = L"Speaker Config\0" L"None\0" L"Stereo\0" L"Dolby Stereo\0" L"4.0 (2 Front 2 Rear)\0"  L"4.1 (2 Front 2 Rear 1 Sub)\0" L"5.0 (3 Front 2 Rear)\0" L"5.1 (3 Front 2 Rear 1 Sub)\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(SpeakerText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, SpeakerText, sizeof(SpeakerText));
    return S_OK;
}

HRESULT CAudioDecoder::GetEnumTextConnectType(WCHAR **ppwchText)
{
    wchar_t TypeText[] = L"Preferred Connection Type\0" L"None\0" L"16 bit PCM\0" L"24 bit PCM\0" L"32 bit PCM\0"  L"32 bit IEEE float\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(TypeText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, TypeText, sizeof(TypeText));
    return S_OK;
}

HRESULT CAudioDecoder::GetOutputSampleAndPointer()
{
    m_pDataOut = NULL;

    HRESULT hr = m_AudioOutPin->GetOutputSample(m_CurrentOutputSample.GetReleasedInterfaceReference(), NULL, NULL, m_IsDiscontinuity);
    CHECK(hr);

    hr = m_CurrentOutputSample->GetPointer(&m_pDataOut);
    CHECK(hr);

    hr = m_CurrentOutputSample->SetActualDataLength(m_InternalMT.lSampleSize);
    CHECK(hr);
    
    m_BytesLeftInBuffer = m_InternalMT.lSampleSize;

    return hr;
}

BOOL CAudioDecoder::IsMediaTypeAC3(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_DOLBY_AC3 ||
            pMediaType->subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3);
}

BOOL CAudioDecoder::IsMediaTypeDTS(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_DTS ||
            pMediaType->subtype == MEDIASUBTYPE_WAVE_DTS);
}

BOOL CAudioDecoder::IsMediaTypeMP3(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_MP3 ||
            pMediaType->subtype == MEDIASUBTYPE_MPEG1AudioPayload ||
            pMediaType->subtype == MEDIASUBTYPE_MPEG1Payload ||
            pMediaType->subtype == MEDIASUBTYPE_MPEG1Packet ||
            pMediaType->subtype == MEDIASUBTYPE_MPEG2_AUDIO ||
            pMediaType->subtype == MEDIASUBTYPE_MPEG2_AUDIO_MPCBUG);
}

BOOL CAudioDecoder::IsMediaTypePCM(const AM_MEDIA_TYPE* pMediaType)
{
    return (pMediaType->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO);
}

HRESULT CAudioDecoder::SendDigitalData(WORD HeaderWord, short DigitalLength, long FinalLength, const char* pData)
{
    HRESULT hr = S_OK;
    if(m_BytesLeftInBuffer == 0)
    {
        hr = GetOutputSampleAndPointer();
        CHECK(hr);
    }

    *m_pDataOut++ = 0x72;
    *m_pDataOut++ = 0xf8;
    *m_pDataOut++ = 0x1f;
    *m_pDataOut++ = 0x4e;

    m_BytesLeftInBuffer -= 4;
    ASSERT(m_BytesLeftInBuffer >=0);
    if(m_BytesLeftInBuffer == 0)
    {
        hr = Deliver(true);
        if(hr != S_OK)
        {
            return hr;
        }
        hr = GetOutputSampleAndPointer();
        CHECK(hr);
    }
	//m_AC3SilenceFrames = 0;
    if(m_AC3SilenceFrames == 0)
    {
        *m_pDataOut++ = (BYTE)HeaderWord;
        *m_pDataOut++ = (BYTE)(HeaderWord >> 8);
        *m_pDataOut++ = (BYTE)(DigitalLength << 3);
        *m_pDataOut++ = (BYTE)(DigitalLength >> 5);
    }
    else
    {
        *m_pDataOut++ = 0x00;
        *m_pDataOut++ = 0x03;
        *m_pDataOut++ = 0xbf;
        *m_pDataOut++ = 0xc0;
    }

    m_BytesLeftInBuffer -= 4;
    ASSERT(m_BytesLeftInBuffer >=0);
    if(m_BytesLeftInBuffer == 0)
    {
        hr = Deliver(true);
        if(hr != S_OK)
        {
            return hr;
        }
        hr = GetOutputSampleAndPointer();
        CHECK(hr);
    }

    long BytesToGo;

    if(m_AC3SilenceFrames == 0)
    {
        BytesToGo = DigitalLength;

        if(DigitalLength > m_BytesLeftInBuffer)
        {
            _swab((char*)pData, (char*)m_pDataOut, m_BytesLeftInBuffer);
            pData += m_BytesLeftInBuffer;
            BytesToGo -= m_BytesLeftInBuffer;
            hr = Deliver(true);
            if(hr != S_OK)
            {
                return hr;
            }
            CHECK(hr);
            hr = GetOutputSampleAndPointer();
            CHECK(hr);
        }
        _swab((char*)pData, (char*)m_pDataOut, BytesToGo);
        m_pDataOut += BytesToGo;
        m_BytesLeftInBuffer -= BytesToGo;
        BytesToGo = FinalLength - DigitalLength - 8;
    }
    else
    {
        BytesToGo =  FinalLength - 8;
        --m_AC3SilenceFrames;
    }

    if(BytesToGo > 0)
    {
		if(m_BytesLeftInBuffer == 0)
		{
			hr = Deliver(true);
			if(hr != S_OK)
			{
				return hr;
			}
			hr = GetOutputSampleAndPointer();
			CHECK(hr);
		}

        if (BytesToGo > m_BytesLeftInBuffer)
        {
            ZeroMemory(m_pDataOut, m_BytesLeftInBuffer);
            BytesToGo -= m_BytesLeftInBuffer;
            hr = Deliver(true);
            if(hr != S_OK)
            {
                return hr;
            }
            CHECK(hr);
            hr = GetOutputSampleAndPointer();
            CHECK(hr);
        }
        ZeroMemory(m_pDataOut, BytesToGo);
        m_pDataOut += BytesToGo;
        m_BytesLeftInBuffer -= BytesToGo;
        ASSERT(m_BytesLeftInBuffer >= 0);
    }

    if(m_BytesLeftInBuffer == 0)
    {
        hr = Deliver(true);
        if(hr != S_OK)
        {
            return hr;
        }
        CHECK(hr);
    }

    return hr;
}

HRESULT CAudioDecoder::UpdateStartTime()
{
    if(m_rtNextFrameStart != _I64_MIN)
    {
        REFERENCE_TIME OutputStart = m_rtNextFrameStart;

        if(m_BytesLeftInBuffer > 0)
        {
            OutputStart -= 10000000i64 * (m_InternalMT.lSampleSize - m_BytesLeftInBuffer) / (m_OutputSampleRate * m_ChannelsRequested * m_SampleSize);
        }
        // always go forward but never go back for small clock adjustments
        // this sort of readjustement seems to happen with MPEG streams
        // as the times are encoded as PTS values which are less accurate than 
        // the MS time values
        if(m_rtOutputStart == 0 || (OutputStart - m_rtOutputStart) >= 0 || (OutputStart - m_rtOutputStart) < -2000)
        {
            m_rtOutputStart = OutputStart;
        }
        else 
        {
            LOG(DBGLOG_FLOW, ("Minor adjustment ignored %I64d - %I64d\n", OutputStart, m_rtOutputStart));
        }
   }
    
    m_rtNextFrameStart = _I64_MIN;
    return S_OK;
}
