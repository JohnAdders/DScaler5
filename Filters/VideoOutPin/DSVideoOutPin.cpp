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
#include "DSVideoOutPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "DSUtil.h"
#include "MediaTypes.h"
#include <vmr9.h>
#include <Mpconfig.h>
#include "evcode.h"
#include "MoreUuids.h"

CDSVideoOutPin::CDSVideoOutPin(CVideoFormatNegotiator& Negotiator) : 
        CDSOutputPin(),
        m_Negotiator(Negotiator)
{
    LOG(DBGLOG_ALL, ("CDSVideoOutPin::CDSVideoOutPin\n"));
    m_ConnectedType = DEFAULT_OUTFILTER;
    m_NeedToAttachFormat = false;
    m_InsideReconnect = false;
}

CDSVideoOutPin::~CDSVideoOutPin()
{
    LOG(DBGLOG_ALL, ("CDSVideoOutPin::~CDSVideoOutPin\n"));
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

void CDSVideoOutPin::SetAspectX(DWORD AspectX)
{
    m_Negotiator.SetAspectX(AspectX);
}

void CDSVideoOutPin::SetAspectY(DWORD AspectY)
{
    m_Negotiator.SetAspectY(AspectY);
}

DWORD CDSVideoOutPin::GetAspectX()
{
    return m_Negotiator.GetAspectX();
}

DWORD CDSVideoOutPin::GetAspectY()
{
    return m_Negotiator.GetAspectY();
}

void CDSVideoOutPin::SetWidth(int Width)
{
    m_Negotiator.SetWidth(Width);
}

int CDSVideoOutPin::GetWidth()
{
    return m_Negotiator.GetWidth();
}

void CDSVideoOutPin::SetHeight(int Height)
{
    m_Negotiator.SetHeight(Height);
}

int CDSVideoOutPin::GetHeight()
{
    return m_Negotiator.GetHeight();
}

HRESULT CDSVideoOutPin::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, int TypeNum, DWORD VideoControlFlags, DWORD ControlFlags)
{
    return m_Negotiator.CreateSuitableMediaType(pmt,TypeNum, VideoControlFlags, ControlFlags);
}

HRESULT CDSVideoOutPin::CheckForReconnection()
{
    HRESULT hr = S_OK;

    if(m_Negotiator.NeedReconnect() || m_NeedToAttachFormat)
    {
	    m_NeedToAttachFormat = false;

        m_InsideReconnect = true;

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

        m_Negotiator.SetConnectedType(GetMediaType());
    }
    return hr;
}

HRESULT CDSVideoOutPin::ReconnectVMR()
{
    HRESULT hr = S_OK;

    bool NeedReconnect = false;

    hr = m_Negotiator.CreateInternalTypeVMR(GetMediaType(), NeedReconnect);
    if(hr != S_OK)
    {
        LOG(DBGLOG_FLOW, ("CreateInternalTypeVMR failed in ReconnectOutput %08x\n", hr));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    SI(IPinConnection) m_PinConnection = m_ConnectedPin;
    if(m_PinConnection)
    {
        hr = m_PinConnection->DynamicQueryAccept(m_Negotiator.GetMediaType());
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else
    {
        hr = m_ConnectedPin->QueryAccept(m_Negotiator.GetMediaType());
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

        SetType(m_Negotiator.GetMediaType());

        m_InsideReconnect = true;
        hr = m_ConnectedPin->ReceiveConnection(this, GetMediaType());
        m_InsideReconnect = false;
        LOG(DBGLOG_FLOW, ("ReceiveConnection %08x\n", hr));

        m_InsideReconnect = true;
        hr = m_MemInputPin->NotifyAllocator(m_Allocator.GetNonAddRefedInterface(), FALSE);
        m_InsideReconnect = false;
        CHECK(hr);

        // reset bmi for code below
        // in case we have had a pitch change
        BITMAPINFOHEADER* bmi = ExtractBIH(GetMediaType());


        ALLOCATOR_PROPERTIES AllocatorProps;
        hr = m_Allocator->GetProperties(&AllocatorProps);
        LOG(DBGLOG_FLOW, ("GetProperties %08x\n", hr));
        CHECK(hr);
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

    bool NeedReconnect = false;

    hr = m_Negotiator.CreateInternalTypeOverlay(GetMediaType(), NeedReconnect);
    if(hr != S_OK)
    {
        LOG(DBGLOG_FLOW, ("CreateInternalTypeVMR failed in ReconnectOutput %08x\n", hr));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if(NeedReconnect)
    {
        SI(IPinConnection) m_PinConnection = m_ConnectedPin;
        if(m_PinConnection)
        {
            hr = m_PinConnection->DynamicQueryAccept(m_Negotiator.GetMediaType());
            if(hr != S_OK)
            {
                LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
        else
        {
            hr = m_ConnectedPin->QueryAccept(m_Negotiator.GetMediaType());
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

        hr = SetType(m_Negotiator.GetMediaType());
        CHECK(hr);

        m_InsideReconnect = true;
        hr = NegotiateAllocator(NULL);
        m_InsideReconnect = false;
        CHECK(hr);

        m_InsideReconnect = true;
        hr = m_ConnectedPin->ReceiveConnection(this, GetMediaType());
        m_InsideReconnect = false;
        CHECK(hr);

        hr = m_Allocator->Commit();
        CHECK(hr);
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

    bool NeedReconnect = false;

    hr = m_Negotiator.CreateInternalTypeOther(GetMediaType(), NeedReconnect);
    if(hr != S_OK)
    {
        LOG(DBGLOG_FLOW, ("CreateInternalTypeOther failed in ReconnectOutput %08x\n", hr));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    if(NeedReconnect)
    {
        SI(IPinConnection) m_PinConnection = m_ConnectedPin;
        if(m_PinConnection)
        {
            hr = m_PinConnection->DynamicQueryAccept(m_Negotiator.GetMediaType());
            if(hr != S_OK)
            {
                LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
                return S_OK;
                return VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
        else
        {
            hr = m_ConnectedPin->QueryAccept(m_Negotiator.GetMediaType());
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
            //hr = GraphConfig->Reconnect(this, m_ConnectedPin.GetNonAddRefedInterface(), m_Negotiator.GetMediaType(), NULL, NULL, AM_GRAPH_CONFIG_RECONNECT_DIRECTCONNECT);
            return S_OK;
            CHECK(hr);
        }

        return S_OK;
    }
    return hr;
}

HRESULT CDSVideoOutPin::ReconnectWM10()
{
    HRESULT hr = S_OK;

    bool NeedReconnect = false;

    hr = m_Negotiator.CreateInternalTypeWM10(GetMediaType(), NeedReconnect);
    if(hr != S_OK)
    {
        LOG(DBGLOG_FLOW, ("CreateInternalTypeWM10 failed in ReconnectOutput %08x\n", hr));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

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
    m_Negotiator.SetConnectedType(pMediaType);
}


HRESULT CDSVideoOutPin::SendSample(IMediaSample* OutSample)
{
    if(m_NeedToAttachFormat)
    {
        OutSample->SetMediaType(m_Negotiator.GetMediaType());
        LogMediaType(m_Negotiator.GetMediaType(), "AttachFormat", DBGLOG_FLOW);
        m_NeedToAttachFormat = false;

        SI(IMediaEventSink) pMES = m_Filter->m_Graph;
        if(pMES)
        {
            // some renderers don't send this
            pMES->Notify(EC_VIDEO_SIZE_CHANGED, MAKELPARAM(m_Negotiator.GetWidth(), m_Negotiator.GetHeight()), 0);
        }
    }
    return CDSOutputPin::SendSample(OutSample);
}

HRESULT CDSVideoOutPin::AdjustRenderersMediaType()
{
    HRESULT hr = S_OK;

    m_NeedToAttachFormat = true;
    
    hr = m_Negotiator.AdjustRenderersMediaType(GetMediaType());
    if(hr != S_OK)
    {
        LOG(DBGLOG_FLOW, ("AdjustRenderersMediaType failed in ReconnectOutput %08x\n", hr));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    SI(IPinConnection) m_PinConnection = m_ConnectedPin;

    if(m_PinConnection)
    {
        hr = m_PinConnection->DynamicQueryAccept(m_Negotiator.GetMediaType());
        if(hr != S_OK)
        {
            LOG(DBGLOG_FLOW, ("DynamicQueryAccept failed in ReconnectOutput %08x\n", hr));
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else
    {
        hr = m_ConnectedPin->QueryAccept(m_Negotiator.GetMediaType());
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
    LOG(DBGLOG_ALL, ("CDSVideoOutPin::QueryAccept\n"));
    LogMediaType(pmt, "CDSVideoOutPin::QueryAccept", DBGLOG_FLOW);

    if(m_Filter->IsThisATypeWeCanWorkWith(pmt, this) == false)
    {
        return S_FALSE;
    }
    int wout = 0, hout = 0, pitch = 0;
    long arxout = 0, aryout = 0;
    ExtractDim(pmt, wout, hout, arxout, aryout, pitch);
    if(abs(wout) != abs(m_Negotiator.GetWidth()) || abs(hout) != abs(m_Negotiator.GetHeight()))
    {
        return S_FALSE;
    }
    else
    {
        if(m_InsideReconnect || IsConnected())
        {
            // if we are in the middle of a reconnection then we need to
            // accept pitch changes from upstream
            if(pitch > wout)
            {
			    LOG(DBGLOG_FLOW, ("CDSVideoOutPin::Got Pitch change request\n"));
				SetType(pmt);
                m_Negotiator.SetConnectedType(pmt);
                m_NeedToAttachFormat = true;
            }
        }
        return S_OK;
    }
}

void CDSVideoOutPin::SetAvgTimePerFrame(REFERENCE_TIME AvgTimePerFrame)
{
    m_Negotiator.SetAvgTimePerFrame(AvgTimePerFrame);
}

STDMETHODIMP CDSVideoOutPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    m_InsideReconnect = true;
    HRESULT hr = CDSOutputPin::Connect(pReceivePin, pmt);
    m_InsideReconnect = false;
    return hr;
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
