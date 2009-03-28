///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009 John Adcock
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

#pragma once

#include "DSOutputPin.h"

/* Holder for data associated with a single video frame
   Data store in install as pointers to planes
   for YUV types data stored as 
   m_Data[0] = Y plane
   m_Data[1] = U plane
   m_Data[2] = V plane
*/
class CVideoData
{
public:
    /* construct from planar data pointers.
       for YUV data stored as Y the U then V
    */
    CVideoData(const GUID& Format, BYTE* Data[4], int Width, int Height, int Pitch, int BitCount = 0);
    /* Construct from a DirectShow Media type and a media sample
    */
    CVideoData(const AM_MEDIA_TYPE* MediaType, SI(IMediaSample)& Sample);

    /*  not sure if we need these yet
    int GetWidth() const {return m_Width;}
    int GetHeight() const {return m_Height;}
    int GetPitch() const {return m_Pitch;}
    const GUID& GetFormat() {return m_Format;}
    const BYTE* GetData(int Plane) const {return m_Data[Plane];}
    BYTE* GetData(int Plane) {return m_Data[Plane];}
*/
    static HRESULT Copy(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma);
private:
    static HRESULT Copy420(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma);
    static HRESULT Copy422(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy444(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy420ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma);
    static HRESULT Copy420ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy420ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy420ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut, bool ProgressiveChroma);
    static HRESULT Copy422ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy422ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy422ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy422ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy444ToYUY2(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy444ToYV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy444ToNV12(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    static HRESULT Copy444ToRGB(const CVideoData& VideoDataIn, CVideoData& VideoDataOut);
    GUID m_Format;
    BYTE* m_Data[4];
    int m_Width;
    int m_Height;
    int m_Pitch;
    int m_BitCount;
};
