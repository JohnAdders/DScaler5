///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// AudioDetect.dll - DirectShow filter for detecting audio type in PCM streams
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
#include "AudioDetect.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"

extern HINSTANCE g_hInstance;

CAudioDetect::CAudioDetect() :
    CDSBaseFilter(L"DScaler Audio Detect Filter", 1, 1)
{
    InitMediaType(&m_InternalMT);

    m_NeedToAttachFormat = false;

    LOG(DBGLOG_FLOW, ("CAudioDetect::CreatePins\n"));

    m_AudioInPin = new CDSInputPin;
    if(m_AudioInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_AudioInPin->AddRef();
    m_AudioInPin->SetupObject(this, L"Input");

    m_AudioOutPin = new CDSOutputPin();
    if(m_AudioOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_AudioOutPin->AddRef();
    m_AudioOutPin->SetupObject(this, L"Output");

    m_StreamType = STREAM_PCM;
    m_SamplesPerSec = 48000;
    m_BufferStartTime = 0;
}

CAudioDetect::~CAudioDetect()
{
    LOG(DBGLOG_FLOW, ("CAudioDetect::~CAudioDetect\n"));
    // don't clear media type as we use our own storage for format
}

STDMETHODIMP CAudioDetect::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CAudioDetect::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CAudioDetect;
    return S_OK;
}

HRESULT CAudioDetect::ParamChanged(DWORD dwParamIndex)
{
    switch(dwParamIndex)
    {
    case DETECT_TYPE:
        switch(GetParamEnum(DETECT_TYPE))
        {
        case DETECT_AUTO:
        case DETECT_FORCE_PCM:
            m_StreamType = STREAM_PCM;
            break;
        case DETECT_FORCE_DTS:
            m_StreamType = STREAM_DTS;
            break;
        case DETECT_FORCE_AC3:
            m_StreamType = STREAM_AC3;
            break;
        }
        CreateInternalMediaType();
    }
    return S_OK;
}

HRESULT CAudioDetect::GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    if(dwParamIndex == DETECT_TYPE)
    {
        return GetEnumTextDetectType(ppwchText);
    }
    else
    {
        return E_NOTIMPL;
    }
}

STDMETHODIMP CAudioDetect::get_Name(BSTR* Name)
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

STDMETHODIMP CAudioDetect::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CAudioDetect::get_Authors(BSTR* Authors)
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

HRESULT CAudioDetect::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        pProps->cBuffers = 3;
        pProps->cbAlign = 1;
        pProps->cbBuffer = 8192;
        return S_OK;
    }
    else if(pPin == m_AudioOutPin)
    {
        pProps->cBuffers = 3;
        pProps->cbBuffer = 2048;
        pProps->cbAlign = 1;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }

}

HRESULT CAudioDetect::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CAudioDetect::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    return S_OK;
}


HRESULT CAudioDetect::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CAudioDetect::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CAudioDetect::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CAudioDetect::Flush(CDSBasePin* pPin)
{
    m_buff.resize(0);
    return S_OK;
}

HRESULT CAudioDetect::SendOutLastSamples(CDSBasePin* pPin)
{
    m_buff.resize(0);
    return S_OK;
}

HRESULT CAudioDetect::Activate()
{
    return S_OK;
}

HRESULT CAudioDetect::Deactivate()
{
    m_buff.resize(0);
    return S_OK;
}


bool CAudioDetect::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool result = false;

    if(pPin == m_AudioInPin)
    {
        result = !!(pmt->majortype == MEDIATYPE_Audio);

        result &= !!(pmt->formattype == FORMAT_WaveFormatEx);

        result &= !!(pmt->subtype == MEDIASUBTYPE_PCM);

        WAVEFORMATEX* wf = (WAVEFORMATEX*)pmt->pbFormat;

        if(wf != NULL)
        {
            result &= !!(wf->nChannels == 2);

            result &= !!(wf->nBlockAlign == 4);

            result &= !!(wf->nSamplesPerSec <= 48000);
        }
        else
        {
            result = false;
        }
    }
    else if(pPin == m_AudioOutPin)
    {
        result = true;
    }
    return result;
}

HRESULT CAudioDetect::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        WAVEFORMATEX* wf = (WAVEFORMATEX*)pMediaType->pbFormat;
        m_SamplesPerSec = wf->nSamplesPerSec;
        CreateInternalMediaType();
    }
    else if(pPin == m_AudioOutPin)
    {
    }
    return S_OK;
}

HRESULT CAudioDetect::NotifyConnected(CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        SI(IAMStreamConfig) StreamConfig = pPin->m_ConnectedPin;
        if(StreamConfig)
        {
            AM_MEDIA_TYPE NewMediaType;
            HRESULT hr = CopyMediaType(&NewMediaType, &pPin->m_ConnectedMediaType);
            CHECK(hr);
            WAVEFORMATEX* wfe = (WAVEFORMATEX*)NewMediaType.pbFormat;
            if(wfe->nSamplesPerSec < 48000)
            {
                wfe->nSamplesPerSec = 48000;

                hr = StreamConfig->SetFormat(&NewMediaType);
                CHECK(hr);
            }
        }
        m_StreamType = STREAM_PCM;
        CreateInternalMediaType();
    }
    return S_OK;
}

HRESULT CAudioDetect::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{
    if(pPin == m_AudioOutPin)
    {
        if(!m_AudioInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

        if(TypeNum < 0) return E_INVALIDARG;
        if(TypeNum >= 1) return VFW_S_NO_MORE_ITEMS;

        return CopyMediaType(pmt, &m_InternalMT);
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }

}

void CAudioDetect::CreateInternalMediaType()
{
    m_InternalMT.majortype = MEDIATYPE_Audio;
    switch(m_StreamType)
    {
    case STREAM_SILENCE:
    case STREAM_PCM:
        m_InternalMT.subtype = MEDIASUBTYPE_DVD_LPCM_AUDIO;
        break;
    case STREAM_AC3:
        m_InternalMT.subtype = MEDIASUBTYPE_DOLBY_AC3;
        break;
    case STREAM_DTS:
        m_InternalMT.subtype = MEDIASUBTYPE_DTS;
        break;
    }
    m_InternalMT.formattype = FORMAT_WaveFormatEx;

    m_InternalWF.nSamplesPerSec = m_SamplesPerSec;
    m_InternalWF.nChannels = 2;
    m_InternalWF.nBlockAlign = 2 * 16 / 8;
    m_InternalWF.nAvgBytesPerSec = m_SamplesPerSec * m_InternalWF.nBlockAlign;
    m_InternalWF.wBitsPerSample = 16;
    m_InternalWF.cbSize = 0;

    m_InternalWF.wFormatTag = (WORD)m_InternalMT.subtype.Data1;

    m_InternalMT.cbFormat = sizeof(m_InternalWF);
    m_InternalMT.pbFormat = (BYTE*)&m_InternalWF;

    m_NeedToAttachFormat = true;
}


HRESULT CAudioDetect::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    if(pPin != m_AudioInPin)
    {
        return E_UNEXPECTED;
    }

    // if there was a discontinuity then we need to ask for the buffer
    // differently
    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
    {
        m_IsDiscontinuity = true;
    }

    HRESULT hr = S_OK;

    long len = pSampleProperties->lActual;
    BYTE* pDataIn = pSampleProperties->pbBuffer;
    int tmp = m_buff.size();
    m_buff.resize(m_buff.size() + len);
    memcpy(&m_buff[0] + tmp, pDataIn, len);

    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        m_BufferStartTime = pSampleProperties->tStart;
        m_BufferStartTime -= tmp * 10000000i64  / (m_SamplesPerSec * 4);
    }

    // only process if we have at least one full ac3 buffer
    // in the buffer and at least 2 spdif headers
    // worst case is that we are two bytes in to one 0x1800 long structure
    while(SUCCEEDED(hr) && m_buff.size() >= 0x3010)
    {
        switch(m_StreamType)
        {
        case STREAM_PCM:
            hr = ProcessPCM();
            break;
        case STREAM_AC3:
            hr = ProcessAC3();
            break;
        case STREAM_DTS:
            hr = ProcessDTS();
            break;
        case STREAM_SILENCE:
            hr = ProcessSilence();
            break;
        }
    }
    return hr;
}

HRESULT CAudioDetect::LookForSync()
{
    for(DWORD i = 0; i < m_buff.size(); i += 4)
    {
        DWORD PosSync = *(DWORD*)&m_buff[i];
        if(PosSync == 0xe8001fff)
        {
            m_StreamType = STREAM_DTS;
            break;
        }
        else if(PosSync == 0x00e8ff1f)
        {
            m_StreamType = STREAM_DTS;
            break;
        }
        else if(PosSync == 0x80017ffe)
        {
            m_StreamType = STREAM_DTS;
            break;
        }
        else if(PosSync == 0x0180fe7f)
        {
            m_StreamType = STREAM_DTS;
            break;
        }
        else
        {
            ;
        }
    }
    return S_OK;
}

HRESULT CAudioDetect::ProcessPCM()
{
    HRESULT hr = S_OK;
    int Chunks = m_buff.size() / 0x1800;

    WORD* pData = (WORD*)&m_buff[0];

    while(Chunks--)
    {
        WORD* pTest = pData;
        for(int i(0); i < 0x1800 / 2; ++i)
        {
            if(pTest[0] == 0xf872 && pTest[1] == 0x4e1f && pTest[0x1800] == 0xf872 && pTest[0x1801] == 0x4e1f)
            {
                int ByteToChuck = (BYTE*)pTest - (BYTE*)&m_buff[0];
                ChangeTypeBasedOnSpdifType(pTest[2]);
                int Size = m_buff.size() - ByteToChuck;
                memmove(&m_buff[0], pTest, Size);
                m_buff.resize(Size);
                m_BufferStartTime += ByteToChuck * 10000000i64  / (m_SamplesPerSec * 4);
                return hr;
            }
            ++pTest;
        }
        pData += 0x1800 / 2;
    }

    int ByteToChuck = (BYTE*)pData - (BYTE*)&m_buff[0];
    if(ByteToChuck != 0)
    {
        int Size = m_buff.size() - ByteToChuck;
        memmove(&m_buff[0], pData, Size);
        m_buff.resize(Size);
        m_BufferStartTime += ByteToChuck * 10000000i64  / (m_SamplesPerSec * 4);
    }
    return hr;
}

HRESULT CAudioDetect::ProcessSilence()
{
    HRESULT hr = S_OK;
    int Chunks = m_buff.size() / 0x1800;

    WORD* pData = (WORD*)&m_buff[0];

    while(Chunks--)
    {
        WORD* pTest = pData;
        for(int i(0); i < 0x1800 / 2; ++i)
        {
            ++pTest;
        }
        pData += 0x1800 / 2;
    }

    int ByteToChuck = (BYTE*)pData - (BYTE*)&m_buff[0];
    if(ByteToChuck != 0)
    {
        int Size = m_buff.size() - ByteToChuck;
        memmove(&m_buff[0], pData, Size);
        m_buff.resize(Size);
    }
    return hr;
}


HRESULT CAudioDetect::ProcessAC3()
{
    WORD* pDataIn = (WORD*)&m_buff[0];
    DWORD lenIn = m_buff.size();
    HRESULT hr = S_OK;
    while(lenIn >= 0x1800)
    {
        if(pDataIn[0] != 0xf872 || pDataIn[1] != 0x4e1f)
        {
            m_StreamType = STREAM_PCM;
            CreateInternalMediaType();
            if(lenIn > 0)
            {
                memmove(&m_buff[0], pDataIn, lenIn);
            }
            m_buff.resize(lenIn);

            return hr;
        }
        if(pDataIn[2] != 0x0001)
        {
            ChangeTypeBasedOnSpdifType(pDataIn[2]);
            if(lenIn > 0)
            {
                memmove(&m_buff[0], pDataIn, lenIn);
            }
            m_buff.resize(lenIn);
            return hr;
        }
        DWORD Size = pDataIn[3] >> 3;

        SI(IMediaSample) pOut;
        BYTE* pDataOut = NULL;

        hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, Size);
        CHECK(hr);

        _swab((char*)pDataIn + 8, (char*)pDataOut, Size);

        hr = Deliver(pOut.GetNonAddRefedInterface());
        if(hr != S_OK)
        {
            return hr;
        }
        pDataIn += 0x1800 / 2;
        lenIn -= 0x1800;
        m_BufferStartTime += 0x1800 * 10000000i64  / (m_SamplesPerSec * 4);
    }

    if(lenIn > 0)
    {
        memmove(&m_buff[0], pDataIn, lenIn);
    }
    m_buff.resize(lenIn);

    return hr;
}

HRESULT CAudioDetect::ProcessDTS()
{
    WORD* pDataIn = (WORD*)&m_buff[0];
    DWORD lenIn = m_buff.size();
    HRESULT hr = S_OK;
    while(lenIn >= 0x800)
    {
        if(pDataIn[0] != 0xf872 || pDataIn[1] != 0x4e1f)
        {
            m_StreamType = STREAM_PCM;
            CreateInternalMediaType();
            return hr;
        }
        if(pDataIn[2] != 0x000b)
        {
            ChangeTypeBasedOnSpdifType(pDataIn[2]);
            return hr;
        }
        DWORD Size = pDataIn[3] >> 3;

        SI(IMediaSample) pOut;
        BYTE* pDataOut = NULL;

        hr = GetOutputSampleAndPointer(pOut.GetReleasedInterfaceReference(), &pDataOut, Size);
        CHECK(hr);

        _swab((char*)pDataIn + 8, (char*)pDataOut, Size);

        hr = Deliver(pOut.GetNonAddRefedInterface());
        if(hr != S_OK)
        {
            return hr;
        }
        pDataIn += 0x800 / 2;
        lenIn -= 0x800;
    }

    if(lenIn > 0)
    {
        memmove(&m_buff[0], pDataIn, lenIn);
    }
    m_buff.resize(lenIn);

    return hr;
}


HRESULT CAudioDetect::GetEnumTextDetectType(WCHAR **ppwchText)
{
    wchar_t DetectText[] = L"Detection Type\0" L"None\0" L"Automatic\0" L"Force PCM\0" L"Force DTS\0" L"Force DD\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(DetectText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, DetectText, sizeof(DetectText));
    return S_OK;
}


HRESULT CAudioDetect::GetOutputSampleAndPointer(IMediaSample** pOut, BYTE** ppDataOut, DWORD Len)
{
    *ppDataOut = NULL;

    HRESULT hr = m_AudioOutPin->GetOutputSample(pOut, NULL, NULL, m_IsDiscontinuity);
    CHECK(hr);

    hr = (*pOut)->GetPointer(ppDataOut);
    CHECK(hr);

    hr = (*pOut)->SetActualDataLength(Len);
    CHECK(hr);

    return hr;
}

HRESULT CAudioDetect::Deliver(IMediaSample* pOut)
{
    HRESULT hr = S_OK;


    pOut->SetPreroll(FALSE);
    pOut->SetSyncPoint(TRUE);
    if(m_BufferStartTime != 0)
    {
        pOut->SetTime(&m_BufferStartTime, NULL);
    }

    if(m_NeedToAttachFormat)
    {
        m_AudioOutPin->SetType(&m_InternalMT);
        pOut->SetMediaType(&m_InternalMT);
        pOut->SetDiscontinuity(TRUE);
    }
    else
    {
        pOut->SetDiscontinuity(m_IsDiscontinuity);
    }

    m_IsDiscontinuity = false;

    hr = m_AudioOutPin->SendSample(pOut);

    m_NeedToAttachFormat = false;
    return hr;
}

void CAudioDetect::ChangeTypeBasedOnSpdifType(WORD SpdifType)
{
    switch(SpdifType)
    {
    case 0x0001:
        m_StreamType = STREAM_AC3;
        break;
    case 0x000b:
        m_StreamType = STREAM_DTS;
        break;
    case 0x0000:
    case 0x0300:
        m_StreamType = STREAM_SILENCE;
        break;
    default:
        m_StreamType = STREAM_SILENCE;
        break;
    }
    CreateInternalMediaType();
}
