///////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.14 2003-10-31 17:19:37 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
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
// Revision 1.13  2003/09/19 16:12:14  adcockj
// Further improvements
//
// Revision 1.12  2003/08/21 16:17:58  adcockj
// Changed filter to wrap the deinterlacing DMO, fixed many bugs
//
// Revision 1.11  2003/05/20 16:50:58  adcockj
// Interim checkin, preparation for DMO processing path
//
// Revision 1.10  2003/05/09 15:51:04  adcockj
// Code tidy up
// Added aspect ratio parameters
//
// Revision 1.9  2003/05/08 15:58:37  adcockj
// Better error handling, threading and format support
//
// Revision 1.8  2003/05/07 16:27:41  adcockj
// Slightly better properties implementation
//
// Revision 1.7  2003/05/06 16:38:00  adcockj
// Changed to fixed size output buffer and changed connection handling
//
// Revision 1.6  2003/05/02 16:22:22  adcockj
// Switched to ISpecifyPropertyPagesImpl
//
// Revision 1.5  2003/05/02 16:05:22  adcockj
// Logging with file and line numbers
//
// Revision 1.4  2003/05/02 10:53:07  adcockj
// Returns test parameter for testing property page
//
// Revision 1.3  2003/05/01 18:15:17  adcockj
// Moved IMedaiSeeking to output pin
//
// Revision 1.2  2003/05/01 16:19:02  adcockj
// Changed property pages ready for generic page
//
// Revision 1.1.1.1  2003/04/30 13:01:20  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DScaler.h"
#include "EnumTwoPins.h"
#include "InputPin.h"
#include "OutputPin.h"

// define any parameters used
MP_PARAMINFO CDScaler::m_ParamInfos[CDScaler::PARAMS_LASTONE] = 
{
    { MPT_INT, 0, 1, 50, 1, L"None", L"Aspect Ratio Adjustment X" },
    { MPT_INT, 0, 1, 50, 1, L"None", L"Aspect Ratio Adjustment Y" },
    { MPT_BOOL, 0, 0, 1, 0, L"None", L"Is Input Anamorphic" }, 
    { MPT_ENUM, 0, 0, 1, 0, L"None", L"Deinterlace Mode" }, 
    { MPT_BOOL, 0, 0, 1, 0, L"None", L"Manual Pulldown Mode" }, 
    { MPT_ENUM, 0, FULLRATEVIDEO, PULLDOWN_32, 0, L"None", L"Pulldown Mode" }, 
    { MPT_INT, 0, 0, 4, 0, L"None", L"Pulldown Mode Index" },
};


CDScaler::CDScaler()
{
	m_pUnkMarshaler = NULL;
    m_State = State_Stopped;
    m_Graph = NULL;
    m_IsDirty = FALSE;
    wcscpy(m_Name, L"DScaler Filter");
    for(int i(0); i < PARAMS_LASTONE; ++i)
    {
        m_ParamValues[i] = m_ParamInfos[i].mpdNeutralValue;
    }
    m_TypesChanged = TRUE;
    m_RebuildRequired = TRUE;
	m_ChangeTypes = FALSE;
}

HRESULT CDScaler::FinalConstruct()
{
    LOG(DBGLOG_FLOW, ("CDScaler::FinalConstruct\n"));
    m_InputPin = new CComObject<CInputPin>;
    m_InputPin->AddRef();
    m_OutputPin = new CComObject<COutputPin>;
    m_OutputPin->AddRef();
    m_InputPin->m_Filter = this;
    m_InputPin->m_OutputPin = m_OutputPin;
    m_OutputPin->m_Filter = this;
    m_OutputPin->m_InputPin = m_InputPin;
    HRESULT hr = LoadDMOs();
    CHECK(hr)
	return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), &m_pUnkMarshaler.p);
}

void CDScaler::FinalRelease()
{
    LOG(DBGLOG_FLOW, ("CDScaler::FinalRelease\n"));
    m_InputPin->Release();
    m_OutputPin->Release();
	m_pUnkMarshaler.Release();
    UnloadDMOs();
}

STDMETHODIMP CDScaler::EnumPins(IEnumPins **ppEnum)
{
    LOG(DBGLOG_FLOW, ("CDScaler::EnumPins\n"));
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CComObject<CEnumTwoPins>* NewEnum = new CComObject<CEnumTwoPins>;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

    NewEnum->SetPins(m_InputPin, m_OutputPin);
    NewEnum->AddRef();
    *ppEnum = NewEnum;
    
    return S_OK;
}

STDMETHODIMP CDScaler::FindPin(LPCWSTR Id, IPin **ppPin)
{
    LOG(DBGLOG_FLOW, ("CDScaler::FindPin\n"));
    if(wcscmp(Id, L"Input") == 0)
    {
        *ppPin = m_InputPin;
        (*ppPin)->AddRef();
        return S_OK;
    }
    else if(wcscmp(Id, L"Output") == 0)
    {
        *ppPin = m_OutputPin;
        (*ppPin)->AddRef();
        return S_OK;
    }
    else
    {
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }
}

STDMETHODIMP CDScaler::QueryFilterInfo(FILTER_INFO *pInfo)
{
    LOG(DBGLOG_FLOW, ("CDScaler::QueryFilterInfo\n"));
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    wcscpy(pInfo->achName,m_Name);
    pInfo->pGraph = m_Graph;
    if(m_Graph != NULL)
    {
        pInfo->pGraph->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CDScaler::JoinFilterGraph(IFilterGraph *pGraph, LPCWSTR pName)
{
    LOG(DBGLOG_FLOW, ("CDScaler::JoinFilterGraph\n"));
    if(pGraph != NULL)
    {
        // we need the FilterGraph2 interface
        // so that we can use ReconnectEx for dynamic connections
        if(FAILED(pGraph->QueryInterface(__uuidof(IFilterGraph2), (void**)&m_Graph)))
        {
            return VFW_E_NO_INTERFACE;
        }
        // need to release here to avoid circular references
        m_Graph->Release();
        if(pName != NULL && wcslen(pName) < MAX_FILTER_NAME)
        {
            wcscpy(m_Name, pName);
        }
    }
    else
    {
        m_Graph = NULL;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::QueryVendorInfo(LPWSTR *pVendorInfo)
{
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::Stop(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Stop\n"));

    CProtectCode WhileVarInScope(this);

	// make sure no more Receives come through
    HRESULT hr = m_InputPin->FinishProcessing();
    CHECK(hr);

    if(m_OutputPin->m_Allocator != NULL)
    {
        hr = m_OutputPin->m_Allocator->Decommit();
        CHECK(hr);
    }
    m_State = State_Stopped;

	LOG(DBGLOG_FLOW, ("CDScaler::End Stop\n"));

    return S_OK;
}

STDMETHODIMP CDScaler::Pause(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Pause\n"));

    CProtectCode WhileVarInScope(this);

    if(m_OutputPin->m_Allocator != NULL)
    {
        if(m_State == State_Stopped)
        {
            HRESULT hr = m_OutputPin->m_Allocator->Commit();
            CHECK(hr);
        }
    }
    m_State = State_Paused;
    return S_OK;
}

STDMETHODIMP CDScaler::Run(REFERENCE_TIME tStart)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Run\n"));

    CProtectCode WhileVarInScope(this);

	if(m_OutputPin->m_Allocator != NULL)
    {
        if(m_State == State_Stopped)
        {
            HRESULT hr = m_OutputPin->m_Allocator->Commit();
            CHECK(hr);
        }
    }

    m_State = State_Running;
	m_StartTime = tStart;
    return S_OK;
}

STDMETHODIMP CDScaler::GetState(DWORD dwMilliSecsTimeout, FILTER_STATE *State)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetState\n"));
    if(State == NULL)
    {
        return E_POINTER;
    }
    *State = m_State;
    return S_OK;
}

STDMETHODIMP CDScaler::SetSyncSource(IReferenceClock *pClock)
{
    LOG(DBGLOG_FLOW, ("CDScaler::SetSyncSource\n"));
    m_RefClock = pClock;
    return S_OK;
}

STDMETHODIMP CDScaler::GetSyncSource(IReferenceClock **pClock)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetSyncSource\n"));
    return m_RefClock.CopyTo(pClock);
}

STDMETHODIMP CDScaler::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = __uuidof(CDScaler);
    return S_OK;
}

STDMETHODIMP CDScaler::IsDirty(void)
{
    LOG(DBGLOG_FLOW, ("CDScaler::IsDirty\n"));
    return m_IsDirty?S_OK:S_FALSE;
}

STDMETHODIMP CDScaler::Load(IStream __RPC_FAR *pStm)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Load\n"));
    if(pStm == NULL)
    {
        return E_POINTER;
    }
    DWORD NumParams(0);

    HRESULT hr = GetParamCount(&NumParams);
    CHECK(hr);

    for(DWORD i(0); i < NumParams; ++i)
    {
        MP_DATA ParamValue(0);
        DWORD BytesRead(0);
        hr = pStm->Read(&ParamValue, sizeof(MP_DATA), &BytesRead);
        CHECK(hr);
        if(BytesRead == sizeof(MP_DATA))
        {
            hr = SetParam(i, ParamValue);
            CHECK(hr);
        }
        else
        {
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}

STDMETHODIMP CDScaler::Save(IStream __RPC_FAR *pStm, BOOL fClearDirty)
{
    LOG(DBGLOG_FLOW, ("CDScaler::Save\n"));
    if(pStm == NULL)
    {
        return E_POINTER;
    }
    DWORD NumParams(0);

    HRESULT hr = GetParamCount(&NumParams);
    CHECK(hr);

    for(DWORD i(0); i < NumParams; ++i)
    {
        MP_DATA ParamValue(0);
        DWORD BytesWriten(0);
        hr = GetParam(i, &ParamValue);
        CHECK(hr);
        hr = pStm->Write(&ParamValue, sizeof(MP_DATA), &BytesWriten);
        CHECK(hr);
        if(BytesWriten != sizeof(MP_DATA)) return E_UNEXPECTED;
    }

    if(fClearDirty)
    {
        m_IsDirty = FALSE;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::GetSizeMax(ULARGE_INTEGER __RPC_FAR *pcbSize)
{
    LOG(DBGLOG_FLOW, ("CDScaler::GetSizeMax\n"));
    if(pcbSize == NULL)
    {
        return E_POINTER;
    }
    DWORD NumParams(0);

    HRESULT hr = GetParamCount(&NumParams);
    CHECK(hr);

    pcbSize->QuadPart = sizeof(MP_DATA) * NumParams;
    return S_OK;
}

STDMETHODIMP CDScaler::GetParam(DWORD dwParamIndex, MP_DATA *pValue)
{
    if(pValue == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex < PARAMS_LASTONE)
    {
        CProtectCode WhileVarInScope(this);
        *pValue = m_ParamValues[dwParamIndex];
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CDScaler::SetParam(DWORD dwParamIndex,MP_DATA value)
{
    if(dwParamIndex < PARAMS_LASTONE)
    {
        CProtectCode WhileVarInScope(this);
        m_ParamValues[dwParamIndex] = value;
        m_IsDirty = TRUE;
        switch(dwParamIndex)
        {
        case ASPECTINCREASEX:
        case ASPECTINCREASEY:
        case INPUTISANAMORPHIC:
            m_ChangeTypes = TRUE;
            break;
		case DEINTERLACEMODE:
            m_RebuildRequired = TRUE;
            break;  
        case MANUALPULLDOWN:
            break;
        case PULLDOWNMODE:
            ResetPullDownIndexRange();
            break;
        case PULLDOWNINDEX:
            break;
        default:
	        return E_INVALIDARG;
            break;
        }
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

STDMETHODIMP CDScaler::AddEnvelope(DWORD dwParamIndex,DWORD cPoints,MP_ENVELOPE_SEGMENT *ppEnvelope)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::FlushEnvelope( DWORD dwParamIndex,REFERENCE_TIME refTimeStart,REFERENCE_TIME refTimeEnd)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::SetTimeFormat( GUID guidTimeFormat,MP_TIMEDATA mpTimeData)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetParamCount(DWORD *pdwParams)
{
    if(pdwParams == NULL)
    {
        return E_POINTER;
    }
    *pdwParams = PARAMS_LASTONE;
    return S_OK;
}

STDMETHODIMP CDScaler::GetParamInfo(DWORD dwParamIndex,MP_PARAMINFO *pInfo)
{
    if(pInfo == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex >= PARAMS_LASTONE)
    {
        return E_INVALIDARG;
    }
    memcpy(pInfo, &m_ParamInfos[dwParamIndex], sizeof(MP_PARAMINFO));
    return S_OK;
}

STDMETHODIMP CDScaler::GetParamText(DWORD dwParamIndex,WCHAR **ppwchText)
{
    // \todo get working
    if(ppwchText == NULL)
    {
        return E_POINTER;
    }
    if(dwParamIndex >= PARAMS_LASTONE)
    {
        return E_INVALIDARG;
    }
    if(m_ParamInfos[dwParamIndex].mpType != MPT_ENUM)
    {
            *ppwchText = (WCHAR*)CoTaskMemAlloc(200);
            if(*ppwchText == NULL) return E_OUTOFMEMORY;
            wcscpy(*ppwchText, m_ParamInfos[dwParamIndex].szLabel);
            wcscpy(*ppwchText + wcslen(m_ParamInfos[dwParamIndex].szLabel) + 1,
                    m_ParamInfos[dwParamIndex].szUnitText);
            *(*ppwchText + wcslen(m_ParamInfos[dwParamIndex].szLabel) + wcslen(m_ParamInfos[dwParamIndex].szUnitText) + 1) = L'\0';
            return S_OK;
    }
    else
    {
        if(dwParamIndex == DEINTERLACEMODE)
        {
            *ppwchText = (WCHAR*)CoTaskMemAlloc(2 * m_DeinterlaceNames.length() + 2 + 22 * 2);
            if(*ppwchText == NULL) return E_OUTOFMEMORY;
			memcpy(*ppwchText, L"Deinterlace Mode\0None\0", 22 * 2);
            for(size_t i(0); i < m_DeinterlaceNames.length(); ++i)
            {
                if(m_DeinterlaceNames[i] != L'~')
                {
                    (*ppwchText)[i + 22] = m_DeinterlaceNames[i];
                }
                else
                {
                    (*ppwchText)[i + 22] = 0;
                }
            }
            (*ppwchText)[i + 22] = 0;
            return S_OK;
        }
        else if(dwParamIndex == PULLDOWNMODE)
        {
            wchar_t PulldownText[] = L"Pulldown Mode\0" L"None\0" L"Full Rate Video\0" L"Half Rate Video\0" L"2:2 Pulldown\0" L"3:2 Pulldown\0";
            *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(PulldownText));
            memcpy(*ppwchText, PulldownText, sizeof(PulldownText));
        }
    }
    return S_OK;
}

STDMETHODIMP CDScaler::GetNumTimeFormats(DWORD *pdwNumTimeFormats)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetSupportedTimeFormat(DWORD dwFormatIndex,GUID *pguidTimeFormat)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::GetCurrentTimeFormat( GUID *pguidTimeFormat,MP_TIMEDATA *pTimeData)
{
    // \todo get working
    return E_NOTIMPL;
}

STDMETHODIMP CDScaler::get_Name(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_NAME))
    {
        return Result.CopyTo(Name);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CDScaler::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CDScaler::get_Authors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_AUTHORS))
    {
        return Result.CopyTo(Authors);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

float CDScaler::GetParamFloat(eDScalerFilterParams ParamId)
{
    ATLASSERT(ParamId >= 0 && ParamId < PARAMS_LASTONE);
    ATLASSERT(m_ParamInfos[ParamId].mpType == MPT_FLOAT);
    CProtectCode WhileVarInScope(this);
    return (float)m_ParamValues[ParamId];
}

long CDScaler::GetParamInt(eDScalerFilterParams ParamId)
{
    ATLASSERT(ParamId >= 0 && ParamId < PARAMS_LASTONE);
    ATLASSERT(m_ParamInfos[ParamId].mpType == MPT_INT);
    CProtectCode WhileVarInScope(this);
    return (long)m_ParamValues[ParamId];
}

BOOL CDScaler::GetParamBool(eDScalerFilterParams ParamId)
{
    ATLASSERT(ParamId >= 0 && ParamId < PARAMS_LASTONE);
    ATLASSERT(m_ParamInfos[ParamId].mpType == MPT_BOOL);
    CProtectCode WhileVarInScope(this);
    return (BOOL)m_ParamValues[ParamId];
}

long CDScaler::GetParamEnum(eDScalerFilterParams ParamId)
{
    ATLASSERT(ParamId >= 0 && ParamId < PARAMS_LASTONE);
    ATLASSERT(m_ParamInfos[ParamId].mpType == MPT_ENUM);
    CProtectCode WhileVarInScope(this);
    return (BOOL)m_ParamValues[ParamId];
}

HRESULT CDScaler::LoadDMOs()
{
    CComPtr<IEnumDMO> EnumDMO;

    HRESULT hr = DMOEnum(
                            DMOCATEGORY_VIDEO_EFFECT,
                            0,
                            0,
                            NULL,
                            0,
                            NULL,
                            &EnumDMO
                        );
    CHECK(hr);

    CLSID DMOClsid;
    WCHAR* wszName;
    while((hr = EnumDMO->Next(1, &DMOClsid, &wszName, NULL)) == S_OK)
    {
        CComPtr<IMediaObject> DMO;
        hr = DMO.CoCreateInstance(DMOClsid, NULL, CLSCTX_INPROC);
        if(SUCCEEDED(hr))
        {
            CComQIPtr<IDScalerVideoFilterPlugin> DScalerDMO = DMO;        
            if(DScalerDMO != NULL)
            {
                CComQIPtr<IDeinterlace> DeinterlaceDMO = DMO;        
                if(DeinterlaceDMO != NULL)
                {
                    m_Deinterlacers.push_back(DMO.Detach());
                    m_DeinterlaceNames += wszName;
                    m_DeinterlaceNames += L"~";
                }
                else
                {
                    // \todo need to see if these are film detection
                    // DMO's or where in the chain they should go
                    m_Filters.push_back(DMO.Detach());
                }
            }
            else
            {
                DMO.Release();
            }
        }
        // need to free the name
        CoTaskMemFree(wszName);
    }

    // we need at least one deinterlacing method
    if(m_Deinterlacers.size() > 0)
    {
		m_ParamInfos[DEINTERLACEMODE].mpdMaxValue = (MP_DATA)(m_Deinterlacers.size() - 1);
		if(m_ParamValues[DEINTERLACEMODE] > m_ParamInfos[DEINTERLACEMODE].mpdMaxValue)
		{
			m_ParamValues[DEINTERLACEMODE] = m_ParamInfos[DEINTERLACEMODE].mpdMaxValue;
		}
        return S_OK;
    }
    else
    {
        // \todo better error reporting
        return E_UNEXPECTED;
    }
}

void CDScaler::UnloadDMOs()
{
    // release our hold on all the Deinterlacing DMO's
    EmptyVector(m_Deinterlacers); 
    // release our hold on all the Filter DMO's
    EmptyList(m_Filters); 
}


void CDScaler::EmptyList(std::list<IMediaObject*>& List)
{
    for(std::list<IMediaObject*>::iterator it = List.begin(); 
        it != List.end(); 
        ++it)
    {
        (*it)->Release();
    }
    List.empty();
}

void CDScaler::EmptyVector(std::vector<IMediaObject*>& Vector)
{
    for(std::vector<IMediaObject*>::iterator it = Vector.begin(); 
        it != Vector.end(); 
        ++it)
    {
        (*it)->Release();
    }
    Vector.empty();
}

void CDScaler::SetTypesChangedFlag()
{
    m_TypesChanged = TRUE;
}

HRESULT CDScaler::CheckProcessingLine()
{
    HRESULT hr = S_OK;
    if(m_RebuildRequired == TRUE)
    {
        CProtectCode WhileVarInScope(this);
        hr = RebuildProcessingLine();
        CHECK(hr);
		m_RebuildRequired = FALSE;
    }
    if(m_TypesChanged == TRUE)
    {
        CProtectCode WhileVarInScope(this);
        hr = UpdateTypes();
        CHECK(hr);
		m_TypesChanged = FALSE;
    }
    
    if(m_ChangeTypes)
    {
        m_ChangeTypes = FALSE;
        return S_FALSE;
    }
    else
    {
        return S_OK;
    }
}

HRESULT CDScaler::RebuildProcessingLine()
{
    HRESULT hr = S_OK;
	
	// set it to the new one
	m_CurrentDeinterlacingMethod = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]];
    CComQIPtr<IDScalerVideoFilterPlugin> pVPI = m_CurrentDeinterlacingMethod;
    if(pVPI != NULL)
    {
	    hr = pVPI->get_NumFieldsBuffered(&m_NumberOfFieldsToBuffer);
    }
    else
    {
        m_NumberOfFieldsToBuffer = 1;
    }
	// make sure we set the types up
	m_TypesChanged = TRUE;
    return hr;
}

HRESULT CDScaler::UpdateTypes()
{
    HRESULT hr = S_OK;
	hr = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]]->Flush();
	CHECK(hr);
	hr = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]]->SetInputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	hr = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]]->SetOutputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	hr = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]]->SetInputType(0, &m_InputPin->m_InternalMediaType, 0);
	CHECK(hr);
	hr = m_Deinterlacers[(size_t)m_ParamValues[DEINTERLACEMODE]]->SetOutputType(0, &m_OutputPin->m_CurrentMediaType, 0);
	CHECK(hr);
    return hr;
}

void CDScaler::ResetPullDownIndexRange()
{
    switch((eDeinterlaceType)(long)m_ParamValues[PULLDOWNMODE])
    {
    case FULLRATEVIDEO:
    case HALFRATEVIDEO:
        m_ParamInfos[PULLDOWNINDEX].mpdMaxValue = 0;
        break;
    case PULLDOWN_22:
        m_ParamInfos[PULLDOWNINDEX].mpdMaxValue = 1;
        break;
    case PULLDOWN_32:
        m_ParamInfos[PULLDOWNINDEX].mpdMaxValue = 4;
        break;
    default:
        m_ParamInfos[PULLDOWNINDEX].mpdMaxValue = 0;
        break;
    }
    if(m_ParamValues[PULLDOWNINDEX] > m_ParamInfos[PULLDOWNINDEX].mpdMaxValue)
    {
        m_ParamValues[PULLDOWNINDEX] = m_ParamInfos[PULLDOWNINDEX].mpdMaxValue;
    }
}
