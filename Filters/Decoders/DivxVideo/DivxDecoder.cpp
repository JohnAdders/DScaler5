///////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "stdafx.h"
#include "DivxDecoder.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSBufferedInputPin.h"
#include "DSVideoOutPin.h"
#include "VideoData.h"
#include "MediaBufferWrapper.h"
#include "MediaTypes.h"
#include "MoreUuids.h"
#include "DSUtil.h"

extern HINSTANCE g_hInstance;


CDivxDecoder::CDivxDecoder() :
    CDSBaseFilter(L"DivxVideo Filter", 1, 1),
    m_Negotiator(CVideoFormatNegotiator::NORMAL_420, true)
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
    m_OutputPins[0] = new CDSVideoOutPin(m_Negotiator);
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

    m_ARDivxX = 4;
    m_ARDivxY = 3;

    m_Rate = 10000;

    m_CodecID = CODEC_ID_NONE;
    m_Codec = NULL;
    m_CodecContext = NULL;

    m_NalSize = 0;

    av_log_set_callback(avlog);
    avcodec_init();
    avcodec_register_all();
    m_Rate = 10000;
}

CDivxDecoder::~CDivxDecoder()
{
    LOG(DBGLOG_FLOW, ("CDivxDecoder::~CDivxDecoder\n"));
    for(size_t i(0); i < m_Buffers.size(); ++i)
    {
        delete m_Buffers[i];
    }
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
        pProperties->cBuffers = 1;
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
        Result = pmt->majortype == MEDIATYPE_Video?true:false;
        GUID test = MEDIASUBTYPE_xvid;
        SCodecList* CodecList = getCodecList();
        while(CodecList->FourCC)
        {
            test.Data1 = UpperFourCC(CodecList->FourCC);
            if(test == pmt->subtype)
            {
                return Result;
            }

            test.Data1 = LowerFourCC(CodecList->FourCC);
            if(test == pmt->subtype)
            {
                return Result;
            }

            ++CodecList;
        }
        Result = false;
    }
    else if(pPin == m_VideoOutPin)
    {
        int wout = 0, hout = 0, pitch = 0;
        long arxout = 0, aryout = 0;
        Result = (ExtractDim(pmt, wout, hout, arxout, aryout, pitch) &&
                  (pmt->majortype == MEDIATYPE_Video) &&
                  (pmt->subtype == MEDIASUBTYPE_YUY2 ||
                   pmt->subtype == MEDIASUBTYPE_YV12 ||
                   pmt->subtype == MEDIASUBTYPE_NV12 ||
                    pmt->subtype == MEDIASUBTYPE_ARGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB32 ||
                    pmt->subtype == MEDIASUBTYPE_RGB24 ||
                    pmt->subtype == MEDIASUBTYPE_RGB565 ||
                    pmt->subtype == MEDIASUBTYPE_RGB555));
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
    wchar_t Text[] = L"Output Colour Space\0" L"None\0" L"YV12\0" L"YUY2\0" L"NV12\0";
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

    AVFrame NextFrame;

    avcodec_decode_video(m_CodecContext, &NextFrame, &GotPicture, NULL, 0);

    CFrameBuffer* CurrentPicture = (CFrameBuffer*)NextFrame.opaque;

    if(GotPicture)
    {
        hr = Deliver(NextFrame, CurrentPicture);
    }
    return S_OK;
}


HRESULT CDivxDecoder::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
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
        int pitch = 0;
        ExtractDim(pMediaType, m_DivxWidth, m_DivxHeight, m_ARDivxX, m_ARDivxY, pitch);

        BITMAPINFOHEADER* bih = NULL;
        m_ExtraSize = 0;
        m_ExtraData.clear();
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
                m_ExtraData.assign(pMediaType->pbFormat + sizeof(VIDEOINFOHEADER2), pMediaType->pbFormat + sizeof(VIDEOINFOHEADER2) + m_ExtraSize);
                m_ExtraData.resize(m_ExtraData.size() + 8, 0);
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
                m_ExtraData.assign(pMediaType->pbFormat + sizeof(VIDEOINFOHEADER), pMediaType->pbFormat + sizeof(VIDEOINFOHEADER) + m_ExtraSize);
                m_ExtraData.resize(m_ExtraData.size() + 8, 0);
            }
            pSourceRect = &vih->rcSource;
        }
        else if(pMediaType->formattype == FORMAT_MPEG2_VIDEO)
        {
            MPEG2VIDEOINFO* mvih = (MPEG2VIDEOINFO*)pMediaType->pbFormat;
            m_AvgTimePerFrame = mvih->hdr.AvgTimePerFrame;
            bih = &mvih->hdr.bmiHeader;
            if(UpperFourCC(pMediaType->subtype.Data1) == MAKEFOURCC('A', 'V', 'C', '1'))
            {
                m_ExtraData.push_back(0x01);
                m_ExtraData.push_back((uint8_t)mvih->dwProfile);
                m_ExtraData.push_back(0x00);
                m_ExtraData.push_back((uint8_t)mvih->dwLevel);
                m_ExtraData.push_back((uint8_t)(mvih->dwFlags - 1));
                m_ExtraData.push_back(0x01);

                BYTE* pExtra = (BYTE*)mvih->dwSequenceHeader;
                long len = mvih->cbSequenceHeader;
                int firstLen = pExtra[0] << 8 | pExtra[1] + 2;
                if(firstLen <= len)
                {
                    while(firstLen)
                    {
                        m_ExtraData.push_back(*pExtra++);
                        --firstLen;
                    }
                    len -= firstLen;
                }

                m_ExtraData.push_back(0x01);

                int secondLen = pExtra[0] << 8 | pExtra[1] + 2;
                if(secondLen <= len)
                {
                    while(secondLen)
                    {
                        m_ExtraData.push_back(*pExtra++);
                        --secondLen;
                    }
                }
                m_ExtraSize = mvih->cbSequenceHeader + 7;
                m_ExtraData.resize(mvih->cbSequenceHeader + 7 + 8, 0);
            }
            else
            {
                if(mvih->cbSequenceHeader)
                {
                    m_ExtraSize = mvih->cbSequenceHeader;
                    m_ExtraData.assign((BYTE*)mvih->dwSequenceHeader, (BYTE*)mvih->dwSequenceHeader + mvih->cbSequenceHeader);
                    m_ExtraData.resize(mvih->cbSequenceHeader + 7 + 8, 0);
                }
            }
            pSourceRect = &mvih->hdr.rcSource;
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

        m_VideoOutPin->SetAspectX(m_ARDivxX);
        m_VideoOutPin->SetAspectY(m_ARDivxY);
        m_VideoOutPin->SetWidth(m_DivxWidth);
        m_VideoOutPin->SetHeight(m_DivxHeight);

        m_VideoOutPin->SetAvgTimePerFrame(m_AvgTimePerFrame);

        if(bih->biCompression != 0)
        {
            m_FourCC = UpperFourCC(bih->biCompression);
        }
        else
        {
            m_FourCC = UpperFourCC(pMediaType->subtype.Data1);
        }
        m_CodecID = lookupCodec(m_FourCC);

        if(m_CodecID == CODEC_ID_NONE)
        {
            return E_UNEXPECTED;
        }
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

    LOG(DBGLOG_FLOW, ("Process %010I64d - %010I64d - %d\n", pSampleProperties->tStart, pSampleProperties->tStop, len));

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
    {
        m_IsDiscontinuity = true;
    }

    if(m_CodecID == CODEC_ID_H264)
    {
        if(m_fWaitForKeyFrame && (pSampleProperties->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) == 0)
        {
            // completely skip pre-roll
            if(pSampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL)
            {
                return S_FALSE;
            }
            else
            {
                LOG(DBGLOG_ALL, ("Start code %x %x %x %x %x\n", pSampleProperties->pbBuffer[0], pSampleProperties->pbBuffer[1], pSampleProperties->pbBuffer[2], pSampleProperties->pbBuffer[3], pSampleProperties->pbBuffer[4]));
                // return early if until we get the sync point flag
                if(m_CodecContext->codec_tag == MAKEFOURCC('A', 'V', 'C', '1'))
                {
                    if(pSampleProperties->pbBuffer[0] != 0 ||
                        pSampleProperties->pbBuffer[1] != 0 ||
                        pSampleProperties->pbBuffer[2] != 0 ||
                        //pSampleProperties->pbBuffer[3] != 0x01 ||
                        pSampleProperties->pbBuffer[4] != 0x67)
                    {
                        return S_OK;
                    }
                }
                else
                {
                    if(pSampleProperties->pbBuffer[0] != 0 ||
                        pSampleProperties->pbBuffer[1] != 0 ||
                        pSampleProperties->pbBuffer[2] != 0x01 ||
                        pSampleProperties->pbBuffer[3] != 0x67)
                    {
                        return S_OK;
                    }
                }
            }
        }
        m_fWaitForKeyFrame = false;
    }

    if(m_AvgTimePerFrame == 0)
    {
        if(pSampleProperties->tStop > pSampleProperties->tStart)
        {
            m_AvgTimePerFrame = pSampleProperties->tStop - pSampleProperties->tStart;
        }
        else if(m_LastInputTime > 0)
        {
            m_AvgTimePerFrame = pSampleProperties->tStart - m_LastInputTime;
        }
    }

    m_LastInputTime = pSampleProperties->tStart;

    int GotPicture = 0;
    int Count = 0;
    static std::vector<uint8_t> buffer(len + 8);
    while(len - Count > 0)
    {
        AVFrame NextFrame;

        buffer.resize(len - Count + 8, 0);
        buffer.assign((uint8_t*)(pDataIn + Count), (uint8_t*)(pDataIn + len));

        int Used = avcodec_decode_video(m_CodecContext, &NextFrame, &GotPicture, &buffer[0], len - Count);

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
            CFrameBuffer* CurrentPicture = (CFrameBuffer*)NextFrame.opaque;
            CurrentPicture->m_rtStartDisplay = m_LastInputTime;

            // deinterlace
            switch(GetParamEnum(DEINTMODE))
            {
            case DIAuto:
                if(NextFrame.interlaced_frame == 0)
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

            hr = Deliver(NextFrame, CurrentPicture);
        }
    }
    return hr;
}

HRESULT CDivxDecoder::Deliver(AVFrame& NextFrame, CFrameBuffer* CurrentPicture)
{
    HRESULT hr = S_OK;
    TCHAR frametype[] = {'?','I', 'P', 'B', 'S', '1', '2',};
    LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [%c] [repeat %d keyframe %d int %d tff %d] %d\n",
        CurrentPicture->m_rtStartCoded, CurrentPicture->m_rtStartDisplay,
        frametype[NextFrame.pict_type],
        NextFrame.repeat_pict,
        NextFrame.key_frame,
        NextFrame.interlaced_frame,
        NextFrame.top_field_first,
        NextFrame.coded_picture_number));

    if(m_fWaitForKeyFrame)
    {
        if(NextFrame.key_frame || NextFrame.pict_type == FF_I_TYPE)
        {
            m_fWaitForKeyFrame = false;
            m_IsDiscontinuity = true;
        }
        else
        {
            return hr;
        }
    }


    REFERENCE_TIME rtStart;
    REFERENCE_TIME rtStartFromFile;
    REFERENCE_TIME rtStop;

    // AVC1 and MP4V seems to be flaged like MPEG 2 so the timestamps are when
    // you get the frame start not when you show the frame
    if(m_CodecContext->codec_tag == MAKEFOURCC('A', 'V', 'C', '1') || m_CodecContext->codec_tag == MAKEFOURCC('M', 'P', '4', 'V'))
    {
        rtStartFromFile = CurrentPicture->m_rtStartCoded;
    }
    else
    {
        rtStartFromFile = CurrentPicture->m_rtStartDisplay;
    }

    if(m_IsDiscontinuity)
    {
        // if we're at a Discontinuity use the times we're being sent in
        rtStart = rtStartFromFile;
        rtStop = rtStartFromFile + m_AvgTimePerFrame * m_Rate / 10000 ;
    }
    else
    {
        // if we're not at a Discontinuity
        // make sure that time are contiguous
        rtStart = m_LastOutputTime;
        if(NextFrame.pict_type == FF_I_TYPE && abs(long(rtStartFromFile - m_LastOutputTime)) > 100000 )
        {
            LOG(DBGLOG_FLOW, ("Adjusting timestamps\n"));
            rtStop = rtStartFromFile + m_AvgTimePerFrame * m_Rate / 10000 ;
        }
        else
        {
            rtStop = rtStart + m_AvgTimePerFrame * m_Rate / 10000 ;
        }
    }


    rtStop += GetParamInt(VIDEODELAY) * 10000;

    // ensure that times always go up
    // and that Stop is always greater than start
    if(rtStart < m_LastOutputTime && m_LastOutputTime != 0)
    {
        rtStart = m_LastOutputTime;
    }

    if(rtStop <= rtStart)
    {
        rtStop = rtStart + 100;
    }

    LOG(DBGLOG_FLOW, ("Picture Time %010I64d  - %010I64d  - %010I64d  - %010I64d\n", rtStart, rtStop, rtStop - rtStart, rtStop - m_LastOutputTime));

    m_LastOutputTime = rtStop;

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

            Props.dwTypeSpecificFlags = 0;

            // tell the next filter that this is film
            if(m_NextFrameDeint == DIWeave)
            {
                Props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_WEAVE;
            }
            else
            {
                if(NextFrame.top_field_first)
                {
                    Props.dwTypeSpecificFlags |= AM_VIDEO_FLAG_FIELD1FIRST;
                }
            }

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

        if(FAILED(hr = pOut->GetPointer(&pDataOut)))
        {
            LogBadHRESULT(hr, __FILE__, __LINE__);
            return hr;
        }

        // wrap up the input and output buffers
        CVideoData Input(MEDIASUBTYPE_YV12, (BYTE**)&(NextFrame.data[0]), m_DivxWidth, m_DivxHeight, NextFrame.linesize[0]);
        CVideoData Output(m_VideoOutPin->GetMediaType(), pOut);

        // copy the data to whatever format we've negotiated
        CVideoData::Copy(Input, Output, true);

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

    avcodec_flush_buffers(m_CodecContext);
}

HRESULT CDivxDecoder::ResetDivxDecoder()
{
    LOG(DBGLOG_FLOW, ("ResetDivxDecoder()\n"));

    CProtectCode WhileVarInScope(&m_DeliverLock);

    if(m_Codec != NULL)
    {
        avcodec_flush_buffers(m_CodecContext);
        ResetBuffers();
        avcodec_close(m_CodecContext);
        m_Codec = NULL;
    }

    if(m_CodecContext != NULL)
    {
        m_CodecContext = NULL;
    }

    m_CodecContext = avcodec_alloc_context();

    m_CodecContext->opaque = this;
    m_OldGetBuffer = m_CodecContext->get_buffer;
    m_CodecContext->get_buffer = GetBuffer;
    m_OldReleaseBuffer = m_CodecContext->release_buffer;
    m_CodecContext->release_buffer = ReleaseBuffer;
    m_OldRegetBuffer = m_CodecContext->reget_buffer;
    m_CodecContext->reget_buffer = RegetBuffer;

    m_CodecContext->width = m_DivxWidth;
    m_CodecContext->height = m_DivxHeight;
    m_CodecContext->idct_algo = FF_IDCT_AUTO;
    m_CodecContext->codec_tag = m_FourCC;
    m_CodecContext->error_concealment = 0;
    m_CodecContext->workaround_bugs = FF_BUG_AUTODETECT;
    m_CodecContext->error_recognition = FF_ER_CAREFUL;
#ifdef DEBUG
    m_CodecContext->debug = FF_DEBUG_STARTCODE | FF_DEBUG_BITSTREAM;
#endif

    if(m_CodecID == CODEC_ID_H264 || m_CodecID == CODEC_ID_VC1)
    {
        m_CodecContext->skip_loop_filter = AVDISCARD_ALL;
        m_CodecContext->flags2 = CODEC_FLAG2_FAST;
    }

    if(m_ExtraSize > 0 && !m_ExtraData.empty())
    {
        m_CodecContext->extradata = (uint8_t*) &m_ExtraData[0];
        m_CodecContext->extradata_size = m_ExtraSize;
    }

    m_Codec = avcodec_find_decoder(m_CodecID);
    if(m_Codec == 0)
    {
        av_free(m_CodecContext);
        m_CodecContext = NULL;
        return E_UNEXPECTED;
    }

    if (avcodec_open(m_CodecContext, m_Codec) < 0)
    {
        av_free(m_CodecContext);
        m_CodecContext = NULL;
        m_Codec = NULL;
        return E_UNEXPECTED;
    }

    m_fWaitForKeyFrame = true;
    return S_OK;
}


HRESULT CDivxDecoder::Activate()
{
    BYTE* pSequenceHeader = NULL;
    DWORD cbSequenceHeader = 0;

    HRESULT hr = ResetDivxDecoder();
    CHECK(hr);

    m_IsDiscontinuity = true;
    m_LastOutputTime = 0;

    return hr;
}

HRESULT CDivxDecoder::Deactivate()
{
    CProtectCode WhileVarInScope2(&m_DeliverLock);

    if(m_Codec != NULL)
    {
        avcodec_flush_buffers(m_CodecContext);
        ResetBuffers();
        avcodec_close(m_CodecContext);
        m_Codec = NULL;
    }
    if(m_CodecContext != NULL)
    {
        av_free(m_CodecContext);
        m_CodecContext = NULL;
    }
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
    size_t i(0);
    for(; i < m_Buffers.size(); ++i)
    {
        if(m_Buffers[i]->NotInUse())
        {
            m_Buffers[i]->AddRef();
            return m_Buffers[i];
        }
    }
    m_Buffers.push_back(new CFrameBuffer);
    LOG(DBGLOG_FLOW, ("Increasing internal buffer size %d \n", m_Buffers.size()));
    return m_Buffers[i];
}

void CDivxDecoder::ResetBuffers()
{
    for(size_t i(0); i < m_Buffers.size(); ++i)
    {
        m_Buffers[i]->Clear();
    }
}

void CDivxDecoder::avlog(void*,int,const char* szFormat, va_list Args)
{
    char szMessage[2048];

    int result=_vsnprintf(szMessage,2048, szFormat, Args);
    if(result==-1)
    {
        OutputDebugStringA("DebugString too long, truncated!!\n");
    }
    else
    {
        OutputDebugStringA(szMessage);
    }
}

SCodecList* CDivxDecoder::getCodecList()
{
    static SCodecList CodecList[] =
    {
        { MAKEFOURCC('D', 'I', 'V', 'X'), CODEC_ID_MPEG4 },
        { MAKEFOURCC('X', 'V', 'I', 'D'), CODEC_ID_MPEG4 },
        { MAKEFOURCC('D', 'X', '5', '0'), CODEC_ID_MPEG4 },
        { MAKEFOURCC('M', 'P', '4', 'V') ,CODEC_ID_MPEG4 },
        { MAKEFOURCC('D', 'I', 'V', '3'), CODEC_ID_MSMPEG4V3 },
        { MAKEFOURCC('M', 'P', '4', '3'), CODEC_ID_MSMPEG4V3 },
        { MAKEFOURCC('M', 'P', '4', '2'), CODEC_ID_MSMPEG4V2 },
        { MAKEFOURCC('M', 'P', '4', '1'), CODEC_ID_MSMPEG4V1 },
        { MAKEFOURCC('A', 'V', 'C', '1'), CODEC_ID_H264 },
        { MAKEFOURCC('H', '2', '6', '4'), CODEC_ID_H264 },
        { MAKEFOURCC('X', '2', '6', '4'), CODEC_ID_H264 },
        { MAKEFOURCC('V', 'S', 'S', 'H'), CODEC_ID_H264 },
        { MAKEFOURCC('D', 'A', 'V', 'C'), CODEC_ID_H264 },
        { MAKEFOURCC('P', 'A', 'V', 'C'), CODEC_ID_H264 },
        { MAKEFOURCC('P', 'A', 'V', 'C'), CODEC_ID_H264 },
        { MAKEFOURCC('W', 'V', 'C', '1'), CODEC_ID_VC1 },
        { 0, CODEC_ID_NONE },
    };
    return CodecList;
}

namespace
{
    char upper(char in)
    {
        if(in >= 'a' && in <= 'z')
        {
            in += 'A' - 'a';
        }
        return in;
    }

    char lower(char in)
    {
        if(in >= 'A' && in <= 'Z')
        {
            in += 'a' - 'A';
        }
        return in;
    }
}

unsigned long CDivxDecoder::UpperFourCC(unsigned long inFourCC)
{
    char* asChars = (char*)&inFourCC;
    asChars[0] = upper(asChars[0]);
    asChars[1] = upper(asChars[1]);
    asChars[2] = upper(asChars[2]);
    asChars[3] = upper(asChars[3]);
    return inFourCC;
}

unsigned long CDivxDecoder::LowerFourCC(unsigned long inFourCC)
{
    char* asChars = (char*)&inFourCC;
    asChars[0] = lower(asChars[0]);
    asChars[1] = lower(asChars[1]);
    asChars[2] = lower(asChars[2]);
    asChars[3] = lower(asChars[3]);
    return inFourCC;
}

CodecID CDivxDecoder::lookupCodec(unsigned long inFourCC)
{
    SCodecList* CodecList = getCodecList();
    while(CodecList->FourCC)
    {
        if(CodecList->FourCC == inFourCC)
        {
            return CodecList->FFMpegCodecId;
        }
        ++CodecList;
    }
    return CODEC_ID_NONE;
}

int __cdecl CDivxDecoder::GetBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    CDivxDecoder* Decoder = (CDivxDecoder*)c->opaque;
    return Decoder->InternalGetBuffer(c, pic);
}

void __cdecl CDivxDecoder::ReleaseBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    CDivxDecoder* Decoder = (CDivxDecoder*)c->opaque;
    Decoder->InternalReleaseBuffer(c, pic);
}

int __cdecl CDivxDecoder::RegetBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    CDivxDecoder* Decoder = (CDivxDecoder*)c->opaque;
    return Decoder->InternalRegetBuffer(c, pic);
}

int CDivxDecoder::InternalGetBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    int RetVal = m_OldGetBuffer(c, pic);
    CFrameBuffer* FrameBuffer = GetNextBuffer();
    LOG(DBGLOG_ALL, ("Get ref - %d type - %d age - %d cpn - %d\n", pic->reference, pic->type,  pic->age, pic->coded_picture_number));
    if(pic->reference)
    {
        FrameBuffer->m_rtStartCoded = m_LastInputTime;
    }
    else
    {
        FrameBuffer->m_rtStartCoded = m_LastInputTime;
    }
    pic->opaque = FrameBuffer;
    return RetVal;
}

void CDivxDecoder::InternalReleaseBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    CFrameBuffer* FrameBuffer = (CFrameBuffer*)pic->opaque;
    FrameBuffer->Release();
    m_OldReleaseBuffer(c, pic);
}

int CDivxDecoder::InternalRegetBuffer(struct AVCodecContext *c, AVFrame *pic)
{
    int RetVal = m_OldRegetBuffer(c, pic);
    CFrameBuffer* FrameBuffer = (CFrameBuffer*)pic->opaque;
    FrameBuffer->m_rtStartCoded = m_LastInputTime;
    return RetVal;
}
