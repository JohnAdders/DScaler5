///////////////////////////////////////////////////////////////////////////////
// $Id: DivxDecoder.cpp,v 1.7 2004-11-25 17:22:09 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DivxVideo.dll - DirectShow filter for decoding Divx streams
// Copyright (c) 2004 John Adcock
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2004/11/18 21:26:23  adcockj
// fix accedental checkin of conflict
//
// Revision 1.5  2004/11/09 17:21:37  adcockj
// Seeking fixes
//
// Revision 1.4  2004/11/06 14:36:08  adcockj
// VS6 project update
//
// Revision 1.3  2004/11/06 14:07:00  adcockj
// Fixes for WM10 and seeking
//
// Revision 1.2  2004/11/05 18:09:44  adcockj
// Fix for unusual headers
//
// Revision 1.1  2004/11/05 17:45:53  adcockj
// Added new decoder
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DivxDecoder.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSBufferedInputPin.h"
#include "DSVideoOutPin.h"
#include "MediaBufferWrapper.h"
#include "MediaTypes.h"
#include "MoreUuids.h"
#include "DSUtil.h"

extern HINSTANCE g_hInstance;

extern "C"
{
void av_set_memory(av_malloc_fc mal,av_free_fc fre,av_realloc_fc rel);
void av_register_all(void);
}


CDivxDecoder::CDivxDecoder() :
    CDSBaseFilter(L"DivxVideo Filter", 1, 1)
{
    LOG(DBGLOG_FLOW, ("CDivxDecoder::CreatePins\n"));
    
    m_VideoInPin = new CDSInputPin;
    if(m_VideoInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_VideoInPin->AddRef();
    m_VideoInPin->SetupObject(this, L"Video In");

    // can't use m_VideoOutPin due to casting
    m_OutputPins[0] = new CDSVideoOutPin();
    if(m_VideoOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 3"));
    }
    m_VideoOutPin->AddRef();
    m_VideoOutPin->SetupObject(this, L"Video Out");

    m_LastOutputTime = 0;

    m_NextFrameDeint = DIBob;

    m_DivxWidth = 0;
    m_DivxHeight = 0;

    m_CurrentPicture = NULL;

    m_ARDivxX = 4;
    m_ARDivxY = 3;

    m_Rate = 10000;

    m_CodecID = CODEC_ID_NONE;
    m_Codec = NULL;
    m_CodecContext = NULL;

    av_set_memory(malloc,free,realloc);
    avcodec_init();
    av_log_set_callback(avlog);
    m_Rate = 10000;
}

CDivxDecoder::~CDivxDecoder()
{
    LOG(DBGLOG_FLOW, ("CDivxDecoder::~CDivxDecoder\n"));
}

STDMETHODIMP CDivxDecoder::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_ALL, ("CDivxDecoder::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CDivxDecoder;
    return S_OK;
}


STDMETHODIMP CDivxDecoder::get_Name(BSTR* Name)
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

STDMETHODIMP CDivxDecoder::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CDivxDecoder::get_Authors(BSTR* Authors)
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

HRESULT CDivxDecoder::ParamChanged(DWORD dwParamIndex)
{
    HRESULT hr = S_OK;
    switch(dwParamIndex)
    {
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
        break;
    case OUTPUTSPACE:
        if(m_VideoOutPin->m_ConnectedPin)
        {
            return S_FALSE;
        }
        break;
    }
    return hr;
}


HRESULT CDivxDecoder::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDivxDecoder::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDivxDecoder::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDivxDecoder::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
	return E_FAIL;
}

STDMETHODIMP CDivxDecoder::GetDecoderCaps(DWORD dwCapIndex,DWORD* lpdwCap)
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

HRESULT CDivxDecoder::GetAllocatorRequirements(ALLOCATOR_PROPERTIES* pProperties, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        pProperties->cBuffers = 200;
        pProperties->cbBuffer = 2048;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else if(pPin == m_VideoOutPin)
    {
        pProperties->cBuffers = 3;
        pProperties->cbBuffer = ExtractBIH(&m_VideoOutPin->m_ConnectedMediaType)->biSizeImage;
        pProperties->cbAlign = 1;
        pProperties->cbPrefix = 0;
    }
    else
    {
        return E_UNEXPECTED;
    }
    return S_OK;
}

bool CDivxDecoder::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool Result = false;
    if(pPin == m_VideoInPin)
    {
        Result = (pmt->majortype == MEDIATYPE_Video && 
                    (pmt->subtype == MEDIASUBTYPE_xvid ||
                     pmt->subtype == MEDIASUBTYPE_XVID ||
                     pmt->subtype == MEDIASUBTYPE_divx ||
                     pmt->subtype == MEDIASUBTYPE_DIVX ||
                     pmt->subtype == MEDIASUBTYPE_div3 ||
                     pmt->subtype == MEDIASUBTYPE_DIV3 ||
                     pmt->subtype == MEDIASUBTYPE_dx50 ||
                     pmt->subtype == MEDIASUBTYPE_DX50 ||
                     pmt->subtype == MEDIASUBTYPE_mp43 ||
                     pmt->subtype == MEDIASUBTYPE_MP43 ||
                     pmt->subtype == MEDIASUBTYPE_mp42 ||
                     pmt->subtype == MEDIASUBTYPE_MP42 ||
                     pmt->subtype == MEDIASUBTYPE_mp41 ||
                     pmt->subtype == MEDIASUBTYPE_MP41));
    }
    else if(pPin == m_VideoOutPin)
    {
        int wout = 0, hout = 0;
        long arxout = 0, aryout = 0;
        Result = (ExtractDim(pmt, wout, hout, arxout, aryout) && 
                  (pmt->majortype == MEDIATYPE_Video) && 
                  (pmt->subtype == MEDIASUBTYPE_YUY2 ||
                   pmt->subtype == MEDIASUBTYPE_YV12 ||
                    pmt->subtype == MEDIASUBTYPE_ARGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB24 ||
                    pmt->subtype == MEDIASUBTYPE_RGB565 ||
                    pmt->subtype == MEDIASUBTYPE_RGB555));
        Result &= (abs(m_VideoOutPin->GetHeight()) == abs(hout)) && (m_VideoOutPin->GetWidth() == wout);
    }
    return Result;
}

HRESULT CDivxDecoder::GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText)
{
    if(dwParamIndex == DEINTMODE)
    {
        return GetEnumTextDeintMode(ppwchText);
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


HRESULT CDivxDecoder::GetEnumTextDeintMode(WCHAR **ppwchText)
{
    wchar_t Text[] = L"Deinterlace Mode\0" L"None\0" L"Automatic\0" L"Force Weave\0" L"Force Bob\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}

HRESULT CDivxDecoder::GetEnumTextOutputSpace(WCHAR **ppwchText)
{
    wchar_t Text[] = L"Output Colour Space\0" L"None\0" L"YV12\0" L"YUY2\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(Text));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, Text, sizeof(Text));
    return S_OK;
}


HRESULT CDivxDecoder::Flush(CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        FlushDivx();
    }
    return S_OK;
}

HRESULT CDivxDecoder::SendOutLastSamples(CDSBasePin* pPin)
{
    HRESULT hr = S_OK;
    int GotPicture = 0;

    if(m_CurrentPicture == NULL)
    {
        m_CurrentPicture = GetNextBuffer();
    }
    avcodec_decode_video(m_CodecContext, &m_CurrentPicture->m_Picture, &GotPicture, NULL, 0);

    if(GotPicture)
    {
        hr = Deliver();
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
    }
    return S_OK;
}


HRESULT CDivxDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{

    if(pPin == m_VideoOutPin)
    {
        if(!m_VideoInPin->IsConnected()) return VFW_E_NOT_CONNECTED;
        DWORD VideoFlags = (GetParamEnum(OUTPUTSPACE) == SPACE_YUY2)?VIDEOTYPEFLAG_FORCE_YUY2:0;
        return m_VideoOutPin->CreateSuitableMediaType(pmt, TypeNum, VideoFlags, 0);
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }
}

HRESULT CDivxDecoder::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    HRESULT hr = S_OK;

    if(pPin == m_VideoInPin)
    {
        m_DivxWidth = m_DivxHeight = 0;
        m_ARDivxX = 4; 
        m_ARDivxY = 3;
        ExtractDim(pMediaType, m_DivxWidth, m_DivxHeight, m_ARDivxX, m_ARDivxY);

        BITMAPINFOHEADER* bih = NULL;
        m_ExtraSize = 0;
        m_ExtraData = NULL;
        RECT* pSourceRect;
    
        m_fWaitForKeyFrame = true;

        if(pMediaType->formattype == FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pMediaType->pbFormat;
            m_AvgTimePerFrame = vih->AvgTimePerFrame;
            bih = &vih->bmiHeader;
            if(pMediaType->cbFormat > sizeof(VIDEOINFOHEADER2))
            {
                m_ExtraSize = pMediaType->cbFormat - sizeof(VIDEOINFOHEADER2);
                m_ExtraData = pMediaType->pbFormat + sizeof(VIDEOINFOHEADER2);
            }
	        pSourceRect = &vih->rcSource;
    	}
		else if(pMediaType->formattype == FORMAT_VideoInfo)
		{
            VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pMediaType->pbFormat;
            m_AvgTimePerFrame = vih->AvgTimePerFrame;
            bih = &vih->bmiHeader;
            if(pMediaType->cbFormat > sizeof(VIDEOINFOHEADER))
            {
                m_ExtraSize = pMediaType->cbFormat - sizeof(VIDEOINFOHEADER);
                m_ExtraData = pMediaType->pbFormat + sizeof(VIDEOINFOHEADER);
            }
            pSourceRect = &vih->rcSource;
		}
        else
        {
            return E_UNEXPECTED;
        }

        if(pSourceRect->right != 0)
        {
            m_DivxWidth = min(m_DivxWidth, pSourceRect->right);
        }

        if(pSourceRect->bottom != 0)
        {
            m_DivxHeight = min(abs(m_DivxHeight), pSourceRect->bottom);
        }

        m_VideoOutPin->SetAspectX(m_DivxWidth);
        m_VideoOutPin->SetAspectY(m_DivxHeight);
        m_VideoOutPin->SetWidth(m_DivxWidth);
        m_VideoOutPin->SetHeight(m_DivxHeight);
        m_VideoOutPin->SetPanScanX(0);
        m_VideoOutPin->SetPanScanY(0);

        m_VideoOutPin->SetAvgTimePerFrame(m_AvgTimePerFrame);

        switch(bih->biCompression)
        {
        case MAKEFOURCC('D', 'I', 'V', 'X'):
        case MAKEFOURCC('d', 'i', 'v', 'x'):
        case MAKEFOURCC('X', 'V', 'I', 'D'):
        case MAKEFOURCC('x', 'v', 'i', 'd'):
        case MAKEFOURCC('D', 'X', '5', '0'):
        case MAKEFOURCC('d', 'x', '5', '0'):
            m_CodecID = CODEC_ID_MPEG4;
            break;
        case MAKEFOURCC('D', 'I', 'V', '3'):
        case MAKEFOURCC('d', 'i', 'v', '3'):
        case MAKEFOURCC('M', 'P', '4', '3'):
        case MAKEFOURCC('m', 'p', '4', '3'):
            m_CodecID = CODEC_ID_MSMPEG4V3;
            break;
        case MAKEFOURCC('M', 'P', '4', '2'):
        case MAKEFOURCC('m', 'p', '4', '2'):
            m_CodecID = CODEC_ID_MSMPEG4V2;
            break;
        case MAKEFOURCC('M', 'P', '4', '1'):
        case MAKEFOURCC('m', 'p', '4', '1'):
            m_CodecID = CODEC_ID_MSMPEG4V1;
            break;
        default:
            return E_UNEXPECTED;
            break;
        }
        m_FourCC = bih->biCompression;

    }
    else if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyFormatChange(pMediaType);
    }
    return S_OK;
}

HRESULT CDivxDecoder::NotifyConnected(CDSBasePin* pPin)
{
    if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyConnected();
    }
    return S_OK;
}


HRESULT CDivxDecoder::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    HRESULT hr = S_OK;

    CProtectCode WhileVarInScope(&m_DeliverLock);

    if(pSampleProperties->dwStreamId != AM_STREAM_MEDIA)
        return m_VideoOutPin->SendSample(InSample);

    BYTE* pDataIn = pSampleProperties->pbBuffer;

    long len = pSampleProperties->lActual;

    if(len <= 0) 
        return S_OK;

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
    {
        m_IsDiscontinuity = true;
    }

    if(m_CurrentPicture == NULL)
    {
        m_CurrentPicture = GetNextBuffer();
    }

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        m_CurrentPicture->m_rtStart = pSampleProperties->tStart;
        m_CurrentPicture->m_rtStop = pSampleProperties->tStart + m_AvgTimePerFrame * m_Rate / 10000;
    }

    int GotPicture = 0;
    int Count = 0;
    while(len - Count > 0)
    {
        int Used = avcodec_decode_video(m_CodecContext, &m_CurrentPicture->m_Picture, &GotPicture, pDataIn + Count, len - Count);
        if(Used > 0)
        {
            Count += Used;
        }
        else
        {
            Count++;
        }
        if(GotPicture)
        {
            hr = Deliver();
            m_CurrentPicture->Release();
            m_CurrentPicture = GetNextBuffer();
        }
    }
    return hr;
}

HRESULT CDivxDecoder::Deliver()
{
    TCHAR frametype[] = {'?','I', 'P', 'B', 'S', '1', '2',};
    LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [%c] [repeat %d keyframe %d int %d tff %d]\n", 
        m_CurrentPicture->m_rtStart, m_CurrentPicture->m_rtStop,
        frametype[m_CurrentPicture->m_Picture.pict_type],
        m_CurrentPicture->m_Picture.repeat_pict,
        m_CurrentPicture->m_Picture.key_frame,
        m_CurrentPicture->m_Picture.interlaced_frame,
        m_CurrentPicture->m_Picture.top_field_first));

    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStop;
    
    if(m_IsDiscontinuity)
    {
        // if we're at a Discontinuity use the times we're being sent in
        rtStart = m_CurrentPicture->m_rtStart;
    }
    else
    {
        // if we're not at a Discontinuity
        // make sure that time are contiguous
        rtStart = m_LastOutputTime;
    }

    rtStop = m_CurrentPicture->m_rtStop;

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

    HRESULT hr = S_OK;
    m_LastOutputTime = rtStop;
    
    if(m_fWaitForKeyFrame)
    {
        if(m_CurrentPicture->m_Picture.key_frame)
        {
            m_fWaitForKeyFrame = false;
        }
        else
        {
            return hr;
        }
    }

    if(rtStop > 0)
    {


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
        
        hr = m_VideoOutPin->GetOutputSample(pOut.GetReleasedInterfaceReference(), &rtStart, &rtStop, m_IsDiscontinuity);
        if(FAILED(hr))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }

        LOG(DBGLOG_FLOW, ("%010I64d - %010I64d - %010I64d - %010I64d\n", m_CurrentPicture->m_rtStart, m_CurrentPicture->m_rtStop, rtStart, rtStop));

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

            // ffdshow quality control requires stop times
            // nothing else needs these and setting stop times
            // seems to sometimes results in frames being dropped
            // which we really wanted to show

            if(m_IsDiscontinuity)
            {
                Props.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
            }
            else
            {
                Props.dwSampleFlags &= ~AM_SAMPLE_DATADISCONTINUITY;
            }
            Props.dwSampleFlags |= AM_SAMPLE_TIMEVALID;
            Props.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;

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
            // ffdshow quality control requires stop times
            // nothing else needs these and setting stop times
            // seems to sometimes results in frames being dropped
            // which we really wanted to show
            pOut->SetTime(&rtStart, &rtStop);

            if(m_IsDiscontinuity)
            {
                pOut->SetDiscontinuity(TRUE);
            }
            pOut->SetSyncPoint(TRUE);
        }

        BYTE** buf = &m_CurrentPicture->m_Picture.data[0];

        if(FAILED(hr = pOut->GetPointer(&pDataOut)))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }

        m_VideoOutPin->Copy420(pDataOut, buf, m_DivxWidth, m_DivxHeight, m_CurrentPicture->m_Picture.linesize[0], true);

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
            m_IsDiscontinuity = false;
        }
    }

    return hr;
}

void CDivxDecoder::FlushDivx()
{
    CProtectCode WhileVarInScope(m_VideoInPin);
    CProtectCode WhileVarInScope2(&m_DeliverLock);
        
    m_fWaitForKeyFrame = true;
    m_fFilm = false;
    m_IsDiscontinuity = true;

	if(m_CurrentPicture != NULL)
    {
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
    }
	avcodec_flush_buffers(m_CodecContext);
}

void CDivxDecoder::ResetDivxDecoder()
{
    LOG(DBGLOG_FLOW, ("ResetDivxDecoder()\n"));

    CProtectCode WhileVarInScope(&m_DeliverLock);

    if(m_CurrentPicture != NULL)
    {
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
    }

    if(m_CodecContext != NULL)
    {
        avcodec_close(m_CodecContext);
        m_CodecContext = NULL;
    }
    m_Codec = NULL;

    m_CodecContext = avcodec_alloc_context();

    //m_CodecContext->flags |= CODEC_FLAG_TRUNCATED;

    m_CodecContext->width = m_DivxWidth;
    m_CodecContext->height = m_DivxHeight;
    m_CodecContext->idct_algo = FF_IDCT_AUTO;
    m_CodecContext->codec_tag = m_FourCC;
    m_CodecContext->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
    m_CodecContext->error_resilience = FF_ER_COMPLIANT;

    switch(m_CodecID)
    {
    case CODEC_ID_MSMPEG4V3:
        m_Codec = &msmpeg4v3_decoder;
        break;
    case CODEC_ID_MSMPEG4V2:
        m_Codec = &msmpeg4v2_decoder;
        break;
    case CODEC_ID_MSMPEG4V1:
        m_Codec = &msmpeg4v1_decoder;
        break;
    default:
    case CODEC_ID_MPEG4:
        m_Codec = &mpeg4_decoder;
        break;
    }

    if (avcodec_open(m_CodecContext, m_Codec) < 0) 
    {
        avcodec_close(m_CodecContext);
        free(m_CodecContext);
        m_CodecContext = NULL;
        m_Codec = NULL;
        return;
    }

    m_fWaitForKeyFrame = true;
}


HRESULT CDivxDecoder::Activate()
{
    BYTE* pSequenceHeader = NULL;
    DWORD cbSequenceHeader = 0;

    m_CurrentPicture = NULL;

    ResetDivxDecoder();

    m_IsDiscontinuity = true;
    m_LastOutputTime = 0;

    return S_OK;
}

HRESULT CDivxDecoder::Deactivate()
{
    if(m_CurrentPicture)
    {
        m_CurrentPicture->Release();
        m_CurrentPicture = NULL;
    }
    if(m_CodecContext != NULL)
    {
        avcodec_close(m_CodecContext);
        free(m_CodecContext);
        m_CodecContext = NULL;
    }
    m_Codec = NULL;
    return S_OK;
}

HRESULT CDivxDecoder::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        LOG(DBGLOG_FLOW, ("New Segment %010I64d - %010I64d  @ %f\n", tStart, tStop, dRate));
        m_LastOutputTime = 0;
        m_Rate = (DWORD)(10000 / dRate);
        m_IsDiscontinuity = true;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}

CDivxDecoder::CFrameBuffer* CDivxDecoder::GetNextBuffer()
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

void CDivxDecoder::avlog(void*,int,const char* szFormat, va_list Args)
{
    char szMessage[2048];

    int result=_vsnprintf(szMessage,2048, szFormat, Args);
    if(result==-1)
    {
        OutputDebugString("DebugString too long, truncated!!\n");
    }
    OutputDebugString(szMessage);
}
