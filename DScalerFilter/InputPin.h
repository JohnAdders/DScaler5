///////////////////////////////////////////////////////////////////////////////
// $Id: InputPin.h,v 1.11 2003-10-31 17:19:37 adcockj Exp $
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

#include "EnumMediaTypes.h"
#include "InputMemAlloc.h"

class CDScaler;
class COutputPin;

/////////////////////////////////////////////////////////////////////////////
// CInputPin
class ATL_NO_VTABLE CInputPin : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IPin,
    public IMemInputPin,
    public IPinConnection,
    public IUpdateMediaTypes,
    public IInterlacedBufferStack
{
public:
	CInputPin();
	~CInputPin();


BEGIN_COM_MAP(CInputPin)
	COM_INTERFACE_ENTRY(IPin)
    COM_INTERFACE_ENTRY(IMemInputPin)
    COM_INTERFACE_ENTRY(IPinConnection)
	COM_INTERFACE_ENTRY(IInterlacedBufferStack)
END_COM_MAP()

// IPin
public:
    STDMETHOD(Connect)(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(ReceiveConnection)(IPin *pConnector, const AM_MEDIA_TYPE *pmt);
    STDMETHOD(Disconnect)(void);
    STDMETHOD(ConnectedTo)(IPin **pPin);
    STDMETHOD(ConnectionMediaType)(AM_MEDIA_TYPE *pmt);
    STDMETHOD(QueryPinInfo)(PIN_INFO *pInfo);
    STDMETHOD(QueryDirection)(PIN_DIRECTION *pPinDir);
    STDMETHOD(QueryId)(LPWSTR *Id);
    STDMETHOD(QueryAccept)(const AM_MEDIA_TYPE *pmt);
    STDMETHOD(EnumMediaTypes)(IEnumMediaTypes **ppEnum);
    STDMETHOD(QueryInternalConnections)(IPin **apPin, ULONG *nPin);
    STDMETHOD(EndOfStream)(void);
    STDMETHOD(BeginFlush)(void);
    STDMETHOD(EndFlush)(void);
    STDMETHOD(NewSegment)(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

// IMemInputPin
public:
    STDMETHOD(GetAllocator)(IMemAllocator **ppAllocator);
    STDMETHOD(NotifyAllocator)(IMemAllocator *pAllocator, BOOL bReadOnly);
    STDMETHOD(GetAllocatorRequirements)(ALLOCATOR_PROPERTIES *pProps);
    STDMETHOD(Receive)(IMediaSample *pSample);
    STDMETHOD(ReceiveMultiple)(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
    STDMETHOD(ReceiveCanBlock)(void);

// IPinConnection
public:
    STDMETHOD(DynamicQueryAccept)(const AM_MEDIA_TYPE *pmt);
    STDMETHOD(NotifyEndOfStream)(HANDLE hNotifyEvent);
    STDMETHOD(IsEndPin)(void);
    STDMETHOD(DynamicDisconnect)(void);

// IInterlacedBufferStack
public:
    STDMETHOD(get_NumFields)(DWORD* Count);
    STDMETHOD(GetField)(DWORD Index, IInterlacedField** Field);
    STDMETHOD(PopStack)();
    STDMETHOD(ClearAll)();

public:
    ULONG FormatVersion();
    HRESULT SetTypes(ULONG& NumTypes, AM_MEDIA_TYPE* Types);
    HRESULT FinishProcessing();
    HRESULT SetInputType(const AM_MEDIA_TYPE *pmt);

public:
    BITMAPINFOHEADER* GetBitmapInfo();
    CDScaler* m_Filter;
    COutputPin* m_OutputPin;
    CComPtr<IPin> m_ConnectedPin;
    CComPtr<IMemAllocator> m_Allocator;
    BOOL m_Flushing;
    BOOL m_bReadOnly;
    AM_MEDIA_TYPE m_InputMediaType;
    AM_MEDIA_TYPE m_InternalMediaType;
    HANDLE m_NotifyEvent;
    ULONG m_FormatVersion;
    DWORD m_VideoSampleFlag;
    DWORD m_VideoSampleMask;
    BOOL m_Block;
    HANDLE m_BlockEvent;
    ALLOCATOR_PROPERTIES m_AllocatorProperties;
	DWORD m_dwFlags;
	CComPtr<IMemAllocator> m_MyMemAlloc;
	REFERENCE_TIME m_ExpectedStartIn;
	REFERENCE_TIME m_LastStartEnd;
    REFERENCE_TIME m_FieldTiming;
	BYTE m_Counter;
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
		STDMETHOD(QueryInterface)(const IID& iid, void** pInf) {*pInf = NULL; return S_OK;};
		ULONG STDMETHODCALLTYPE AddRef(void) {return 1;};
		ULONG STDMETHODCALLTYPE Release(void) {return 1;};
		void Clear()
		{
			IsTopLine = FALSE;
			m_Sample.Release();
		};
		const CField& operator=(const CField& RHS)
		{
			IsTopLine = RHS.IsTopLine;
			m_Sample = RHS.m_Sample;
			return *this;
		}
    public:
        BOOL IsTopLine;
        CComPtr<IMediaSample> m_Sample;
        REFERENCE_TIME m_EndTime;
        DWORD m_FieldNumber;
    };

    CField m_IncomingFields[6];
    DWORD m_FieldsInBuffer;

private:
    enum eHowToProcess
    {
        PROCESS_IGNORE,
        PROCESS_WEAVE,
        PROCESS_DEINTERLACE,
    };

    enum eSourceType
    {
        SOURCE_DEFAULT,
        SOURCE_ELECARD,
        SOURCE_SONIC,
        SOURCE_NVDVD,
        SOURCE_WINDVD,
        SOURCE_DV,
    };

    void InternalDisconnect();
    eHowToProcess WorkOutHowToProcess(REFERENCE_TIME& FrameEndTime);
    HRESULT WeaveOutput(REFERENCE_TIME& FrameEndTime);
    HRESULT DeinterlaceOutput(REFERENCE_TIME& FrameEndTime);

    STDMETHOD(Weave)(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer);
    void ProcessPlanarChroma(BYTE* pInputData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo);

    void GuessInterlaceFlags(AM_SAMPLE2_PROPERTIES* Props);
    HRESULT GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
    HRESULT SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
    BOOL IsThisATypeWeWorkWith(const AM_MEDIA_TYPE *pmt);
	void FixupMediaType(AM_MEDIA_TYPE *pmt);
    void CheckForBlocking();
    HRESULT InternalReceive(IMediaSample *InSample);
    HRESULT InternalProcessOutput(BOOL HurryUp);
    HRESULT CreateInternalMediaType(const AM_MEDIA_TYPE* InputType, AM_MEDIA_TYPE* NewType);
    HRESULT UpdateMediaTypeInFilters();
    HRESULT GetOutputSample(IMediaSample** OutSample);
	BOOL WorkOutWhoWeAreTalkingTo(IPin* pConnector);
    DWORD m_NextFieldNumber;

    HRESULT PushSample(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT PushSampleDefault(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT PushSampleElecard(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT PushSampleSonic(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT PushSampleNVDVD(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    HRESULT PushSampleWinDVD(IMediaSample* InputSample, AM_SAMPLE2_PROPERTIES* InSampleProperties);
    void ShiftUpSamples(int NumberToShift, IMediaSample* InputSample);
    eSourceType m_SourceType;
};

