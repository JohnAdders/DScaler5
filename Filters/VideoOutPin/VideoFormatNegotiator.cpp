///////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "stdafx.h"
#include "VideoFormatNegotiator.h"
//#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "DSUtil.h"
#include "MediaTypes.h"
//#include <vmr9.h>
//#include <Mpconfig.h>
//#include "evcode.h"
#include "MoreUuids.h"

CVideoFormatNegotiator::CVideoFormatNegotiator()
{
    LOG(DBGLOG_ALL, ("CVideoFormatNegotiator::CVideoFormatNegotiator\n"));
    m_AspectX = 0;
    m_AspectY = 0;
    m_Width = 0;
    m_Height = 0;
    m_AvgTimePerFrame = 333333;
    InitMediaType(&m_InternalMT);
}

CVideoFormatNegotiator::~CVideoFormatNegotiator()
{
    LOG(DBGLOG_ALL, ("CVideoFormatNegotiator::~CVideoFormatNegotiator\n"));
    ClearMediaType(&m_InternalMT);
}

HRESULT CVideoFormatNegotiator::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, int TypeNum, DWORD VideoControlFlags, DWORD ControlFlags)
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

HRESULT CVideoFormatNegotiator::CreateInternalTypeVMR(const AM_MEDIA_TYPE* pmt, bool NeedReconnect)
{
    HRESULT hr = S_OK;
    CopyMediaType(&m_InternalMT, pmt);

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
    
    return hr;
}

HRESULT CVideoFormatNegotiator::CreateInternalTypeOverlay(const AM_MEDIA_TYPE* pmt, bool NeedReconnect)
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, pmt);

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

    if((bmi->biWidth != m_Width) ||
        (abs(bmi->biHeight) != abs(m_Height)))
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

        NeedReconnect = true;
    }

    return hr;
}

HRESULT CVideoFormatNegotiator::CreateInternalTypeOther(const AM_MEDIA_TYPE* pmt, bool NeedReconnect)
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, pmt);

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

    NeedReconnect = true;

    return hr;
}

HRESULT CVideoFormatNegotiator::CreateInternalTypeWM10(const AM_MEDIA_TYPE* pmt, bool NeedReconnect)
{
    HRESULT hr = S_OK;

    CopyMediaType(&m_InternalMT, pmt);

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

    return S_OK;
}

HRESULT CVideoFormatNegotiator::SetConnectedType(const AM_MEDIA_TYPE* pmt)
{
    CopyMediaType(&m_InternalMT, pmt);
    int wout = 0, hout = 0, pitch = 0;
    long arxout = 0, aryout = 0;
    ExtractDim(pmt, wout, hout, arxout, aryout, pitch);
    m_Width = wout;
    m_Height = abs(hout);
    m_AspectX = arxout;
    m_AspectY = aryout;
    return S_OK;
}

HRESULT CVideoFormatNegotiator::AdjustRenderersMediaType(const AM_MEDIA_TYPE* pmt)
{
    HRESULT hr = S_OK;
    CopyMediaType(&m_InternalMT, pmt);

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
    return hr;
}

bool CVideoFormatNegotiator::NeedReconnect()
{
    int wout = 0, hout = 0, pitch = 0;
    long arxout = 0, aryout = 0;
    ExtractDim(&m_InternalMT, wout, hout, arxout, aryout, pitch);
    return (m_Width != wout ||
            m_Height != abs(hout) ||
            m_AspectX != arxout ||
            m_AspectY != aryout);
}


void CVideoFormatNegotiator::SetAvgTimePerFrame(REFERENCE_TIME AvgTimePerFrame)
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

void CVideoFormatNegotiator::SetAspectX(DWORD AspectX)
{
    m_AspectX = AspectX;
}

void CVideoFormatNegotiator::SetAspectY(DWORD AspectY)
{
    m_AspectY = AspectY;
}

void CVideoFormatNegotiator::SetWidth(int Width)
{
    m_Width = Width;
}

void CVideoFormatNegotiator::SetHeight(int Height)
{
    m_Height = Height;
}
