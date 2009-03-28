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
#include "VideoData.h"
#include "MediaTypes.h"
#include "DSUtil.h"
#include "PlanarYUVToRGB.h"
#include "PlanarYUVToYUY2.h"

// MEDIASUBTYPE_ARGB32.Data1
const DWORD ARGB32_DATA1(0x773c9ac0);
// MEDIASUBTYPE_RGB32.Data1
const DWORD RGB32_DATA1(0xe436eb7e);
// MEDIASUBTYPE_RGB24.Data1
const DWORD RGB24_DATA1(0xe436eb7d);
// MEDIASUBTYPE_RGB565.Data1
const DWORD RGB565_DATA1(0xe436eb7b);
// MEDIASUBTYPE_RGB555.Data1
const DWORD RGB555_DATA1(0xe436eb7c);

CVideoData::CVideoData(const GUID& Format, BYTE** Data, int Width, int Height, int Pitch, int BitCount) :
    m_Format(Format),
    m_Width(Width),
    m_Height(Height),
    m_Pitch(Pitch),
    m_BitCount(BitCount)
{
    m_Data[0] = Data[0];
    m_Data[1] = Data[1];
    m_Data[2] = Data[2];
    m_Data[3] = Data[3];
}

CVideoData::CVideoData(const AM_MEDIA_TYPE* MediaType, SI(IMediaSample)& Sample) :
    m_Format(MediaType->subtype),
    m_BitCount(0)
{
    long TempX;
    long TempY;

    ExtractDim(MediaType, m_Width, m_Height, TempX, TempY, m_Pitch);

    const BITMAPINFOHEADER* bih = ExtractBIH(MediaType);

    BYTE* Buffer = 0;

    if(FAILED(Sample->GetPointer(&Buffer)))
    {
        throw std::logic_error("Failed to get pointer from sample");
    }

    m_Data[0] = Buffer;
    m_Data[1] = 0;
    m_Data[2] = 0;
    m_Data[3] = 0;

    switch(m_Format.Data1)
    {
    case ARGB32_DATA1:
        m_BitCount = 32;
        m_Pitch *= 4;
        break;
    case RGB32_DATA1:
        m_BitCount = 32;
        m_Pitch *= 4;
        break;
    case RGB24_DATA1:
        m_BitCount = 24;
        m_Pitch *= 3;
        break;
    case RGB565_DATA1:
        m_BitCount = 16;
        m_Pitch *= 2;
        break;
    case RGB555_DATA1:
        m_BitCount = 15;
        m_Pitch *= 2;
        break;
    case MAKEFOURCC('Y', 'U', 'Y', '2'):
        m_Pitch *= 2;
        m_Height = abs(m_Height);
        break;
    case MAKEFOURCC('N', 'V', '1', '2'):
        m_Data[1] = Buffer + bih->biWidth * bih->biHeight;
        m_Height = abs(m_Height);
        break;
    case MAKEFOURCC('Y', 'V', '1', '2'):
        // Data is Y V U
        m_Data[2] = Buffer + bih->biWidth * bih->biHeight;
        m_Data[1] = m_Data[2] + bih->biWidth * bih->biHeight / 4;
        m_Height = abs(m_Height);
        break;
    default:
        throw std::logic_error("Invalid Format");
        break;
    }
}


HRESULT CVideoData::Copy420(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma)
{
    switch(VideoDataOut.m_Format.Data1)
    {
    case MAKEFOURCC('Y', 'V', '1', '2'):
        return Copy420ToYV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('N', 'V', '1', '2'):
        return Copy420ToNV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('Y', 'U', 'Y', '2'):
        return Copy420ToYUY2(VideoDataIn, VideoDataOut, ProgressiveChroma);
    case ARGB32_DATA1:
    case RGB32_DATA1:
    case RGB24_DATA1:
    case RGB565_DATA1:
    case RGB555_DATA1:
        return Copy420ToRGB(VideoDataIn, VideoDataOut, ProgressiveChroma);
    default:
        throw std::logic_error("Unexpected oputput format");
    }
}

HRESULT CVideoData::Copy422(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    switch(VideoDataOut.m_Format.Data1)
    {
    case MAKEFOURCC('Y', 'V', '1', '2'):
        return Copy422ToYV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('N', 'V', '1', '2'):
        return Copy422ToNV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('Y', 'U', 'Y', '2'):
        return Copy422ToYUY2(VideoDataIn, VideoDataOut);
    case ARGB32_DATA1:
    case RGB32_DATA1:
    case RGB24_DATA1:
    case RGB565_DATA1:
    case RGB555_DATA1:
        return Copy422ToRGB(VideoDataIn, VideoDataOut);
    default:
        throw std::logic_error("Unexpected oputput format");
    }
}

HRESULT CVideoData::Copy444(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    switch(VideoDataOut.m_Format.Data1)
    {
    case MAKEFOURCC('Y', 'V', '1', '2'):
        return Copy444ToYV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('N', 'V', '1', '2'):
        return Copy444ToNV12(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('Y', 'U', 'Y', '2'):
        return Copy444ToYUY2(VideoDataIn, VideoDataOut);
    case ARGB32_DATA1:
    case RGB32_DATA1:
    case RGB24_DATA1:
    case RGB565_DATA1:
    case RGB555_DATA1:
        return Copy444ToRGB(VideoDataIn, VideoDataOut);
    default:
        throw std::logic_error("Unexpected oputput format");
    }
}


HRESULT CVideoData::Copy(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma)
{
    switch(VideoDataIn.m_Format.Data1)
    {
    case MAKEFOURCC('Y', 'V', '1', '2'):
        return Copy420(VideoDataIn, VideoDataOut, ProgressiveChroma);
    case MAKEFOURCC('P', '4', '2', '2'):
        return Copy422(VideoDataIn, VideoDataOut);
    case MAKEFOURCC('P', '4', '4', '4'):
        return Copy444(VideoDataIn, VideoDataOut);
    default:
        break;
    }
    throw std::logic_error("unexpected input format");
}

HRESULT CVideoData::Copy420ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma)
{
    bool Result;
    if(ProgressiveChroma)
    {
        Result = BitBltFromI420ToYUY2(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                     );
    }
    else
    {
        Result = BitBltFromI420ToYUY2_Int(
                                            VideoDataIn.m_Width,
                                            VideoDataIn.m_Height,
                                            VideoDataOut.m_Data[0],
                                            VideoDataOut.m_Pitch,
                                            VideoDataIn.m_Data[0],
                                            VideoDataIn.m_Data[1],
                                            VideoDataIn.m_Data[2],
                                            VideoDataIn.m_Pitch
                                         );
    }
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy420ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI420ToI420(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Data[1],
                                        VideoDataOut.m_Data[2],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy420ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI420ToNV12(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Data[1],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy420ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma)
{
    bool Result;
    
    if(VideoDataOut.m_Height > 0)
    {
        // +ve Height means bottom up
        BYTE* VideoOutBuf = VideoDataOut.m_Data[0] + VideoDataOut.m_Pitch * (abs(VideoDataIn.m_Height) - 1);
        Result = BitBltFromI420ToRGB(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoOutBuf,
                                        -VideoDataOut.m_Pitch,
                                        VideoDataOut.m_BitCount,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                    );
    }
    else
    {
        // -ve Height means top down
        Result = BitBltFromI420ToRGB(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Pitch,
                                        VideoDataOut.m_BitCount,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                    );
    }
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy422ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI422ToYUY2(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy422ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI422ToI420(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Data[1],
                                        VideoDataOut.m_Data[2],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy422ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    return E_FAIL;
}

HRESULT CVideoData::Copy422ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result;
    
    if(VideoDataIn.m_Height > 0)
    {
        // +ve Height means bottom up
        BYTE* VideoOutBuf = VideoDataOut.m_Data[0] + VideoDataOut.m_Pitch * (VideoDataIn.m_Height - 1);
        Result = BitBltFromI422ToRGB(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoOutBuf,
                                        -VideoDataOut.m_Pitch,
                                        VideoDataOut.m_BitCount,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                    );
    }
    else
    {
        // +ve Height means top down
        Result = BitBltFromI422ToRGB(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Pitch,
                                        VideoDataOut.m_BitCount,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                    );
    }
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy444ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI444ToYUY2(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy444ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    bool Result = BitBltFromI444ToI420(
                                        VideoDataIn.m_Width,
                                        VideoDataIn.m_Height,
                                        VideoDataOut.m_Data[0],
                                        VideoDataOut.m_Data[1],
                                        VideoDataOut.m_Data[2],
                                        VideoDataOut.m_Pitch,
                                        VideoDataIn.m_Data[0],
                                        VideoDataIn.m_Data[1],
                                        VideoDataIn.m_Data[2],
                                        VideoDataIn.m_Pitch
                                      );
    return Result?S_OK:E_FAIL;
}

HRESULT CVideoData::Copy444ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    return E_FAIL;
}

HRESULT CVideoData::Copy444ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut)
{
    return E_FAIL;
}
