///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// ConvolverWrapper.dll - DirectShow filter for detecting audio type in PCM streams
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
// Revision 1.2  2006/03/07 15:12:41  adcockj
// fix for convolver under audacity
//
// Revision 1.1  2005/07/21 12:42:33  adcockj
// firstt cut of wrapper for John Pavel's convolver
//
// Revision 1.3  2004/10/21 18:52:30  adcockj
// Half works with ac3 now
//
// Revision 1.2  2004/09/13 14:28:45  adcockj
// Connection changes and crash fixes
//
// Revision 1.1  2004/09/10 16:55:46  adcockj
// Initial version of spdif input filter
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ConvolverWrapper.h"
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"
#include "MediaBufferWrapper.h"

extern HINSTANCE g_hInstance;

CConvolverWrapper::CConvolverWrapper() :
    CDSBaseFilter(L"Convolver Wrapper Filter", 1, 1)
{
    LOG(DBGLOG_FLOW, ("CConvolverWrapper::CreatePins\n"));

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
}

CConvolverWrapper::~CConvolverWrapper()
{
    LOG(DBGLOG_FLOW, ("CConvolverWrapper::~CConvolverWrapper\n"));
}

STDMETHODIMP CConvolverWrapper::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CConvolverWrapper::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CConvolverWrapper;
    return S_OK;
}

HRESULT CConvolverWrapper::ParamChanged(DWORD dwParamIndex)
{
    return S_OK;
}

HRESULT CConvolverWrapper::GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    return E_NOTIMPL;
}

STDMETHODIMP CConvolverWrapper::get_Name(BSTR* Name)
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

STDMETHODIMP CConvolverWrapper::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = GPL;
    return S_OK;
}

STDMETHODIMP CConvolverWrapper::get_Authors(BSTR* Authors)
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

HRESULT CConvolverWrapper::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        pProps->cBuffers = 0;
        pProps->cbAlign = 0;
		pProps->cbBuffer = 0;
        pProps->cbPrefix = 0;
        return S_OK;
    }
    else if(pPin == m_AudioOutPin)
    {
        if(m_AudioInPin->IsConnected())
        {
            HRESULT hr = m_AudioInPin->m_Allocator->GetProperties(pProps);
            if(pProps->cBuffers < 3)
            {
                pProps->cBuffers = 3;
                pProps->cbAlign = 0;
                pProps->cbPrefix = 0;
            }

            return S_OK;
        }
        else
        {
            return E_UNEXPECTED;
        }
    }
    else
    {
        return E_UNEXPECTED;
    }

}

HRESULT CConvolverWrapper::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CConvolverWrapper::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
    return S_OK;
}


HRESULT CConvolverWrapper::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CConvolverWrapper::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CConvolverWrapper::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CConvolverWrapper::Flush(CDSBasePin* pPin)
{
    HRESULT hr = CheckConvolver();
    CHECK(hr);
    hr = m_Convolver->Flush();
    CHECK(hr);

    return S_OK;
}

HRESULT CConvolverWrapper::SendOutLastSamples(CDSBasePin* pPin)
{
    return S_OK;
}

HRESULT CConvolverWrapper::Activate()
{
    HRESULT hr = CheckConvolver();
    CHECK(hr);
    hr = m_Convolver->AllocateStreamingResources();
    CHECK(hr);
    return S_OK;
}

HRESULT CConvolverWrapper::Deactivate()
{
    HRESULT hr = CheckConvolver();
    CHECK(hr);
    hr = m_Convolver->FreeStreamingResources();
    CHECK(hr);
    return S_OK;
}


bool CConvolverWrapper::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool result = false;

    if(pPin == m_AudioInPin)
    {
        HRESULT hr = CheckConvolver();
        if(FAILED(hr)) return false;

        hr = m_Convolver->SetInputType(0, (DMO_MEDIA_TYPE*)pmt, DMO_SET_TYPEF_TEST_ONLY);
        if(FAILED(hr)) return false;

        result = (hr == S_OK);
    }
	else if(pPin == m_AudioOutPin)
	{
        HRESULT hr = CheckConvolver();
        if(FAILED(hr)) return false;

        hr = m_Convolver->SetOutputType(0, (DMO_MEDIA_TYPE*)pmt, DMO_SET_TYPEF_TEST_ONLY);
        if(FAILED(hr)) return false;

        result = (hr == S_OK);
	}
    return result;
}

HRESULT CConvolverWrapper::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    if(pPin == m_AudioInPin)
    {
        HRESULT hr = CheckConvolver();
        CHECK(hr);

        hr = m_Convolver->SetInputType(0, (DMO_MEDIA_TYPE*)pMediaType, DMO_SET_TYPEF_CLEAR);
        CHECK(hr);

        hr = m_Convolver->SetInputType(0, (DMO_MEDIA_TYPE*)pMediaType, 0);
        CHECK(hr);
    }
    else if(pPin == m_AudioOutPin)
    {
        HRESULT hr = CheckConvolver();
        CHECK(hr);

        hr = m_Convolver->SetOutputType(0, (DMO_MEDIA_TYPE*)pMediaType, DMO_SET_TYPEF_CLEAR);
        CHECK(hr);

        hr = m_Convolver->SetOutputType(0, (DMO_MEDIA_TYPE*)pMediaType, 0);
        CHECK(hr);
    }
    return S_OK;
}

HRESULT CConvolverWrapper::NotifyConnected(CDSBasePin* pPin)
{
	if(pPin == m_AudioInPin)
	{
        UpdateTypes(&pPin->m_ConnectedMediaType);
	}
    return S_OK;
}

HRESULT CConvolverWrapper::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{
    if(pPin == m_AudioOutPin)
    {
        if(!m_AudioInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

        if(TypeNum < 0) return E_INVALIDARG;
	    if(TypeNum >= 1) return VFW_S_NO_MORE_ITEMS;
        
        return CopyMediaType(pmt, &m_AudioInPin->m_ConnectedMediaType);;
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }

}


HRESULT CConvolverWrapper::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
	HRESULT hr = S_OK;

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

    SI(IMediaBuffer) InputBuffer = CMediaBufferWrapper::CreateBuffer(InSample);

    if(m_Convolver)
    {
       hr = m_Convolver->ProcessInput(0, InputBuffer.GetNonAddRefedInterface(), 0, 0, 0);
       CHECK(hr);
       
       if(hr == S_OK)
       {
            SI(IMediaSample) OutputSample;

            hr = m_AudioOutPin->GetOutputSample(OutputSample.GetReleasedInterfaceReference(), NULL, NULL, m_IsDiscontinuity);
            CHECK(hr);

            SI(IMediaBuffer) OutputBuffer = CMediaBufferWrapper::CreateBuffer(OutputSample.GetNonAddRefedInterface());

            DMO_OUTPUT_DATA_BUFFER Output;
            Output.pBuffer = OutputBuffer.GetNonAddRefedInterface();
            Output.dwStatus = 0;
            Output.rtTimestamp = 0;
            Output.rtTimelength = 0;

            DWORD Status = 0;

            hr = m_Convolver->ProcessOutput(0, 1, &Output, &Status);
            CHECK(hr);

            BYTE* pBuffer = NULL;
            DWORD Length = 0;

            hr = OutputBuffer->GetBufferAndLength(&pBuffer, &Length);
            CHECK(hr);

            if(Length > 0)
            {
                if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
	            {
		            OutputSample->SetDiscontinuity(TRUE);
	            }
                else
                {
		            OutputSample->SetDiscontinuity(FALSE);
                }
                
                if(pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
                {
                    if(pSampleProperties->dwSampleFlags & AM_SAMPLE_STOPVALID)
                    {
                        OutputSample->SetTime(&pSampleProperties->tStart, &pSampleProperties->tStop);
                    }
                    else
                    {
                        OutputSample->SetTime(&pSampleProperties->tStart, NULL);
                    }
                }
                else
                {
                    OutputSample->SetTime(NULL, NULL);
                }

                OutputSample->SetActualDataLength(Length);

                hr = m_AudioOutPin->SendSample(OutputSample.GetNonAddRefedInterface());
            }
       }
    }
    else
    {
        // todo memcpy
    }

    return hr;
}

HRESULT CConvolverWrapper::UpdateTypes(const AM_MEDIA_TYPE* MediaType)
{
    HRESULT hr = S_OK;
    hr = CheckConvolver();
    CHECK(hr);
	m_Convolver->Flush();
	CHECK(hr);
	m_Convolver->SetInputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	m_Convolver->SetOutputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	m_Convolver->SetInputType(0, MediaType, 0);
	CHECK(hr);
	m_Convolver->SetOutputType(0, MediaType, 0);
	CHECK(hr);
    return hr;
}


HRESULT CConvolverWrapper::CheckConvolver()
{
    HRESULT hr = S_OK;
    if(!m_Convolver)
    {
        hr = m_Convolver.CreateInstance(CLSID_Convolver);

        m_ConvolverPrivate = m_Convolver;
    }
    return hr;
}

///////////////////////////////////////////////////////////////////////////////
// wrapping of Get property pages and private access inrterface so that we
// can display the property page.
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CConvolverWrapper::GetPages(CAUUID *pPages)
{
    CheckConvolver();
    SI(ISpecifyPropertyPages) PropPages = m_Convolver;
    if(PropPages)
    {
        return PropPages->GetPages(pPages);
    }
    else
    {
        return E_NOTIMPL;
    }
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::get_wetmix(double *pVal)
{
    return m_ConvolverPrivate->get_wetmix(pVal);
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::put_wetmix(double newVal)
{
    return m_ConvolverPrivate->put_wetmix(newVal);
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::get_filterfilename(TCHAR* *pVal)
{
    return m_ConvolverPrivate->get_filterfilename(pVal);
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::put_filterfilename(TCHAR* newVal)
{
    return m_ConvolverPrivate->put_filterfilename(newVal);
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::get_attenuation(double *pVal)
{
    return m_ConvolverPrivate->get_attenuation(pVal);
}

HRESULT STDMETHODCALLTYPE CConvolverWrapper::put_attenuation(double newVal)
{
    return m_ConvolverPrivate->put_attenuation(newVal);
}

double	CConvolverWrapper::decode_Attenuationdb(const DWORD dwValue)
{
    return m_ConvolverPrivate->decode_Attenuationdb(dwValue);
}

DWORD	CConvolverWrapper::encode_Attenuationdb(const double fValue)
{
    return m_ConvolverPrivate->encode_Attenuationdb(fValue);
}

