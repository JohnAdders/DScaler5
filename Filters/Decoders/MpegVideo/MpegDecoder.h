///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder.h,v 1.1 2004-02-06 12:17:16 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// MpegVideo.dll - DirectShow filter for decoding Mpeg2 streams
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

extern "C"
{
#include "./mpeg2dec/vc++/inttypes.h"
#include "./mpeg2dec/include/mpeg2.h"
}


DEFINE_GUID(CLSID_CMpegDecoder, 0xf8904f1f, 0x371, 0x4471, 0x88, 0x66, 0x90, 0xe6, 0x28, 0x1a, 0xbd, 0xb6);

class CMpegDecoder : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IAMDecoderCaps
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CMpegDecoder, "{F8904F1F-0371-4471-8866-90E6281ABDB6}", "MpegVideo Filter", "Filter.MpegVideo.1", "Filter.MpegVideo", "both")
	IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
	IMPLEMENTS_INTERFACE(IBaseFilter)
	IMPLEMENTS_INTERFACE(IMediaFilter)
	IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE(IAMDecoderCaps)
END_INTERFACE_TABLE()

public:
	CMpegDecoder();
    ~CMpegDecoder();


BEGIN_PARAM_LIST()
END_PARAM_LIST()

public:
    // IBaseFilter
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);

public:
    // IAmFreeSoftwareLicensed
    STDMETHOD(get_Name)(BSTR* Name);
	STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

public:
    // IAMDecoderCaps
	STDMETHOD(GetDecoderCaps)(DWORD dwCapIndex, DWORD* lpdwCap);

public:
    HRESULT NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin);
    HRESULT ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin);
    HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum);
    bool IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin);
    HRESULT SendOutLastSamples(CDSBasePin* pPin);
    HRESULT FinishProcessing(CDSBasePin* pPin);
    HRESULT NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin);
    HRESULT Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin);
    HRESULT GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin);
    HRESULT Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin);
    HRESULT Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin);
    HRESULT QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin);

protected:
    HRESULT ParamChanged(DWORD dwParamIndex);

private:
	mpeg2dec_t* m_dec;
	REFERENCE_TIME m_AvgTimePerFrame;
	REFERENCE_TIME m_LastOutputTime;
	bool m_fWaitForKeyFrame;
	bool m_fFilm;
	bool m_ProgressiveChroma;
	bool m_Discont;
	CCanLock m_DeliverLock;
	
    typedef enum 
	{
		CHROMA_420,
		CHROMA_422,
		CHROMA_444
	} eChromaType;
	eChromaType m_ChromaType;

	struct framebuf 
	{
		int w, h, pitch;
		BYTE* buf[6];
		void* actualbuf[6];
		REFERENCE_TIME rtStart, rtStop;
		DWORD flags;
		unsigned int nb_fields;
        framebuf()
		{
			w = h = pitch = 0;
			memset(&buf, 0, sizeof(buf));
			memset(&actualbuf, 0, sizeof(actualbuf));
			rtStart = rtStop = 0;
			flags = 0;
			nb_fields = 0;
		}
        ~framebuf() {free();}
		void alloc(int w, int h, int pitch)
		{
            free();
			this->w = w; this->h = h; this->pitch = pitch;
			actualbuf[0] = malloc(pitch*h + 16);
			// the most we have to cope with is 4:2:2
			// for the time being 4:4:4 will have chroma decimated
			actualbuf[1] = malloc(pitch*h/2 + 16);
			actualbuf[2] = malloc(pitch*h/2 + 16);
			actualbuf[3] = malloc(pitch*h + 16);
			actualbuf[4] = malloc(pitch*h/4 + 16);
			actualbuf[5] = malloc(pitch*h/4 + 16);

			// align all the buffer
			for(int i(0); i < 6; ++i)
			{
				buf[i] = (BYTE*)(((DWORD)(actualbuf[i]) + 15) & ~15);
			}
		}
		void free() {for(int i = 0; i < 6; i++) {::free(actualbuf[i]); buf[i] = NULL; actualbuf[i] = NULL;}}
	} m_fb;

	int m_win, m_hin, m_arxin, m_aryin;
	int m_wout, m_hout, m_arxout, m_aryout;
    
    typedef enum 
	{
		DIAuto,
		DIWeave,
		DIBob,
	} eDeintType;
	eDeintType m_di;

	void Copy420(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);
	void Copy422(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);

    HRESULT ProcessMPEGSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties);
    HRESULT Deliver(bool fRepeatFrame);
    void FinishProcessingMPEG();
    HRESULT ReconnectOutput(int w, int h);
    void ResetMpeg2Decoder();


private:
    // Rate change Stuff
	AM_SimpleRateChange m_rate;
	AM_SimpleRateChange m_ratechange;
    bool m_CorrectTS;
    HRESULT SetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData);
    HRESULT GetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT SupportPropSetRate(DWORD dwPropID, DWORD *pTypeSupport);

private:
    // Subpicture Stuff
	AM_PROPERTY_COMPOSIT_ON m_spon;
	AM_DVD_YUV m_sppal[16];
	AM_PROPERTY_SPHLI* m_sphli; // temp
    bool m_EnableForcedSubtitles;
	class sp_t
	{
	public:
		sp_t();
		~sp_t();
		REFERENCE_TIME rtStart, rtStop; 
		std::vector<BYTE> pData;
		AM_PROPERTY_SPHLI* sphli; // hli
		bool fForced;
	};
	std::list<sp_t*> m_sps;
	CCanLock m_SubPictureLock;


    HRESULT SetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData);
    HRESULT GetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT SupportPropSetSubPic(DWORD dwPropID, DWORD *pTypeSupport);
    HRESULT ProcessSubPicSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties);
    bool DecodeSubpic(sp_t* sp, AM_PROPERTY_SPHLI& sphli, DWORD& offset1, DWORD& offset2);
    void FinishProcessingSubPic();
    void RenderSubpics(REFERENCE_TIME rt, BYTE** p, int w, int h);
    void ClearOldSubpics(REFERENCE_TIME rt);
    bool HasSubpicsToRender(REFERENCE_TIME rt);
    void RenderSubpic(sp_t* sp, BYTE** p, int w, int h, AM_PROPERTY_SPHLI* sphli_hli);
};

#define m_VideoInPin m_InputPins[0]
#define m_SubpictureInPin m_InputPins[1]
#define m_VideoOutPin m_OutputPins[0]
#define m_CCOutPin m_OutputPins[1]