///////////////////////////////////////////////////////////////////////////////
// $Id: DScaler.h,v 1.13 2004-12-18 13:26:48 adcockj Exp $
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

#pragma once 

#include "resource.h"       // main symbols
#include "DSBaseFilter.h"
#include "moreuuids.h"
//#include "Statistics.h"

class CDScaler : 
    public CDSBaseFilter,
    public IInterlacedBufferStack,
    public IAmFreeSoftwareLicensed
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CDScaler, "{0D71870A-7563-11D7-B84A-0002A5623377}", "DScaler Filter Class", "Filter.DScaler.1", "Filter.DScaler", "both")
	IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
	IMPLEMENTS_INTERFACE(IInterlacedBufferStack)
	IMPLEMENTS_INTERFACE(IBaseFilter)
	IMPLEMENTS_INTERFACE(IMediaFilter)
	IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE_AS(IPersist, IPersistStream)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
//    IMPLEMENTS_INTERFACE(IHaveStatistics)
END_INTERFACE_TABLE()

public:
	CDScaler();
    ~CDScaler();


BEGIN_PARAM_LIST()
    DEFINE_PARAM_INT(1, 50, 1, L"None", L"Aspect Ratio Adjustment X")
    DEFINE_PARAM_INT(1, 50, 1, L"None", L"Aspect Ratio Adjustment Y")
    DEFINE_PARAM_BOOL(0, L"Is Input Anamorphic")
    DEFINE_PARAM_ENUM(0, 0, L"Deinterlace Mode")
    DEFINE_PARAM_BOOL(0, L"Manual Pulldown Mode")
    DEFINE_PARAM_ENUM(FULLRATEVIDEO, PULLDOWN_32, L"Pulldown Mode")
    DEFINE_PARAM_INT(0, 4, 0, L"None", L"Pulldown Mode Index")
    DEFINE_PARAM_ENUM(0, 0, L"Film Detect Mode")
END_PARAM_LIST()

//BEGIN_STATISTICS_LIST()
//    DEFINE_STATISTIC(L"Current State")
//    DEFINE_STATISTIC(L"Current Pulldown Index")
//END_STATISTICS_LIST()

    enum eDScalerFilterParams
    {
        ASPECTINCREASEX,
        ASPECTINCREASEY,
        INPUTISANAMORPHIC,
		DEINTERLACEMODE,
		MANUALPULLDOWN,
		PULLDOWNMODE,
		PULLDOWNINDEX,
		FILMDETECTMODE,
        PARAMS_LASTONE,
    };

// IBaseFilter
public:
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);


// IAmFreeSoftwareLicensed
public:
    STDMETHOD(get_Name)(BSTR* Name);
	STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

// IInterlacedBufferStack
public:
    STDMETHOD(get_NumFields)(DWORD* Count);
    STDMETHOD(GetField)(DWORD Index, IInterlacedField** Field);
    STDMETHOD(GetMovementMap)(IMediaBuffer** MovementMap);
    STDMETHOD(PopStack)();
    STDMETHOD(ClearAll)();

// IHaveStatistics
public:
	STDMETHOD(get_StatisticValue)(DWORD Index, BSTR* StatName);

public:
    HRESULT NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin);
    HRESULT NotifyConnected(CDSBasePin* pPin);
    HRESULT ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin);
    HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum);
    bool IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin);
    HRESULT SendOutLastSamples(CDSBasePin* pPin);
    HRESULT Flush(CDSBasePin* pPin);
    HRESULT NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin);
    HRESULT Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin);
    HRESULT GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin);
    HRESULT Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin);
    HRESULT Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin);
    HRESULT QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin);
    HRESULT Activate();
    HRESULT Deactivate();
    
    void SetTypesChangedFlag();
    HRESULT CheckProcessingLine();
    
public:
    BOOL m_IsDirty;
    std::list<IMediaObject*> m_Filters;
    std::vector<IMediaObject*> m_Deinterlacers;
    std::vector<IMediaObject*> m_FilmDetectors;
    SI(IDeinterlace) m_CurrentDeinterlacingMethod;
    SI(IFilmDetect) m_CurrentFilmDetectMethod;
	REFERENCE_TIME m_StartTime;
    std::wstring m_DeinterlaceNames;
    std::wstring m_FilmDetectorNames;
    DWORD m_NumberOfFieldsToBuffer;
    
private:
    enum eHowToProcess
    {
        PROCESS_IGNORE,
        PROCESS_WEAVE,
        PROCESS_DEINTERLACE,
    };

    HRESULT LoadDMOs();
    void UnloadDMOs();
    void EmptyList(std::list<IMediaObject*>& List);
	void EmptyVector(std::vector<IMediaObject*>& Vector);
    HRESULT RebuildProcessingLine();
    HRESULT UpdateTypes();
    HRESULT UpdateTypes(IMediaObject* pDMO);
    void ResetPullDownIndexRange();

    HRESULT InternalProcessOutput();
    eHowToProcess WorkOutHowToProcess(REFERENCE_TIME& FrameEndTime);
    HRESULT WeaveOutput(REFERENCE_TIME& FrameEndTime);
    HRESULT DeinterlaceOutput(REFERENCE_TIME& FrameEndTime);
    HRESULT Weave(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer);
    void WeavePlanarChroma(BYTE* pUpperChroma, BYTE* pLowerChroma, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo);
    HRESULT PushSample(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT ShowNow(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    void ShiftUpSamples(int NumberToShift, IMediaSample* InputSample);
    HRESULT CreateInternalMediaTypes();
    HRESULT UpdateMovementMap();
    void DumpField(IInterlacedField* Field, LPCSTR FileName);
    void DumpMap(LPCSTR FileName);
    void DumpOutput(IMediaBuffer* OutBuffer, LPCSTR FileName);
    void DumpLine(FILE* hFile, BYTE* pData, int Length, int YOffset);

protected:
    class CField: public IInterlacedField
    {
    public:
		CField(){};
		~CField(){};
	    STDMETHOD(GetBufferAndLength)(BYTE** ppBuffer, DWORD* pcbLength);
	    STDMETHOD(GetMaxLength)(DWORD* pcbMaxLength);
	    STDMETHOD(SetLength)(DWORD cbLength);
        STDMETHOD(get_TopFieldFirst)(BOOLEAN* TopFieldFirst);
		STDMETHOD(get_Hint)(eDetectionHint *HintValue);
        STDMETHOD(get_FieldNumber)(DWORD* FieldNumber);
		STDMETHOD(QueryInterface)(const IID& iid, void** pInf) {*pInf = NULL; return S_OK;};
		ULONG STDMETHODCALLTYPE AddRef(void) {return 1;};
		ULONG STDMETHODCALLTYPE Release(void) {return 1;};
		void Clear()
		{
			m_IsTopLine = FALSE;
            m_EndTime = 0;
            m_FieldNumber = 0;
			m_Sample.Detach();
			m_Hint = HINT_NONE;
		};
		const CField& operator=(const CField& RHS)
		{
			m_IsTopLine = RHS.m_IsTopLine;
			m_Sample = RHS.m_Sample;
            m_EndTime = RHS.m_EndTime;
            m_FieldNumber = RHS.m_FieldNumber;
			m_Hint = RHS.m_Hint;
			return *this;
		}
    public:
        BOOL m_IsTopLine;
        SI(IMediaSample) m_Sample;
        REFERENCE_TIME m_EndTime;
        DWORD m_FieldNumber;
		eDetectionHint m_Hint;
    };

    // leave plenty of room in case
	// we get odd field numbers
    CField m_IncomingFields[9];
    DWORD m_FieldsInBuffer;

    class CMap: public IMediaBuffer
    {
    public:
		CMap();
		~CMap();
	    STDMETHOD(GetBufferAndLength)(BYTE** ppBuffer, DWORD* pcbLength);
	    STDMETHOD(GetMaxLength)(DWORD* pcbMaxLength);
	    STDMETHOD(SetLength)(DWORD cbLength);
		STDMETHOD(QueryInterface)(const IID& iid, void** pInf) {*pInf = NULL; return S_OK;};
		ULONG STDMETHODCALLTYPE AddRef(void) {return 1;};
		ULONG STDMETHODCALLTYPE Release(void) {return 1;};
		void ReAlloc(DWORD NewLength);
        void Clear();
        BYTE* GetMap() {return m_Map;};
    public:
        BYTE* m_Map;
        DWORD m_Length;
    };

protected:

    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText);
    HRESULT GetEnumTextDeinterlaceMode(WCHAR **ppwchText);
    HRESULT GetEnumTextPuldownMode(WCHAR **ppwchText);
    HRESULT GetEnumTextFilmDetectMode(WCHAR **ppwchText);

private:
    BOOL m_TypesChanged;
    BOOL m_ChangeTypes;
    BOOL m_RebuildRequired;
    AM_MEDIA_TYPE m_InternalMTInput;
    DWORD m_DetectedPulldownIndex;
    eDeinterlaceType m_DetectedPulldownType;
    DWORD m_NextFieldNumber;
	REFERENCE_TIME m_LastStartEnd;
    REFERENCE_TIME m_FieldTiming;
    CMap m_MovementMap;
};

#define m_VideoInPin m_InputPins[0]
#define m_VideoOutPin ((CDSVideoOutPin*)m_OutputPins[0])
