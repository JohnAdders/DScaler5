///////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.cpp,v 1.19 2005-02-17 09:41:22 adcockj Exp $
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
// Revision 1.18  2005/01/04 17:53:44  adcockj
// added option to force dscalewr filter to be loaded2
//
// Revision 1.17  2004/12/21 14:47:00  adcockj
// fixed connection and settings issues
//
// Revision 1.16  2004/12/15 13:04:08  adcockj
// added simple statistics display
//
// Revision 1.15  2004/12/13 16:59:57  adcockj
// flag based film detection
//
// Revision 1.14  2004/12/07 08:44:00  adcockj
// fixed VS6 issues
//
// Revision 1.13  2004/12/06 18:05:01  adcockj
// Major improvements to deinterlacing
//
// Revision 1.12  2004/11/25 21:55:54  adcockj
// fix issue with interlaced flags on output
//
// Revision 1.11  2004/11/01 14:09:39  adcockj
// More DScaler filter insipred changes
//
// Revision 1.10  2004/08/31 16:33:42  adcockj
// Minor improvements to quality control
// Preparation for next version
// Start on integrating film detect
//
// Revision 1.9  2004/05/06 06:38:07  adcockj
// Interim fixes for connection and PES streams
//
// Revision 1.8  2004/04/28 16:32:37  adcockj
// Better dynamic connection
//
// Revision 1.7  2004/04/20 16:30:31  adcockj
// Improved Dynamic Connections
//
// Revision 1.6  2004/03/08 17:20:05  adcockj
// Minor bug fixes
//
// Revision 1.5  2004/03/06 20:51:10  adcockj
// Correct isue with aspect ratio changes
//
// Revision 1.4  2004/03/05 17:21:32  adcockj
// Better handling of dynamic format changes
//
// Revision 1.3  2004/03/05 15:56:29  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.2  2004/02/12 17:06:45  adcockj
// Libary Tidy up
// Fix for stopping problems
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.15  2003/12/09 11:45:51  adcockj
// Improved implementation of EnumPins
//
// Revision 1.14  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
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
#include "EnumPins.h"
#include "DSInputPin.h"
#include "DSVideoOutPin.h"
#include "MediaBufferWrapper.h"

extern HINSTANCE g_hInstance;

CDScaler::CDScaler() :
    CDSBaseFilter(L"DScaler Filter", 1, 1)
{
    InitMediaType(&m_InternalMTInput);
    m_TypesChanged = TRUE;
    m_RebuildRequired = TRUE;
	m_ChangeTypes = FALSE;
    m_NextFieldNumber = 0;
    m_DetectedPulldownIndex = 0;
    m_DetectedPulldownType = FULLRATEVIDEO;
	m_LastStartEnd = 0;
    m_FieldTiming = 0;
	m_FieldsInBuffer = 0;

    LOG(DBGLOG_FLOW, ("CDScaler::CreatePins\n"));
    
    m_VideoInPin = new CDSInputPin;
    if(m_VideoInPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 1"));
    }
    m_VideoInPin->AddRef();
    m_VideoInPin->SetupObject(this, L"Input");
    
    m_OutputPins[0] = new CDSVideoOutPin();
    if(m_VideoOutPin == NULL)
    {
        throw(std::runtime_error("Can't create memory for pin 2"));
    }
    m_VideoOutPin->AddRef();
    m_VideoOutPin->SetupObject(this, L"Output");

    HRESULT hr = LoadDMOs();
    if(FAILED(hr))
    {
        throw(std::runtime_error("Can't load DMOs"));
    }
    m_CatchUpModulo = 100;
}

CDScaler::~CDScaler()
{
    LOG(DBGLOG_FLOW, ("CDScaler::~CDScaler\n"));
    ClearMediaType(&m_InternalMTInput);
    UnloadDMOs();
}

STDMETHODIMP CDScaler::GetClassID(CLSID __RPC_FAR *pClassID)
{
    LOG(DBGLOG_ALL, ("CDScaler::GetClassID\n"));
    if(pClassID == NULL)
    {
        return E_POINTER;
    }
    *pClassID = CLSID_CDScaler;
    return S_OK;
}

HRESULT CDScaler::ParamChanged(DWORD dwParamIndex)
{
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
	case FILMDETECTMODE:
        m_RebuildRequired = TRUE;
        break;  
    default:
        break;
    }
    return S_OK;
}

HRESULT CDScaler::GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    switch(dwParamIndex)
    {
    case DEINTERLACEMODE:
        return GetEnumTextDeinterlaceMode(ppwchText);
        break;
    case PULLDOWNMODE:
        return GetEnumTextPuldownMode(ppwchText);
        break;
    case FILMDETECTMODE:
        return GetEnumTextFilmDetectMode(ppwchText);
        break;
    }
    return E_NOTIMPL;
}

HRESULT CDScaler::GetEnumTextDeinterlaceMode(WCHAR **ppwchText)
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

HRESULT CDScaler::GetEnumTextFilmDetectMode(WCHAR **ppwchText)
{
    *ppwchText = (WCHAR*)CoTaskMemAlloc(2 * m_FilmDetectorNames.length() + 2 + 22 * 2);
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
	memcpy(*ppwchText, L"Film Detect Mode\0None\0", 22 * 2);
    for(size_t i(0); i < m_FilmDetectorNames.length(); ++i)
    {
        if(m_FilmDetectorNames[i] != L'~')
        {
            (*ppwchText)[i + 22] = m_FilmDetectorNames[i];
        }
        else
        {
            (*ppwchText)[i + 22] = 0;
        }
    }
    (*ppwchText)[i + 22] = 0;
    return S_OK;
}


HRESULT CDScaler::GetEnumTextPuldownMode(WCHAR **ppwchText)
{
    wchar_t PulldownText[] = L"Pulldown Mode\0" L"None\0" L"Full Rate Video\0" L"Half Rate Video\0" L"2:2 Pulldown\0" L"3:2 Pulldown\0";
    *ppwchText = (WCHAR*)CoTaskMemAlloc(sizeof(PulldownText));
    if(*ppwchText == NULL) return E_OUTOFMEMORY;
    memcpy(*ppwchText, PulldownText, sizeof(PulldownText));
    return S_OK;
}

STDMETHODIMP CDScaler::get_Name(BSTR* Name)
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
STDMETHODIMP CDScaler::get_StatisticValue(DWORD Index, BSTR* StatValue)
{
    HRESULT hr = S_OK;
    
    CProtectCode WhileVarInScope(m_VideoInPin);

    switch(Index)
    {
    case 0:
        switch(m_DetectedPulldownType)
        {
        case FULLRATEVIDEO:
            *StatValue = SysAllocString(L"Full Rate Video");
            break;
        case HALFRATEVIDEO:
            *StatValue = SysAllocString(L"Half Rate Video");
            break;
        case PULLDOWN_22:
            *StatValue = SysAllocString(L"2:2 Pulldown");
            break;
        case PULLDOWN_32:
            *StatValue = SysAllocString(L"3:2 Pulldown");
            break;
        }
        break;
    case 1:
        {
            wchar_t Buffer[256];
            wsprintfW(Buffer, L"%d", m_DetectedPulldownIndex);
            *StatValue = SysAllocString(Buffer);
        }
        break;
    default:
        hr = E_UNEXPECTED;
        break;
    }

    return hr;
}


HRESULT CDScaler::LoadDMOs()
{
    SI(IEnumDMO) EnumDMO;

    HRESULT hr = DMOEnum(
                            DMOCATEGORY_VIDEO_EFFECT,
                            0,
                            0,
                            NULL,
                            0,
                            NULL,
                            EnumDMO.GetReleasedInterfaceReference()
                        );
    CHECK(hr);

    CLSID DMOClsid;
    WCHAR* wszName;
    while((hr = EnumDMO->Next(1, &DMOClsid, &wszName, NULL)) == S_OK)
    {
        SI(IMediaObject) DMO;
        hr = DMO.CreateInstance(DMOClsid, CLSCTX_INPROC);
        if(SUCCEEDED(hr))
        {
            SI(IDScalerVideoFilterPlugin) DScalerDMO = DMO;        
            if(DScalerDMO)
            {
                SI(IDeinterlace) DeinterlaceDMO = DMO;        
                if(DeinterlaceDMO)
                {
                    m_Deinterlacers.push_back(DMO.GetAddRefedInterface());
                    m_DeinterlaceNames += wszName;
                    m_DeinterlaceNames += L"~";
                }
                else
                {
                    SI(IFilmDetect) FilmDetectDMO = DMO;        
                    if(FilmDetectDMO)
                    {
                        m_FilmDetectors.push_back(DMO.GetAddRefedInterface());
                        m_FilmDetectorNames += wszName;
                        m_FilmDetectorNames += L"~";
                    }
                    else
                    {
                        // \todo need to see if these are film detection
                        // DMO's or where in the chain they should go
                        m_Filters.push_back(DMO.GetAddRefedInterface());
                    }
                }
            }
            else
            {
                DMO.Detach();
            }
        }
        // need to free the name
        CoTaskMemFree(wszName);
    }

    // we need at least one deinterlacing method
    if(m_Deinterlacers.size() > 0)
    {
		_GetParamList()[DEINTERLACEMODE].MParamInfo.mpdMaxValue = (MP_DATA)(m_Deinterlacers.size() - 1);
		if(_GetParamList()[DEINTERLACEMODE].Value > _GetParamList()[DEINTERLACEMODE].MParamInfo.mpdMaxValue)
		{
			_GetParamList()[DEINTERLACEMODE].Value = _GetParamList()[DEINTERLACEMODE].MParamInfo.mpdMaxValue;
		}
        return S_OK;
    }
    else
    {
        // \todo better error reporting
        return E_UNEXPECTED;
    }

    // and at least one deinterlacing method
    if(m_FilmDetectors.size() > 0)
    {
		_GetParamList()[FILMDETECTMODE].MParamInfo.mpdMaxValue = (MP_DATA)(m_FilmDetectors.size() - 1);
		if(_GetParamList()[FILMDETECTMODE].Value > _GetParamList()[FILMDETECTMODE].MParamInfo.mpdMaxValue)
		{
			_GetParamList()[FILMDETECTMODE].Value = _GetParamList()[FILMDETECTMODE].MParamInfo.mpdMaxValue;
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
    // release our hold on all the Film Detect DMO's
    EmptyVector(m_FilmDetectors); 
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
    if(m_ChangeTypes)
    {
        hr = CreateInternalMediaTypes();
        CHECK(hr);
        m_ChangeTypes = FALSE;
    }
    if(m_TypesChanged == TRUE)
    {
        CProtectCode WhileVarInScope(this);
        hr = UpdateTypes();
        CHECK(hr);
		m_TypesChanged = FALSE;
    }
    return S_OK;
}

HRESULT CDScaler::RebuildProcessingLine()
{
    HRESULT hr = S_OK;
	
	// set it to the new one
	m_CurrentDeinterlacingMethod = m_Deinterlacers[GetParamInt(DEINTERLACEMODE)];
    SI(IDScalerVideoFilterPlugin) pVPI = m_CurrentDeinterlacingMethod;
    if(pVPI)
    {
	    hr = pVPI->get_NumFieldsBuffered(&m_NumberOfFieldsToBuffer);
    }
    else
    {
        m_NumberOfFieldsToBuffer = 1;
    }

    m_CurrentFilmDetectMethod = m_FilmDetectors[GetParamInt(FILMDETECTMODE)];

    pVPI = m_CurrentFilmDetectMethod;
    DWORD FilmBuffersRequired = 0;
    if(pVPI)
    {
	    hr = pVPI->get_NumFieldsBuffered(&FilmBuffersRequired);
        m_NumberOfFieldsToBuffer = max(m_NumberOfFieldsToBuffer, FilmBuffersRequired);
    }

	// make sure we set the types up
	m_TypesChanged = TRUE;
    return hr;
}

HRESULT CDScaler::UpdateTypes()
{
    HRESULT hr = S_OK;
	hr = UpdateTypes(m_Deinterlacers[GetParamInt(DEINTERLACEMODE)]);
	CHECK(hr);
	hr = UpdateTypes(m_FilmDetectors[GetParamInt(FILMDETECTMODE)]);
	CHECK(hr);

    VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)m_InternalMTInput.pbFormat;

    m_MovementMap.ReAlloc(vih2->bmiHeader.biSizeImage);

    return hr;
}

HRESULT CDScaler::UpdateTypes(IMediaObject* pDMO)
{
    HRESULT hr = S_OK;
	pDMO->Flush();
	CHECK(hr);
	pDMO->SetInputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	pDMO->SetOutputType(0, NULL, DMO_SET_TYPEF_CLEAR );
	CHECK(hr);
	pDMO->SetInputType(0, &m_InternalMTInput, 0);
	CHECK(hr);
	pDMO->SetOutputType(0, &m_VideoOutPin->m_ConnectedMediaType, 0);
	CHECK(hr);
    return hr;
}


void CDScaler::ResetPullDownIndexRange()
{
    switch((eDeinterlaceType)GetParamEnum(PULLDOWNMODE))
    {
    case FULLRATEVIDEO:
    case HALFRATEVIDEO:
        _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue = 0;
        break;
    case PULLDOWN_22:
        _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue = 1;
        break;
    case PULLDOWN_32:
        _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue = 4;
        break;
    default:
        _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue = 0;
        break;
    }
    if(GetParamInt(PULLDOWNINDEX) > _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue)
    {
        _GetParamList()[PULLDOWNINDEX].Value = _GetParamList()[PULLDOWNINDEX].MParamInfo.mpdMaxValue;
    }
}

STDMETHODIMP CDScaler::get_NumFields(DWORD* Count)
{
    *Count = m_FieldsInBuffer;
    return S_OK;
}

STDMETHODIMP CDScaler::GetField(DWORD Index, IInterlacedField** Field)
{
    if(*Field != NULL)
    {
        (*Field)->Release();
    }
    if(Index < m_FieldsInBuffer)
    {
        *Field = &(m_IncomingFields[Index]);
        (*Field)->AddRef();
    }
    else
    {
        *Field = NULL;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::GetMovementMap(IMediaBuffer** MovementMap)
{
    *MovementMap = (IMediaBuffer*)&m_MovementMap;
    m_MovementMap.AddRef();
    return S_OK;
}

STDMETHODIMP CDScaler::PopStack()
{
    ASSERT(m_FieldsInBuffer > 0);
    m_IncomingFields[m_FieldsInBuffer - 1].Clear();
    --m_FieldsInBuffer;
    return S_OK;
}

STDMETHODIMP CDScaler::ClearAll()
{
    while(m_FieldsInBuffer)
    {
        PopStack();
    }
    m_NextFieldNumber = 0;
    m_DetectedPulldownIndex = 0;
    m_DetectedPulldownType = FULLRATEVIDEO;
    m_MovementMap.Clear();
    return S_OK;
}

HRESULT CDScaler::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        pProps->cBuffers = 3;
        pProps->cbAlign = 1;
        return S_OK;
    }
    else if(pPin == m_VideoOutPin)
    {
		if(pPin->GetMediaType()->formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pPin->GetMediaType()->pbFormat;
			pProps->cbBuffer = vih->bmiHeader.biSizeImage;
		}
		else if(pPin->GetMediaType()->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pPin->GetMediaType()->pbFormat;
			pProps->cbBuffer = vih->bmiHeader.biSizeImage;
		}

	    pProps->cBuffers = 1;
        pProps->cbAlign = 1;
        return S_OK;
    }
    else
    {
        return E_UNEXPECTED;
    }

}

HRESULT CDScaler::Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin)
{
    return E_FAIL;
}

HRESULT CDScaler::NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin)
{
	m_LastStartEnd = 0;
    m_FieldTiming = 0;
    return S_OK;
}


HRESULT CDScaler::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDScaler::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDScaler::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin)
{
    return E_NOTIMPL;
}

HRESULT CDScaler::Flush(CDSBasePin* pPin)
{
	ClearAll();
    return S_OK;
}

HRESULT CDScaler::SendOutLastSamples(CDSBasePin* pPin)
{
    return InternalProcessOutput();
}


HRESULT CDScaler::InternalProcessOutput()
{
    REFERENCE_TIME FrameEndTime = 0;
	HRESULT hr = S_OK;

	if(m_FieldsInBuffer > 2)
    {
        hr = UpdateMovementMap();
        CHECK(hr)
    }

	if(m_FieldsInBuffer >= m_NumberOfFieldsToBuffer)
	{
        switch(WorkOutHowToProcess(FrameEndTime))
        {
        case PROCESS_IGNORE:
            break;
        case PROCESS_WEAVE:
            hr = WeaveOutput(&FrameEndTime);
            break;
        case PROCESS_DEINTERLACE:
            hr = DeinterlaceOutput(FrameEndTime);
            break;
        }
	}

    while(m_FieldsInBuffer > max(2, m_NumberOfFieldsToBuffer))
    {
        PopStack();
    }
	return hr;
}

HRESULT CDScaler::UpdateMovementMap()
{
    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMTInput.pbFormat);

    SI(IInterlacedField) pNewerBuffer;
    SI(IInterlacedField) pOlderBuffer;

    HRESULT hr = GetField(0, pNewerBuffer.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = GetField(2, pOlderBuffer.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    BOOLEAN IsTopLine = FALSE;

    hr = pNewerBuffer->get_TopFieldFirst(&IsTopLine);
    if(FAILED(hr)) return hr;

    BYTE* pInputDataNewer;
    DWORD InputLengthNewer;
    BYTE* pInputDataOlder;
    DWORD InputLengthOlder;

    BYTE* pMovementMap = m_MovementMap.GetMap();

    
    hr = pNewerBuffer->GetBufferAndLength(&pInputDataNewer, &InputLengthNewer);
    if(FAILED(hr)) return hr;

    hr = pOlderBuffer->GetBufferAndLength(&pInputDataOlder, &InputLengthOlder);
    if(FAILED(hr)) return hr;

    // do 4:2:2 format up here and we need to 
    // worry about deinterlacing both luma and chroma
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
		if(!IsTopLine)
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
            pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            pMovementMap += InputInfo->bmiHeader.biWidth * 2;
        }

        for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
        {
            for(int j(0); j < InputInfo->bmiHeader.biWidth * 2; ++j)
            {
                int Move = pInputDataNewer[j] - pInputDataOlder[j];
                if(Move < 8 && Move > -8)
                {
                    if(pMovementMap[j] < 127)
                    {
                        ++(pMovementMap[j]);
                    }
                }
                else
                {
                    pMovementMap[j] = 0;
                }
            }
            pInputDataNewer += InputInfo->bmiHeader.biWidth * 4;
            pInputDataOlder += InputInfo->bmiHeader.biWidth * 4;
            pMovementMap += InputInfo->bmiHeader.biWidth * 4;
        }
    }
    else
    {
		if(!IsTopLine)
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth;
            pInputDataOlder += InputInfo->bmiHeader.biWidth;
            pMovementMap += InputInfo->bmiHeader.biWidth;
        }
        
        int i;

        for(i = 0; i < InputInfo->bmiHeader.biHeight/2; ++i)
        {
            for(int j(0); j < InputInfo->bmiHeader.biWidth; ++j)
            {
                int Move = pInputDataNewer[j] - pInputDataOlder[j];
                if(Move < 8 && Move > -8)
                {
                    if(pMovementMap[j] < 127)
                    {
                        ++pMovementMap[j];
                    }
                }
                else
                {
                    pMovementMap[j] = 0;
                }
            }
            pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
            pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            pMovementMap += InputInfo->bmiHeader.biWidth * 2;
        }

        if(!IsTopLine)
        {
            pInputDataNewer -= InputInfo->bmiHeader.biWidth;
            pInputDataOlder -= InputInfo->bmiHeader.biWidth;
            pMovementMap -= InputInfo->bmiHeader.biWidth;
        }

        // do chroma movement map as well
        if(!IsTopLine)
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth / 2;
            pInputDataOlder += InputInfo->bmiHeader.biWidth / 2;
            pMovementMap += InputInfo->bmiHeader.biWidth / 2;
        }

        for(i = 0; i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            for(int j(0); j < InputInfo->bmiHeader.biWidth / 2; ++j)
            {
                int Move = pInputDataNewer[j] - pInputDataOlder[j];
                if(Move < 8 && Move > -8)
                {
                    if(pMovementMap[j] < 127)
                    {
                        ++pMovementMap[j];
                    }
                }
                else
                {
                    pMovementMap[j] = 0;
                }
            }
            pInputDataNewer += InputInfo->bmiHeader.biWidth;
            pInputDataOlder += InputInfo->bmiHeader.biWidth;
            pMovementMap += InputInfo->bmiHeader.biWidth;
        }
        
    }

    return hr;
}

HRESULT CDScaler::WeaveOutput(REFERENCE_TIME* FrameEndTime)
{
	HRESULT hr = S_OK;

    hr = m_VideoOutPin->CheckForReconnection();

    SI(IMediaSample) OutSample;

    hr = m_VideoOutPin->GetOutputSample(OutSample.GetReleasedInterfaceReference(), NULL, NULL, m_IsDiscontinuity);
    CHECK(hr);

    // if we get a new type in then we need to change internal type here
    if(hr == S_FALSE)
    {
        CHECK(hr);
    }

	SI(IMediaBuffer) OutBuffer = CMediaBufferWrapper::CreateBuffer(OutSample.GetNonAddRefedInterface());

	DWORD Status(0);

    // reconfigure the filters if the input format has changed
    hr = CheckProcessingLine();
    CHECK(hr);

    hr = Weave((IInterlacedBufferStack*)this, OutBuffer.GetNonAddRefedInterface());

	if(hr == S_OK && !m_VideoInPin->IsFlushing())
	{
        if(FrameEndTime)
        {
            // just in case things go a bit funny
            if(m_LastStartEnd >= *FrameEndTime)
            {
                m_LastStartEnd = *FrameEndTime - 1;
            }
		    hr = OutSample->SetTime(&m_LastStartEnd, FrameEndTime);
		    CHECK(hr);
        
            LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [Weave]\n", 
                            m_LastStartEnd, *FrameEndTime));

            m_LastStartEnd = *FrameEndTime;
        }
        else
        {
		    hr = OutSample->SetTime(NULL, NULL);
		    CHECK(hr);
        }
		
        // finally send the processed sample on it's way
		hr = m_VideoOutPin->m_MemInputPin->Receive(OutSample.GetNonAddRefedInterface());
	}
	// if we are at the start then we might need to send 
	// a couple of frames in before getting a response
	else if(hr == S_FALSE)
	{
		LOG(DBGLOG_FLOW, ("Skipped\n"));
		hr = S_OK;
	}

    return hr;
}


HRESULT CDScaler::DeinterlaceOutput(REFERENCE_TIME& FrameEndTime)
{
	HRESULT hr = S_OK;

    hr = m_VideoOutPin->CheckForReconnection();

    SI(IMediaSample) OutSample;

    hr = m_VideoOutPin->GetOutputSample(OutSample.GetReleasedInterfaceReference(), NULL, NULL, m_IsDiscontinuity);
    CHECK(hr);

	SI(IMediaBuffer) OutBuffer = CMediaBufferWrapper::CreateBuffer(OutSample.GetNonAddRefedInterface());

	DWORD Status(0);

    // reconfigure the filters if the input format has changed
    hr = CheckProcessingLine();
    CHECK(hr);

    hr = m_CurrentDeinterlacingMethod->Process((IInterlacedBufferStack*)this, OutBuffer.GetNonAddRefedInterface());

    if(FALSE)
    {
        DumpField(&m_IncomingFields[0], "Field1.txt");
        DumpField(&m_IncomingFields[1], "Field2.txt");
        DumpMap("map.txt");
        DumpOutput(OutBuffer.GetNonAddRefedInterface(), "Output.txt");
    }

	if(hr == S_OK && m_VideoInPin->IsFlushing() == FALSE)
	{
		hr = OutSample->SetTime(&m_LastStartEnd, &FrameEndTime);
		CHECK(hr);

        LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [Deint]\n", 
                        m_LastStartEnd, FrameEndTime));


        // finally send the processed sample on it's way
		hr = m_VideoOutPin->m_MemInputPin->Receive(OutSample.GetNonAddRefedInterface());
	}
	// if we are at the start then we might need to send 
	// a couple of frames in before getting a response
	else if(hr == S_FALSE)
	{
		LOG(DBGLOG_FLOW, ("Skipped\n"));
		hr = S_OK;
	}

    m_LastStartEnd = FrameEndTime;

    return hr;
}

CDScaler::eHowToProcess CDScaler::WorkOutHowToProcess(REFERENCE_TIME& FrameEndTime)
{
    eHowToProcess HowToProcess = PROCESS_DEINTERLACE;

    //\todo make delay adjustable
    DWORD Delay = 1;

    if(m_FieldsInBuffer > Delay)
    {
        int FrameNum = m_IncomingFields[0].m_FieldNumber;
        FrameEndTime = m_IncomingFields[0].m_EndTime;

        DWORD Index;
        eDeinterlaceType CurrentType;

        if(GetParamBool(CDScaler::MANUALPULLDOWN) == FALSE)
        {
            HRESULT hr = m_CurrentFilmDetectMethod->DetectFilm(this, &CurrentType, & Index);
            if(FAILED(hr))
            {
                return PROCESS_IGNORE;
            }
        }
        else
        {
            // force the manually selected mode
            CurrentType = (eDeinterlaceType)GetParamEnum(CDScaler::PULLDOWNMODE);
            Index = GetParamInt(CDScaler::PULLDOWNINDEX);
        }

        switch(CurrentType)
        {
        case PULLDOWN_32:
            switch((FrameNum + Index) % 5)
            {
            case 1:
                HowToProcess = PROCESS_WEAVE;
                //FrameEndTime += m_FieldTiming / 2;
                break;
            case 4:
                HowToProcess = PROCESS_WEAVE;
                break;
            default:
                HowToProcess = PROCESS_IGNORE;
                break;
            }
            break;
        case PULLDOWN_22:
            if(((FrameNum + Index) & 1) == 1)
            {
                HowToProcess = PROCESS_WEAVE;
            }
            else
            {
                HowToProcess = PROCESS_IGNORE;
            }
            break;
        case HALFRATEVIDEO:
            if((FrameNum & 1) == 0)
            {
                HowToProcess = PROCESS_DEINTERLACE;
            }
            else
            {
                HowToProcess = PROCESS_IGNORE;
            }
            break;
        default:
        case FULLRATEVIDEO:
            break;
        }
    }
    else
    {
        HowToProcess = PROCESS_IGNORE;
    }

    return HowToProcess;
}


HRESULT CDScaler::Activate()
{
    m_CatchUpModulo = 100;
    return S_OK;
}

HRESULT CDScaler::Deactivate()
{
	ClearAll();
    return S_OK;
}


bool CDScaler::IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin)
{
    bool result = false;

    if(pPin == m_VideoInPin || pPin == m_VideoOutPin)
    {
        result = !!(pmt->majortype == MEDIATYPE_Video || pmt->majortype == CLSID_CDScaler);

        result &= (pmt->formattype == FORMAT_VIDEOINFO2 || 
                (pmt->formattype == FORMAT_VideoInfo && pPin == m_VideoInPin));

        result &= (pmt->subtype == MEDIASUBTYPE_YUY2 || 
               pmt->subtype == MEDIASUBTYPE_YV12);
    
		if(result)
		{
            BITMAPINFOHEADER* BitmapInfo;
            if(pmt->formattype == FORMAT_VIDEOINFO2)
            {
                VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
                BitmapInfo = &Format->bmiHeader;
            }
            else
            {
                VIDEOINFOHEADER* Format = (VIDEOINFOHEADER*)pmt->pbFormat;
                BitmapInfo = &Format->bmiHeader;
            }

            // check that the incoming format is SDTV    
            result &= (BitmapInfo->biHeight >= -576 && BitmapInfo->biHeight <= 576);

            //result &= (BitmapInfo->biWidth <= 768);
        }
    }
    return result;
}

HRESULT CDScaler::NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        ClearAll();
        CreateInternalMediaTypes();
		m_RebuildRequired = TRUE;
    }
    else if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyFormatChange(pMediaType);
        m_TypesChanged = TRUE;
    }
    return S_OK;
}

HRESULT CDScaler::NotifyConnected(CDSBasePin* pPin)
{
    if(pPin == m_VideoInPin)
    {
        SI(IKsPropertySet) PropSet = m_VideoInPin->m_ConnectedPin;
        if(PropSet)
        {
            GUID PinCategory = GUID_NULL;
            DWORD cbReturned;
            HRESULT hr = PropSet->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &PinCategory, sizeof(GUID), &cbReturned);
            if(hr == S_OK && PinCategory == PIN_CATEGORY_PREVIEW)
            {
                return VFW_E_NO_INTERFACE;
            }
        }
    }
    else if(pPin == m_VideoOutPin)
    {
        m_VideoOutPin->NotifyConnected();
    }
    return S_OK;
}

HRESULT CDScaler::CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum)
{
    if(pPin == m_VideoOutPin)
    {
        if(!m_VideoInPin->IsConnected()) return VFW_E_NOT_CONNECTED;

        DWORD VideoFlags = VIDEOTYPEFLAG_PREVENT_VIDEOINFOHEADER | VIDEOTYPEFLAG_PROGRESSIVE;
        if(m_InternalMTInput.subtype == MEDIASUBTYPE_YUY2)
        {
            VideoFlags |= VIDEOTYPEFLAG_FORCE_YUY2;
        }
        else
        {
            VideoFlags |= VIDEOTYPEFLAG_FORCE_YV12;
        }
        return m_VideoOutPin->CreateSuitableMediaType(pmt, TypeNum, VideoFlags, 0);
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }

}

HRESULT CDScaler::ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin)
{
    if(pPin != m_VideoInPin)
    {
        return E_UNEXPECTED;
    }

	HRESULT hr = CheckProcessingLine();
	CHECK(hr);

    if((pSampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID) == AM_SAMPLE_TIMEVALID)
    {
        // if there was a discontinuity then we need to ask for the buffer
        // differently 
        if(pSampleProperties->dwSampleFlags & AM_SAMPLE_DATADISCONTINUITY)
	    {
		    m_IsDiscontinuity = true;
            m_LastStartEnd = pSampleProperties->tStart;
	    }

        if((pSampleProperties->dwSampleFlags & AM_SAMPLE_STOPVALID) != AM_SAMPLE_STOPVALID ||
            pSampleProperties->tStop <= pSampleProperties->tStart ||
            (pSampleProperties->tStop - pSampleProperties->tStart) == 1)
	    {
            pSampleProperties->tStop = pSampleProperties->tStart + m_VideoOutPin->GetAvgTimePerFrame();
	    }

        if(m_VideoInPin->m_ConnectedMediaType.formattype == FORMAT_VideoInfo)
        {
            VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMTInput.pbFormat);
            if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
            {
                pSampleProperties->dwTypeSpecificFlags |= AM_VIDEO_FLAG_FIELD1FIRST;
            }
        }

        LOG(DBGLOG_FLOW, ("%010I64d - %010I64d [TypeSpec %x Flags %x]\n", 
                        pSampleProperties->tStart, pSampleProperties->tStop,
                        pSampleProperties->dwTypeSpecificFlags,
                        pSampleProperties->dwSampleFlags));

        hr = PushSample(InSample, pSampleProperties);
    }
    else
    {
        hr = ShowNow(InSample, pSampleProperties);
		m_IsDiscontinuity = true;
    }

	if(FAILED(hr) || hr == S_FALSE)
	{
		hr = ClearAll();
		CHECK(hr);

		m_IsDiscontinuity = true;

		return S_FALSE;
	}

    return hr;
}

HRESULT CDScaler::PushSample(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    HRESULT hr = S_OK;

    if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_REPEAT_FIELD)
    {
        if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST)
        {
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = TRUE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_AFTERNEXT_32;
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = FALSE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + 2 * (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_NEXT_32;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = TRUE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_PREV_32;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
        }
        else
        {
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = FALSE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_AFTERNEXT_32;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = TRUE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + 2 * (InSampleProperties->tStop - InSampleProperties->tStart) / 3;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_NEXT_32;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = FALSE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_PREV_32;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
        }
    }
    else
    {
        if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST)
        {
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = TRUE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_NEXT_22;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = FALSE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_PREV_22;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
        }
        else
        {
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = FALSE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStart + (InSampleProperties->tStop - InSampleProperties->tStart) / 2;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_NEXT_22;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
            ShiftUpSamples(1, InputSample);
            m_IncomingFields[0].m_IsTopLine = TRUE;
            m_IncomingFields[0].m_EndTime = InSampleProperties->tStop;
            if(InSampleProperties->dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE)
            {
                m_IncomingFields[0].m_Hint = WEAVE_WITH_PREV_22;    
            }
            hr = InternalProcessOutput();
            CHECK(hr);
        }
    }
    return S_OK;
}

HRESULT CDScaler::ShowNow(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties)
{
    HRESULT hr = S_OK;

    ShiftUpSamples(1, InputSample);
    m_IncomingFields[0].m_IsTopLine = TRUE;
    m_IncomingFields[0].m_EndTime = 0;
    ShiftUpSamples(1, InputSample);
    m_IncomingFields[0].m_IsTopLine = FALSE;
    m_IncomingFields[0].m_EndTime = 0;
    hr = WeaveOutput(NULL);
    CHECK(hr);
    PopStack();
    PopStack();
    return S_OK;
}


void CDScaler::ShiftUpSamples(int NumberToShift, IMediaSample* InputSample)
{
    if(m_FieldsInBuffer > 0)
    {
        for(int i(m_FieldsInBuffer - 1); i >= 0; --i)
		{
			m_IncomingFields[i + NumberToShift] = m_IncomingFields[i];
		}
    }
    m_FieldsInBuffer += NumberToShift;
    while(NumberToShift--)
    {
        m_IncomingFields[NumberToShift].Clear();
        m_IncomingFields[NumberToShift].m_Sample = InputSample;
        m_IncomingFields[NumberToShift].m_FieldNumber = m_NextFieldNumber++;
    }
}

HRESULT CDScaler::CreateInternalMediaTypes()
{
    const AM_MEDIA_TYPE* InputType = m_VideoInPin->GetMediaType();
    BITMAPINFOHEADER* BitmapInfo = NULL;
    m_InternalMTInput.majortype = MEDIATYPE_Video;
    m_InternalMTInput.subtype = InputType->subtype;
    m_InternalMTInput.bFixedSizeSamples = TRUE;
    m_InternalMTInput.bTemporalCompression = FALSE;
    m_InternalMTInput.formattype = FORMAT_VIDEOINFO2;
    m_InternalMTInput.cbFormat = sizeof(VIDEOINFOHEADER2);
    VIDEOINFOHEADER2* NewFormat = (VIDEOINFOHEADER2*)CoTaskMemAlloc(sizeof(VIDEOINFOHEADER2));
    if(NewFormat == NULL)
    {
        ClearMediaType(&m_InternalMTInput);
        return E_OUTOFMEMORY;
    }
    ZeroMemory(NewFormat, sizeof(VIDEOINFOHEADER2));
    m_InternalMTInput.pbFormat = (BYTE*)NewFormat;

    NewFormat->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeBobOrWeave;

    if(InputType->formattype == FORMAT_VIDEOINFO2)
    {
        VIDEOINFOHEADER2* OldFormat = (VIDEOINFOHEADER2*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;
        NewFormat->dwPictAspectRatioX = OldFormat->dwPictAspectRatioX;
        NewFormat->dwPictAspectRatioY = OldFormat->dwPictAspectRatioY;
        NewFormat->dwBitRate = OldFormat->dwBitRate;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame;
    }
    else if(InputType->formattype == FORMAT_VideoInfo)
    {
        m_InternalMTInput.pbFormat =(BYTE*)NewFormat;
        VIDEOINFOHEADER* OldFormat = (VIDEOINFOHEADER*)InputType->pbFormat;
        BitmapInfo = &OldFormat->bmiHeader;

        // if the input format is a known TV style one then
        // it should be assumed to be 4:3
        if((BitmapInfo->biWidth == 704)  &&
           (BitmapInfo->biHeight == 480 || BitmapInfo->biHeight == 576))
        {
            // adjustment to make 704 video the same shape as 720
            NewFormat->dwPictAspectRatioX = 4 * 44;
            NewFormat->dwPictAspectRatioY = 3 * 45;
        }
        else if((BitmapInfo->biWidth == 704 || BitmapInfo->biWidth == 720 || BitmapInfo->biWidth == 768) &&
            (BitmapInfo->biHeight == 480 || BitmapInfo->biHeight == 576))
        {
            NewFormat->dwPictAspectRatioX = 4;
            NewFormat->dwPictAspectRatioY = 3;
        }
        else
        {
            // first guess is square pixels
            NewFormat->dwPictAspectRatioX = BitmapInfo->biWidth;
            NewFormat->dwPictAspectRatioY = BitmapInfo->biHeight;

            // the update the aspect ratio with the pels per meter info if both
            // are present, this was the old way of handling aspect ratio
            if(BitmapInfo->biXPelsPerMeter > 0 && BitmapInfo->biYPelsPerMeter > 0)
            {
                NewFormat->dwPictAspectRatioX *= BitmapInfo->biYPelsPerMeter;
                NewFormat->dwPictAspectRatioY *= BitmapInfo->biXPelsPerMeter;
            }
        }

        NewFormat->dwBitRate = OldFormat->dwBitRate;
        NewFormat->dwBitErrorRate = OldFormat->dwBitErrorRate;
        NewFormat->AvgTimePerFrame = OldFormat->AvgTimePerFrame;
    }
    else
    {
        ClearMediaType(&m_InternalMTInput);
        return E_UNEXPECTED;
    }

    if(BitmapInfo->biHeight == 576)
    {
        NewFormat->dwInterlaceFlags |= AMINTERLACE_Field1First;
    }    


    if(GetParamBool(INPUTISANAMORPHIC) != 0)
    {
        NewFormat->dwPictAspectRatioX *= 4;
        NewFormat->dwPictAspectRatioY *= 3;
    }

	memcpy(&NewFormat->bmiHeader, BitmapInfo, sizeof(BITMAPINFOHEADER));

    m_InternalMTInput.lSampleSize = BitmapInfo->biSizeImage;


    m_VideoOutPin->SetAspectX(NewFormat->dwPictAspectRatioX);
    m_VideoOutPin->SetAspectY(NewFormat->dwPictAspectRatioY);
    m_VideoOutPin->SetHeight(BitmapInfo->biHeight);
    m_VideoOutPin->SetWidth(BitmapInfo->biWidth);
    m_VideoOutPin->SetAvgTimePerFrame(NewFormat->AvgTimePerFrame);

    m_TypesChanged = true;

    return S_OK;
}

void CDScaler::DumpField(IInterlacedField* Field, LPCSTR FileName)
{
    FILE* hFile = fopen(FileName, "w");

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMTInput.pbFormat);

    BOOLEAN IsTopLine = FALSE;

    Field->get_TopFieldFirst(&IsTopLine);

    BYTE* pInputData;
    DWORD InputLength;
    
    Field->GetBufferAndLength(&pInputData, &InputLength);

    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
        if(!IsTopLine)
        {
            pInputData += InputInfo->bmiHeader.biWidth * 2;
        }

        for(int i = 0; i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            DumpLine(hFile, pInputData, InputInfo->bmiHeader.biWidth, 2);
            pInputData += InputInfo->bmiHeader.biWidth * 4;
        }
    }
    else
    {
        if(!IsTopLine)
        {
            pInputData += InputInfo->bmiHeader.biWidth;
        }

        int i;
        for(i = 0; i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            DumpLine(hFile, pInputData, InputInfo->bmiHeader.biWidth, 1);
            pInputData += InputInfo->bmiHeader.biWidth * 2;
        }
        for(i = 0; i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            DumpLine(hFile, pInputData, InputInfo->bmiHeader.biWidth / 2, 1);
            pInputData += InputInfo->bmiHeader.biWidth;
        }
    }

    fclose(hFile);
}

void CDScaler::DumpMap(LPCSTR FileName)
{
    FILE* hFile = fopen(FileName, "w");

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMTInput.pbFormat);

    BYTE* pMapData = m_MovementMap.GetMap();

    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
        for(int i = 0; i < InputInfo->bmiHeader.biHeight; ++i)
        {
            DumpLine(hFile, pMapData, InputInfo->bmiHeader.biWidth * 2, 1);
            pMapData += InputInfo->bmiHeader.biWidth;
        }
    }
    else
    {
        int i;
        for(i = 0; i < InputInfo->bmiHeader.biHeight; ++i)
        {
            DumpLine(hFile, pMapData, InputInfo->bmiHeader.biWidth, 1);
            pMapData += InputInfo->bmiHeader.biWidth;
        }
        for(i = 0; i < InputInfo->bmiHeader.biHeight/2; ++i)
        {
            DumpLine(hFile, pMapData, InputInfo->bmiHeader.biWidth/2, 1);
            pMapData += InputInfo->bmiHeader.biWidth/2;
        }
        for(i = 0; i < InputInfo->bmiHeader.biHeight/2; ++i)
        {
            DumpLine(hFile, pMapData, InputInfo->bmiHeader.biWidth/2, 1);
            pMapData += InputInfo->bmiHeader.biWidth/2;
        }
    }

    fclose(hFile);
}

void CDScaler::DumpOutput(IMediaBuffer* OutBuffer, LPCSTR FileName)
{
    FILE* hFile = fopen(FileName, "w");

    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(m_VideoOutPin->m_ConnectedMediaType.pbFormat);

    BOOLEAN IsTopLine = FALSE;

    BYTE* pData;
    DWORD Length;
    
    OutBuffer->GetBufferAndLength(&pData, &Length);

    if(OutputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
        for(int i = 0; i < abs(OutputInfo->bmiHeader.biHeight); ++i)
        {
            DumpLine(hFile, pData, OutputInfo->bmiHeader.biWidth, 2);
            pData += OutputInfo->bmiHeader.biWidth * 2;
        }
    }
    else
    {
        for(int i = 0; i < abs(OutputInfo->bmiHeader.biHeight); ++i)
        {
            DumpLine(hFile, pData, OutputInfo->bmiHeader.biWidth, 1);
            pData += OutputInfo->bmiHeader.biWidth;
        }
    }

    fclose(hFile);
}

void CDScaler::DumpLine(FILE* hFile, BYTE* pData, int Length, int YOffset)
{
    for(int i = 0; i < Length; ++i)
    {
        fprintf(hFile, "%02x ", pData[i * YOffset]);
    }
    fprintf(hFile, "\n");
}
