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
#include "TVSource.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "TVSourceOutPin.h"
#include "MediaBufferWrapper.h"
#include "MoreUuids.h"

extern HINSTANCE g_hInstance;

CTVSource::CTVSource() :
    CDSBaseFilter(L"TVSource Filter", 0, 1)
{
    LOG(DBGLOG_FLOW, ("CTVSource::CreatePins\n"));
    
    m_OutPin = new CTVSourceOutPin();
    if(m_OutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin"));
    }
    m_OutPin->AddRef();
    m_OutPin->SetupObject(this, L"Output");

    m_ThreadStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_WorkerThread = NULL;
    m_ThreadRetCode = S_OK;

}

CTVSource::~CTVSource()
{
    CloseHandle(m_ThreadStopEvent);
    LOG(DBGLOG_FLOW, ("CTVSource::~CTVSource\n"));
}

STDMETHODIMP CTVSource::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CTVSource::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CTVSource;
    return S_OK;
}

HRESULT CTVSource::ParamChanged(DWORD dwParamIndex)
{
    return S_OK;
}

HRESULT CTVSource::GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    return E_NOTIMPL;
}

STDMETHODIMP CTVSource::get_Name(BSTR* Name)
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

STDMETHODIMP CTVSource::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CTVSource::get_Authors(BSTR* Authors)
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


HRESULT CTVSource::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin)
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

HRESULT CTVSource::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTVSource::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    return S_OK;
}


HRESULT CTVSource::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTVSource::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTVSource::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CTVSource::Flush(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTVSource::SendOutLastSamples(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTVSource::Activate()
{
    HRESULT hr = S_OK;
    return hr;
}

HRESULT CTVSource::Deactivate()
{
    return S_OK;
}


bool CTVSource::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool result = true;

    return result;
}

HRESULT CTVSource::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTVSource::NotifyConnected(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CTVSource::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
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

HRESULT CTVSource::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    return E_UNEXPECTED;
}

HRESULT CTVSource::Render(IPin* pPin, IGraphBuilder* pGraphBuilder)
{
    m_NumOutputPins = 0;

    m_OutPin = NULL;

    return BuildGraph(pGraphBuilder);
}

HRESULT CTVSource::Backout(IPin* pPin, IGraphBuilder* pGraphBuilder)
{
    return S_OK;
}


STDMETHODIMP CTVSource::Load(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt)
{
    m_FileName = pszFileName;
    return S_OK;
}

STDMETHODIMP CTVSource::GetCurFile(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt)
{
    *ppszFileName = (LPOLESTR)CoTaskMemAlloc((m_FileName.size() + 1) * 2);
    wcscpy(*ppszFileName, m_FileName.c_str());
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



void CTVSource::ProcessingThread(void* pParam)
{
    HRESULT hr = S_OK;
    CTVSource* pThis = (CTVSource*)pParam;
}



HRESULT CTVSource::PushFile()
{
    return S_OK;
}

