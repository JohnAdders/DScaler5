///////////////////////////////////////////////////////////////////////////////
// $Id: MpegDecoder.h,v 1.13 2004-04-16 16:19:44 adcockj Exp $
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


#define NUM_BUFFERS 4

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
    IMPLEMENTS_INTERFACE_AS(IPersist, IPersistStream)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
    IMPLEMENTS_INTERFACE(IAMDecoderCaps)
END_INTERFACE_TABLE()

public:
	CMpegDecoder();
    ~CMpegDecoder();


BEGIN_PARAM_LIST()
    DEFINE_PARAM_BOOL(1, L"Display Forced Subtitles")
    DEFINE_PARAM_BOOL(1, L"3:2 playback smoothing")
    DEFINE_PARAM_ENUM(DIBob, DIAuto, L"Deinterlace Mode")
    DEFINE_PARAM_INT(0, 200, 0, L"ms", L"Video Delay")
    DEFINE_PARAM_BOOL(0, L"Use accurate aspect ratios")
    DEFINE_PARAM_ENUM(DVBLETTERBOX, DVB169, L"DVB Aspect Preferences")
END_PARAM_LIST()

    enum eMpegVideoParams
    {
        DISPLAYFORCEDSUBS,
        FRAMESMOOTH32,
        DEINTMODE,
        VIDEODELAY,
		DOACCURATEASPECT,
		DVBASPECTPREFS
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
    // IAMDecoderCaps
	STDMETHOD(GetDecoderCaps)(DWORD dwCapIndex, DWORD* lpdwCap);

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

private:
	mpeg2dec_t* m_dec;
    AM_MEDIA_TYPE m_InternalMT;
    bool m_NeedToAttachFormat;
	REFERENCE_TIME m_AvgTimePerFrame;
	REFERENCE_TIME m_LastOutputTime;
	bool m_fWaitForKeyFrame;
	bool m_fFilm;
	bool m_ProgressiveChroma;
	CCanLock m_DeliverLock;
    int m_OutputWidth;
    int m_OutputHeight;
    int m_InternalPitch;
	bool m_DoPanAndScan;
	bool m_FilmCameraModeHint;
	bool m_LetterBoxed;
	DWORD m_PanScanOffsetX;
	DWORD m_PanScanOffsetY;
	DWORD m_ControlFlags;
	BYTE m_AFD;
	
    typedef enum 
	{
		CHROMA_420,
		CHROMA_422,
		CHROMA_444
	} eChromaType;
	eChromaType m_ChromaType;

    class CFrameBuffer
    {
    public:
        CFrameBuffer();
        ~CFrameBuffer();
        HRESULT AllocMem(int YSize, int UVSize);
        void FreeMem();
        void Clear();
		BYTE* m_Buf[3];
		REFERENCE_TIME m_rtStart;
        REFERENCE_TIME m_rtStop;
		DWORD m_Flags;
		unsigned int m_NumFields;
		int m_CurrentSize;
        CFrameBuffer& operator=(CFrameBuffer& RHS);
        void AddRef() {m_UseCount++;};
        void Release() {m_UseCount--;};
        bool NotInUse() {return (m_UseCount <= 0);};
    private:
        void* m_ActualBuf;
		int m_AllocatedSize;
        int m_UseCount;
    };
    CFrameBuffer m_Buffers[NUM_BUFFERS];
    CFrameBuffer* m_CurrentPicture;
    CFrameBuffer m_SubPicBuffer;

	int m_MpegWidth;
	int m_MpegHeight;
	int m_CurrentWidth;
	int m_CurrentHeight;
	long m_ARMpegX;
	long m_ARMpegY;
	long m_ARAdjustX;
	long m_ARAdjustY;
	long m_ARCurrentOutX;
	long m_ARCurrentOutY;
    
    typedef enum 
	{
		DIAuto,
		DIWeave,
		DIBob,
	} eDeintType;
	eDeintType m_NextFrameDeint;

	typedef enum
	{
		DVB169,
		DVBCCO,
		DVBLETTERBOX
	} eDVBAspectPrefs;

	void Copy420(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);
	void Copy422(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);
	void Copy444(BYTE* pOut, BYTE** ppIn, DWORD w, DWORD h, DWORD pitchIn);

    HRESULT ProcessMPEGSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties);
    HRESULT Deliver(bool fRepeatFrame);
    void FlushMPEG();
    HRESULT ReconnectOutput(bool ForceReconnect);
    void ResetMpeg2Decoder();
    HRESULT GetEnumTextDeintMode(WCHAR **ppwchText);
    HRESULT ProcessNewSequence();
    HRESULT ProcessPictureStart(AM_SAMPLE2_PROPERTIES* pSampleProperties);
    HRESULT ProcessPictureDisplay();
    HRESULT ProcessUserData(mpeg2_state_t State, const BYTE* const UserData, int UserDataLen);
    HRESULT GetEnumTextDVBAspectPrefs(WCHAR **ppwchText);

    CFrameBuffer* GetNextBuffer();

	void DrawPixel(BYTE** yuv, POINT pt, int pitch, BYTE color, BYTE contrast, AM_DVD_YUV* sppal);
	void DrawPixels(BYTE** yuv, POINT pt, int pitch, int len, BYTE color, 
								AM_PROPERTY_SPHLI& sphli, RECT& rc,
								AM_PROPERTY_SPHLI* sphli_hli, RECT& rchli,
								AM_DVD_YUV* sppal);

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
	class CSubPicture
	{
	public:
		CSubPicture();
		~CSubPicture();
		REFERENCE_TIME rtStart;
		REFERENCE_TIME rtStop; 
		std::vector<BYTE> pData;
		bool fForced;
	};
	std::list<CSubPicture*> m_SubPicureList;
	class CHighlight
	{
	public:
		CHighlight();
		~CHighlight();
		AM_PROPERTY_SPHLI m_Hi;
		REFERENCE_TIME rtStart;
		REFERENCE_TIME rtStop; 
	};
	std::list<CHighlight*> m_HighlightList;
	CCanLock m_SubPictureLock;


    HRESULT SetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData);
    HRESULT GetPropSetSubPic(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned);
    HRESULT SupportPropSetSubPic(DWORD dwPropID, DWORD *pTypeSupport);
    HRESULT ProcessSubPicSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties);
    bool DecodeSubpic(CSubPicture* sp, AM_PROPERTY_SPHLI& sphli, DWORD& offset1, DWORD& offset2);
    void FlushSubPic();
    void RenderSubpics(REFERENCE_TIME rt, BYTE** p, int w, int h);
    void ClearOldSubpics(REFERENCE_TIME rt);
    bool HasSubpicsToRender(REFERENCE_TIME rt);
    void RenderSubpic(CSubPicture* sp, BYTE** p, int w, int h, AM_PROPERTY_SPHLI* sphli_hli);
    void RenderHighlight(BYTE** p, int w, int h, AM_PROPERTY_SPHLI* sphli_hli);
	void Simplify(long& u, long& v);
	void Simplify(unsigned long& u, unsigned long& v);
	void CorrectSourceTarget(RECT& rcSource, RECT& rcTarget);
	void CorrectOutputSize();
	void LetterBox(long YAdjust, long XAdjust, bool IsTop = false);
	void PillarBox(long YAdjust, long XAdjust);

};

#define m_VideoInPin m_InputPins[0]
#define m_SubpictureInPin m_InputPins[1]
#define m_VideoOutPin m_OutputPins[0]
#define m_CCOutPin m_OutputPins[1]