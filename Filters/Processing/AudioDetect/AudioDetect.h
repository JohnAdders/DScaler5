///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDetect.h,v 1.1 2004-09-10 16:55:46 adcockj Exp $
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

DEFINE_GUID(CLSID_CAudioDetect, 0xe0f74c2b, 0xb3d5, 0x41cd, 0xa1, 0x86, 0x3, 0xb9, 0x60, 0xe9, 0x8f, 0x2c);

class CAudioDetect : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CAudioDetect, "{E0F74C2B-B3D5-41cd-A186-03B960E98F2C}", "DScaler Audio Detect Class", "Filter.AudioDetect.1", "Filter.AudioDetect", "both")
	IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
	IMPLEMENTS_INTERFACE(IBaseFilter)
	IMPLEMENTS_INTERFACE(IMediaFilter)
	IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE_AS(IPersist, IPersistStream)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
END_INTERFACE_TABLE()

public:
	CAudioDetect();
    ~CAudioDetect();

    enum eDetectType
    {
        DETECT_AUTO,
        DETECT_FORCE_PCM,
        DETECT_FORCE_DTS,
        DETECT_FORCE_AC3,
    };

BEGIN_PARAM_LIST()
    DEFINE_PARAM_ENUM(DETECT_FORCE_AC3, DETECT_AUTO, L"Detection Type")
END_PARAM_LIST()

    enum eAudioDetectParams
    {
		DETECT_TYPE,
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
    
public:
    BOOL m_IsDirty;
    
private:

protected:
	enum eStreamType
	{
		STREAM_AC3,
		STREAM_DTS,
		STREAM_PCM,
		STREAM_SILENCE,
	};

	eStreamType m_StreamType;

	bool m_NeedToAttachFormat;
	WAVEFORMATEX m_InternalWF;
	AM_MEDIA_TYPE m_InternalMT;
	DWORD m_SamplesPerSec;

	std::vector<BYTE> m_buff;

protected:

    HRESULT GetEnumTextDetectType(WCHAR **ppwchText);
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText);
	void CreateInternalMediaType();
    HRESULT ProcessPCM();
    HRESULT ProcessAC3();
    HRESULT ProcessDTS();
    HRESULT ProcessSilence();
    HRESULT LookForSync();
	HRESULT GetOutputSampleAndPointer(IMediaSample** pOut, BYTE** ppDataOut, DWORD Len);
	HRESULT Deliver(IMediaSample* pOut);
    void ChangeTypeBasedOnSpdifType(WORD SpdifType);

private:
};

#define m_AudioInPin m_InputPins[0]
#define m_AudioOutPin m_OutputPins[0]
