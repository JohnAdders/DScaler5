///////////////////////////////////////////////////////////////////////////////
// $Id: DSVideoOutPin.h,v 1.1 2004-10-28 16:00:48 adcockj Exp $
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

#pragma once 

#include "DSOutputPin.h"

class CDSVideoOutPin : public CDSOutputPin
{
public:

IMPLEMENT_UNKNOWN(CDSVideoOutPin)

BEGIN_INTERFACE_TABLE(CDSVideoOutPin)
    IMPLEMENTS_INTERFACE(IPin)
    IMPLEMENTS_INTERFACE(IPinFlowControl)
    IMPLEMENTS_INTERFACE(IQualityControl)
    IMPLEMENTS_INTERFACE(IMediaSeeking)
    IMPLEMENTS_INTERFACE(IKsPropertySet)
END_INTERFACE_TABLE()

public:
    CDSVideoOutPin();
    ~CDSVideoOutPin();

    STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE *pmt);
    HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, int TypeNum, bool NotYV12, DWORD ControlFlags);
    HRESULT NotifyConnected();
	void OnConnectToVMR7();
	void OnConnectToVMR9();
	void OnConnectToOverlay();
    HRESULT CheckForReconnection();
    HRESULT ReconnectVMR();
    HRESULT ReconnectGabest();
    HRESULT ReconnectOverlay();
    HRESULT ReconnectOther();


    void Copy420(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn, bool ProgressiveChroma);
    void Copy422(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);
    void Copy444(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);

    HRESULT GetOutputSample(IMediaSample** OutSample, REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop, bool PrevFrameSkipped);
    void NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType);
    HRESULT SendSample(IMediaSample* OutSample);
    HRESULT AdjustRenderersMediaType();
    void SetAvgTimePerFrame(REFERENCE_TIME AvgTimePerFrame);
    
    void SetPanScanX(DWORD OffsetX);
    void SetPanScanY(DWORD OffsetY);
    DWORD GetPanScanX();
    DWORD GetPanScanY();

    void SetAspectX(DWORD AspectX);
    void SetAspectY(DWORD AspectY);
    DWORD GetAspectX();
    DWORD GetAspectY();

    void SetWidth(int Width);
    int GetWidth();
    void SetHeight(int Height);
    int GetHeight();

    typedef enum
    {
        DEFAULT_OUTFILTER,
        FFDSHOW_OUTFILTER,
        GABEST_OUTFILTER,
        OVERLAY_OUTFILTER,
        VMR7_OUTFILTER,
        VMR9_OUTFILTER
    } OUT_TYPE;

    OUT_TYPE GetConnectedType();


private:
    bool m_DoPanAndScan;
    DWORD m_PanScanOffsetX;
    DWORD m_PanScanOffsetY;
    DWORD m_AspectX;
    DWORD m_AspectY;
    int m_Width;
    int m_Height;
    OUT_TYPE m_ConnectedType;
    bool m_NeedToAttachFormat;
    int m_CurrentWidth;
    int m_CurrentHeight;
    long m_CurrentAspectX;
    long m_CurrentAspectY;
    bool m_InsideReconnect;
    long m_PitchWidth;
    long m_PitchHeight;
    REFERENCE_TIME m_AvgTimePerFrame;


    AM_MEDIA_TYPE m_InternalMT;
};
