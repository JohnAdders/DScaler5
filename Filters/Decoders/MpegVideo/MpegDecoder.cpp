///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder.cpp,v 1.27 2004-05-24 06:29:26 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2003 Gabest
//	http://www.gabest.org
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
#include "DSInputPin.h"
#include "DSBufferedInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"
#include "MediaTypes.h"
#include "DSUtil.h"
#include "evcode.h"
#include "PlanarYUVToRGB.h"
#include "PlanarYUVToYUY2.h"

extern HINSTANCE g_hInstance;

CMpegDecoder::CMpegDecoder() :
    CDSBaseFilter(L"MpegVideo Filter", 2, 2)
{
    LOG(DBGLOG_FLOW, ("CMpegDecoder::CreatePins\n"));
    
    m_VideoInPin = new CDSBufferedInputPin;
    if(m_VideoInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_VideoInPin->AddRef();
    m_VideoInPin->SetupObject(this, L"Video In");

    m_SubpictureInPin = new CDSInputPin;
    if(m_SubpictureInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_SubpictureInPin->AddRef();
    m_SubpictureInPin->SetupObject(this, L"SubPicture");
    
    m_VideoOutPin = new CDSOutputPin();
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
	m_CurrentWidth = 0;
	m_CurrentHeight = 0;

    m_OutputWidth = 0;
    m_OutputHeight = 0;
    m_InternalPitch = 0;
    m_CurrentPicture = NULL;

	m_ControlFlags = 0;
	m_DoPanAndScan = false;
	m_FilmCameraModeHint = false;
	m_LetterBoxed = false;
	m_PanScanOffsetX = 0;
	m_PanScanOffsetY = 0;

	m_ARMpegX = 4;
	m_ARMpegY = 3;
	m_ARCurrentOutX = 0;
	m_ARCurrentOutY = 0;
	m_ARAdjustX = 1;
	m_ARAdjustY = 1;
	m_AFD = 0;

	m_InsideReconnect = false;
	m_ConnectedToIn = DEFAULT_INFILTER;
	m_ConnectedToOut = DEFAULT_OUTFILTER;

    InitMediaType(&m_InternalMT);
    m_NeedToAttachFormat = false;

	ZeroMemory(&m_CurrentSequence, sizeof(mpeg2_sequence_t));
}

CMpegDecoder::~CMpegDecoder()
{
    LOG(DBGLOG_FLOW, ("CMpegDecoder::~CMpegDecoder\n"));
}

STDMETHODIMP CMpegDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CMpegDecoder::GetClassID\n"));
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
        // \todo we should care when this changes
		// but we would need to store the sequence
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
    }
    return S_OK;
}


HRESULT CMpegDecoder::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    if(guidPropSet == AM_KSPROPSETID_DvdSubPic && pPin == m_SubpictureInPin)
    {
        return SupportPropSetSubPic(dwPropID, pTypeSupport);
    }
	else if(guidPropSet == AM_KSPROPSETID_TSRateChange && pPin == m_VideoInPin)
    {
        return SupportPropSetSubPic(dwPropID, pTypeSupport);
    }
    return E_NOTIMPL;
}

HRESULT CMpegDecoder::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, CDSBasePin* pPin)
{
	if(guidPropSet == AM_KSPROPSETID_DvdSubPic)
    {
        return SetPropSetSubPic(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData);
    }
	else if(guidPropSet == AM_KSPROPSETID_TSRateChange)
    {
        return SetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData);
    }
    else
    {
		return E_NOTIMPL;
    }

	return S_OK;
}

HRESULT CMpegDecoder::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
	if(guidPropSet == AM_KSPROPSETID_DvdSubPic)
    {
        return GetPropSetSubPic(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData, pcbReturned);
    }
	else if(guidPropSet == AM_KSPROPSETID_TSRateChange)
    {
        return GetPropSetRate(dwPropID, pInstanceData, cbInstanceData, pPropertyData, cbPropData, pcbReturned);
    }
    else
    {
		return E_NOTIMPL;
    }

	return S_OK;
}

HRESULT CMpegDecoder::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    if(pPin == m_VideoOutPin)
    {
	    if(q.Type == Famine)
	    {
		    SI(IQualityControl) QualityControl = m_VideoInPin->m_ConnectedPin;
		    if(QualityControl && q.Late < 5000000)
		    {
                LOG(DBGLOG_ALL, ("Coped With Famine - %d\n", q.Late));
			    return QualityControl->Notify(pSelf, q);
		    }
		    else
		    {
                LOG(DBGLOG_ALL, ("Ignored Famine - %d\n", q.Late));
			    return E_NOTIMPL;
		    }
	    }
	    if(q.Type == Flood)
	    {
            LOG(DBGLOG_ALL, ("Ignored Flood - %d\n", q.Late));
			return E_NOTIMPL;
        }
    }
    return E_NOTIMPL;
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
	    BITMAPINFOHEADER bih;
	    ExtractBIH(pPin->GetMediaType(), &bih);

	    pProperties->cBuffers = 3;
		if(m_InsideReconnect)
		{
			pProperties->cbBuffer = m_InternalMT.lSampleSize;
		}
		else
		{
			pProperties->cbBuffer = bih.biSizeImage;
		}
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
    	Result = (ExtractDim(pmt, wout, hout, arxout, aryout) && 
                  (pmt->majortype == MEDIATYPE_Video) && 
                  (pmt->subtype == MEDIASUBTYPE_YUY2 ||
				    pmt->subtype == MEDIASUBTYPE_ARGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB24 ||
                    pmt->subtype == MEDIASUBTYPE_RGB565 ||
                    pmt->subtype == MEDIASUBTYPE_RGB555));
		if(m_InsideReconnect)
		{
			m_Pitch = wout;
			m_Height = hout;
		}
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
    else
    {
        return E_NOTIMPL;
    }
}


HRESULT CMpegDecoder::GetEnumTextDeintMode(WCHAR **ppwchText)
{
    wchar_t DeintText[] = L"Deinterlace Mode\0" L"None\0" L"Automatic\0" L"Force Weave\0" L"Force Bob\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(DeintText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, DeintText, sizeof(DeintText));
    return S_OK;
}

HRESULT CMpegDecoder::GetEnumTextDVBAspectPrefs(WCHAR **ppwchText)
{
    wchar_t DeintText[] = L"DVB Aspect Preferences\0" L"None\0" L"16:9 Display\0" L"4:3 Display Center Cut out\0" L"4:3 Display LetterBox\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(DeintText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, DeintText, sizeof(DeintText));
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

void CMpegDecoder::CorrectSourceTarget(RECT& rcSource, RECT& rcTarget)
{
	SetRect(&rcSource, 0, 0, m_OutputWidth, m_OutputHeight);
	if(m_ConnectedToIn == DVD_INFILTER && m_ConnectedToOut != DEFAULT_OUTFILTER)
	{
		if(m_OutputWidth == 352 && (m_OutputHeight == 240 || m_OutputHeight == 288))
		{
			SetRect(&rcTarget, 8, 0, m_OutputWidth * 2, m_OutputHeight * 2);
		}
		else if(m_OutputWidth == 352 && (m_OutputHeight == 480 || m_OutputHeight == 576))
		{
			SetRect(&rcTarget, 8, 0, m_OutputWidth * 2, m_OutputHeight);
		}
		else if(m_OutputWidth == 704 && (m_OutputHeight == 480 || m_OutputHeight == 576))
		{
			SetRect(&rcTarget, 8, 0, m_OutputWidth, m_OutputHeight);
		}
		else
		{
			SetRect(&rcTarget, 0, 0, m_OutputWidth, m_OutputHeight);
		}
	}
	else
	{
		SetRect(&rcTarget, 0, 0, m_OutputWidth, m_OutputHeight);
	}
	SetRect(&rcTarget, 0, 0, 0, 0);
}


HRESULT CMpegDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{

    if(pPin == m_VideoOutPin)
    {
        if(!m_VideoInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

	    struct {const GUID* subtype; WORD biPlanes, biBitCount; DWORD biCompression;} fmts[] =
	    {
		    {&MEDIASUBTYPE_YUY2, 1, 16, '2YUY'},
		    {&MEDIASUBTYPE_ARGB32, 1, 32, BI_RGB},
		    {&MEDIASUBTYPE_RGB32, 1, 32, BI_RGB},
		    {&MEDIASUBTYPE_RGB24, 1, 24, BI_RGB},
		    {&MEDIASUBTYPE_RGB565, 1, 16, BI_RGB},
		    {&MEDIASUBTYPE_RGB555, 1, 16, BI_RGB},
		    {&MEDIASUBTYPE_ARGB32, 1, 32, BI_BITFIELDS},
		    {&MEDIASUBTYPE_RGB32, 1, 32, BI_BITFIELDS},
		    {&MEDIASUBTYPE_RGB24, 1, 24, BI_BITFIELDS},
		    {&MEDIASUBTYPE_RGB565, 1, 16, BI_BITFIELDS},
		    {&MEDIASUBTYPE_RGB555, 1, 16, BI_BITFIELDS},
	    };

	    // this will make sure we won't connect to the old renderer in dvd mode
	    // that renderer can't switch the format dynamically
	    if(TypeNum < 0) return E_INVALIDARG;
	    if(TypeNum >= 3*sizeof(fmts)/sizeof(fmts[0])) return VFW_S_NO_MORE_ITEMS;


        int FormatNum = TypeNum/3;
	    const AM_MEDIA_TYPE* mt = m_VideoInPin->GetMediaType();

	    pmt->majortype = MEDIATYPE_Video;
	    pmt->subtype = *fmts[FormatNum].subtype;

	    BITMAPINFOHEADER bihOut;
	    memset(&bihOut, 0, sizeof(bihOut));
	    bihOut.biSize = sizeof(bihOut);
	    bihOut.biWidth = m_OutputWidth;
	    bihOut.biHeight = m_OutputHeight;
	    bihOut.biPlanes = fmts[FormatNum].biPlanes;
	    bihOut.biBitCount = fmts[FormatNum].biBitCount;
	    bihOut.biCompression = fmts[FormatNum].biCompression;
	    bihOut.biSizeImage = bihOut.biWidth * bihOut.biHeight * bihOut.biBitCount>>3;

	    if(TypeNum%3 == 2)
	    {
		    pmt->formattype = FORMAT_VideoInfo;
		    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
		    memset(vih, 0, sizeof(VIDEOINFOHEADER));
		    vih->bmiHeader = bihOut;
		    vih->bmiHeader.biXPelsPerMeter = vih->bmiHeader.biWidth * m_ARMpegX * m_ARAdjustX;
		    vih->bmiHeader.biYPelsPerMeter = vih->bmiHeader.biHeight * m_ARMpegY * m_ARAdjustY;
			Simplify(vih->bmiHeader.biXPelsPerMeter, vih->bmiHeader.biYPelsPerMeter);
            if(mt->pbFormat)
            {
	            vih->AvgTimePerFrame = ((VIDEOINFOHEADER*)mt->pbFormat)->AvgTimePerFrame;
	            vih->dwBitRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitRate;
	            vih->dwBitErrorRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitErrorRate;
            }
            pmt->pbFormat = (BYTE*)vih;
            pmt->cbFormat = sizeof(VIDEOINFOHEADER);
			CorrectSourceTarget(vih->rcSource, vih->rcTarget);
        }
	    else
	    {
		    pmt->formattype = FORMAT_VideoInfo2;
		    VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
		    memset(vih, 0, sizeof(VIDEOINFOHEADER2));
		    vih->bmiHeader = bihOut;
		    vih->dwPictAspectRatioX = m_ARMpegX * m_ARAdjustX;
		    vih->dwPictAspectRatioY = m_ARMpegY * m_ARAdjustY;
			Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
            if(TypeNum%3 == 0)
            {
    		    vih->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave | AMINTERLACE_FieldPatBothRegular;
            }
            if(mt->pbFormat)
            {
    	        vih->AvgTimePerFrame = ((VIDEOINFOHEADER*)mt->pbFormat)->AvgTimePerFrame;
	            vih->dwBitRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitRate;
	            vih->dwBitErrorRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitErrorRate;
            }
			vih->dwControlFlags = m_ControlFlags;
            pmt->pbFormat = (BYTE*)vih;
            pmt->cbFormat = sizeof(VIDEOINFOHEADER2);	    
			CorrectSourceTarget(vih->rcSource, vih->rcTarget);
	    }
		
	    CorrectMediaType(pmt);
        return S_OK;
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
		m_ControlFlags = 0;
		m_DoPanAndScan = false;
		m_FilmCameraModeHint = false;
		m_LetterBoxed = false;
		m_AFD = 0;

		if(pMediaType->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pMediaType->pbFormat;
			m_ControlFlags = vih->dwControlFlags;
		}
		else if(pMediaType->formattype == FORMAT_MPEG2_VIDEO)
		{
			MPEG2VIDEOINFO* mvih = (MPEG2VIDEOINFO*)pMediaType->pbFormat;
			m_ControlFlags = mvih->hdr.dwControlFlags;
			if(mvih->dwFlags & AMMPEG2_DoPanScan)
			{
			    LOG(DBGLOG_FLOW, ("Pan & Scan Sent\n"));
				m_DoPanAndScan = true;
			}
			if(mvih->dwFlags & AMMPEG2_FilmCameraMode)
			{
			    LOG(DBGLOG_FLOW, ("Film Hint Sent\n"));
				m_FilmCameraModeHint = true;
			}
			if(mvih->dwFlags & AMMPEG2_SourceIsLetterboxed)
			{
			    LOG(DBGLOG_FLOW, ("L Sent\n"));
				m_LetterBoxed = true;
			}
		}

		CorrectOutputSize();
    }
    else if(pPin == m_VideoOutPin)
    {
        int wout = 0, hout = 0;
		long arxout = 0, aryout = 0; 
        ExtractDim(pMediaType, wout, hout, arxout, aryout); 

		if(wout == m_OutputWidth && hout == m_OutputHeight &&
			arxout == m_ARAdjustX * m_ARMpegX &&
			aryout == m_ARAdjustY * m_ARMpegY)
		{
			m_CurrentWidth = wout; 
			m_CurrentHeight = hout; 
			m_ARCurrentOutX = arxout; 
			m_ARCurrentOutY = aryout; 

		}
    }
    return S_OK;
}

HRESULT CMpegDecoder::NotifyConnected(CDSBasePin* pPin)
{
	HRESULT hr = S_OK;
	if(pPin == m_VideoInPin)
	{
		CLSID Clsid;

		HRESULT hr = m_VideoOutPin->GetConnectedFilterCLSID(&Clsid);
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
		CLSID Clsid;

		HRESULT hr = m_VideoOutPin->GetConnectedFilterCLSID(&Clsid);
		if(Clsid == CLSID_VideoMixingRenderer9)
		{
			m_ConnectedToOut = VMR9_OUTFILTER;
		}
		else if(Clsid == CLSID_VideoMixingRenderer ||
				Clsid == CLSID_VideoRendererDefault)
		{
			m_ConnectedToOut = VMR7_OUTFILTER;
		}
        else if(Clsid == CLSID_FFDShow)
        {
			m_ConnectedToOut = FFDSHOW_OUTFILTER;
        }
		else
		{
            SI(IPinConnection) PinConnection = pPin->m_ConnectedPin;
            if(!PinConnection)
            {
                return VFW_E_NO_TRANSPORT;
            }
			m_ConnectedToOut = DEFAULT_OUTFILTER;
		}
	}
	return S_OK;
}


HRESULT CMpegDecoder::ProcessMPEGSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties)
{
	CProtectCode WhileVarInScope(&m_DeliverLock);

	HRESULT hr;

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

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
	{
		m_IsDiscontinuity = true;
	}

	mpeg2_buffer(m_dec, pDataIn, pDataIn + len);
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
            hr = ProcessPictureDisplay();
            CHECK(hr);
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

	{
		// cope with a change in rate		
		if(m_rate.Rate != m_ratechange.Rate && m_CurrentPicture->m_rtStart >= m_ratechange.StartTime)
		{
			m_rate.Rate = m_ratechange.Rate;
			// looks like we need to do this ourselves
			// as the time past originally seems like a best guess only
			m_rate.StartTime = m_CurrentPicture->m_rtStart;
			m_LastOutputTime = m_CurrentPicture->m_rtStart;
			m_IsDiscontinuity = true;
			LOG(DBGLOG_FLOW, ("Got Rate %010I64d %d\n", m_rate.StartTime, m_rate.Rate));
		}
	}


	REFERENCE_TIME rtStart;
	REFERENCE_TIME rtStop;
    
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
    if(GetParamBool(FRAMESMOOTH32) && m_fFilm && m_CurrentPicture->m_NumFields == 3)
    {
    	rtStop = m_rate.StartTime + (m_CurrentPicture->m_rtStop - m_rate.StartTime - m_AvgTimePerFrame /2) * abs(m_rate.Rate) / 10000;
    }
    else
    {
    	rtStop = m_rate.StartTime + (m_CurrentPicture->m_rtStop - m_rate.StartTime) * abs(m_rate.Rate) / 10000;
    }

    rtStop += GetParamInt(VIDEODELAY) * 10000;

    // ensure that times always go up
    // and that Stop is always greater than start
    if(rtStart < m_LastOutputTime)
    {
        rtStart = m_LastOutputTime;
    }

	if(fRepeatLast)
	{
		// if we are being called to refresh the subpicture
		// and the proposed start time is in the past then
		// we need to pass an up to date media time
		// otherwise the renderer may drop the sample
		// which means the user can't see the new subpicture
		REFERENCE_TIME CurTime;
		m_RefClock->GetTime(&CurTime);
		if(rtStart < CurTime)
		{
			rtStart = CurTime - m_rtStartTime;
			rtStop = rtStart + 10000;
		}
	}
	
	if(rtStop <= rtStart)
	{
		rtStop = rtStart + 100;
	}

    m_LastOutputTime = rtStop;

	HRESULT hr;

	// cope with dynamic format changes from our side
	// will possibly call NegotiateAllocator on the output pins
	// which flushes so we shouldn't have any samples outstanding here
	if(FAILED(hr = ReconnectOutput(false)))
	{
        LogBadHRESULT(hr, __FILE__, __LINE__);
		return hr;
	}

	SI(IMediaSample) pOut;
	BYTE* pDataOut = NULL;
    
    hr = m_VideoOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), &rtStart, &rtStop, m_IsDiscontinuity);
    if(FAILED(hr))
	{
        LogBadHRESULT(hr, __FILE__, __LINE__);
        return hr;
	}

	// cope with dynamic format changes from the renderer
	// we care about this when we think we need to change the format
	// and the video renderer sends up a new format too
	// calling ReconnectOutput again will merge the two 
	// formats properly and should never call NegotiateAllocator
	if(hr == S_FALSE && m_NeedToAttachFormat)
	{
		if(FAILED(hr = ReconnectOutput(true)))
		{
			LogBadHRESULT(hr, __FILE__, __LINE__);
			return hr;
		}
	}

	LOG(DBGLOG_ALL, ("%010I64d - %010I64d - %010I64d - %010I64d\n", m_CurrentPicture->m_rtStart, m_CurrentPicture->m_rtStop, rtStart, rtStop));

	SI(IMediaSample2) pOut2 = pOut;
    if(pOut2)
    {
        AM_SAMPLE2_PROPERTIES Props;
        if(FAILED(hr = pOut2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&Props)))
		{
			LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
		}
        Props.tStart = rtStart;
        Props.tStop = rtStop;

        if(m_IsDiscontinuity || fRepeatLast || m_NeedToAttachFormat)
		{
		    Props.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
		}
		else
		{
		    Props.dwSampleFlags &= ~AM_SAMPLE_DATADISCONTINUITY;
		}
        if(fRepeatLast)
		{
			Props.dwSampleFlags |= AM_SAMPLE_TIMEDISCONTINUITY;
		}
        Props.dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        Props.dwSampleFlags |= AM_SAMPLE_STOPVALID;
        Props.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;

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


        // tell the next filter that this is film
        if(m_NextFrameDeint == DIWeave)
            Props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_WEAVE;    

        if(FAILED(hr = pOut2->SetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&Props)))
		{
			LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }
    }
    else
    {
		pOut->SetTime(&rtStart, &rtStop);
		pOut->SetMediaTime(NULL, NULL);

        if(m_IsDiscontinuity || fRepeatLast || m_NeedToAttachFormat)
			pOut->SetDiscontinuity(TRUE);
        else
    	    pOut->SetDiscontinuity(FALSE);
	    pOut->SetSyncPoint(TRUE);
    }

	if(m_NeedToAttachFormat)
	{
		m_VideoOutPin->SetType(&m_InternalMT);
		pOut->SetMediaType(&m_InternalMT);
	    LogMediaType(&m_InternalMT, "AttachFormat", DBGLOG_FLOW);
		m_NeedToAttachFormat = false;
	}


    SI(IMediaEventSink) pMES = m_Graph;
    if(pMES)
    {
		// some renderers don't send this
		pMES->Notify(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(m_OutputWidth, m_OutputHeight), 0);
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
		Copy420(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch);
	}
	else if(m_ChromaType == CHROMA_422)
	{
		Copy422(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch);
	}
	else
	{
		Copy444(pDataOut, buf, m_OutputWidth, m_OutputHeight, m_InternalPitch);
	}

	hr = m_VideoOutPin->SendSample(pOut.GetNonAddRefedInterface());
	if(FAILED(hr))
	{
        if(hr == E_FAIL)
        {
            // sometime happens with overlay
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
	    m_IsDiscontinuity = false;
    }

	return hr;
}

void CMpegDecoder::FlushMPEG()
{
	CProtectCode WhileVarInScope(m_VideoInPin);
    	
    m_fWaitForKeyFrame = true;
    m_fFilm = false;
	m_IsDiscontinuity = true;
	ResetMpeg2Decoder();
}

HRESULT CMpegDecoder::ReconnectOutput(bool ForceReconnect)
{
	HRESULT hr = S_OK;

	if(m_ConnectedToOut == DEFAULT_OUTFILTER && ForceReconnect == TRUE)
	{
		return S_OK;
	}

	if((m_OutputWidth != m_CurrentWidth) || 
		(m_OutputHeight != m_CurrentHeight) || 
		(m_ARMpegX * m_ARAdjustX != m_ARCurrentOutX) || 
		(m_ARMpegY * m_ARAdjustY != m_ARCurrentOutY) ||
		ForceReconnect)
	{
        CopyMediaType(&m_InternalMT, m_VideoOutPin->GetMediaType());
	
    	BITMAPINFOHEADER* bmi = NULL;

		if(m_InternalMT.formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
			bmi = &vih->bmiHeader;
            vih->AvgTimePerFrame = m_AvgTimePerFrame;
			bmi->biXPelsPerMeter = m_MpegWidth * m_ARMpegX * m_ARAdjustX;
			bmi->biYPelsPerMeter = m_MpegHeight * m_ARMpegY * m_ARAdjustY;
			Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);
			CorrectSourceTarget(vih->rcSource, vih->rcTarget);
		}
		else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
			bmi = &vih->bmiHeader;
            vih->AvgTimePerFrame = m_AvgTimePerFrame;
		    vih->dwPictAspectRatioX = m_ARMpegX * m_ARAdjustX;
		    vih->dwPictAspectRatioY = m_ARMpegY * m_ARAdjustY;
			Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
			CorrectSourceTarget(vih->rcSource, vih->rcTarget);
		}

		if(!ForceReconnect)
		{
            if(m_ConnectedToOut == VMR9_OUTFILTER)
            {
				if(!ForceReconnect)
				{
					bmi->biWidth = max(m_OutputWidth, bmi->biWidth);
				}
            }
            else if(m_ConnectedToOut != DEFAULT_OUTFILTER)
			{
				if(!ForceReconnect)
				{
					bmi->biWidth = m_OutputWidth;
				}
			}
			else
			{
				bmi->biWidth = m_OutputWidth;
			}

            if(bmi->biHeight > 0)
			{
    			bmi->biHeight = m_OutputHeight;
			}
			else
			{
    			bmi->biHeight = -m_OutputHeight;
			}

			bmi->biSizeImage = m_OutputHeight*bmi->biWidth*bmi->biBitCount>>3;

		}
        
		m_InternalMT.bFixedSizeSamples = 1;
		m_InternalMT.lSampleSize = bmi->biSizeImage;

		m_InsideReconnect = true;
		m_Pitch = 0;
		m_Height = 0;


		SI(IPinConnection) m_PinConnection = m_VideoOutPin->m_ConnectedPin;
		if(m_PinConnection)
		{

			hr = m_PinConnection->DynamicQueryAccept(&m_InternalMT);
			if(hr != S_OK)
			{
    			LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
		}
		else
		{
			hr = m_VideoOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
			if(hr != S_OK)
			{
    			LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
				return VFW_E_TYPE_NOT_ACCEPTED;
			}
		}

		if(m_ConnectedToOut == DEFAULT_OUTFILTER)
		{
			if(!ForceReconnect)
			{
				if(!m_VideoOutPin->m_Allocator) return E_NOINTERFACE;

				SI(IGraphConfig) GraphConfig = m_Graph;

				if(GraphConfig)
				{
					hr = GraphConfig->Reconnect(m_VideoOutPin, m_VideoOutPin->m_ConnectedPin.GetNonAddRefedInterface(), &m_InternalMT, NULL, NULL, AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT);
					CHECK(hr);

					if(m_Pitch != 0)
					{
						bmi->biWidth = m_Pitch;
						bmi->biHeight = m_Height;
						bmi->biSizeImage = m_OutputHeight*bmi->biWidth*bmi->biBitCount>>3;
						m_InternalMT.lSampleSize = bmi->biSizeImage;

						hr = GraphConfig->Reconnect(m_VideoOutPin, m_VideoOutPin->m_ConnectedPin.GetNonAddRefedInterface(), &m_InternalMT, NULL, NULL, AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT);
						CHECK(hr);
					}
					m_NeedToAttachFormat = true; 
				}
			}
		}
		else if(m_ConnectedToOut == VMR9_OUTFILTER)
        {
			ALLOCATOR_PROPERTIES AllocatorProps;
			hr = m_VideoOutPin->m_Allocator->GetProperties(&AllocatorProps);
			CHECK(hr);

			// if the new type would be greater than the old one then
			// we need to reconnect otherwise just attach the type to the next sample
            // for the VMR9 the ReceiveConnection only works when 
            // you haven't displayed anything but at least 
            // attaching new formats seems to work properly
			if(bmi->biSizeImage > (DWORD)AllocatorProps.cbBuffer)
			{
                hr = m_VideoOutPin->m_ConnectedPin->ReceiveConnection(m_VideoOutPin, &m_InternalMT);
                CHECK(hr);

                if(m_Pitch != 0)
                {
	                bmi->biWidth = m_Pitch;
	                bmi->biHeight = m_Height;
	                bmi->biSizeImage = m_OutputHeight*bmi->biWidth*bmi->biBitCount>>3;
	                m_InternalMT.lSampleSize = bmi->biSizeImage;

	                hr = m_VideoOutPin->m_ConnectedPin->ReceiveConnection(m_VideoOutPin, &m_InternalMT);
	                CHECK(hr);
                }
            }
    		m_NeedToAttachFormat = true; 
        }
        else
		{
			if(!ForceReconnect)
			{
				hr = m_VideoOutPin->m_ConnectedPin->ReceiveConnection(m_VideoOutPin, &m_InternalMT);
				CHECK(hr);

				ALLOCATOR_PROPERTIES AllocatorProps;
				hr = m_VideoOutPin->m_Allocator->GetProperties(&AllocatorProps);
				CHECK(hr);

				// if the new type would be greater than the old one then
				// we need to reconnect otherwise just attach the type to the next sample
				if(bmi->biSizeImage > (DWORD)AllocatorProps.cbBuffer)
				{
					hr = m_VideoOutPin->NegotiateAllocator(NULL, &m_InternalMT);
					CHECK(hr);
				}
			}
			m_NeedToAttachFormat = true; 
		}
		
		m_InsideReconnect = false;


		m_CurrentWidth = m_OutputWidth;
		m_CurrentHeight = m_OutputHeight;
        m_ARCurrentOutX = m_ARMpegX * m_ARAdjustX; 
        m_ARCurrentOutY = m_ARMpegY * m_ARAdjustY; 
	}
	else
	{
		m_NeedToAttachFormat = false; 
	}

	return S_OK;
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
	else if(m_VideoInPin->GetMediaType()->formattype == FORMAT_MPEG2_VIDEO)
	{
		pSequenceHeader = (BYTE*)((MPEG2VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->dwSequenceHeader;
		cbSequenceHeader = ((MPEG2VIDEOINFO*)m_VideoInPin->GetMediaType()->pbFormat)->cbSequenceHeader;
	}

    if(m_dec != NULL)
    {
    	mpeg2_free(m_dec);
        m_dec = mpeg2_init();
        if(cbSequenceHeader > 0)
        {
    	    mpeg2_buffer(m_dec, pSequenceHeader, pSequenceHeader + cbSequenceHeader);
        }
    }

	if(m_CurrentPicture != NULL)
	{
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
	}

	m_fWaitForKeyFrame = true;
	m_fFilm = false;
}

void CMpegDecoder::Copy420(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn)
{
	BITMAPINFOHEADER bihOut;
	ExtractBIH(m_VideoOutPin->GetMediaType(), &bihOut);

	BYTE* pIn = ppIn[0];
	BYTE* pInU = ppIn[1];
	BYTE* pInV = ppIn[2];

	w = (w+7)&~7;
	ASSERT(w <= pitchIn);

	if(m_DoPanAndScan)
	{
		pIn += m_PanScanOffsetX + pitchIn*m_PanScanOffsetY;
		pInU += m_PanScanOffsetX / 2 + pitchIn*m_PanScanOffsetY/4;
		pInV += m_PanScanOffsetX / 2 + pitchIn*m_PanScanOffsetY/4;
	}


	if(bihOut.biCompression == '2YUY')
	{
		if(m_ProgressiveChroma)
		{
			BitBltFromI420ToYUY2(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
		}
		else
		{
			if(m_NextFrameDeint == DIWeave)
			{
				BitBltFromI420ToYUY2(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
			}
			else
			{
				BitBltFromI420ToYUY2_Int(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
			}
		}
	}
	else if(bihOut.biCompression == BI_RGB || bihOut.biCompression == BI_BITFIELDS)
	{
		int pitchOut = bihOut.biWidth*bihOut.biBitCount>>3;

		if(bihOut.biHeight > 0)
		{
			pOut += pitchOut*(h-1);
			pitchOut = -pitchOut;
		}

		if(!BitBltFromI420ToRGB(w, h, pOut, pitchOut, bihOut.biBitCount, pIn, pInU, pInV, pitchIn))
		{
			for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
				memset(pOut, 0, pitchOut);
		}
	}
}

void CMpegDecoder::Copy422(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn)
{
	BITMAPINFOHEADER bihOut;
	ExtractBIH(m_VideoOutPin->GetMediaType(), &bihOut);

	BYTE* pIn = ppIn[0];
	BYTE* pInU = ppIn[1];
	BYTE* pInV = ppIn[2];

	w = (w+7)&~7;
	ASSERT(w <= pitchIn);

	if(bihOut.biCompression == '2YUY')
	{
		BitBltFromI422ToYUY2(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == BI_RGB || bihOut.biCompression == BI_BITFIELDS)
	{
		int pitchOut = bihOut.biWidth*bihOut.biBitCount>>3;

		if(bihOut.biHeight > 0)
		{
			pOut += pitchOut*(h-1);
			pitchOut = -pitchOut;
		}

		if(!BitBltFromI422ToRGB(w, h, pOut, pitchOut, bihOut.biBitCount, pIn, pInU, pInV, pitchIn))
		{
			for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
				memset(pOut, 0, pitchOut);
		}
	}
}

void CMpegDecoder::Copy444(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn)
{
	BITMAPINFOHEADER bihOut;
	ExtractBIH(m_VideoOutPin->GetMediaType(), &bihOut);

	BYTE* pIn = ppIn[0];
	BYTE* pInU = ppIn[1];
	BYTE* pInV = ppIn[2];

	w = (w+7)&~7;
	ASSERT(w <= pitchIn);

	if(bihOut.biCompression == '2YUY')
	{
		BitBltFromI444ToYUY2(w, h, pOut, bihOut.biWidth*2, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == BI_RGB || bihOut.biCompression == BI_BITFIELDS)
	{
		int pitchOut = bihOut.biWidth*bihOut.biBitCount>>3;

		if(bihOut.biHeight > 0)
		{
			pOut += pitchOut*(h-1);
			pitchOut = -pitchOut;
		}

        // \todo Since 4:4:4 isn't really all that common I haven't bothered
        // implementing the RGB conversion
		for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
			memset(pOut, 0, pitchOut);
	}
}


HRESULT CMpegDecoder::Activate()
{
    if(m_dec != NULL)
    {
        mpeg2_free(m_dec);
    }
    m_dec = mpeg2_init();
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
        mpeg2_free(m_dec);
        m_dec = NULL;
    }
    return S_OK;
}

// function taken from 
// header.c
// Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
// Copyright (C) 2003      Regis Duchesne <hpreg@zoy.org>
// Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
//
// This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
// See http://libmpeg2.sourceforge.net/ for updates.
void CMpegDecoder::Simplify(long& u, long& v)
{
    long a, b, tmp;

    a = u;	
    b = v;
    
    while (a) 
    {	
        /* find greatest common divisor */
	    tmp = a;	
        a = b % tmp;	
        b = tmp;
    }
    u /= b;
    v /= b;
}

void CMpegDecoder::Simplify(unsigned long& u, unsigned long& v)
{
    unsigned long a, b, tmp;

    a = u;	
    b = v;
    
    while (a) 
    {	
        /* find greatest common divisor */
	    tmp = a;	
        a = b % tmp;	
        b = tmp;
    }
    u /= b;
    v /= b;
}


HRESULT CMpegDecoder::ProcessNewSequence()
{
    HRESULT hr = S_OK;
    int ChromaSizeDivider;

	m_AvgTimePerFrame = 10i64 * mpeg2_info(m_dec)->sequence->frame_period / 27;
	if(m_AvgTimePerFrame == 0) m_AvgTimePerFrame = ((VIDEOINFOHEADER*)m_VideoInPin->GetMediaType()->pbFormat)->AvgTimePerFrame;
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
	// via this mechinism however we need to ensure  that we handle the case
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
	}
	else
	{
		m_ARMpegX = m_CurrentSequence.pixel_width;
		m_ARMpegY = m_CurrentSequence.pixel_height;
	}

	m_ARMpegX *= m_CurrentSequence.display_width;
	m_ARMpegY *= m_CurrentSequence.display_height;

    Simplify(m_ARMpegX, m_ARMpegY);

	LOG(DBGLOG_FLOW, ("New Sequence %d %d %d %d %d %d\n", m_CurrentSequence.pixel_width,
                                                    m_CurrentSequence.pixel_height,
                                                    m_CurrentSequence.display_width,
                                                    m_CurrentSequence.display_height,
                                                    m_OutputWidth,
                                                    m_OutputHeight));


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
    // skip preroll pictures
	if(pSampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL)
	{
		LOG(DBGLOG_FLOW, ("Skip preroll frame\n"));
		mpeg2_skip(m_dec,0);
		m_IsDiscontinuity = true;
	}

	// naughty but the mpeg2_tag_picture function doesn't let us only tag I frames
	// which is what we need in the bda case when we get 
	// loads of timestamps
	mpeg2_picture_t* CurrentPicture = (mpeg2_picture_t*)mpeg2_info(m_dec)->current_picture;

    if((CurrentPicture->flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I && pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
		LOG(DBGLOG_FLOW, ("Got I_Frame Time %010I64d\n", pSampleProperties->tStart));
		LARGE_INTEGER temp;
		temp.QuadPart = pSampleProperties->tStart;
		CurrentPicture->tag = temp.HighPart;
		CurrentPicture->tag2 = temp.LowPart;
		CurrentPicture->flags |= PIC_FLAG_TAGS;
		// make sure we only tag one i-frame per timestamp
		pSampleProperties->dwSampleFlags &=  ~AM_SAMPLE_TIMEVALID;
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

HRESULT CMpegDecoder::ProcessPictureDisplay()
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
        if(LastPicture != NULL)
        {
		    // flags
		    if(!(mpeg2_info(m_dec)->sequence->flags&SEQ_FLAG_PROGRESSIVE_SEQUENCE)
			    && (picture->flags&PIC_FLAG_PROGRESSIVE_FRAME))
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

        m_CurrentPicture = (CFrameBuffer*)fbuf->id;
        m_CurrentPicture->AddRef();
        m_CurrentPicture->m_Flags = picture->flags;

		m_CurrentPicture->m_NumFields = picture->nb_fields;
        if(picture_2nd != NULL)
        {
		    m_CurrentPicture->m_NumFields += picture_2nd->nb_fields;
        }

		if((mpeg2_info(m_dec)->sequence->flags&SEQ_FLAG_PROGRESSIVE_SEQUENCE) == SEQ_FLAG_PROGRESSIVE_SEQUENCE)
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

        if(/*m_CurrentPicture->m_rtStart >= 0 &&*/ !m_fWaitForKeyFrame)
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
	m_ARAdjustX = YAdjust;
	m_ARAdjustY = XAdjust;
	
	if(!IsTop)
	{
		m_PanScanOffsetY += (OriginalHeight - m_OutputHeight) / 2;
		m_DoPanAndScan = true;
	}
}

void CMpegDecoder::PillarBox(long YAdjust, long XAdjust)
{
	long OriginalWidth = m_OutputWidth;
	m_OutputWidth = m_OutputWidth * XAdjust;
	m_OutputWidth /= YAdjust;
	m_ARAdjustX = XAdjust;
	m_ARAdjustY = YAdjust;
	
	m_PanScanOffsetX += (OriginalWidth - m_OutputWidth) / 2;
	m_DoPanAndScan = true;
}

void CMpegDecoder::CorrectOutputSize()
{

	if(m_MpegHeight == 1088)
	{
		m_OutputHeight = 1080;
	}
	else
	{
		m_OutputHeight = m_MpegHeight;
	}

	m_OutputWidth = m_MpegWidth;

	m_ARAdjustX = 1;
	m_ARAdjustY = 1;
	m_PanScanOffsetX = 0;
	m_PanScanOffsetY = 0;

	if(m_AFD)
	{
		// reset all the adjustments
		m_DoPanAndScan = false;
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
	else if(m_DoPanAndScan)
	{
		if(m_MpegWidth >= 704)
		{
			m_PanScanOffsetX = (m_MpegWidth - 540) / 2;
			m_OutputWidth = 540;
			m_ARMpegX = 4;
			m_ARMpegY = 3;
		}
	}

}

