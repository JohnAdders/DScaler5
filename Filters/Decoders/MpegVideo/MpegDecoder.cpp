///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder.cpp,v 1.74 2007-02-22 07:18:55 adcockj Exp $
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
// This file was based on the mpeg2dec filter which is part of the MPC
// program see http://sf.net/projects/guliverkli/ for more details
//
// Changes made to files by John Adcock 06/02/04
//  - Removed use of MFC
//  - Replaced use of ATL with YACL
//  - Replaced Baseclasses with FilterLib
//  - Removed DeCSS
//  - Modified to use libmpeg2 0.4.0 instead of customised version
//  - Fixed various timestamp issues
//  - Added Support for GetDecoderCaps
//  - Removed Contrast/Brightness etc adjustments
//  - Improved 3:2 film detection
//  - Removed own deinterlacing and set media type so that VMR can deinterlace
//
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.73  2005/10/05 16:21:16  adcockj
// get NV12 mode connection working
//
// Revision 1.72  2005/10/05 14:30:42  adcockj
// implemented NV12 just to see if we get connection, will produce corrupted output at present
//
// Revision 1.71  2005/03/20 14:18:14  adcockj
// aspect fixes and new deinterlacing test parameter
//
// Revision 1.70  2005/03/08 13:35:09  adcockj
// Better handling of square pixel formats
// Turn off quality control
//
// Revision 1.69  2005/03/04 17:54:37  adcockj
// Menu fixes & added rate stuff locking
//
// Revision 1.68  2005/02/17 09:31:45  adcockj
// Added analog blanking option
// Removed force Dscaler filter option
// Another proposed fix for menus
//
// Revision 1.67  2005/02/09 07:27:32  adcockj
// fixed buffer handling on reset
//
// Revision 1.66  2005/02/08 15:32:34  adcockj
// Added CSS handling
//
// Revision 1.65  2005/02/03 13:40:54  adcockj
// Improved seek support
//
// Revision 1.64  2005/01/21 13:54:45  adcockj
// Reset some test changes and added soem clock monitoring code
//
// Revision 1.63  2005/01/04 17:53:43  adcockj
// added option to force dscalewr filter to be loaded2
//
// Revision 1.62  2004/12/20 08:51:52  adcockj
// added back statistics
//
// Revision 1.61  2004/12/13 16:59:57  adcockj
// flag based film detection
//
// Revision 1.60  2004/12/12 20:35:15  adcockj
// fixed leak
//
// Revision 1.59  2004/12/06 18:04:58  adcockj
// Major improvements to deinterlacing
//
// Revision 1.58  2004/11/27 22:15:22  adcockj
// whoops, shouldn't have checked in css tests
//
// Revision 1.57  2004/11/26 15:15:03  adcockj
// Fixed overlay stutter
//
// Revision 1.56  2004/11/25 17:22:10  adcockj
// Fixed some more connection issues
//
// Revision 1.55  2004/11/18 07:40:56  adcockj
// a few test bug fixes
//
// Revision 1.54  2004/11/09 17:20:38  adcockj
// Stutter fix
//
// Revision 1.53  2004/11/06 14:36:09  adcockj
// VS6 project update
//
// Revision 1.52  2004/11/05 16:58:31  adcockj
// Fix for use with other containers
//
// Revision 1.51  2004/11/01 14:09:38  adcockj
// More DScaler filter insipred changes
//
// Revision 1.50  2004/10/31 14:19:49  adcockj
// fixed issues with avgtimeperframe
//
// Revision 1.49  2004/10/28 15:52:24  adcockj
// Moved video output pin code into new class
//
// Revision 1.48  2004/10/28 09:05:25  adcockj
// Fixed issue with negative settings
//
// Revision 1.47  2004/10/26 16:23:44  adcockj
// Improve subpicture performance
//
// Revision 1.46  2004/10/22 07:34:40  adcockj
// fix for some connection issues
//
// Revision 1.45  2004/10/21 18:51:41  adcockj
// Simple VMR compatable quality control
//
// Revision 1.44  2004/09/27 20:59:17  adcockj
// Not fully tested MCE fixes
//
// Revision 1.43  2004/09/23 14:27:58  adcockj
// preliminary fixed for reconnection issues
//
// Revision 1.42  2004/09/13 14:28:44  adcockj
// Connection changes and crash fixes
//
// Revision 1.41  2004/09/10 15:35:57  adcockj
// Bug fixes for problems found in 0.0.2 with MPEG-1 & overlay
//
// Revision 1.40  2004/08/31 16:33:41  adcockj
// Minor improvements to quality control
// Preparation for next version
// Start on integrating film detect
//
// Revision 1.39  2004/08/06 08:38:53  adcockj
// Added optional YV12 output type
//
// Revision 1.38  2004/08/03 08:55:56  adcockj
// Fixes for seeking issues
//
// Revision 1.37  2004/07/29 13:44:59  adcockj
// More fixes for Laurent's issues
//
// Revision 1.36  2004/07/29 08:31:07  adcockj
// Fixed issue with 100% CPU with MPC and overlay
//
// Revision 1.35  2004/07/28 16:32:34  adcockj
// Fixes Blight's problems from the forum
//
// Revision 1.34  2004/07/23 21:00:12  adcockj
// Fixed compilation problem in VS6
//
// Revision 1.33  2004/07/21 15:05:24  adcockj
// Fixed some issues with ff & rew
//
// Revision 1.32  2004/07/20 16:37:48  adcockj
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
// Revision 1.31  2004/07/16 15:58:01  adcockj
// Fixed compilation issues under .NET
// Changed name of filter
// Some performance improvements to libmpeg2
//
// Revision 1.30  2004/07/11 14:36:00  adcockj
// Improved performance under debug
//
// Revision 1.29  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.28  2004/05/25 16:59:29  adcockj
// fixed issues with new buffered pin
//
// Revision 1.27  2004/05/24 06:29:26  adcockj
// Interim buffer fix
//
// Revision 1.26  2004/05/12 17:01:03  adcockj
// Hopefully correct treatment of timestamps at last
//
// Revision 1.25  2004/05/10 16:48:50  adcockj
// Imporved handling of sequence changes
//
// Revision 1.24  2004/05/10 06:40:27  adcockj
// Fixes for better compatability with PES streams
//
// Revision 1.23  2004/05/06 06:38:06  adcockj
// Interim fixes for connection and PES streams
//
// Revision 1.22  2004/04/29 16:16:45  adcockj
// Yet more reconnection fixes
//
// Revision 1.21  2004/04/28 16:32:36  adcockj
// Better dynamic connection
//
// Revision 1.20  2004/04/20 16:30:16  adcockj
// Improved Dynamic Connections
//
// Revision 1.19  2004/04/16 16:19:44  adcockj
// Better reconnection and improved AFD support
//
// Revision 1.18  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.17  2004/04/13 06:23:42  adcockj
// Start to improve aspect handling
//
// Revision 1.16  2004/04/08 16:41:57  adcockj
// Tidy up subpicture support
//
// Revision 1.15  2004/04/06 16:46:12  adcockj
// DVD Test Annex Compatability fixes
//
// Revision 1.14  2004/03/15 17:16:02  adcockj
// Better PES header handling - Inspired by Gabest's latest MPC patch
//
// Revision 1.13  2004/03/11 16:52:21  adcockj
// Improved subpicture drawing with different video widths/heights
//
// Revision 1.12  2004/03/08 17:04:01  adcockj
// Removed all inline assembler to remove dependence on MS compilers
//
// Revision 1.11  2004/03/02 07:54:57  adcockj
// Slightly improved discontinity handling
//
// Revision 1.10  2004/02/29 19:06:36  adcockj
// Futher dynamic format change fix
//
// Revision 1.9  2004/02/29 13:47:48  adcockj
// Format change fixes
// Minor library updates
//
// Revision 1.8  2004/02/27 17:07:01  adcockj
// Fixes for improved handling of two way dynamic format changes
// Support for library fixes
//
// Revision 1.7  2004/02/25 17:14:02  adcockj
// Fixed some timing bugs
// Tidy up of code
//
// Revision 1.6  2004/02/16 17:25:01  adcockj
// Fix build errors, locking problems and DVD compatability
//
// Revision 1.5  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.4  2004/02/10 13:24:12  adcockj
// Lots of bug fixes + corrected interlaced YV12 upconversion
//
// Revision 1.3  2004/02/09 07:57:33  adcockj
// Stopping big fix
// Timestamps issue test fix
//
// Revision 1.2  2004/02/06 16:41:41  adcockj
// Added frame smoothing and forced subs parameters
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MpegDecoder.h"
#include "EnumPins.h"
#include "DSCSSInputPin.h"
#include "DSBufferedInputPin.h"
#include "DSVideoOutPin.h"
#include "MediaBufferWrapper.h"
#include "MediaTypes.h"
#include "DSUtil.h"
#include "MoreUuids.h"

extern HINSTANCE g_hInstance;

const long CMpegDecoder::MAX_SPEED = 4;

CMpegDecoder::CMpegDecoder() :
    CDSBaseFilter(L"MpegVideo Filter", 2, 2)
{
    LOG(DBGLOG_FLOW, ("CMpegDecoder::CreatePins\n"));
    
    m_VideoInPin = new CDSCSSInputPin;
    if(m_VideoInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_VideoInPin->AddRef();
    m_VideoInPin->SetupObject(this, L"Video In");

    m_SubpictureInPin = new CDSCSSInputPin;
    if(m_SubpictureInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_SubpictureInPin->AddRef();
    m_SubpictureInPin->SetupObject(this, L"SubPicture");
    
    // can't use m_VideoOutPin due to casting
    m_OutputPins[0] = new CDSVideoOutPin();
    if(m_VideoOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 3"));
    }
    m_VideoOutPin->AddRef();
    m_VideoOutPin->SetupObject(this, L"Video Out");

    m_CCOutPin = new CDSOutputPin();
    if(m_CCOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 4"));
    }
    m_CCOutPin->AddRef();
    m_CCOutPin->SetupObject(this, L"~CC");

    AM_MEDIA_TYPE MT;
    InitMediaType(&MT);
    MT.majortype = MEDIATYPE_AUXLine21Data;
    MT.subtype = MEDIASUBTYPE_Line21_GOPPacket;
    MT.formattype = GUID_NULL;
    
    m_CCOutPin->SetType(&MT);
    
    ClearMediaType(&MT);

    m_spon = TRUE;

    m_rate.Rate = 10000;
    m_rate.StartTime = 0;

    m_LastOutputTime = 0;
    m_PicturesSinceSequence = 0;

    m_ProgressiveChroma = false;
    m_NextFrameDeint = DIBob;
    m_dec = NULL;
    m_ChromaType = CHROMA_420;

    m_ratechange.Rate = 10000;
    m_ratechange.StartTime = -1;
    m_CorrectTS = false;
    m_dec = NULL;

    m_MpegWidth = 0;
    m_MpegHeight = 0;

    m_OutputWidth = 0;
    m_OutputHeight = 0;
    m_InternalPitch = 0;
    m_CurrentPicture = NULL;

    m_ControlFlags = 0;
    m_FilmCameraModeHint = false;
    m_LetterBoxed = false;

    m_ARMpegX = 4;
    m_ARMpegY = 3;
    m_ARAdjustX = 1;
    m_ARAdjustY = 1;
    m_AFD = 0;
    m_PanAndScanDVD = false;

    m_ConnectedToIn = DEFAULT_INFILTER;

    ZeroMemory(&m_CurrentSequence, sizeof(mpeg2_sequence_t));

    m_LastPictureWasStill = false;
}

CMpegDecoder::~CMpegDecoder()
{
    LOG(DBGLOG_FLOW, ("CMpegDecoder::~CMpegDecoder\n"));
}

STDMETHODIMP CMpegDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_ALL, ("CMpegDecoder::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CMpegDecoder;
    return S_OK;
}


STDMETHODIMP CMpegDecoder::get_Name(BSTR* Name)
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

STDMETHODIMP CMpegDecoder::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CMpegDecoder::get_Authors(BSTR* Authors)
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

HRESULT CMpegDecoder::ParamChanged(DWORD dwParamIndex)
{
    HRESULT hr = S_OK;
    switch(dwParamIndex)
    {
    case DISPLAYFORCEDSUBS:
        // don't care when this changes
        break;
    case FRAMESMOOTH32:
        // don't care when this changes
        break;
    case DEINTMODE:
        // don't care when this changes
        break;
    case VIDEODELAY:
        // don't care when this changes
        break;
    case DOACCURATEASPECT:
        break;
    case DVBASPECTPREFS:
        {
            // reset the sizes when these param change
            // ensure we don't do this while we'rte in the middle of
            // processing
            CProtectCode WhileVarInScope(&m_DeliverLock);
            CorrectOutputSize();
        }
        break;
    case IDCT:
        switch(GetParamEnum(IDCT))
        {
        case REFERENCE_IDCT:
            if(mpeg2_accel(0) != 0)
            {
                hr = S_FALSE;
            }
            break;
        case MMX_IDCT:
            if(mpeg2_accel(MPEG2_ACCEL_X86_MMX) != MPEG2_ACCEL_X86_MMX)
            {
                hr = S_FALSE;
            }
        default:
            if(mpeg2_accel(MPEG2_ACCEL_DETECT) <= MPEG2_ACCEL_X86_MMX)
            {
                hr = S_FALSE;
            }
        }
        break;
    case OUTPUTSPACE:
        if(m_VideoOutPin->m_ConnectedPin)
        {
            return S_FALSE;
        }
        break;
    case ANALOGBLANKING:
        break;
    case FIELD1FIRST:
        if(m_VideoOutPin->m_ConnectedPin)
        {
            return S_FALSE;
        }
        break;
    }
    return hr;
}


HRESULT CMpegDecoder::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_DvdSubPic && pPin == m_SubpictureInPin)
    {
        return SupportPropSetSubPic(dwPropID, pTypeSupport);
    }
    else if(guidPropSet == AM_KSPROPSETID_TSRateChange && pPin == m_VideoInPin)
    {
        return SupportPropSetRate(dwPropID, pTypeSupport);
    }
    return E_NOTIMPL;
}

HRESULT CMpegDecoder::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_DvdSubPic)
    {
        return SetPropSetSubPic(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData);
    }
    else if(guidPropSet == AM_KSPROPSETID_TSRateChange && pPin == m_VideoInPin)
    {
        return SetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData);
    }
    return E_NOTIMPL;
}

HRESULT CMpegDecoder::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_DvdSubPic)
    {
        return GetPropSetSubPic(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData, pcbReturned);
    }
    else if(guidPropSet == AM_KSPROPSETID_TSRateChange && pPin == m_VideoInPin)
    {
        return GetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData, pcbReturned);
    }
    return E_NOTIMPL;
}

HRESULT CMpegDecoder::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
	// temporary quality control
	// basically let the renderer do quality control
	// except when we are showing stills (normally these
	// only occur in DVD menus) in which case
	// we eat the quality control messages
	// and just say we have coped, this should
	// make the renderer dispaly all our updates
	// to stills which should reduce the problems
	// you can get with quality control interfering
	// with updates to subpicture frames on still pages
	if(m_LastPictureWasStill)
	{
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}

    if(pPin == m_VideoOutPin)
    {
        if(q.Type == Famine)
        {
            SI(IQualityControl) QualityControl = m_VideoInPin->m_ConnectedPin;
            if(QualityControl)
            {
                LOG(DBGLOG_FLOW, ("Coped With Famine - %d\n", q.Late));
                return QualityControl->Notify(pSelf, q);
            }
            else
            {
                LOG(DBGLOG_FLOW, ("Ignored Famine - %d\n", q.Late));
                return E_FAIL;
            }
        }
        if(q.Type == Flood)
        {
            LOG(DBGLOG_ALL, ("Ignored Flood - %d\n", q.Late));
            return E_FAIL;
        }
    }
    return S_OK;
}

STDMETHODIMP CMpegDecoder::GetDecoderCaps(DWORD dwCapIndex,DWORD* lpdwCap)
{
    switch(dwCapIndex)
    {
    case AM_QUERY_DECODER_VMR_SUPPORT:
    case AM_QUERY_DECODER_DVD_SUPPORT:
    case AM_QUERY_DECODER_ATSC_SD_SUPPORT:   
    case AM_QUERY_DECODER_ATSC_HD_SUPPORT:
    case AM_GETDECODERCAP_QUERY_VMR9_SUPPORT:
        *lpdwCap = DECODER_CAP_SUPPORTED;
        break;
    // lie so that Media centre will use us
    case AM_QUERY_DECODER_DXVA_1_SUPPORT:
        *lpdwCap = DECODER_CAP_SUPPORTED;
        break;
    default:
        *lpdwCap = DECODER_CAP_NOTSUPPORTED;
        break;
    }
    return S_OK;
}

HRESULT CMpegDecoder::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProperties, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        pProperties->cBuffers = 200;
        pProperties->cbBuffer = 2048;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else if(pPin == m_SubpictureInPin)
    {
        pProperties->cBuffers = 1;
        pProperties->cbBuffer = 2048;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else if(pPin == m_VideoOutPin)
    {
        pProperties->cBuffers = 1;
        pProperties->cbBuffer = ExtractBIH(&m_VideoOutPin->m_ConnectedMediaType)->biSizeImage;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else if(pPin == m_CCOutPin)
    {
        pProperties->cBuffers = 1;
        pProperties->cbBuffer = 2048;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else
    {
        return E_UNEXPECTED;
    }
    return S_OK;
}

bool CMpegDecoder::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool Result = false;
    if(pPin == m_VideoInPin)
    {
        Result = (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                  pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO) || 
                    (pmt->majortype == MEDIATYPE_MPEG2_PACK &&
                     pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO) || 
                    (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                     pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO) || 
                    (pmt->majortype == MEDIATYPE_Video && 
                     pmt->subtype == MEDIASUBTYPE_MPEG2_VIDEO) || 
                    (pmt->majortype == MEDIATYPE_Video && 
                     pmt->subtype == MEDIASUBTYPE_MPEG1Packet) || 
                    (pmt->majortype == MEDIATYPE_Video && 
                     pmt->subtype == MEDIASUBTYPE_MPEG1Payload);
    }
    else if(pPin == m_SubpictureInPin)
    {
        Result = (pmt->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && 
                    pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE) || 
                 (pmt->majortype == MEDIATYPE_MPEG2_PACK && 
                    pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE) ||
                 (pmt->majortype == MEDIATYPE_MPEG2_PES && 
                    pmt->subtype == MEDIASUBTYPE_DVD_SUBPICTURE);
    }
    else if(pPin == m_VideoOutPin)
    {
        int wout = 0, hout = 0;
        long arxout = 0, aryout = 0;
        Result = ((pmt->majortype == MEDIATYPE_Video ||
                    pmt->majortype == CLSID_CDScaler) && 
                  (pmt->subtype == MEDIASUBTYPE_YUY2 ||
                   pmt->subtype == MEDIASUBTYPE_YV12 ||
                   pmt->subtype == MEDIASUBTYPE_NV12 ||
                    pmt->subtype == MEDIASUBTYPE_ARGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB24 ||
                    pmt->subtype == MEDIASUBTYPE_RGB565 ||
                    pmt->subtype == MEDIASUBTYPE_RGB555));
    }
    else if(pPin == m_CCOutPin)
    {
        Result = (pmt->majortype == MEDIATYPE_AUXLine21Data) && 
                 (pmt->subtype == MEDIASUBTYPE_Line21_GOPPacket);
    }
    return Result;
}

HRESULT CMpegDecoder::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        return ProcessMPEGSample(InSample, pSampleProperties);
    }
    else if(pPin == m_SubpictureInPin)
    {
        return ProcessSubPicSample(InSample, pSampleProperties);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CMpegDecoder::GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText)
{
    if(dwParamIndex == DEINTMODE)
    {
        return GetEnumTextDeintMode(ppwchText);
    }
    else if(dwParamIndex == DVBASPECTPREFS)
    {
        return GetEnumTextDVBAspectPrefs(ppwchText);
    }
    else if(dwParamIndex == IDCT)
    {
        return GetEnumTextIDCTToUse(ppwchText);
    }
    else if(dwParamIndex == OUTPUTSPACE)
    {
        return GetEnumTextOutputSpace(ppwchText);
    }
    else
    {
        return E_NOTIMPL;
    }
}


HRESULT CMpegDecoder::GetEnumTextDeintMode(WCHAR **ppwchText)
{
    wchar_t Text[] = L"Deinterlace Mode\0" L"None\0" L"Automatic\0" L"Force Weave\0" L"Force Bob\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}

HRESULT CMpegDecoder::GetEnumTextIDCTToUse(WCHAR **ppwchText)
{
    wchar_t Text[] = L"IDCT To Use\0" L"None\0" L"Reference\0" L"MMX Only\0" L"Accelerated\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}

HRESULT CMpegDecoder::GetEnumTextDVBAspectPrefs(WCHAR **ppwchText)
{
    wchar_t Text[] = L"DVB Aspect Preferences\0" L"None\0" L"16:9 Display\0" L"4:3 Display Center Cut out\0" L"4:3 Display LetterBox\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}

HRESULT CMpegDecoder::GetEnumTextOutputSpace(WCHAR **ppwchText)
{
    wchar_t Text[] = L"Output Colour Space\0" L"None\0" L"YV12\0" L"YUY2\0" L"NV12\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}


HRESULT CMpegDecoder::Flush(CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        FlushMPEG();
    }
    else if(pPin == m_SubpictureInPin)
    {
        FlushSubPic();
    }
    return S_OK;
}

HRESULT CMpegDecoder::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        LOG(DBGLOG_FLOW, ("New Segment %010I64d - %010I64d  @ %f\n", tStart, tStop, dRate));
        m_LastOutputTime = 0;

        CProtectCode WhileVarInScope(&m_RateLock);
        m_rate.Rate = (LONG)(10000 / dRate);
        m_rate.StartTime = 0;

        m_IsDiscontinuity = true;
        return S_OK;
    }
    else if(pPin == m_SubpictureInPin)
    {
        return S_FALSE;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

HRESULT CMpegDecoder::SendOutLastSamples(CDSBasePin* pPin)
{
    return S_OK;
}


HRESULT CMpegDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{

    if(pPin == m_VideoOutPin)
    {
        if(!m_VideoInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

		DWORD VideoFlags = 0;
		switch(GetParamEnum(OUTPUTSPACE))
		{
		case SPACE_YUY2:
			VideoFlags |= VIDEOTYPEFLAG_FORCE_YUY2;
			break;
		case SPACE_YV12:
			VideoFlags |= VIDEOTYPEFLAG_FORCE_YV12;
			break;
		}
        VideoFlags |= (GetParamBool(FIELD1FIRST))?VIDEOTYPEFLAG_SET_FIELD1FIRST:0;
        return m_VideoOutPin->CreateSuitableMediaType(pmt, TypeNum, VideoFlags, m_ControlFlags);
    }
    else if(pPin == m_SubpictureInPin)
    {
        if(TypeNum == 0)
        {
            pmt->majortype = MEDIATYPE_DVD_ENCRYPTED_PACK;
            pmt->subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
            pmt->formattype = GUID_NULL;
            return S_OK;
        }
        else if(TypeNum == 1)
        {
            pmt->majortype = MEDIATYPE_MPEG2_PACK;
            pmt->subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
            pmt->formattype = GUID_NULL;
            return S_OK;
        }
        else if(TypeNum == 2)
        {
            pmt->majortype = MEDIASUBTYPE_DVD_SUBPICTURE;
            pmt->subtype = MEDIASUBTYPE_DVD_SUBPICTURE;
            pmt->formattype = GUID_NULL;
            return S_OK;
        }
        else
        {
            return VFW_S_NO_MORE_ITEMS;
        }

    }
    else if(pPin == m_CCOutPin)
    {
        if(TypeNum == 0)
        {
            pmt->majortype = MEDIATYPE_AUXLine21Data;
            pmt->subtype = MEDIASUBTYPE_Line21_GOPPacket;
            pmt->formattype = GUID_NULL;
            return S_OK;
        }
        else
        {
            return VFW_S_NO_MORE_ITEMS;
        }
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }
}

HRESULT CMpegDecoder::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    HRESULT hr = S_OK;

    if(pPin == m_VideoInPin)
    {
        m_MpegWidth = m_MpegHeight = 0;
        m_ARMpegX = 4; 
        m_ARMpegY = 3;
        m_ARAdjustX = m_ARAdjustY = 1;
        ExtractDim(pMediaType, m_MpegWidth, m_MpegHeight, m_ARMpegX, m_ARMpegY);
        m_MpegHeight = abs(m_MpegHeight);
        m_ControlFlags = 0;
        m_PanAndScanDVD = false;
        m_VideoOutPin->SetPanScanX(0);
        m_VideoOutPin->SetPanScanY(0);
        m_FilmCameraModeHint = false;
        m_LetterBoxed = false;
        m_AFD = 0;
        m_fWaitForKeyFrame = true;

        if(pMediaType->formattype == FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pMediaType->pbFormat;
            m_ControlFlags = vih->dwControlFlags;
        	m_VideoOutPin->SetAvgTimePerFrame(vih->AvgTimePerFrame);
		}
        else if(pMediaType->formattype == FORMAT_MPEG2_VIDEO)
        {
            MPEG2VIDEOINFO* mvih = (MPEG2VIDEOINFO*)pMediaType->pbFormat;
            m_ControlFlags = mvih->hdr.dwControlFlags;
            if(mvih->dwFlags & AMMPEG2_DoPanScan)
            {
                LOG(DBGLOG_FLOW, ("Pan & Scan Sent\n"));
                m_PanAndScanDVD = true;
            }
            if(mvih->dwFlags & AMMPEG2_FilmCameraMode)
            {
                LOG(DBGLOG_FLOW, ("Film Hint Sent\n"));
                m_FilmCameraModeHint = true;
            }
            if(mvih->dwFlags & AMMPEG2_SourceIsLetterboxed)
            {
                LOG(DBGLOG_FLOW, ("Letterboxed Sent\n"));
                m_LetterBoxed = true;
            }
            m_VideoOutPin->SetAvgTimePerFrame(mvih->hdr.AvgTimePerFrame);
        }
		else if(pMediaType->formattype == FORMAT_MPEGVideo ||
					pMediaType->formattype == FORMAT_VideoInfo)
		{
            VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
            m_ControlFlags = 0;
        	m_VideoOutPin->SetAvgTimePerFrame(vih->AvgTimePerFrame);
		}

        CorrectOutputSize();
    }
    else if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyFormatChange(pMediaType);
    }
    return S_OK;
}

HRESULT CMpegDecoder::NotifyConnected(CDSBasePin* pPin)
{
    HRESULT hr = S_OK;
    if(pPin == m_VideoInPin)
    {
        CLSID Clsid;

        HRESULT hr = m_VideoInPin->GetConnectedFilterCLSID(&Clsid);
        if(Clsid == CLSID_DVDNavigator)
        {
            m_ConnectedToIn = DVD_INFILTER;
        }
        else
        {
            m_ConnectedToIn = DEFAULT_INFILTER;
        }
    }
    if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyConnected();
    }
    return S_OK;
}


HRESULT CMpegDecoder::ProcessMPEGSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties)
{
    CProtectCode WhileVarInScope(&m_DeliverLock);

    if(pSampleProperties->dwStreamId != AM_STREAM_MEDIA)
        return m_VideoOutPin->SendSample(InSample);

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

    if(len <= 0) 
        return S_OK;

    if(*(DWORD*)pDataIn == 0xBB010000)
    {
        len -= 4; 
        pDataIn += 4;
        int hdrlen = ((pDataIn[0]<<8)|pDataIn[1]) + 2;
        len -= hdrlen; 
        pDataIn += hdrlen;
    }

    if(len <= 0) 
        return S_OK;

    if((*(DWORD*)pDataIn&0xE0FFFFFF) == 0xE0010000)
    {
        // skip past the PES header and Id
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
    
            // skip past all the optional headers and the length byte itself
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

        if(ExpectedLength > 0)
        {
            ExpectedLength -= pDataIn - EndOfPacketLength;
            len = min(len, ExpectedLength); 
        }
    }

    if(len <= 0) 
        return S_OK;


    if(pSampleProperties->tStart == -1)
    {
        pSampleProperties->dwSampleFlags &= ~AM_SAMPLE_TIMEVALID;
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
    {
        LOG(DBGLOG_FLOW, ("Got Discontinuity\n"));
        m_IsDiscontinuity = true;
        m_fWaitForKeyFrame = true;
        m_fFilm = false;
    }

    mpeg2_buffer(m_dec, pDataIn, pDataIn + len);
    return ProcessMPEGBuffer(pSampleProperties);
}

HRESULT CMpegDecoder::ProcessMPEGBuffer(AM_SAMPLE2_PROPERTIES* pSampleProperties)
{
    HRESULT hr = S_OK;
    while(1)
    {
        mpeg2_state_t state = mpeg2_parse(m_dec);

        if(mpeg2_info(m_dec)->user_data_len > 0)
        {
            hr = ProcessUserData(state, mpeg2_info(m_dec)->user_data, mpeg2_info(m_dec)->user_data_len);
            CHECK(hr);
        }

        switch(state)
        {
        case STATE_BUFFER:
            return S_OK;
            break;
        case STATE_INVALID:
            // the decoder seems to recover OK now
            // so we are just going to ignore these
            LOG(DBGLOG_FLOW, ("STATE_INVALID\n"));
            break;
        case STATE_SEQUENCE:
            hr = ProcessNewSequence();
            CHECK(hr);
            break;
        case STATE_SEQUENCE_MODIFIED:
            hr = ProcessModifiedSequence();
            CHECK(hr);
            break;
        case STATE_PICTURE:
            hr = ProcessPictureStart(pSampleProperties);
            CHECK(hr);
            break;
        case STATE_SLICE:
        case STATE_END:
        case STATE_INVALID_END:
            // we will need to refresh subpictures if we are at a clean end of stream
            m_LastPictureWasStill = (state == STATE_END);
            // we tell the decoder to decode single frames progressively
            // these tend to be used as still or DVD menus
            hr = ProcessPictureDisplay(m_LastPictureWasStill && (m_PicturesSinceSequence == 0));
            if(hr != S_OK)
            {
                return hr;
            }
            break;
        case STATE_GOP:
        case STATE_SEQUENCE_REPEATED:
        case STATE_SLICE_1ST:
        case STATE_PICTURE_2ND:
            // don't care about these
            break;
        default:
            LOG(DBGLOG_FLOW, ("Unexpected State in stream %d\n", state));
            break;
        }
    }

    // should never get here
    return E_UNEXPECTED;
}

HRESULT CMpegDecoder::Deliver(bool fRepeatLast)
{
    if(m_CurrentPicture == NULL)
        return S_OK;

    // sometimes we get given a repeat frame request
    // when we're not really ready

    TCHAR frametype[] = {'?','I', 'P', 'B', 'D'};
    LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [%c] [num %d prfr %d tff %d rff %d] (%dx%d %d) (preroll %d) %d\n", 
        m_CurrentPicture->m_rtStart, m_CurrentPicture->m_rtStop,
        frametype[m_CurrentPicture->m_Flags&PIC_MASK_CODING_TYPE],
        m_CurrentPicture->m_NumFields,
        !!(m_CurrentPicture->m_Flags&PIC_FLAG_PROGRESSIVE_FRAME),
        !!(m_CurrentPicture->m_Flags&PIC_FLAG_TOP_FIELD_FIRST),
        !!(m_CurrentPicture->m_NumFields == 3),
        m_OutputWidth, m_OutputHeight, m_InternalPitch,
        !!(m_CurrentPicture->m_rtStop < 0 || m_fWaitForKeyFrame),
        !!(HasSubpicsToRender(m_CurrentPicture->m_rtStart))));

    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;
    
    if(!fRepeatLast)
    {
        CProtectCode WhileVarInScope(&m_RateLock);

        // cope with a change in rate       
        if(m_rate.Rate != m_ratechange.Rate && m_CurrentPicture->m_rtStart >= m_ratechange.StartTime)
        {
            m_rate.Rate = m_ratechange.Rate;
            // looks like we need to do this ourselves
            // as the time past originally seems like a best guess only
            m_rate.StartTime = m_CurrentPicture->m_rtStart;
            m_IsDiscontinuity = true;
            LOG(DBGLOG_FLOW, ("Got Rate %010I64d %d\n", m_rate.StartTime, m_rate.Rate));
        }

        if(m_IsDiscontinuity)
        {
            // if we're at a Discontinuity use the times we're being sent in
            rtStart = m_rate.StartTime + (m_CurrentPicture->m_rtStart - m_rate.StartTime) * abs(m_rate.Rate) / 10000;
        }
        else
        {
            // if we're not at a Discontinuity
            // make sure that time are contiguous
            rtStart = m_LastOutputTime;
        }

        // if we want smooth frames then we simply adjust the stop time by half
        // a frame so that 3:2 becomes 2.5:2.5
        // the way we always use the previous stop time as the start time for the next 
        // frame will mean that we only have to worry about adjusting the 3 field parts.
        rtStop = m_rate.StartTime + (m_CurrentPicture->m_rtStop - m_rate.StartTime) * abs(m_rate.Rate) / 10000;

        if(GetParamBool(FRAMESMOOTH32) && m_fFilm && m_CurrentPicture->m_NumFields == 3)
        {
            rtStop -= m_AvgTimePerFrame / 2 * abs(m_rate.Rate) / 10000;
        }

        rtStop += GetParamInt(VIDEODELAY) * 10000;

        // ensure that times always go up
        // and that Stop is always greater than start
        if(rtStart < m_LastOutputTime && m_LastOutputTime != 0)
        {
            LOG(DBGLOG_FLOW, ("Adjusted time to avoid backwards %010I64d - %010I64d\n", rtStart, m_LastOutputTime));
            rtStart = m_LastOutputTime;
        }

        if(rtStop <= rtStart)
        {
            rtStop = rtStart + 100;
        }

        m_LastOutputTime = rtStop;
    }

    HRESULT hr;

    // cope with dynamic format changes from our side
    // will possibly call NegotiateAllocator on the output pins
    // which flushes so we shouldn't have any samples outstanding here
    if(FAILED(hr = m_VideoOutPin->CheckForReconnection()))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return hr;
    }

    SI(IMediaSample) pOut;
    BYTE* pDataOut = NULL;
    
    hr = m_VideoOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), NULL, NULL, m_IsDiscontinuity);
    if(FAILED(hr))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return hr;
    }

// blocked out code for monitoring current graph clock.
#ifdef _NOT_DEFINED_
	REFERENCE_TIME Now = 0;
    static REFERENCE_TIME Last = 0;
    LARGE_INTEGER Freq;
    LARGE_INTEGER Now2;
    static LARGE_INTEGER Last2;
    QueryPerformanceFrequency(&Freq);
    QueryPerformanceCounter(&Now2);

    m_RefClock->GetTime(&Now);

    LOG(DBGLOG_FLOW, ("%010I64d - %010I64d - %010I64d - %010I64d\n", m_CurrentPicture->m_rtStart, rtStart, Now - m_rtStartTime, rtStop));
    Last = Now;
    Last2 = Now2;
#endif

    SI(IMediaSample2) pOut2 = pOut;
    if(pOut2)
    {
        AM_SAMPLE2_PROPERTIES Props;
        if(FAILED(hr = pOut2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&Props)))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }

        Props.dwSampleFlags = AM_SAMPLE_SPLICEPOINT;
        // the overlay seems to stutter unless we set the discontinuity flag
        // all the time (should have believed Gabest's comment in the first place....)
        if(m_IsDiscontinuity || fRepeatLast || m_VideoOutPin->GetConnectedType() == CDSVideoOutPin::OVERLAY_OUTFILTER)
        {
            Props.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
        }

        // do not set times on repeated frames
        if(!fRepeatLast)
        {
            Props.tStart = rtStart;
            Props.dwSampleFlags |= AM_SAMPLE_TIMEVALID;
            Props.tStop = rtStop;
            Props.dwSampleFlags |= AM_SAMPLE_STOPVALID;
        }

        // don't send send video flags to overlay
        // causes wierd strobing effect on film
        if(m_VideoOutPin->GetConnectedType() != CDSVideoOutPin::OVERLAY_OUTFILTER)
        {
            if(m_CurrentPicture->m_NumFields == 3)
                if(m_CurrentPicture->m_Flags&PIC_FLAG_TOP_FIELD_FIRST)
                    Props.dwTypeSpecificFlags = AM_VIDEO_FLAG_FIELD1FIRST | AM_VIDEO_FLAG_REPEAT_FIELD;
                else
                    Props.dwTypeSpecificFlags = AM_VIDEO_FLAG_REPEAT_FIELD;
            else
                if(m_CurrentPicture->m_Flags&PIC_FLAG_TOP_FIELD_FIRST)
                    Props.dwTypeSpecificFlags = AM_VIDEO_FLAG_FIELD1FIRST;
                else
                    Props.dwTypeSpecificFlags = 0;
        }
        else
        {
            Props.dwTypeSpecificFlags = 0;
        }

        // tell the next filter that this is film
        if(m_NextFrameDeint == DIWeave)
        {
            Props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_WEAVE;    
        }
    
        if(FAILED(hr = pOut2->SetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&Props)))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }
    }
    else
    {
        // do not set times on repeated frames
        if(!fRepeatLast)
        {
            pOut->SetTime(&rtStart, &rtStop);
        }
        else
        {
            pOut->SetTime(NULL, NULL);
        }

        // the overlay seems to stutter unless we set the discontinuity flag
        // all the time (should have believed Gabest's comment in the first place....)
        if(m_IsDiscontinuity || fRepeatLast || m_VideoOutPin->GetConnectedType() == CDSVideoOutPin::OVERLAY_OUTFILTER)
        {
            pOut->SetDiscontinuity(TRUE);
        }
        pOut->SetSyncPoint(TRUE);
    }

    BYTE** buf = &m_CurrentPicture->m_Buf[0];

    if(FAILED(hr = pOut->GetPointer(&pDataOut)))
    {
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return hr;
    }

    if(m_ChromaType == CHROMA_420)
    {
        if(HasSubpicsToRender(m_CurrentPicture->m_rtStart))
        {
            // take a copy of the current picture
            // as we need to keep the decoder's copy clean
            // so that it can be used in predictions
            m_SubPicBuffer = *m_CurrentPicture;
            
            buf = &m_SubPicBuffer.m_Buf[0];

            RenderSubpics(m_CurrentPicture->m_rtStart, buf, m_InternalPitch, m_OutputHeight);
        }
        else
        {
            ClearOldSubpics(m_CurrentPicture->m_rtStart);
        }
        m_VideoOutPin->Copy420(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch, (m_ProgressiveChroma || (m_NextFrameDeint == DIWeave)));
    }
    else if(m_ChromaType == CHROMA_422)
    {
        m_VideoOutPin->Copy422(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch);
    }
    else
    {
        m_VideoOutPin->Copy444(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch);
    }

    hr = m_VideoOutPin->SendSample(pOut.GetNonAddRefedInterface());
    if(FAILED(hr))
    {
        if(hr == E_FAIL)
        {
            // sometimes happens with overlay
            // supress the error and  set
            // discontinuity flag so that the
            // overlay will do a mini reset
            LOG(DBGLOG_FLOW, ("SendSample failed\n"));
            m_IsDiscontinuity = true;
            hr = S_OK;
        }
        else
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
        }
    }
    else
    {
        // reset discontinuity flag
        // if we have a repeat then we
        // didn't send a time stamp wityht he sample
        // and so the next frame will be a discontinuity
        m_IsDiscontinuity = fRepeatLast;
    }

    return hr;
}

void CMpegDecoder::FlushMPEG()
{
    CProtectCode WhileVarInScope(m_VideoInPin);
    CProtectCode WhileVarInScope2(&m_RateLock);
        
    m_fWaitForKeyFrame = true;
    m_fFilm = false;
    m_IsDiscontinuity = true;
    m_rate.Rate = 10000;
    m_rate.StartTime = 0;
    ResetMpeg2Decoder();
}

void CMpegDecoder::ResetMpeg2Decoder()
{
    LOG(DBGLOG_FLOW, ("ResetMpeg2Decoder()\n"));

    CProtectCode WhileVarInScope(&m_DeliverLock);

    BYTE* pSequenceHeader = NULL;
    DWORD cbSequenceHeader = 0;

    if(m_VideoInPin->GetMediaType()->formattype == FORMAT_MPEGVideo)
    {
        pSequenceHeader = ((MPEG1VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->bSequenceHeader;
        cbSequenceHeader = ((MPEG1VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->cbSequenceHeader;
    }

    if(m_CurrentPicture != NULL)
    {
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
    }

    if(m_dec != NULL)
    {
		mpeg2_reset(m_dec, 1);
		//mpeg2_close(m_dec);
        //m_dec = mpeg2_init();
        for(int i(0); i < NUM_BUFFERS; ++i)
        {
            m_Buffers[i].Clear();
        }
        if(cbSequenceHeader > 0)
        {
            mpeg2_buffer(m_dec, pSequenceHeader, pSequenceHeader + cbSequenceHeader);
            ProcessMPEGBuffer(NULL);
        }
    }

    m_fWaitForKeyFrame = true;
    m_fFilm = false;
    m_LastPictureWasStill = false;
}


HRESULT CMpegDecoder::Activate()
{
    BYTE* pSequenceHeader = NULL;
    DWORD cbSequenceHeader = 0;

    if(m_VideoInPin->GetMediaType()->formattype == FORMAT_MPEGVideo)
    {
        pSequenceHeader = ((MPEG1VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->bSequenceHeader;
        cbSequenceHeader = ((MPEG1VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->cbSequenceHeader;
    }

    m_dec = mpeg2_init();
    if(cbSequenceHeader > 0)
    {
        mpeg2_buffer(m_dec, pSequenceHeader, pSequenceHeader + cbSequenceHeader);
        ProcessMPEGBuffer(NULL);
    }

    if(m_dec == NULL)
    {
        return E_UNEXPECTED;
    }

    m_CurrentPicture = NULL;

    m_fWaitForKeyFrame = true;
    m_IsDiscontinuity = true;
    m_fFilm = false;
    m_LastOutputTime = 0;

    return S_OK;
}

HRESULT CMpegDecoder::Deactivate()
{
    if(m_dec != NULL)
    {
        mpeg2_close(m_dec);
        m_dec = NULL;
    }
    return S_OK;
}

HRESULT CMpegDecoder::ProcessNewSequence()
{
    HRESULT hr = S_OK;
    int ChromaSizeDivider;

    m_AvgTimePerFrame = 10i64 * mpeg2_info(m_dec)->sequence->frame_period / 27;
    // if the sequence draws a blank try the header
    if(m_AvgTimePerFrame == 0)
    {
        m_AvgTimePerFrame = ((VIDEOINFOHEADER*)m_VideoInPin->GetMediaType()->pbFormat)->AvgTimePerFrame;
    }
    // if that doesn't work guess based on height
    // this is so that ffdshow and possibly reclock don't go wierd on us
    if(m_AvgTimePerFrame == 0)
    {
        if(m_CurrentSequence.picture_height == 576 || m_CurrentSequence.picture_height == 288)
        {
            m_AvgTimePerFrame = 400000;
        }
        else
        {
            m_AvgTimePerFrame = 333333;
        }
    }
    m_VideoOutPin->SetAvgTimePerFrame(m_AvgTimePerFrame);
    if(mpeg2_info(m_dec)->sequence->chroma_width <= mpeg2_info(m_dec)->sequence->width / 2)
    {
        if(mpeg2_info(m_dec)->sequence->chroma_height <= mpeg2_info(m_dec)->sequence->height / 2)
        {
            m_ChromaType = CHROMA_420;
            LOG(DBGLOG_FLOW, ("STATE_SEQUENCE 420\n"));
            ChromaSizeDivider = 4;
        }
        else
        {
            m_ChromaType = CHROMA_422;
            LOG(DBGLOG_FLOW, ("STATE_SEQUENCE 422\n"));
            ChromaSizeDivider = 2;
        }
    }
    else
    {
        m_ChromaType = CHROMA_444;
        LOG(DBGLOG_FLOW, ("STATE_SEQUENCE 444\n"));
        ChromaSizeDivider = 1;
    }

    memcpy(&m_CurrentSequence, mpeg2_info(m_dec)->sequence, sizeof(mpeg2_sequence_t));


    // frame buffer
    m_MpegWidth = m_CurrentSequence.picture_width;
    m_MpegHeight = m_CurrentSequence.picture_height;
    m_InternalPitch = m_CurrentSequence.width;

    mpeg2_custom_fbuf(m_dec, 1);

    for(int i(0); i < NUM_BUFFERS; ++i)
    {
        hr = m_Buffers[i].AllocMem(m_CurrentSequence.height * m_InternalPitch, m_CurrentSequence.height * m_InternalPitch / ChromaSizeDivider);
        CHECK(hr);
    }

    hr = m_SubPicBuffer.AllocMem(m_CurrentSequence.height * m_InternalPitch, m_CurrentSequence.height * m_InternalPitch / ChromaSizeDivider);
    CHECK(hr);

    m_CurrentPicture = NULL;

    m_Buffers[0].AddRef();
    m_Buffers[1].AddRef();
    mpeg2_set_buf(m_dec, m_Buffers[0].m_Buf, &m_Buffers[0]);
    mpeg2_set_buf(m_dec, m_Buffers[1].m_Buf, &m_Buffers[1]);

    m_fWaitForKeyFrame = true;
    m_fFilm = false;
    m_PicturesSinceSequence = 0;


    // todo move out to helper function so that we can
    // cope with dynamically changes to sequence without
    // breaking the flow
    m_OutputWidth = m_MpegWidth;
    m_OutputHeight = m_MpegHeight;

    UpdateAspectRatio();

    return S_OK;
}

HRESULT CMpegDecoder::ProcessModifiedSequence()
{
    // On some channels we get notification of aspect ratio change
    // via this mechinism however we need to ensure that we handle the case
    // where nothing really has changed
    if(m_CurrentSequence.pixel_height != mpeg2_info(m_dec)->sequence->pixel_height ||
        m_CurrentSequence.pixel_width != mpeg2_info(m_dec)->sequence->pixel_width ||
        m_CurrentSequence.display_height != mpeg2_info(m_dec)->sequence->display_height ||
        m_CurrentSequence.display_width != mpeg2_info(m_dec)->sequence->display_width)
    {
        memcpy(&m_CurrentSequence, mpeg2_info(m_dec)->sequence, sizeof(mpeg2_sequence_t));
        UpdateAspectRatio();
    }
    return S_OK;
}


void CMpegDecoder::UpdateAspectRatio()
{
    // reset AFD to show all on new sequence
    if(m_AFD)
    {
        m_AFD = 8;
    }

    // optionally use accurate aspect ratio
    if(GetParamBool(DOACCURATEASPECT))
    {
        unsigned int PixelX, PixelY;
        mpeg2_guess_aspect(mpeg2_info(m_dec)->sequence, &PixelX, &PixelY);
        m_ARMpegX = PixelX;
        m_ARMpegY = PixelY;

        Simplify(m_ARMpegX, m_ARMpegY);

        if(m_MpegWidth == 720 && GetParamBool(ANALOGBLANKING))
        {
            m_ARMpegX *= 704;
        }
        else if(m_MpegWidth == 544 && GetParamBool(ANALOGBLANKING))
        {
            m_ARMpegX *= 532;
        }
        else
        {
            m_ARMpegX *= m_MpegWidth;
        }
    }
    else
    {
        m_ARMpegX = m_CurrentSequence.pixel_width;
        m_ARMpegY = m_CurrentSequence.pixel_height;

        Simplify(m_ARMpegX, m_ARMpegY);

        m_ARMpegX *= m_CurrentSequence.display_width;
    }

    m_ARMpegY *= min(m_CurrentSequence.display_height, 1080);
    Simplify(m_ARMpegX, m_ARMpegY);


    LOG(DBGLOG_FLOW, ("New Sequence %d %d %d %d %d %d\n", m_CurrentSequence.pixel_width,
                                                    m_CurrentSequence.pixel_height,
                                                    m_CurrentSequence.display_width,
                                                    m_CurrentSequence.display_height));
    
    CorrectOutputSize();
}

CMpegDecoder::CFrameBuffer* CMpegDecoder::GetNextBuffer()
{
    for(int i(0); i < NUM_BUFFERS; ++i)
    {
        if(m_Buffers[i].NotInUse())
        {
            m_Buffers[i].AddRef();
            return &m_Buffers[i];
        }
    }
    return NULL;
}

HRESULT CMpegDecoder::ProcessPictureStart(AM_SAMPLE2_PROPERTIES* pSampleProperties)
{
    // Removing const cast is naughty but the mpeg2_tag_picture 
    // function doesn't let us only tag I frames
    // which is what we need in the bda case when we get 
    // loads of timestamps
    mpeg2_picture_t* CurrentPicture = (mpeg2_picture_t*)mpeg2_info(m_dec)->current_picture;

    // skip preroll pictures as well as non I frames during ff or rew
    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL || (m_rate.Rate < (10000 / MAX_SPEED) && (CurrentPicture->flags&PIC_MASK_CODING_TYPE) != PIC_FLAG_CODING_TYPE_I))
    {
        LOG(DBGLOG_FLOW, ("Skip preroll frame\n"));
        // the mpeg2_skip function doesn't seem to do what we need so just
        // flip the flag on the picture so that we don't display the non I frames
        //mpeg2_skip(m_dec,0);
        m_IsDiscontinuity = true;
        CurrentPicture->flags |= PIC_FLAG_SKIP;
    }

    if((CurrentPicture->flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I && pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        LOG(DBGLOG_FLOW, ("Got I_Frame Time %010I64d - %010I64d\n", pSampleProperties->tStart, m_LastOutputTime));
        LARGE_INTEGER temp;
        temp.QuadPart = pSampleProperties->tStart;
        CurrentPicture->tag = temp.HighPart;
        CurrentPicture->tag2 = temp.LowPart;
        CurrentPicture->flags |= PIC_FLAG_TAGS;
        // make sure we only tag one i-frame per timestamp
        pSampleProperties->dwSampleFlags &=  ~AM_SAMPLE_TIMEVALID;
    }
    else
    {
        CurrentPicture->flags &= ~PIC_FLAG_TAGS;
    }

    // set up the memory want to receive the buffers into
    CFrameBuffer* Buffer = GetNextBuffer();
    if(Buffer == NULL)
    {
        return E_UNEXPECTED;
    }

    mpeg2_set_buf(m_dec, Buffer->m_Buf, Buffer);

    return S_OK;
}

HRESULT CMpegDecoder::ProcessPictureDisplay(bool ProgressiveHint)
{
    HRESULT hr = S_OK;

    const mpeg2_picture_t* picture = mpeg2_info(m_dec)->display_picture;
    const mpeg2_picture_t* picture_2nd = mpeg2_info(m_dec)->display_picture_2nd;
    const mpeg2_fbuf_t* fbuf = mpeg2_info(m_dec)->display_fbuf;

    if(picture && fbuf && !(picture->flags & PIC_FLAG_SKIP))
    {
        if((picture->flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I)
        {
            m_fWaitForKeyFrame = false;
        }

        CFrameBuffer* LastPicture = m_CurrentPicture;
        if((mpeg2_info(m_dec)->sequence->flags&SEQ_FLAG_PROGRESSIVE_SEQUENCE) == SEQ_FLAG_PROGRESSIVE_SEQUENCE ||
            ((ProgressiveHint == true) && (picture->flags&PIC_FLAG_PROGRESSIVE_FRAME)))
        {
            if(m_fFilm == false)
            {
                // we are being told this frame is progressive
                // either because the MPEG is encoded as progressive
                // which happens rarely (except MPEG1) or we get passed a 
                // sequence with a single progressive frame which is how DVD menus are encoded
                m_fFilm = true;
                LOG(DBGLOG_FLOW, ("Progressive mode m_fFilm = true\n"));
            }
        }
        else
        {
            if(LastPicture != NULL)
            {
                // Otherwise we start looking for 3:2 patterns
                // and switch to film where we get a proper pattern
                // we also switch out if anything odd happens to the
                // pattern
                if(picture->flags&PIC_FLAG_PROGRESSIVE_FRAME)
                {
                    if(!m_fFilm
                        && (picture->nb_fields == 3)
                        && !(LastPicture->m_NumFields == 3))
                    {
                        LOG(DBGLOG_FLOW, ("m_fFilm = true\n"));
                        m_fFilm = true;
                    }
                    else if(m_fFilm
                        && !(picture->nb_fields == 3)
                        && !(LastPicture->m_NumFields == 3))
                    {
                        LOG(DBGLOG_FLOW, ("m_fFilm = false\n"));
                        m_fFilm = false;
                    }
                }
                else
                {
                    if(m_fFilm)
                    {
                        if(!(LastPicture->m_NumFields == 3) || 
                            !(LastPicture->m_Flags&PIC_FLAG_PROGRESSIVE_FRAME) ||
                            (picture->nb_fields > 2))
                        {
                            LOG(DBGLOG_FLOW, ("m_fFilm = false %d %d %d\n", LastPicture->m_NumFields, LastPicture->m_Flags, picture->nb_fields));
                            m_fFilm = false;
                        }
                    }
                }
            }
        }

        m_CurrentPicture = (CFrameBuffer*)fbuf->id;
        m_CurrentPicture->AddRef();
        m_CurrentPicture->m_Flags = picture->flags;

        m_CurrentPicture->m_NumFields = picture->nb_fields;
        if(picture_2nd != NULL)
        {
            m_CurrentPicture->m_NumFields += picture_2nd->nb_fields;
        }

        // if the 
        if((mpeg2_info(m_dec)->sequence->flags&SEQ_FLAG_PROGRESSIVE_SEQUENCE) == SEQ_FLAG_PROGRESSIVE_SEQUENCE ||
            ProgressiveHint == true)
        {
            m_ProgressiveChroma = true;
        }
        else if(m_fFilm || picture->flags&PIC_FLAG_PROGRESSIVE_FRAME)
        {
            m_ProgressiveChroma = true;
        }
        else
        {
            m_ProgressiveChroma = false;
        }


        // deinterlace
        switch(GetParamEnum(DEINTMODE))
        {
        case DIAuto:
            if(m_ProgressiveChroma)
            {
                m_NextFrameDeint = DIWeave;
            }
            else
            {
                m_NextFrameDeint = DIBob;
            }
            break;
        case DIWeave:
            m_NextFrameDeint = DIWeave;
            break;
        case DIBob:
        default:
            m_NextFrameDeint = DIBob;
            break;
        }

        if(picture->flags & PIC_FLAG_TAGS)
        {
            LARGE_INTEGER temp;
            temp.HighPart = picture->tag;
            temp.LowPart = picture->tag2;

            m_CurrentPicture->m_rtStart = temp.QuadPart;
        }
        else
        {
            if(LastPicture != NULL)
            {
                m_CurrentPicture->m_rtStart = LastPicture->m_rtStop;
            }
            else
            {
                m_CurrentPicture->m_rtStart = 0;
            }
        }
        // start - end
        m_CurrentPicture->m_rtStop = m_CurrentPicture->m_rtStart + m_AvgTimePerFrame * m_CurrentPicture->m_NumFields / 2;

        if(!m_fWaitForKeyFrame)
        {
            hr = Deliver(false);
        }
        else
        {
            LOG(DBGLOG_FLOW, ("Preroll Skipped\n"));
            //m_CurrentPicture->Release();
            //m_CurrentPicture = NULL;
        }

        if(LastPicture != NULL)
        {
            LastPicture->Release();
        }

        ++m_PicturesSinceSequence;
    }

    if (mpeg2_info(m_dec)->discard_fbuf)
    {
        CFrameBuffer* m_Discard = (CFrameBuffer*)mpeg2_info(m_dec)->discard_fbuf->id;
        m_Discard->Release();
    }

    return hr;
}

void CMpegDecoder::LetterBox(long YAdjust, long XAdjust, bool IsTop)
{
    long OriginalHeight = m_OutputHeight;
    m_OutputHeight = m_OutputHeight * XAdjust;
    m_OutputHeight /= YAdjust;
	m_OutputHeight &= ~1;
    m_ARAdjustX = YAdjust;
    m_ARAdjustY = XAdjust;
    
    if(!IsTop)
    {
        m_VideoOutPin->SetPanScanY(m_VideoOutPin->GetPanScanY() + (OriginalHeight - m_OutputHeight) / 2);
    }
}

void CMpegDecoder::PillarBox(long YAdjust, long XAdjust)
{
    long OriginalWidth = m_OutputWidth;
    m_OutputWidth = m_OutputWidth * XAdjust;
    m_OutputWidth /= YAdjust;
	m_OutputWidth &= ~1;
    m_ARAdjustX = XAdjust;
    m_ARAdjustY = YAdjust;
    
    m_VideoOutPin->SetPanScanX(m_VideoOutPin->GetPanScanX() + (OriginalWidth - m_OutputWidth) / 2);
}

void CMpegDecoder::CorrectOutputSize()
{
    m_ARAdjustX = 1;
    m_ARAdjustY = 1;

    if(m_MpegHeight == 1088)
    {
        m_OutputHeight = 1080;
    }
    else if(GetParamBool(HARDCODEFORPAL) && m_MpegHeight < 576)
    {
        m_OutputHeight = 576;
    }
    else
    {
        m_OutputHeight = m_MpegHeight;
    }

    if(m_MpegWidth == 720 && GetParamBool(ANALOGBLANKING))
    {
        m_OutputWidth = 704;
        m_VideoOutPin->SetPanScanX(8);
    }
    else if(m_MpegWidth == 544 && GetParamBool(ANALOGBLANKING))
    {
        m_OutputWidth = 532;
        m_VideoOutPin->SetPanScanX(6);
    }
    else
    {
        m_OutputWidth = m_MpegWidth;
        m_VideoOutPin->SetPanScanX(0);
    }
    m_VideoOutPin->SetPanScanY(0);

    if(m_AFD)
    {
        // reset all the adjustments
        eDVBAspectPrefs AspectPrefs = (eDVBAspectPrefs)GetParamEnum(DVBASPECTPREFS);

        if(m_ARMpegX * 100 / m_ARMpegY  < 144)
        {
            // if we are in a 4:3 type mode
            switch(m_AFD)
            {
            // box 16:9 top
            case 2:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(4,3, true);
                }
                break;
            // box 14:9 top
            case 3:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(7,6, true);
                }
                break;
            // box >16:9 centre
            case 4:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(4,3);
                }
                break;
            // same as coded
            case 8:
                // do nothing;
                break;
            // 4:3 centre
            case 9:
                break;
            // 16:9 centre
            case 10:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(4,3);
                }
                break;
            // 14:9 centre
            case 11:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(7,6);
                }
                break;
            // 4:3 with shoot and protect 14:9
            case 13:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(7,6);
                }
                break;
            // 16:9 with shoot and protect 14:9
            case 14:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(4,3);
                }
                else
                {
                    //LetterBox(4,3);
                    //PillarBox(7,6);
                }
                break;
            // 16:9 with shoot and protect 4:3
            case 15:
                if(AspectPrefs == DVB169)
                {
                    LetterBox(4,3);
                }
                else
                {
                    //LetterBox(4,3);
                    //PillarBox(4,3);
                }
                break;
            // reserved
            default:
                break;
            }
        }
        else
        {
            // otherwise we must be in a 16:9 type mode
            switch(m_AFD)
            {
            // box 16:9 top
            case 2:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                break;
            // box 14:9 top
            case 3:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                else if(AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(7, 6);
                }
                break;
            // box >16:9 centre
            case 4:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                break;
            // same as coded
            case 8:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                break;
            // 4:3 centre
            case 9:
                if(AspectPrefs == DVBCCO || AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(4, 3);
                }
                break;
            // 16:9 centre
            case 10:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                break;
            // 14:9 centre
            case 11:
                if(AspectPrefs == DVBCCO || AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(7, 6);
                }
                break;
            // 4:3 with shoot and protect 14:9
            case 13:
                if(AspectPrefs == DVBCCO || AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(4, 3);
                }
                else if(AspectPrefs == DVB169)
                {
                    PillarBox(4, 3);
                    LetterBox(7, 6);
                }
                break;
            // 16:9 with shoot and protect 14:9
            case 14:
                if(AspectPrefs == DVBCCO)
                {
                    PillarBox(4, 3);
                }
                else if(AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(7, 6);
                }
                break;
            // 16:9 with shoot and protect 4:3
            case 15:
                if(AspectPrefs == DVBCCO || AspectPrefs == DVBLETTERBOX)
                {
                    PillarBox(4, 3);
                }
                break;
            // reserved
            default:
                break;
            }
        }
    }
    else if(m_PanAndScanDVD)
    {
        if(m_MpegWidth >= 704)
        {
            m_VideoOutPin->SetPanScanX((m_MpegWidth - 540) / 2);
            m_OutputWidth = 540;
            m_ARMpegX = 4;
            m_ARMpegY = 3;
        }
    }

    m_VideoOutPin->SetAspectX(m_ARMpegX * m_ARAdjustX);
    m_VideoOutPin->SetAspectY(m_ARMpegY * m_ARAdjustY);
    m_VideoOutPin->SetWidth(m_OutputWidth);
    m_VideoOutPin->SetHeight(m_OutputHeight);
}

