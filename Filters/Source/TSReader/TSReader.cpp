///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
//
// You should have received a copy of the GNU General Public
// License along with this package; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "TSReader.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "TSReaderOutPin.h"
#include "MediaBufferWrapper.h"
#include "MoreUuids.h"

extern HINSTANCE g_hInstance;

CTSReader::CTSReader() :
    CDSBaseFilter(L"TSReader Filter", 0, 1)
{
    LOG(DBGLOG_FLOW, ("CTSReader::CreatePins\n"));

    m_OutPin = new CTSReaderOutPin();
    if(m_OutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin"));
    }
    m_OutPin->AddRef();
    m_OutPin->SetupObject(this, L"Output");

    m_ThreadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_WorkerThread = NULL;
    m_ThreadRetCode = S_OK;

    m_PSIDataArray = NULL;
    m_NumPrograms = 0;
    m_bNotYetSetPins = true;
    m_GotPMTS = 0;
}

CTSReader::~CTSReader()
{
    CloseHandle(m_ThreadStopEvent);
    ClearPrograms();
    LOG(DBGLOG_FLOW, ("CTSReader::~CTSReader\n"));
}

STDMETHODIMP CTSReader::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CTSReader::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CTSReader;
    return S_OK;
}

HRESULT CTSReader::ParamChanged(DWORD dwParamIndex)
{
    return S_OK;
}

HRESULT CTSReader::GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTSReader::get_Name(BSTR* Name)
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

STDMETHODIMP CTSReader::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CTSReader::get_Authors(BSTR* Authors)
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


HRESULT CTSReader::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin)
{
    if(pPin == m_OutPin)
    {
        pProps->cbBuffer = 188;
        pProps->cBuffers = 3;
        pProps->cbAlign = 1;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }

}

HRESULT CTSReader::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTSReader::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    return S_OK;
}


HRESULT CTSReader::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTSReader::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTSReader::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTSReader::Flush(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTSReader::SendOutLastSamples(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTSReader::Activate()
{
    HRESULT hr = S_OK;
    if(m_bNotYetSetPins)
    {
        SI(IPin) VideoPin;
        SI(IPin) AudioPin;

        HRESULT hr = m_OutPin->GetConnectedFilter()->FindPin(L"Audio", AudioPin.GetReleasedInterfaceReference());
        CHECK(hr);

        hr = m_OutPin->GetConnectedFilter()->FindPin(L"Video", VideoPin.GetReleasedInterfaceReference());
        CHECK(hr);

        for(int i(0); i < m_NumPrograms; ++i)
        {
            if(m_PSIDataArray[i]->m_VideoPid > 0)
            {
                SI(IMPEG2PIDMap) PIDMap = VideoPin;
                ULONG PidToMap = m_PSIDataArray[i]->m_VideoPid;
                PIDMap->MapPID(1, &PidToMap, MEDIA_ELEMENTARY_STREAM);
                if(m_PSIDataArray[i]->m_AudioPid > 0)
                {
                    AM_MEDIA_TYPE* pMediaType;
                    switch(m_PSIDataArray[i]->m_AudioType)
                    {
                    case 6:
                        pMediaType = GetAACMediaType();
                        break;
                    case 3:
                        pMediaType = GetMPAMediaType();
                        break;
                    default:
                        pMediaType = GetAC3MediaType();
                        break;
                    }

                    hr = m_MpegDemux->SetOutputPinMediaType(L"Audio", pMediaType);
                    CHECK(hr);

                    PIDMap = AudioPin;
                    ULONG PidToMap = m_PSIDataArray[i]->m_AudioPid;
                    hr = PIDMap->MapPID(1, &PidToMap, MEDIA_ELEMENTARY_STREAM);
                    CHECK(hr);

                    SI(IPin) AudioInPin;
                    hr = AudioPin->ConnectedTo(AudioInPin.GetReleasedInterfaceReference());
                    CHECK(hr);

                    //hr = AudioInPin->ReceiveConnection(AudioPin.GetNonAddRefedInterface(), pMediaType);
                    //CHECK(hr);

                }

                m_bNotYetSetPins = false;
                break;
            }
        }
    }


    ResetEvent(m_ThreadStopEvent);

    m_ThreadRetCode = S_OK;

    m_WorkerThread = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE) ProcessingThread,
            this,  // pass event handle
            0, &m_ThreadId);

    return hr;
}

HRESULT CTSReader::Deactivate()
{
    SetEvent(m_ThreadStopEvent);

    DWORD dwWaitResult = WaitForSingleObject(m_WorkerThread, 100);

    if(dwWaitResult == WAIT_TIMEOUT)
    {
        TerminateThread(m_WorkerThread, 0);
    }

    CloseHandle(m_WorkerThread);
    m_WorkerThread = NULL;

    return S_OK;
}


bool CTSReader::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool result = false;

    if(pPin == m_OutPin)
    {
        result = !!(pmt->majortype == MEDIATYPE_Stream);

        result &= !!(pmt->formattype == MEDIASUBTYPE_MPEG2_TRANSPORT);
    }
    return result;
}

HRESULT CTSReader::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTSReader::NotifyConnected(CDSBasePin* pPin)
{
    m_MpegDemux = pPin->GetConnectedFilter();
    if(!m_MpegDemux)
    {
        return E_UNEXPECTED;
    }

    return S_OK;
}

HRESULT CTSReader::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{
    if(pPin == m_OutPin)
    {
        if(TypeNum < 0) return E_INVALIDARG;
        if(TypeNum >= 1) return VFW_S_NO_MORE_ITEMS;
        ClearMediaType(pmt);
        pmt->majortype = MEDIATYPE_Stream;
        pmt->subtype = MEDIASUBTYPE_MPEG2_TRANSPORT;
        pmt->bFixedSizeSamples = TRUE;
        pmt->lSampleSize = 188;
        return S_OK;
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }

}

HRESULT CTSReader::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    return E_UNEXPECTED;
}

AM_MEDIA_TYPE* CTSReader::GetVideoMediaType()
{
    static AM_MEDIA_TYPE MediaType;
    InitMediaType(&MediaType);

    MediaType.majortype = MEDIATYPE_Video;
    MediaType.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
    MediaType.formattype = FORMAT_MPEG2_VIDEO;

    static MPEG2VIDEOINFO m2vi;
    ZeroMemory(&m2vi, sizeof(m2vi));
    m2vi.hdr.rcSource.bottom = 480;
    m2vi.hdr.rcSource.right = 720;
    m2vi.hdr.dwBitRate = 4000000;
    m2vi.hdr.AvgTimePerFrame = 333667;
    m2vi.hdr.dwPictAspectRatioX = 4;
    m2vi.hdr.dwPictAspectRatioY = 3;
    m2vi.hdr.bmiHeader.biSize = sizeof(m2vi.hdr.bmiHeader);
    m2vi.hdr.bmiHeader.biWidth = 720;
    m2vi.hdr.bmiHeader.biHeight = 480;
    m2vi.hdr.bmiHeader.biYPelsPerMeter = 53031;
    m2vi.hdr.bmiHeader.biXPelsPerMeter = 2000;
    m2vi.cbSequenceHeader = 0;
    m2vi.dwProfile = AM_MPEG2Profile_Main;
    m2vi.dwLevel = AM_MPEG2Level_Main;

    MediaType.cbFormat = sizeof(m2vi);
    MediaType.pbFormat = (BYTE*)&m2vi;
    return &MediaType;
}

AM_MEDIA_TYPE* CTSReader::GetAC3MediaType()
{
    static AM_MEDIA_TYPE MediaType;
    InitMediaType(&MediaType);

    MediaType.majortype = MEDIATYPE_Audio;
    MediaType.subtype = MEDIASUBTYPE_DOLBY_AC3;
    MediaType.formattype = FORMAT_WaveFormatEx;
    static WAVEFORMATEX wfe;
    wfe.wFormatTag = 0;
    wfe.nChannels = 2;
    wfe.nSamplesPerSec = 48000;
    wfe.nAvgBytesPerSec = 0;
    wfe.nBlockAlign = 0;
    wfe.wBitsPerSample = 16;
    wfe.cbSize = 0;
    MediaType.cbFormat = sizeof(wfe);
    MediaType.pbFormat = (BYTE*)&wfe;
    return &MediaType;
}

AM_MEDIA_TYPE* CTSReader::GetMPAMediaType()
{
    static AM_MEDIA_TYPE MediaType;
    InitMediaType(&MediaType);

    MediaType.majortype = MEDIATYPE_Audio;
    MediaType.subtype = MEDIASUBTYPE_MPEG1Payload;
    MediaType.formattype = FORMAT_WaveFormatEx;
    static WAVEFORMATEX wfe;
    wfe.wFormatTag = 80;
    wfe.nChannels = 2;
    wfe.nSamplesPerSec = 48000;
    wfe.nAvgBytesPerSec = 0;
    wfe.nBlockAlign = 0;
    wfe.wBitsPerSample = 16;
    wfe.cbSize = 0;
    MediaType.cbFormat = sizeof(wfe);
    MediaType.pbFormat = (BYTE*)&wfe;
    return &MediaType;
}

AM_MEDIA_TYPE* CTSReader::GetAACMediaType()
{
    static AM_MEDIA_TYPE MediaType;
    InitMediaType(&MediaType);

    MediaType.majortype = MEDIATYPE_Audio;
    MediaType.subtype = MEDIASUBTYPE_MP4A;
    MediaType.formattype = FORMAT_WaveFormatEx;
    static WAVEFORMATEX wfe;
    wfe.wFormatTag = 80;
    wfe.nChannels = 2;
    wfe.nSamplesPerSec = 48000;
    wfe.nAvgBytesPerSec = 0;
    wfe.nBlockAlign = 0;
    wfe.wBitsPerSample = 16;
    wfe.cbSize = 0;
    MediaType.cbFormat = sizeof(wfe);
    MediaType.pbFormat = (BYTE*)&wfe;
    return &MediaType;
}

HRESULT CTSReader::Render(IPin* pPin, IGraphBuilder* pGraphBuilder)
{
    SI(IBaseFilter) MpegDemuxFilter;

    HRESULT hr = MpegDemuxFilter.CreateInstance(CLSID_MPEG2Demultiplexer);
    CHECK(hr);

    hr = pGraphBuilder->AddFilter(MpegDemuxFilter.GetNonAddRefedInterface(), L"MPEG-2 Demultiplexer");
    CHECK(hr);

    SI(IEnumPins) EnumPins;
    SI(IPin) InputPin;
    hr = MpegDemuxFilter->EnumPins(EnumPins.GetReleasedInterfaceReference());
    CHECK(hr);
    if (EnumPins->Next(1, InputPin.GetReleasedInterfaceReference(), 0) != S_OK)
    {
        return E_UNEXPECTED;
    }

    hr = pGraphBuilder->ConnectDirect(pPin, InputPin.GetNonAddRefedInterface(), NULL);
    CHECK(hr);

    m_MpegDemux = MpegDemuxFilter;

    SI(IPin) VideoPin;
    SI(IPin) AudioPin;

    hr = m_MpegDemux->CreateOutputPin(GetAC3MediaType(), L"Audio", AudioPin.GetReleasedInterfaceReference());
    CHECK(hr);

    hr = m_MpegDemux->CreateOutputPin(GetVideoMediaType(), L"Video", VideoPin.GetReleasedInterfaceReference());
    CHECK(hr);

    hr = pGraphBuilder->Render(AudioPin.GetNonAddRefedInterface());
    CHECK(hr);

    SI(IBaseFilter) analyseFilter;

    hr = analyseFilter.CreateInstance(CLSID_Mpeg2VideoStreamAnalyzer);
    CHECK(hr);

    hr = pGraphBuilder->AddFilter(analyseFilter.GetNonAddRefedInterface(), L"MPEG-2 Analyser");
    CHECK(hr);

    SI(IPin) AnalysePin;
    hr = analyseFilter->FindPin(L"in", AnalysePin.GetReleasedInterfaceReference());
    CHECK(hr);

    hr = pGraphBuilder->Connect(VideoPin.GetNonAddRefedInterface(), AnalysePin.GetNonAddRefedInterface());
    CHECK(hr);

    hr = analyseFilter->FindPin(L"out", AnalysePin.GetReleasedInterfaceReference());
    CHECK(hr);

    hr = pGraphBuilder->Render(AnalysePin.GetNonAddRefedInterface());
    CHECK(hr);

    return hr;
}

HRESULT CTSReader::Backout(IPin* pPin, IGraphBuilder* pGraphBuilder)
{
    m_MpegDemux.Detach();
    return S_OK;
}


STDMETHODIMP CTSReader::Load(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
    wcscpy(m_FileName, pszFileName);
    HRESULT hr = AnalyseFile();
    return S_OK;
}

STDMETHODIMP CTSReader::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((wcslen(m_FileName) + 1) * 2);
    wcscpy(*ppszFileName, m_FileName);
    if(pmt)
    {
        InitMediaType(pmt);
        pmt->majortype = MEDIATYPE_Stream;
        pmt->subtype = MEDIASUBTYPE_MPEG2_TRANSPORT;
        pmt->bFixedSizeSamples = TRUE;
        pmt->lSampleSize = 188;
    }
    return S_OK;
}

void CTSReader::ClearPrograms()
{
    if(m_NumPrograms > 0)
    {
        for(int i = 0; i < m_NumPrograms; ++i)
        {
            delete m_PSIDataArray[i];
        }
        delete [] m_PSIDataArray;
        m_PSIDataArray = NULL;
        m_NumPrograms = 0;
        m_GotPMTS = 0;
        m_bNotYetSetPins = true;
    }
}


void CTSReader::ProcessingThread(void* pParam)
{
    HRESULT hr = S_OK;
    CTSReader* pThis = (CTSReader*)pParam;

    HANDLE hFile;
    hFile = CreateFileW(pThis->m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return;
    }

    while(SUCCEEDED(hr))
    {
        hr = pThis->PushFile(hFile);
        CloseHandle(hFile);

        long namelength(wcslen(pThis->m_FileName));

        if(namelength >= 4)
        {
            if(pThis->m_FileName[namelength - 4] >= '0' && pThis->m_FileName[namelength - 4] <= '8')
            {
                ++pThis->m_FileName[namelength - 4];
            }
            else
            {
                int offset = 4;
                while((namelength - offset) >= 0 && pThis->m_FileName[namelength - offset] == '9')
                {
                    ++offset;
                }
                if((namelength - offset) >= 0)
                {
                    if(pThis->m_FileName[namelength - offset] >= '0' && pThis->m_FileName[namelength - offset] <= '8')
                    {
                        ++(pThis->m_FileName[namelength - offset]);
                        while(offset > 4)
                        {
                            --offset;
                            pThis->m_FileName[namelength - offset] = '0';
                        }
                    }
                }
                else
                {
                    hr = E_UNEXPECTED;
                }
            }
            if(SUCCEEDED(hr))
            {
                hFile = CreateFileW(pThis->m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    hr = E_UNEXPECTED;
                }
            }
        }
    }

    pThis->m_OutPin->m_ConnectedPin->EndOfStream();
}

HRESULT CTSReader::AnalyseFile()
{
    HRESULT hr = S_OK;


    HANDLE hFile = CreateFileW(m_FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    dvbpsi_handle h_dvbpsi = dvbpsi_AttachPAT(StaticUpdatePAT, this);

    ClearPrograms();

    while(m_NumPrograms == 0 || m_GotPMTS != m_NumPrograms)
    {
        // Access the sample's data buffer
        BYTE pData[188];
        DWORD nBytesRead = 0;

        if(!ReadFile(hFile, pData, 188, &nBytesRead, NULL) || nBytesRead < 188)
        {
            break;
        }
        else
        {
            uint16_t i_pid = ((uint16_t)(pData[1] & 0x1f) << 8) + pData[2];
            if(i_pid == 0x0)
            {
                dvbpsi_PushPacket(h_dvbpsi, pData);
            }
            else
            {
                for(int i = 0 ; i < m_NumPrograms; ++i)
                {
                    if(i_pid == m_PSIDataArray[i]->m_pid)
                    {
                        dvbpsi_PushPacket(m_PSIDataArray[i]->m_PMTHandle, pData);
                    }
                }
            }
        }
    }

    dvbpsi_DetachPAT(h_dvbpsi);
    CloseHandle(hFile);

    if(m_GotPMTS > 0)
    {
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }
}


HRESULT CTSReader::PushFile(HANDLE hFile)
{
    HRESULT hr = S_OK;

    m_bNotYetSetPins = true;

    while(1)
    {
        SI(IMediaSample) OutSample;
        hr = m_OutPin->GetOutputSample(OutSample.GetReleasedInterfaceReference(), NULL, NULL, FALSE);
        CHECK(hr);
        // Access the sample's data buffer
        BYTE* pData;

        hr = OutSample->GetPointer(&pData);
        CHECK(hr);
        hr = OutSample->SetActualDataLength(188);
        CHECK(hr);

        DWORD nBytesRead = 0;

        if(!ReadFile(hFile, pData, 188, &nBytesRead, NULL) || nBytesRead < 188)
        {
            return S_FALSE;
        }

        hr = m_OutPin->SendSample(OutSample.GetNonAddRefedInterface());
        CHECK(hr);

    }
    return E_UNEXPECTED;
}

void CTSReader::UpdatePAT(dvbpsi_pat_t* p_pat)
{
    if(m_PSIDataArray == NULL)
    {
        m_NumPrograms = 0;

        dvbpsi_pat_program_t* pNext = p_pat->p_first_program;
        while(pNext != NULL)
        {
            LOG(DBGLOG_FLOW, ("UpdatePAT %d %d\n", pNext->i_number, pNext->i_pid));
            if(pNext->i_number > 0)
            {
                ++m_NumPrograms;
            }
            pNext = pNext->p_next;
        }
        m_PSIDataArray = new CPSIData*[m_NumPrograms];

        int i(0);
        pNext = p_pat->p_first_program;
        while(pNext != NULL)
        {
            if(pNext->i_number > 0)
            {
                m_PSIDataArray[i] = new CPSIData(pNext, this);
                ++i;
            }
            pNext = pNext->p_next;
        }
    }
    dvbpsi_DeletePAT(p_pat);
}

void CTSReader::StaticUpdatePAT(void* p_zero, dvbpsi_pat_t* p_pat)
{
    CTSReader* pThis = (CTSReader*)p_zero;
    pThis->UpdatePAT(p_pat);
}


CTSReader::CPSIData::CPSIData(dvbpsi_pat_program_t* pProgram, CTSReader* Parent)
{
    m_PMTHandle = dvbpsi_AttachPMT(pProgram->i_number, StaticUpdatePMT, this);
    m_pid = pProgram->i_pid;
    m_program = pProgram->i_number;
    m_VideoPid = -1;
    m_AudioPid = -1;
    m_Parent = Parent;
    m_AudioType = 129;
}


CTSReader::CPSIData::~CPSIData()
{
    dvbpsi_DetachPMT(m_PMTHandle);
}

void CTSReader::CPSIData::StaticUpdatePMT(void* p_zero, dvbpsi_pmt_t* p_pmt)
{
    CPSIData* pThis = (CPSIData*)p_zero;
    pThis->UpdatePMT(p_pmt);
}

void CTSReader::CPSIData::UpdatePMT(dvbpsi_pmt_t* p_pmt)
{
    LOG(DBGLOG_FLOW, ("UpdatePMT %d\n", p_pmt->i_program_number));
    if(m_VideoPid == -1)
    {
        dvbpsi_pmt_es_t* pEs = p_pmt->p_first_es;
        while(pEs)
        {
            LOG(DBGLOG_FLOW, (" PID %x Type %d\n", pEs->i_pid, pEs->i_type));
            if((pEs->i_type == 1 || pEs->i_type == 2) && m_VideoPid == -1)
            {
                m_VideoPid = pEs->i_pid;
            }
            if((pEs->i_type == 3 || pEs->i_type == 4) && m_AudioPid == -1)
            {
                m_AudioPid = pEs->i_pid;
                m_AudioType = 3;
            }
            if((pEs->i_type == 15) && m_AudioPid == -1)
            {
                m_AudioPid = pEs->i_pid;
                m_AudioType = 6;
            }
            if(pEs->i_type == 129)
            {
                m_AudioPid = pEs->i_pid;
                m_AudioType = 129;
            }
            pEs = pEs->p_next;
        }
        m_Parent->m_GotPMTS++;
    }
    dvbpsi_DeletePMT(p_pmt);
}
