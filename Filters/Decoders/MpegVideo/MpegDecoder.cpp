///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder.cpp,v 1.9 2004-02-29 13:47:48 adcockj Exp $
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
    
    m_VideoInPin = new CDSInputPin;
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
    
    m_VideoOutPin = new CDSOutputPin;
    if(m_VideoOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 3"));
    }
    m_VideoOutPin->AddRef();
    m_VideoOutPin->SetupObject(this, L"Video Out");

    m_CCOutPin = new CDSOutputPin;
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

	m_sphli = NULL;
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

    m_InternalWidth = 0;
    m_InternalHeight = 0;
    m_InternalPitch = 0;
    m_CurrentPicture = NULL;

    InitMediaType(&m_InternalMT);
    m_NeedToAttachFormat = false;
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
	    pProperties->cBuffers = 1;
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
        // make sure we've always got enough for standard definition TV YUY2
	    pProperties->cbBuffer = max(bih.biSizeImage, 768*576*2);
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
    	int wout = 0, hout = 0, arxout = 0, aryout = 0;
    	Result = (ExtractDim(pmt, wout, hout, arxout, aryout) &&
		            m_hin == abs((int)hout)) && 
                  (pmt->majortype == MEDIATYPE_Video) && 
                  (pmt->subtype == MEDIASUBTYPE_YUY2 ||
				    pmt->subtype == MEDIASUBTYPE_ARGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB24 ||
                    pmt->subtype == MEDIASUBTYPE_RGB565 ||
                    pmt->subtype == MEDIASUBTYPE_RGB555);
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
		if(m_sphli)
		{
			delete m_sphli;
		}
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
	    bihOut.biWidth = m_win;
	    bihOut.biHeight = m_hin;
	    bihOut.biPlanes = fmts[FormatNum].biPlanes;
	    bihOut.biBitCount = fmts[FormatNum].biBitCount;
	    bihOut.biCompression = fmts[FormatNum].biCompression;
	    bihOut.biSizeImage = m_win*m_hin*bihOut.biBitCount>>3;

	    if(TypeNum%3 == 2)
	    {
		    pmt->formattype = FORMAT_VideoInfo;
		    VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
		    memset(vih, 0, sizeof(VIDEOINFOHEADER));
		    vih->bmiHeader = bihOut;
		    vih->bmiHeader.biXPelsPerMeter = vih->bmiHeader.biWidth * m_aryin;
		    vih->bmiHeader.biYPelsPerMeter = vih->bmiHeader.biHeight * m_arxin;
	        vih->AvgTimePerFrame = ((VIDEOINFOHEADER*)mt->pbFormat)->AvgTimePerFrame;
	        vih->dwBitRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitRate;
	        vih->dwBitErrorRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitErrorRate;
            pmt->pbFormat = (BYTE*)vih;
            pmt->cbFormat = sizeof(VIDEOINFOHEADER);	    
        }
	    else
	    {
		    pmt->formattype = FORMAT_VideoInfo2;
		    VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
		    memset(vih, 0, sizeof(VIDEOINFOHEADER2));
		    vih->bmiHeader = bihOut;
		    vih->dwPictAspectRatioX = m_arxin;
		    vih->dwPictAspectRatioY = m_aryin;
            if(TypeNum%3 == 0)
            {
    		    vih->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave | AMINTERLACE_FieldPatBothRegular;
            }
	        vih->AvgTimePerFrame = ((VIDEOINFOHEADER*)mt->pbFormat)->AvgTimePerFrame;
	        vih->dwBitRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitRate;
	        vih->dwBitErrorRate = ((VIDEOINFOHEADER*)mt->pbFormat)->dwBitErrorRate;
            pmt->pbFormat = (BYTE*)vih;
            pmt->cbFormat = sizeof(VIDEOINFOHEADER2);	    
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
    if(pPin == m_VideoInPin)
    {
		m_win = m_hin = m_arxin = m_aryin = 0;
		ExtractDim(pMediaType, m_win, m_hin, m_arxin, m_aryin);
        ResetMpeg2Decoder();
    }
    else if(pPin == m_VideoOutPin)
    {
        int wout = 0, hout = 0, arxout = 0, aryout = 0; 
        ExtractDim(pMediaType, wout, hout, arxout, aryout); 
        if(m_win == wout && m_hin == hout && m_arxin == arxout && m_aryin == aryout) 
        { 
            m_wout = wout; 
            m_hout = hout; 
            m_arxout = arxout; 
            m_aryout = aryout; 
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
		if(m_VideoInPin->GetMediaType()->subtype == MEDIASUBTYPE_MPEG1Packet)
		{
			len -= 4+2+12; 
            pDataIn += 4+2+12;
		}
		else if(m_VideoInPin->GetMediaType()->subtype == MEDIASUBTYPE_MPEG2_VIDEO)
		{
			len -= 8; 
            pDataIn += 8;
			len -= *pDataIn+1; 
            pDataIn += *pDataIn+1;
		}
	}

	if(len <= 0) 
        return S_OK;

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
	{
		m_IsDiscontinuity = true;
	}

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
		LOG(DBGLOG_ALL, ("GetTime %010I64d %010I64d\n", pSampleProperties->tStart, pSampleProperties->tStop));
		LARGE_INTEGER temp;
		temp.QuadPart = pSampleProperties->tStart;
		mpeg2_tag_picture(m_dec, temp.HighPart, temp.LowPart);
    }

	mpeg2_buffer(m_dec, pDataIn, pDataIn + len);
	while(1)
	{
		mpeg2_state_t state = mpeg2_parse(m_dec);

		__asm emms; // this one is missing somewhere in the precompiled mmx obj files

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
            // this state seems to happen quite a lot in DVD
            // streams but ignoring it doesn't seem to do amything bad
            // although the docs for the decoder says that things should 
            // be screwed up
 			LOG(DBGLOG_FLOW, ("STATE_INVALID\n"));
 			break;
		case STATE_SEQUENCE:
            hr = ProcessNewSequence();
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
		default:
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
    //LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [%c] [prsq %d prfr %d tff %d rff %d nb_fields %d ref %d] (%dx%d/%dx%d)\n", 
	LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [%c] [num %d prfr %d tff %d rff %d] (%dx%d %d) (preroll %d) %d\n", 
		m_CurrentPicture->m_rtStart, m_CurrentPicture->m_rtStop,
		frametype[m_CurrentPicture->m_Flags&PIC_MASK_CODING_TYPE],
		m_CurrentPicture->m_NumFields,
		!!(m_CurrentPicture->m_Flags&PIC_FLAG_PROGRESSIVE_FRAME),
		!!(m_CurrentPicture->m_Flags&PIC_FLAG_TOP_FIELD_FIRST),
		!!(m_CurrentPicture->m_NumFields == 3),
		m_InternalWidth, m_InternalHeight, m_InternalPitch,
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
	rtStart = m_LastOutputTime;

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

	if(rtStop <= rtStart)
	{
		rtStop = rtStart + 1;
	}

	HRESULT hr;

	// cope with dynamic format changes from our side
	// will possibly call NegotiateAllocator on the output pins
	// which flushes so we shouldn't have any samples outstanding here
	if(FAILED(hr = ReconnectOutput(m_InternalWidth, m_InternalHeight)))
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
	// we care about this we think we need to change the format too
	// and the video renderer sends up a new format
	// calling ReconnectOutput again will merge the two 
	// formats properly and should never call NegotiateAllocator
	if(hr == S_FALSE && m_NeedToAttachFormat)
	{
		if(FAILED(hr = ReconnectOutput(m_InternalWidth, m_InternalHeight)))
		{
			LogBadHRESULT(hr, __FILE__, __LINE__);
			return hr;
		}
	}


	if(m_NeedToAttachFormat)
	{
		m_VideoOutPin->SetType(&m_InternalMT);
		pOut->SetMediaType(&m_InternalMT);

        SI(IMediaEventSink) pMES = m_Graph;
        if(pMES)
        {
		    // some renderers don't send this

		    pMES->Notify(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(m_win, m_hin), 0);
        }
	}

    if(FAILED(hr = pOut->GetPointer(&pDataOut)))
	{
        LogBadHRESULT(hr, __FILE__, __LINE__);
		return hr;
	}
	
	m_LastOutputTime = rtStop;

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
        // FIXME: hell knows why but without this the overlay mixer starts very skippy
    	// (don't enable this for other renderers, the old for example will go crazy if you do)
		if(m_IsDiscontinuity)
		{
		    Props.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
		}
		else
		{
		    Props.dwSampleFlags &= ~AM_SAMPLE_DATADISCONTINUITY;
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

		// FIXME: hell knows why but without this the overlay mixer starts very skippy
		// (don't enable this for other renderers, the old for example will go crazy if you do)
		if(m_IsDiscontinuity)
			pOut->SetDiscontinuity(TRUE);
        else
    	    pOut->SetDiscontinuity(FALSE);
	    pOut->SetSyncPoint(TRUE);
    }

	BYTE** buf = &m_CurrentPicture->m_Buf[0];

	if(m_ChromaType == CHROMA_420)
	{
	    if(HasSubpicsToRender(m_CurrentPicture->m_rtStart))
	    {
            // take a copy of the current picture
            // as we need to keep the decoder's copy clean
            // so that it can be used in predictions
            m_SubPicBuffer = *m_CurrentPicture;
		    
		    buf = &m_SubPicBuffer.m_Buf[0];

		    RenderSubpics(m_CurrentPicture->m_rtStart, buf, m_InternalPitch, m_InternalHeight);
	    }
	    else
	    {
		    ClearOldSubpics(m_CurrentPicture->m_rtStart);
	    }
		Copy420(pDataOut, buf, m_InternalWidth, m_InternalHeight, m_InternalPitch);
	}
	else if(m_ChromaType == CHROMA_422)
	{
		Copy422(pDataOut, buf, m_InternalWidth, m_InternalHeight, m_InternalPitch);
	}
	else
	{
		Copy444(pDataOut, buf, m_InternalWidth, m_InternalHeight, m_InternalPitch);
	}

	hr = m_VideoOutPin->SendSample(pOut.GetNonAddRefedInterface());
	if(FAILED(hr))
	{
        LogBadHRESULT(hr, __FILE__, __LINE__);
	}

	// reset discontinuity flag
	m_IsDiscontinuity = false;

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

HRESULT CMpegDecoder::ReconnectOutput(int w, int h)
{
	bool fForceReconnection = false;
	if(w != m_win || h != m_hin)
	{
		LOG(DBGLOG_FLOW, ("CMpeg2DecFilter : ReconnectOutput (%dx%d %dx%d)\n", w, h, m_win, m_hin));
		fForceReconnection = true;
		m_win = w;
		m_hin = h;
	}

	HRESULT hr = S_OK;

	if(fForceReconnection || m_win > m_wout || m_hin != m_hout || ((m_arxin != m_arxout) && m_arxout) || ((m_aryin != m_aryout) && m_aryout))
	{
        CopyMediaType(&m_InternalMT, m_VideoOutPin->GetMediaType());
	
    	BITMAPINFOHEADER* bmi = NULL;

		if(m_InternalMT.formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
			SetRect(&vih->rcSource, 0, 0, m_win, m_hin);
			SetRect(&vih->rcTarget, 0, 0, m_win, m_hin);
			bmi = &vih->bmiHeader;
			bmi->biXPelsPerMeter = m_win * m_aryin;
			bmi->biYPelsPerMeter = m_hin * m_arxin;
		}
		else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
			SetRect(&vih->rcSource, 0, 0, m_win, m_hin);
			SetRect(&vih->rcTarget, 0, 0, m_win, m_hin);
			bmi = &vih->bmiHeader;
			vih->dwPictAspectRatioX = m_arxin;
			vih->dwPictAspectRatioY = m_aryin;
		}

		bmi->biWidth = max(m_win,bmi->biWidth);
        if(bmi->biHeight > 0)
        {
    		bmi->biHeight = m_hin;
        }
        else
        {
    		bmi->biHeight = -m_hin;
        }
		bmi->biSizeImage = m_hin*bmi->biWidth*bmi->biBitCount>>3;
        
        hr = m_VideoOutPin->m_ConnectedPin->QueryAccept(&m_InternalMT);
        if(hr != S_OK)
        {
    		LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
            return VFW_E_TYPE_NOT_ACCEPTED;
        }

    	if(!m_VideoOutPin->m_Allocator) return E_NOINTERFACE;

	    ALLOCATOR_PROPERTIES AllocatorProps;
	    hr = m_VideoOutPin->m_Allocator->GetProperties(&AllocatorProps);
        CHECK(hr);

        if(bmi->biSizeImage > (DWORD)AllocatorProps.cbBuffer)
        {
            hr = m_VideoOutPin->NegotiateAllocator(NULL, &m_InternalMT);
            if(FAILED(hr))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
                return hr;
            }
        }
        
        m_NeedToAttachFormat = true;

        m_wout = m_win; 
        m_hout = m_hin; 
        m_arxout = m_arxin; 
        m_aryout = m_aryin; 

        return S_OK;
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
    	mpeg2_reset(m_dec, 1);
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
	else if(bihOut.biCompression == 'I420' || bihOut.biCompression == 'VUYI')
	{
		BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h, pOut + bihOut.biWidth*h*5/4, bihOut.biWidth, pIn, pInU, pInV, pitchIn);
	}
	else if(bihOut.biCompression == '21VY')
	{
		BitBltFromI420ToI420(w, h, pOut, pOut + bihOut.biWidth*h*5/4, pOut + bihOut.biWidth*h, bihOut.biWidth, pIn, pInU, pInV, pitchIn);
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
	if(m_sphli)
	{
		delete m_sphli;
	}
    if(m_dec != NULL)
    {
        mpeg2_free(m_dec);
        m_dec = NULL;
    }
    return S_OK;
}

HRESULT CMpegDecoder::ProcessNewSequence()
{
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

	// frame buffer
	m_InternalWidth = mpeg2_info(m_dec)->sequence->picture_width;
	m_InternalHeight = mpeg2_info(m_dec)->sequence->picture_height;
	m_InternalPitch = mpeg2_info(m_dec)->sequence->width;

    mpeg2_custom_fbuf(m_dec, 1);

    for(int i(0); i < NUM_BUFFERS; ++i)
    {
        m_Buffers[i].AllocMem(m_InternalHeight * m_InternalPitch, m_InternalHeight * m_InternalPitch / ChromaSizeDivider);
    }

    m_SubPicBuffer.AllocMem(m_InternalHeight * m_InternalPitch, m_InternalHeight * m_InternalPitch / ChromaSizeDivider);

    m_CurrentPicture = NULL;

    m_Buffers[0].AddRef();
    m_Buffers[1].AddRef();
	mpeg2_set_buf(m_dec, m_Buffers[0].m_Buf, &m_Buffers[0]);
	mpeg2_set_buf(m_dec, m_Buffers[1].m_Buf, &m_Buffers[1]);

	m_fWaitForKeyFrame = true;
	m_fFilm = false;

    return S_OK;
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

        // we should get updates about the timings only on I frames
        // sometimes we get them more frequently
        // \todo check the I frame thing
		if((picture->tag != 0 || picture->tag2 != 0) && ((m_CurrentPicture->m_Flags&PIC_MASK_CODING_TYPE) == PIC_FLAG_CODING_TYPE_I))
		{
			LARGE_INTEGER temp;
			temp.HighPart = picture->tag;
			temp.LowPart = picture->tag2;

			m_CurrentPicture->m_rtStart = temp.QuadPart;
		}
		else
		{
            if((picture->tag != 0 || picture->tag2 != 0))
            {
			    LARGE_INTEGER temp;
			    temp.HighPart = picture->tag;
			    temp.LowPart = picture->tag2;
                LOG(DBGLOG_FLOW, ("Ignored non I frame time - %010I64d\n", temp.QuadPart));
            }
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

        if(m_CurrentPicture->m_rtStart >= 0 && !m_fWaitForKeyFrame)
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