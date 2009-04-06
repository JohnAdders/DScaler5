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

class CVideoFormatNegotiator
{
public:
    CVideoFormatNegotiator();
    ~CVideoFormatNegotiator();

    HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, int TypeNum, DWORD VideoControlFlags, DWORD ControlFlags);
    HRESULT CreateInternalTypeVMR(const AM_MEDIA_TYPE* pmt, bool& NeedReconnect);
    HRESULT CreateInternalTypeOverlay(const AM_MEDIA_TYPE* pmt, bool& NeedReconnect);
    HRESULT CreateInternalTypeOther(const AM_MEDIA_TYPE* pmt, bool& NeedReconnect);
    HRESULT CreateInternalTypeWM10(const AM_MEDIA_TYPE* pmt, bool& NeedReconnect);
    HRESULT SetConnectedType(const AM_MEDIA_TYPE* pmt);
    HRESULT AdjustRenderersMediaType(const AM_MEDIA_TYPE* pmt);
    AM_MEDIA_TYPE* GetMediaType() {return &m_InternalMT;};
    bool NeedReconnect();

    int GetWidth() {return m_Width;};
    int GetHeight() {return m_Height;};
    DWORD GetAspectX() {return m_AspectX;};
    DWORD GetAspectY() {return m_AspectY;};

    void SetAspectX(DWORD AspectX);
    void SetAspectY(DWORD AspectY);
    void SetWidth(int Width);
    void SetHeight(int Height);
    void SetAvgTimePerFrame(REFERENCE_TIME AvgTimePerFrame);

private:
    AM_MEDIA_TYPE m_InternalMT;
    DWORD m_AspectX;
    DWORD m_AspectY;
    int m_Width;
    int m_Height;
    REFERENCE_TIME m_AvgTimePerFrame;
};

#define VIDEOTYPEFLAG_PREVENT_VIDEOINFOHEADER 1
#define VIDEOTYPEFLAG_FORCE_YUY2 2
#define VIDEOTYPEFLAG_FORCE_YV12 4
#define VIDEOTYPEFLAG_PROGRESSIVE 8
#define VIDEOTYPEFLAG_FORCE_DSCALER 16
#define VIDEOTYPEFLAG_SET_FIELD1FIRST 32
#define VIDEOTYPEFLAG_FORCE_NV12 64

