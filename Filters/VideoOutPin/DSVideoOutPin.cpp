///////////////////////////////////////////////////////////////////////////////
// $Id: DSVideoOutPin.cpp,v 1.7 2007-12-03 07:54:26 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.6  2007/10/20 14:55:49  adcockj
// Better EVR Connections
//
// Revision 1.5  2007/10/19 17:05:49  adcockj
// Added Support for setting new color info flags
//
// Revision 1.4  2007/06/25 17:05:35  adcockj
// Fixed connection issue
//
// Revision 1.3  2005/10/07 15:32:48  adcockj
// basic NV12 support
//
// Revision 1.2  2005/10/05 14:30:42  adcockj
// implemented NV12 just to see if we get connection, will produce corrupted output at present
//
// Revision 1.1  2005/04/14 11:21:07  adcockj
// First stage of code reorganisation
//
// Revision 1.13  2005/03/20 14:18:14  adcockj
// aspect fixes and new deinterlacing test parameter
//
// Revision 1.12  2005/03/10 09:03:58  adcockj
// test for for PAL interlacing
//
// Revision 1.11  2005/02/03 15:34:40  adcockj
// better vmr connections
//
// Revision 1.10  2005/01/04 17:53:44  adcockj
// added option to force dscalewr filter to be loaded2
//
// Revision 1.9  2004/12/06 18:05:00  adcockj
// Major improvements to deinterlacing
//
// Revision 1.8  2004/11/26 15:03:52  adcockj
// fixed reconnection issue with old video renderer
//
// Revision 1.7  2004/11/25 17:22:10  adcockj
// Fixed some more connection issues
//
// Revision 1.6  2004/11/18 07:40:57  adcockj
// a few test bug fixes
//
// Revision 1.5  2004/11/06 14:07:01  adcockj
// Fixes for WM10 and seeking
//
// Revision 1.4  2004/11/02 16:57:24  adcockj
// fix for crashing problem on exit
//
// Revision 1.3  2004/11/01 14:09:39  adcockj
// More DScaler filter insipred changes
//
// Revision 1.2  2004/10/31 14:20:39  adcockj
// fixed issues with settings dialog
//
// Revision 1.1  2004/10/28 16:00:48  adcockj
// added new files
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSVideoOutPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "DSUtil.h"
#include "PlanarYUVToRGB.h"
#include "PlanarYUVToYUY2.h"
#include "MediaTypes.h"
#include <vmr9.h>
#include <Mpconfig.h>
#include "evcode.h"
#include "MoreUuids.h"

CDSVideoOutPin::CDSVideoOutPin() : CDSOutputPin()
{
    LOG(DBGLOG_ALL, ("CDSVideoOutPin::CDSVideoOutPin\n"));
    m_DoPanAndScan = false;
    m_PanScanOffsetX = 0;
    m_PanScanOffsetY = 0;
    m_AspectX = 4;
    m_AspectY = 3;
    m_Width = 720;
    m_Height = 480;
    m_ConnectedType = DEFAULT_OUTFILTER;
    m_NeedToAttachFormat = false;
    m_CurrentWidth = 0;
    m_CurrentHeight = 0;
    m_CurrentAspectX = 0;
    m_CurrentAspectY = 0;
    InitMediaType(&m_InternalMT);
    m_InsideReconnect = false;
    m_AvgTimePerFrame = 333333;
}

CDSVideoOutPin::~CDSVideoOutPin()
{
    LOG(DBGLOG_ALL, ("CDSVideoOutPin::~CDSVideoOutPin\n"));
    ClearMediaType(&m_InternalMT);
}

HRESULT CDSVideoOutPin::NotifyConnected()
{
    CLSID Clsid;

    HRESULT hr = GetConnectedFilterCLSID(&Clsid);
    SI(IVMRVideoStreamControl9) VMR9Test = m_ConnectedPin;
    SI(IVMRVideoStreamControl) VMR7Test = m_ConnectedPin;
    if(VMR9Test)
    {
        LOG(DBGLOG_FLOW, ("Connected to VMR9\n"));
        m_ConnectedType = VMR9_OUTFILTER;
        OnConnectToVMR9();
    }
    else if(VMR7Test ||
            Clsid == CLSID_VideoMixingRenderer ||
            Clsid == CLSID_VideoRendererDefault)
    {
        LOG(DBGLOG_FLOW, ("Connected to VMR7\n"));
        m_ConnectedType = VMR7_OUTFILTER;
        OnConnectToVMR7();
    }
    else if(Clsid == CLSID_FFDShow ||
            Clsid == CLSID_FFDShowRaw)
    {
        LOG(DBGLOG_FLOW, ("Connected to ffdshow\n"));
        m_ConnectedType = FFDSHOW_OUTFILTER;
    }
    else if(Clsid == CLSID_DirectVobSubFilter ||
        Clsid == CLSID_DirectVobSubFilter2)
    {
        LOG(DBGLOG_FLOW, ("Connected to Vobsub\n"));
        m_ConnectedType = GABEST_OUTFILTER;
    }
    else if(Clsid == CLSID_OverlayMixer)
    {
        LOG(DBGLOG_FLOW, ("Connected to Overlay\n"));
        OnConnectToOverlay();
        m_ConnectedType = OVERLAY_OUTFILTER;
    }
    else if(Clsid == CLSID_WM10RENDERER)
    {
        LOG(DBGLOG_FLOW, ("Connected to WM10\n"));
        m_ConnectedType = WM10_OUTFILTER;
    }
    else if(Clsid == CLSID_CDScaler)
    {
        LOG(DBGLOG_FLOW, ("Connected to Dscaler\n"));
        m_ConnectedType = DSCALER_OUTFILTER;
    }
    else
    {
        LOG(DBGLOG_FLOW, ("Unknown Renderer - %s\n", GetGUIDName(Clsid)));
        SI(IPinConnection) PinConnection = m_ConnectedPin;
        if(!PinConnection)
        {
            return VFW_E_NO_TRANSPORT;
        }
        m_ConnectedType = DEFAULT_OUTFILTER;
        return VFW_E_NO_TRANSPORT;
    }
    return hr;
}

void CDSVideoOutPin::OnConnectToVMR7()
{
    // when testing it's a pain to have top set up the VMR to display the aspect ratio
    // properly so do it here but only do so if running in graphedit
    if(IsRunningInGraphEdit())
    {
        SI(IVMRAspectRatioControl) AspectRatioControl = GetConnectedFilter();
        if(AspectRatioControl)
        {
            AspectRatioControl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
        }
    }
}

void CDSVideoOutPin::OnConnectToVMR9()
{
    // when testing it's a pain to have top set up the VMR to display the aspect ratio
    // properly so do it here but only do so if running in graphedit
    if(IsRunningInGraphEdit())
    {
        SI(IVMRAspectRatioControl9) AspectRatioControl = GetConnectedFilter();
        if(AspectRatioControl)
        {
            AspectRatioControl->SetAspectRatioMode(VMR_ARMODE_LETTER_BOX);
        }
    }
}

void CDSVideoOutPin::OnConnectToOverlay()
{
    // there seems to be a bug in the letterbox mode of the
    // overlay renderer which means that our format changes
    // don't work properly - this requires that we assume
    // the player does the right thing with the aspect ratios
    SI(IMixerPinConfig) MixerPinConfig = m_ConnectedPin;
    if(MixerPinConfig)
    {
        HRESULT hr = MixerPinConfig->SetAspectRatioMode(AM_ARMODE_STRETCHED);
    }
}


void CDSVideoOutPin::SetPanScanX(DWORD OffsetX)
{
    m_PanScanOffsetX = OffsetX;

    if(m_PanScanOffsetX != 0 || m_PanScanOffsetY != 0)
    {
        m_DoPanAndScan = true;
    }
    else
    {
        m_DoPanAndScan = false;
    }
}

void CDSVideoOutPin::SetPanScanY(DWORD OffsetY)
{
    m_PanScanOffsetY = OffsetY;

    if(m_PanScanOffsetX != 0 || m_PanScanOffsetY != 0)
    {
        m_DoPanAndScan = true;
    }
    else
    {
        m_DoPanAndScan = false;
    }
}

void CDSVideoOutPin::SetAspectX(DWORD AspectX)
{
    m_AspectX = AspectX;
}

void CDSVideoOutPin::SetAspectY(DWORD AspectY)
{
    m_AspectY = AspectY;
}

DWORD CDSVideoOutPin::GetAspectX()
{
    return m_AspectX;
}

DWORD CDSVideoOutPin::GetAspectY()
{
    return m_AspectY;
}

void CDSVideoOutPin::SetWidth(int Width)
{
    m_Width = Width;
}

int CDSVideoOutPin::GetWidth()
{
    return m_Width;
}

void CDSVideoOutPin::SetHeight(int Height)
{
    m_Height = Height;
}

int CDSVideoOutPin::GetHeight()
{
    return m_Height;
}

HRESULT CDSVideoOutPin::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, int TypeNum, DWORD VideoControlFlags, DWORD ControlFlags)
{
    struct {const GUID* subtype; WORD biPlanes, biBitCount; DWORD biCompression;} fmts[] =
    {
        {&MEDIASUBTYPE_NV12, 1, 12, '21VN'},
        {&MEDIASUBTYPE_YV12, 1, 12, '21VY'},
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

    int VariationsPerType = 3;
    int InterlacedIndex = 0;
    int VIHIndex = 2;

    if(VideoControlFlags & VIDEOTYPEFLAG_PROGRESSIVE)
    {
        VariationsPerType--;
        InterlacedIndex = -1;
        VIHIndex--;
    }

    if(VideoControlFlags & VIDEOTYPEFLAG_PREVENT_VIDEOINFOHEADER)
    {
        VariationsPerType--;
        VIHIndex = -1;
    }

    if(VideoControlFlags & VIDEOTYPEFLAG_FORCE_YV12)
    {
        TypeNum += VariationsPerType;
    }

	if(VideoControlFlags & VIDEOTYPEFLAG_FORCE_YUY2)
    {
        TypeNum += 2 * VariationsPerType;
    }
    

    // this will make sure we won't connect to the old renderer in dvd mode
    // that renderer can't switch the format dynamically
    if(TypeNum < 0) return E_INVALIDARG;
    if(VideoControlFlags & VIDEOTYPEFLAG_PREVENT_VIDEOINFOHEADER)
    {
        // there should be no reason why 
        // anything that doesn't support YUY2 or YV12 need the
        // stupid old RGB types
        if(TypeNum >= VariationsPerType * 3) 
            return VFW_S_NO_MORE_ITEMS;
    }
    else
    {
        if(TypeNum >= (int)(VariationsPerType * countof(fmts))) 
            return VFW_S_NO_MORE_ITEMS;
    }

    int FormatNum = TypeNum/VariationsPerType;

    if(VideoControlFlags & VIDEOTYPEFLAG_FORCE_DSCALER)
    {
        pmt->majortype = CLSID_CDScaler;
    }
    else
    {
        pmt->majortype = MEDIATYPE_Video;
    }
    pmt->subtype = *fmts[FormatNum].subtype;

    BITMAPINFOHEADER bihOut;
    memset(&bihOut, 0, sizeof(bihOut));
    bihOut.biSize = sizeof(bihOut);
    bihOut.biWidth = m_Width;
    bihOut.biHeight = m_Height;
    bihOut.biPlanes = fmts[FormatNum].biPlanes;
    bihOut.biBitCount = fmts[FormatNum].biBitCount;
    bihOut.biCompression = fmts[FormatNum].biCompression;
    bihOut.biSizeImage = bihOut.biWidth * bihOut.biHeight * bihOut.biBitCount>>3;

    if(TypeNum%VariationsPerType == VIHIndex)
    {
        pmt->formattype = FORMAT_VideoInfo;
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER));
        memset(vih, 0, sizeof(VIDEOINFOHEADER));
        vih->bmiHeader = bihOut;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        pmt->pbFormat = (BYTE*)vih;
        pmt->cbFormat = sizeof(VIDEOINFOHEADER);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
        bihOut.biXPelsPerMeter = bihOut.biWidth * m_AspectY;
        bihOut.biYPelsPerMeter = bihOut.biHeight * m_AspectX;
        Simplify(bihOut.biXPelsPerMeter, bihOut.biYPelsPerMeter);
    }
    else
    {
        pmt->formattype = FORMAT_VideoInfo2;
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
        memset(vih, 0, sizeof(VIDEOINFOHEADER2));
        vih->bmiHeader = bihOut;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        if(TypeNum%3 == InterlacedIndex)
        {
            vih->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOrWeave | AMINTERLACE_FieldPatBothRegular;
            if(VideoControlFlags & VIDEOTYPEFLAG_SET_FIELD1FIRST)
            {
                vih->dwInterlaceFlags |= AMINTERLACE_Field1First;
            }
        }
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwControlFlags = ControlFlags;
        if(ControlFlags & 0xfff0)
        {
            // set AMCONTROL_COLORINFO_PRESENT
            vih->dwControlFlags |= 8;
        }
        pmt->pbFormat = (BYTE*)vih;
        pmt->cbFormat = sizeof(VIDEOINFOHEADER2);       
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
    }
    pmt->lSampleSize = bihOut.biSizeImage;
    CorrectMediaType(pmt);
    return S_OK;
}


DWORD CDSVideoOutPin::GetPanScanX()
{
    return m_PanScanOffsetX;
}

DWORD CDSVideoOutPin::GetPanScanY()
{
    return m_PanScanOffsetY;
}


void CDSVideoOutPin::Copy420(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn, bool ProgressiveChroma)
{
    const BITMAPINFOHEADER* bihOut = ExtractBIH(GetMediaType());

    BYTE* pIn = ppIn[0];
    BYTE* pInU = ppIn[1];
    BYTE* pInV = ppIn[2];

    w = (w+7)&~7;
    if(pitchIn < w)
    {
        LOG(DBGLOG_FLOW, ("Got picture before new sequence\n"));
        return;
    }


    if(m_DoPanAndScan)
    {
        pIn += m_PanScanOffsetX + pitchIn*m_PanScanOffsetY;
        pInU += m_PanScanOffsetX / 2 + pitchIn*m_PanScanOffsetY/4;
        pInV += m_PanScanOffsetX / 2 + pitchIn*m_PanScanOffsetY/4;
    }


    if(bihOut->biCompression == '2YUY')
    {
        if(ProgressiveChroma)
        {
            BitBltFromI420ToYUY2(w, h, pOut, bihOut->biWidth*2, pIn, pInU, pInV, pitchIn);
        }
        else
        {
            BitBltFromI420ToYUY2_Int(w, h, pOut, bihOut->biWidth*2, pIn, pInU, pInV, pitchIn);
        }
    }
    else if(bihOut->biCompression == '21VY')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;
        BYTE* pOutU = pOutV + abs(bihOut->biHeight) * bihOut->biWidth / 4;

        BitBltFromI420ToI420(w, h, pOut, pOutU, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == '21VN')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;

        BitBltFromI420ToNV12(w, h, pOut, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == BI_RGB || bihOut->biCompression == BI_BITFIELDS)
    {
        int pitchOut = bihOut->biWidth*bihOut->biBitCount>>3;

        if(bihOut->biHeight > 0)
        {
            pOut += pitchOut*(h-1);
            pitchOut = -pitchOut;
        }

        if(!BitBltFromI420ToRGB(w, h, pOut, pitchOut, bihOut->biBitCount, pIn, pInU, pInV, pitchIn))
        {
            for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
                memset(pOut, 0, pitchOut);
        }
    }
}

void CDSVideoOutPin::Copy422(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn)
{
    const BITMAPINFOHEADER* bihOut = ExtractBIH(GetMediaType());

    BYTE* pIn = ppIn[0];
    BYTE* pInU = ppIn[1];
    BYTE* pInV = ppIn[2];

    w = (w+7)&~7;
    if(pitchIn < w)
    {
        LOG(DBGLOG_FLOW, ("Got picture before new sequence\n"));
        return;
    }

    if(bihOut->biCompression == '2YUY')
    {
        BitBltFromI422ToYUY2(w, h, pOut, bihOut->biWidth*2, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == '21VY')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;
        BYTE* pOutU = pOutV + abs(bihOut->biHeight) * bihOut->biWidth / 4;

        BitBltFromI422ToI420(w, h, pOut, pOutU, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == '21VN')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;
        BYTE* pOutU = pOutV + abs(bihOut->biHeight) * bihOut->biWidth / 4;

        BitBltFromI422ToI420(w, h, pOut, pOutU, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == BI_RGB || bihOut->biCompression == BI_BITFIELDS)
    {
        int pitchOut = bihOut->biWidth*bihOut->biBitCount>>3;

        if(bihOut->biHeight > 0)
        {
            pOut += pitchOut*(h-1);
            pitchOut = -pitchOut;
        }

        if(!BitBltFromI422ToRGB(w, h, pOut, pitchOut, bihOut->biBitCount, pIn, pInU, pInV, pitchIn))
        {
            for(DWORD y = 0; y < h; y++, pIn += pitchIn, pOut += pitchOut)
                memset(pOut, 0, pitchOut);
        }
    }
}

void CDSVideoOutPin::Copy444(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn)
{
    const BITMAPINFOHEADER* bihOut = ExtractBIH(GetMediaType());

    BYTE* pIn = ppIn[0];
    BYTE* pInU = ppIn[1];
    BYTE* pInV = ppIn[2];

    w = (w+7)&~7;
    if(pitchIn < w)
    {
        LOG(DBGLOG_FLOW, ("Got picture before new sequence\n"));
        return;
    }

    if(bihOut->biCompression == '2YUY')
    {
        BitBltFromI444ToYUY2(w, h, pOut, bihOut->biWidth*2, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == '21VY')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;
        BYTE* pOutU = pOutV + abs(bihOut->biHeight) * bihOut->biWidth / 4;

        BitBltFromI444ToI420(w, h, pOut, pOutU, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == '21VN')
    {
        BYTE* pOutV = pOut + abs(bihOut->biHeight) * bihOut->biWidth;
        BYTE* pOutU = pOutV + abs(bihOut->biHeight) * bihOut->biWidth / 4;

        BitBltFromI444ToI420(w, h, pOut, pOutU, pOutV, bihOut->biWidth, pIn, pInU, pInV, pitchIn);
    }
    else if(bihOut->biCompression == BI_RGB || bihOut->biCompression == BI_BITFIELDS)
    {
        int pitchOut = bihOut->biWidth*bihOut->biBitCount>>3;

        if(bihOut->biHeight > 0)
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

HRESULT CDSVideoOutPin::CheckForReconnection()
{
    HRESULT hr = S_OK;

    m_NeedToAttachFormat = false; 

    if((m_Width != m_CurrentWidth) || 
        (m_Height != m_CurrentHeight) || 
        (m_AspectX != m_CurrentAspectX) || 
        (m_AspectY != m_CurrentAspectY))
    {
        m_InsideReconnect = true;
        m_PitchWidth = 0;
        m_PitchHeight = 0;

        switch(m_ConnectedType)
        {
        case VMR7_OUTFILTER:
        case VMR9_OUTFILTER:
            hr = ReconnectVMR();
            break;
        case GABEST_OUTFILTER:
        case OVERLAY_OUTFILTER:
        case FFDSHOW_OUTFILTER:
        case DSCALER_OUTFILTER:
            hr = ReconnectOverlay();
            break;
        case WM10_OUTFILTER:
            hr = ReconnectWM10();
            break;
        case DEFAULT_OUTFILTER:
            hr = ReconnectOther();
            break;
        }

        m_InsideReconnect = false;
    

        m_CurrentWidth = m_Width;
        m_CurrentHeight = m_Height;
        m_CurrentAspectX = m_AspectX; 
        m_CurrentAspectY = m_AspectY; 
    }
    return hr;
}

HRESULT CDSVideoOutPin::ReconnectVMR()
{
    HRESULT hr = S_OK;
    CopyMediaType(&m_InternalMT, GetMediaType());

    BITMAPINFOHEADER* bmi = NULL;

    if(m_InternalMT.formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
    }
    else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
    }

    bmi->biXPelsPerMeter = m_Width * m_AspectY;
    bmi->biYPelsPerMeter = m_Height * m_AspectX;
    Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);

    bool NeedReconnect = false;

    if(m_Width != bmi->biWidth)
    {
        bmi->biWidth = m_Width;
        NeedReconnect = true;
    }
    if(bmi->biHeight < 0)
    {
        if(m_Height > -bmi->biHeight)
        {
            bmi->biHeight = -m_Height;
            NeedReconnect = true;
        }
    }
    else
    {
        if(m_Height > bmi->biHeight)
        {
            bmi->biHeight = m_Height;
            NeedReconnect = true;
        }
    }

    bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;
    m_InternalMT.bFixedSizeSamples = 0;
    m_InternalMT.lSampleSize = bmi->biSizeImage;
    LogMediaType(&m_InternalMT, "VMR7 Type", DBGLOG_FLOW);

    SI(IPinConnection) m_PinConnection = m_ConnectedPin;
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
        hr = m_ConnectedPin->QueryAccept(&m_InternalMT);
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    if(NeedReconnect)
    {
        hr = m_ConnectedPin->BeginFlush();
        LOG(DBGLOG_FLOW, ("BeginFlush %08x\n", hr));
        CHECK(hr);
        
        hr = m_ConnectedPin->EndFlush();
        LOG(DBGLOG_FLOW, ("EndFlush %08x\n", hr));
        CHECK(hr);


        hr = m_ConnectedPin->ReceiveConnection(this, &m_InternalMT);
        LOG(DBGLOG_FLOW, ("ReceiveConnection %08x\n", hr));

        hr = m_MemInputPin->NotifyAllocator(m_Allocator.GetNonAddRefedInterface(), FALSE);
        CHECK(hr);

        ALLOCATOR_PROPERTIES AllocatorProps;
        hr = m_Allocator->GetProperties(&AllocatorProps);
        LOG(DBGLOG_FLOW, ("GetProperties %08x\n", hr));
        CHECK(hr);
        if(m_PitchWidth != 0 && (bmi->biWidth != m_PitchWidth || bmi->biHeight != m_PitchHeight))
		{
			bmi->biWidth = max(m_PitchWidth, bmi->biWidth);
			if(m_PitchHeight > 0)
			{
	            bmi->biHeight = max(m_PitchHeight, abs(bmi->biHeight));
			}
			else
			{
	            bmi->biHeight = min(m_PitchHeight, -abs(bmi->biHeight));
			}
            bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;
            m_InternalMT.lSampleSize = bmi->biSizeImage;
            m_NeedToAttachFormat = true;
        }
        // if the new type would be greater than the old one then
        // we need to reconnect otherwise just attach the type to the next sample
        if(bmi->biSizeImage > (DWORD)AllocatorProps.cbBuffer)
        {   
            hr = m_Allocator->Decommit();

            ALLOCATOR_PROPERTIES PropsAct;
            AllocatorProps.cbBuffer = bmi->biSizeImage;
            hr = m_Allocator->SetProperties(&AllocatorProps, &PropsAct);
            CHECK(hr);     
            LOG(DBGLOG_FLOW, ("Allocator Negotiated Buffers - %d Size - %d Align - %d Prefix %d\n", PropsAct.cBuffers, PropsAct.cbBuffer, PropsAct.cbAlign, PropsAct.cbPrefix));

            LOG(DBGLOG_FLOW, ("Decommit %08x\n", hr));
            hr = m_Allocator->Commit();
        }
    }
    else
    {
        m_NeedToAttachFormat = true; 
    }

    return hr;
}

HRESULT CDSVideoOutPin::ReconnectOverlay()
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, GetMediaType());

    BITMAPINFOHEADER* bmi = NULL;

    if(m_InternalMT.formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        //SetRect(&vih->rcTarget, 0, 0, 0, 0);
    }
    else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        //SetRect(&vih->rcTarget, 0, 0, 0, 0);
    }

    bmi->biXPelsPerMeter = m_Width * m_AspectY;
    bmi->biYPelsPerMeter = m_Height * m_AspectX;
    Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);

    if((m_Width != m_CurrentWidth) || 
        (m_Height != m_CurrentHeight))
    {

        bmi->biWidth = m_Width;

        if(bmi->biHeight < 0)
        {
            bmi->biHeight = -m_Height;
        }
        else
        {
            bmi->biHeight = m_Height;
        }
        bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;

        m_InternalMT.bFixedSizeSamples = 1;
        m_InternalMT.lSampleSize = bmi->biSizeImage;


        SI(IPinConnection) m_PinConnection = m_ConnectedPin;
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
            hr = m_ConnectedPin->QueryAccept(&m_InternalMT);
            if(hr != S_OK)
            {
                LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }

        // if the new type would be greater than the old one then
        // we need to reconnect otherwise just attach the type to the next sample
        hr = m_ConnectedPin->BeginFlush();
        CHECK(hr);
        hr = m_ConnectedPin->EndFlush();
        CHECK(hr);
        hr = m_Allocator->Decommit();
        CHECK(hr);

        hr = NegotiateAllocator(NULL, &m_InternalMT);
        CHECK(hr);

        hr = m_ConnectedPin->ReceiveConnection(this, &m_InternalMT);
        CHECK(hr);

        hr = m_Allocator->Commit();
        CHECK(hr);

        if(m_PitchWidth != 0)
        {
            bmi->biWidth = m_PitchWidth;
            bmi->biHeight = m_PitchHeight;
            bmi->biSizeImage = m_Height*bmi->biWidth*bmi->biBitCount>>3;
            m_InternalMT.lSampleSize = bmi->biSizeImage;

            m_NeedToAttachFormat = true; 
        }

    }
    else
    {
        // only an aspect ratio change so just use the format
        m_NeedToAttachFormat = true; 
    }

    return hr;
}

HRESULT CDSVideoOutPin::ReconnectOther()
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, GetMediaType());

    BITMAPINFOHEADER* bmi = NULL;

    if(m_InternalMT.formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
    }
    else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
    }

    bmi->biXPelsPerMeter = m_Width * m_AspectY;
    bmi->biYPelsPerMeter = m_Height * m_AspectX;
    Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);

    bmi->biWidth = max(m_Width, bmi->biWidth);
    if(bmi->biHeight < 0)
    {
        bmi->biHeight = min(-m_Height, bmi->biHeight);
    }
    else
    {
        bmi->biHeight = max(m_Height, bmi->biHeight);
    }

    bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;

    m_InternalMT.bFixedSizeSamples = 1;
    m_InternalMT.lSampleSize = bmi->biSizeImage;


    SI(IPinConnection) m_PinConnection = m_ConnectedPin;
    if(m_PinConnection)
    {
        hr = m_PinConnection->DynamicQueryAccept(&m_InternalMT);
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
			return S_OK;
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else
    {
        hr = m_ConnectedPin->QueryAccept(&m_InternalMT);
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
            return S_OK;
			return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    if(!m_Allocator) return E_NOINTERFACE;

    SI(IGraphConfig) GraphConfig = m_Filter->m_Graph;

    if(GraphConfig)
    {
        //hr = GraphConfig->Reconnect(this, m_ConnectedPin.GetNonAddRefedInterface(), &m_InternalMT, NULL, NULL, AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT);
        return S_OK;
		CHECK(hr);
    }

    return S_OK;
	return hr;
}

HRESULT CDSVideoOutPin::ReconnectWM10()
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, GetMediaType());

    BITMAPINFOHEADER* bmi = NULL;

    if(m_InternalMT.formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
    }
    else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
    }

    bmi->biXPelsPerMeter = m_Width * m_AspectY;
    bmi->biYPelsPerMeter = m_Height * m_AspectX;
    Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);

    bmi->biWidth = max(m_Width, bmi->biWidth);
    if(bmi->biHeight < 0)
    {
        bmi->biHeight = min(-m_Height, bmi->biHeight);
    }
    else
    {
        bmi->biHeight = max(m_Height, bmi->biHeight);
    }

    bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;

    m_InternalMT.bFixedSizeSamples = 1;
    m_InternalMT.lSampleSize = bmi->biSizeImage;

    m_NeedToAttachFormat = true; 
    return S_OK;
}


HRESULT CDSVideoOutPin::GetOutputSample(IMediaSample** OutSample, REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop, bool PrevFrameSkipped)
{
    HRESULT hr = CDSOutputPin::GetOutputSample(OutSample, rtStart, rtStop, PrevFrameSkipped);
    // cope with dynamic format changes from the renderer
    // we care about this when we think we need to change the format
    // and the video renderer sends up a new format too
    // calling AdjustRenderersMediaType again will merge the two 
    // formats properly and should never call NegotiateAllocator
    if(hr == S_FALSE)
    {
        HRESULT hr;
        if(m_NeedToAttachFormat)
        {
            if(FAILED(hr = AdjustRenderersMediaType()))
            {
                LogBadHRESULT(hr, __FILE__, __LINE__);
                return hr;
            }
        }

        hr = (*OutSample)->SetDiscontinuity(TRUE);
        CHECK(hr);
    }
    return hr;
}

void CDSVideoOutPin::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType)
{
    int wout = 0, hout = 0;
    long arxout = 0, aryout = 0; 
    ExtractDim(pMediaType, wout, hout, arxout, aryout); 
    if(wout == m_Width && abs(hout) == m_Height &&
        arxout == m_AspectX &&
        aryout == m_AspectY)
    {
        m_CurrentWidth = wout; 
        m_CurrentHeight = abs(hout); 
        m_CurrentAspectX = arxout; 
        m_CurrentAspectY = aryout; 
    }
}


HRESULT CDSVideoOutPin::SendSample(IMediaSample* OutSample)
{
    if(m_NeedToAttachFormat)
    {
        OutSample->SetMediaType(&m_InternalMT);
        LogMediaType(&m_InternalMT, "AttachFormat", DBGLOG_FLOW);
        m_NeedToAttachFormat = false;

        SI(IMediaEventSink) pMES = m_Filter->m_Graph;
        if(pMES)
        {
            // some renderers don't send this
            pMES->Notify(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(m_Width, m_Height), 0);
        }
    }
    return CDSOutputPin::SendSample(OutSample);
}

HRESULT CDSVideoOutPin::AdjustRenderersMediaType()
{
    HRESULT hr = S_OK;

    m_NeedToAttachFormat = true; 

    CopyMediaType(&m_InternalMT, GetMediaType());

    BITMAPINFOHEADER* bmi = NULL;

    if(m_InternalMT.formattype == FORMAT_VideoInfo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
   }
    else if(m_InternalMT.formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)m_InternalMT.pbFormat;
        bmi = &vih->bmiHeader;
        vih->AvgTimePerFrame = m_AvgTimePerFrame;
        vih->dwPictAspectRatioX = m_AspectX;
        vih->dwPictAspectRatioY = m_AspectY;
        Simplify(vih->dwPictAspectRatioX, vih->dwPictAspectRatioY);
        SetRect(&vih->rcSource, 0, 0, m_Width, m_Height);
        SetRect(&vih->rcTarget, 0, 0, 0, 0);
   }

    bmi->biXPelsPerMeter = m_Width * m_AspectY;
    bmi->biYPelsPerMeter = m_Height * m_AspectX;
    Simplify(bmi->biXPelsPerMeter, bmi->biYPelsPerMeter);

    bmi->biWidth = max(m_Width, bmi->biWidth);
    if(bmi->biHeight < 0)
    {
        bmi->biHeight = min(-m_Height, bmi->biHeight);
    }
    else
    {
        bmi->biHeight = max(m_Height, bmi->biHeight);
    }

    bmi->biSizeImage = abs(bmi->biHeight)*bmi->biWidth*bmi->biBitCount>>3;

    m_InternalMT.bFixedSizeSamples = 1;
    m_InternalMT.lSampleSize = bmi->biSizeImage;
    SI(IPinConnection) m_PinConnection = m_ConnectedPin;

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
        hr = m_ConnectedPin->QueryAccept(&m_InternalMT);
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("QueryAccept failed in ReconnectOutput %08x\n", hr));
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    return S_OK;
}

STDMETHODIMP CDSVideoOutPin::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::QueryAccept\n"));
    LogMediaType(pmt, "CDSOutputPin::QueryAccept", DBGLOG_FLOW);

    if(m_Filter->IsThisATypeWeCanWorkWith(pmt, this) == false)
    {
        return S_FALSE;
    }
    int wout = 0, hout = 0;
    long arxout = 0, aryout = 0;
    ExtractDim(pmt, wout, hout, arxout, aryout);
    if(m_InsideReconnect)
    {
        m_PitchWidth = wout;
        m_PitchHeight = hout;
    }
    else
    {
        if(abs(wout) != abs(m_Width) || abs(hout) != abs(m_Height))
        {
            return S_FALSE;
        }
    }
    return S_OK;
}

void CDSVideoOutPin::SetAvgTimePerFrame(REFERENCE_TIME AvgTimePerFrame)
{
    if(AvgTimePerFrame != 0)
    {
        m_AvgTimePerFrame = AvgTimePerFrame;
    }
    else
    {
        if(m_Height == 576)
        {
            m_AvgTimePerFrame = 400000;
        }
        else
        {
            m_AvgTimePerFrame = 333667;
        }
    }
}

CDSVideoOutPin::OUT_TYPE CDSVideoOutPin::GetConnectedType()
{
    return m_ConnectedType;
}

int CDSVideoOutPin::GetDroppedFrames()
{
    return GetDroppedFrames(m_ConnectedPin.GetNonAddRefedInterface());
}

int CDSVideoOutPin::GetDroppedFrames(IPin* InputPin)
{
    int RetVal = 0;
    PIN_INFO PinInfo;
    HRESULT hr = InputPin->QueryPinInfo(&PinInfo);
    if(hr == S_OK)
    {
        if(PinInfo.pFilter != NULL)
        {
            SI(IQualProp) QualProp = PinInfo.pFilter;
            if(QualProp)
            {
                QualProp->get_FramesDroppedInRenderer(&RetVal);
            }
            else
            {
                SI(IEnumPins) EnumPins;
                hr = PinInfo.pFilter->EnumPins(EnumPins.GetReleasedInterfaceReference());
                if(hr == S_OK)
                {
                    SI(IPin) Pin;
                    hr = EnumPins->Next(1, Pin.GetReleasedInterfaceReference(), NULL);
                    while(hr == S_OK)
                    {
                        PIN_DIRECTION PinDir;
                        hr = Pin->QueryDirection(&PinDir);
                        if(hr == S_OK && PinDir == PINDIR_OUTPUT)
                        {
                            SI(IPin) ConnectedTo;
                            hr = Pin->ConnectedTo(ConnectedTo.GetReleasedInterfaceReference());
                            if(hr == S_OK)
                            {
                                RetVal += GetDroppedFrames(ConnectedTo.GetNonAddRefedInterface());
                            }
                        }
                        // need to do the detach otherwise we
                        // end up releasing a pin twice causing crashes
                        Pin.Detach();
                        hr = EnumPins->Next(1, Pin.GetReleasedInterfaceReference(), NULL);
                    }
                }
            }
            PinInfo.pFilter->Release();
        }
    }
    return RetVal;
}
