///////////////////////////////////////////////////////////////////////////////
// $Id: AudioDecoder.h,v 1.9 2004-04-06 20:08:04 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// MpegAudio.dll - DirectShow filter for decoding Mpeg audio streams
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

#pragma once 

#include "resource.h"       // main symbols
#include "DSBaseFilter.h"

namespace libmad
{
    #include "libmad\msvc++\mad.h"
}


namespace liba52
{
    #include "a52dec\vc++\inttypes.h"
    extern "C" 
    {
        #include "a52dec\include\a52.h"
    }
    #include "a52dec\include\mm_accel.h"
}


namespace libdts
{
    #include "dtsdec\vc++\inttypes.h"
    extern "C" 
    {
        #include "dtsdec\include\dts.h"
    }
}

DEFINE_GUID(CLSID_CAudioDecoder, 0xd2ca75c2, 0x5a1, 0x4915, 0x88, 0xa8, 0xd4, 0x33, 0xf8, 0x76, 0xd1, 0x86);

class CAudioDecoder : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CAudioDecoder, "{D2CA75C2-05A1-4915-88A8-D433F876D186}", "MpegAudio Filter", "Filter.MpegAudio.1", "Filter.MpegAudio", "both")
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
	CAudioDecoder();
    ~CAudioDecoder();

BEGIN_PARAM_LIST()
    DEFINE_PARAM_ENUM(SPCFG_3F2R1S, SPCFG_DOLBY, L"Speaker Config")
    DEFINE_PARAM_BOOL(1, L"Dynamic Range Control")
    DEFINE_PARAM_BOOL(1, L"Normalize")
    DEFINE_PARAM_BOOL(0, L"Use SPDIF for AC3 & DTS")
    DEFINE_PARAM_BOOL(1, L"Jitter Remover")
    DEFINE_PARAM_BOOL(0, L"MPEG Audio over SPDIF")
END_PARAM_LIST()

public:
    enum eSpeakerConfig
    {
        SPCFG_STEREO,
        SPCFG_DOLBY,
        SPCFG_2F2R,
        SPCFG_2F2R1S,
        SPCFG_3F2R,
        SPCFG_3F2R1S,
    };

    enum eMpegAudioParams
    {
        SPEAKERCONFIG,
        DYNAMICRANGECONTROL,
        NORMALIZE,
        USESPDIF,
        JITTERREMOVER,
        MPEGOVERSPDIF,
    };

    enum eOutputSampleType
    {
        OUTSAMPLE_FLOAT,
        OUTSAMPLE_32BIT,
        OUTSAMPLE_24BIT,
        OUTSAMPLE_16BIT,
        OUTSAMPLE_LASTONE
    };

public:
    // IBaseFilter
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);

public:
    // IAmFreeSoftwareLicensed
    STDMETHOD(get_Name)(BSTR* Name);
	STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

public:
    HRESULT NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin);
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

protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText);

protected:
	liba52::a52_state_t* m_a52_state;
    libdts::dts_state_t* m_dts_state;

	struct libmad::mad_stream m_stream;
	struct libmad::mad_frame m_frame;
	struct libmad::mad_synth m_synth;

	std::vector<BYTE> m_buff;
	REFERENCE_TIME m_rtNextFrameStart;
	REFERENCE_TIME m_rtOutputStart;
    DWORD m_OutputBufferSize;

	double m_sample_max;

	HRESULT ProcessLPCM();
	HRESULT ProcessAC3();
	HRESULT ProcessDTS();
	HRESULT ProcessMPA();
    HRESULT Deliver(IMediaSample* pOut, REFERENCE_TIME rtDur);
    HRESULT ReconnectOutput(DWORD Len);

    HRESULT GetEnumTextSpeakerConfig(WCHAR **ppwchText);

    HRESULT CreateInternalSPDIFMediaType(DWORD nSamplesPerSec, WORD BitsPerSample);
    HRESULT CreateInternalPCMMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask, WORD BitsPerSample);
    HRESULT CreateInternalIEEEMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask);
    HRESULT GetOutputSampleAndPointer(IMediaSample** pOut, BYTE** ppDataOut, DWORD Len);

private:
    AM_MEDIA_TYPE m_InternalMT;
	WAVEFORMATEXTENSIBLE m_InternalWFE;
    bool m_NeedToAttachFormat;
    eOutputSampleType m_OutputSampleType;
    int m_SampleSize;

};

#define m_AudioInPin m_InputPins[0]
#define m_AudioOutPin m_OutputPins[0]
