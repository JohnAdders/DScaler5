///////////////////////////////////////////////////////////////////////////////
// $Id$
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

namespace libfaad
{
    #include "libfaad\faad.h"
}

namespace ffmpeg
{
    extern "C"
    {
        #ifdef  _MSC_VER
            #pragma warning(disable: 4244)
        #endif

        #include "avcodec.h"

        #ifdef  _MSC_VER
            #pragma warning(default: 4244)
        #endif
    }
}


DEFINE_GUID(CLSID_CAudioDecoder, 0xd2ca75c2, 0x5a1, 0x4915, 0x88, 0xa8, 0xd4, 0x33, 0xf8, 0x76, 0xd1, 0x86);

class CAudioDecoder :
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IAMFilterMiscFlags
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
    //IMPLEMENTS_INTERFACE(IAMFilterMiscFlags)
END_INTERFACE_TABLE()

public:
    CAudioDecoder();
    ~CAudioDecoder();

BEGIN_PARAM_LIST()
    DEFINE_PARAM_ENUM(SPCFG_3F2R1S, SPCFG_DOLBY, L"Speaker Config")
    DEFINE_PARAM_BOOL(1, L"Dynamic Range Control")
    DEFINE_PARAM_BOOL(0, L"Use SPDIF for AC3 & DTS")
    DEFINE_PARAM_BOOL(0, L"MPEG Audio over SPDIF")
    DEFINE_PARAM_INT(-5000, 5000, 0, L"ms", L"SPDIF Audio Time Offset")
#if defined(LIBDTS_FIXED)
    DEFINE_PARAM_ENUM(CONNECT_32, CONNECT_32, L"Preferred Connection Type")
#else
    DEFINE_PARAM_ENUM(CONNECT_IEEE, CONNECT_IEEE, L"Preferred Connection Type")
#endif
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

    enum eConnectType
    {
        CONNECT_16,
        CONNECT_24,
        CONNECT_32,
        CONNECT_IEEE,
    };

    enum eMpegAudioParams
    {
        SPEAKERCONFIG,
        DYNAMICRANGECONTROL,
        USESPDIF,
        MPEGOVERSPDIF,
        AUDIOTIMEOFFSET,
        CONNECTTYPE,
    };

    enum eOutputSampleType
    {
        OUTSAMPLE_FLOAT,
        OUTSAMPLE_32BIT,
        OUTSAMPLE_24BIT,
        OUTSAMPLE_16BIT,
        OUTSAMPLE_LASTONE
    };

    enum eProcessingType
    {
        PROCESS_AC3,
        PROCESS_MPA,
        PROCESS_DTS,
        PROCESS_PCM,
        PROCESS_AAC,
    };

public:
    // IBaseFilter
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);

    ULONG __stdcall GetMiscFlags()
    {
        return AM_FILTER_MISC_FLAGS_IS_SOURCE;
    }

public:
    // IAmFreeSoftwareLicensed
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



protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText);

protected:
    liba52::a52_state_t* m_a52_state;
    libdts::dts_state_t* m_dts_state;

    libfaad::faacDecHandle m_aac_handle;
    bool m_aac_init;


    struct libmad::mad_stream m_stream;
    struct libmad::mad_frame m_frame;
    struct libmad::mad_synth m_synth;
    bool m_madinit;

    ffmpeg::AVCodec* m_Codec;
    ffmpeg::AVCodecContext* m_CodecContext;

    std::vector<BYTE> m_buff;
    REFERENCE_TIME m_rtNextFrameStart;
    REFERENCE_TIME m_rtOutputStart;

    HRESULT ProcessLPCM();
    HRESULT ProcessAC3();
    HRESULT ProcessDTS();
    HRESULT ProcessMPA();
    HRESULT ProcessAAC();

    void InitAC3();
    void InitDTS();
    void InitMPA();
    void InitAAC();

    void FinishAC3();
    void FinishDTS();
    void FinishMPA();
    void FinishAAC();

    HRESULT Deliver(bool IsSpdif);

    HRESULT GetEnumTextSpeakerConfig(WCHAR **ppwchText);
    HRESULT GetEnumTextConnectType(WCHAR **ppwchText);

    HRESULT CreateInternalSPDIFMediaType(DWORD nSamplesPerSec, WORD BitsPerSample);
    HRESULT CreateInternalPCMMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask, WORD BitsPerSample);
    HRESULT CreateInternalIEEEMediaType(DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask);
    HRESULT GetOutputSampleAndPointer();
    BOOL IsMediaTypeAC3(const AM_MEDIA_TYPE* pMediaType);
    BOOL IsMediaTypeDTS(const AM_MEDIA_TYPE* pMediaType);
    BOOL IsMediaTypeMP3(const AM_MEDIA_TYPE* pMediaType);
    BOOL IsMediaTypePCM(const AM_MEDIA_TYPE* pMediaType);
    BOOL IsMediaTypeAAC(const AM_MEDIA_TYPE* pMediaType);
    HRESULT SendDigitalData(WORD HeaderWord, short DigitalLength, long FinalLength, const char* pData);
    HRESULT UpdateStartTime();
    void InitLibraries();
    void FinishLibraries();

private:
    // Rate change Stuff
    AM_SimpleRateChange m_rate;
    AM_SimpleRateChange m_ratechange;
    bool m_CorrectTS;
    HRESULT SetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData);
    HRESULT GetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT SupportPropSetRate(DWORD dwPropID, DWORD *pTypeSupport);

    // logging for ffmpeg
    static void __cdecl avlog(void*,int,const char*,va_list);

private:
    AM_MEDIA_TYPE m_InternalMT;
    WAVEFORMATEXTENSIBLE m_InternalWFE;
    bool m_NeedToAttachFormat;
    eOutputSampleType m_OutputSampleType;
    int m_SampleSize;
    DWORD m_InputSampleRate;
    int m_OutputSampleRate;
    bool m_ConnectedAsSpdif;
    DWORD m_ChannelMask;
    int m_ChannelsRequested;
    bool m_CanReconnect;
    bool m_DownSample;
    bool m_Preroll;
    long m_BytesLeftInBuffer;
    SI(IMediaSample) m_CurrentOutputSample;
    BYTE* m_pDataOut;
    long m_BufferSizeAtFrameStart;
    long m_AC3SilenceFrames;
    eProcessingType m_ProcessingType;
};

#define m_AudioInPin m_InputPins[0]
#define m_AudioOutPin m_OutputPins[0]
