///////////////////////////////////////////////////////////////////////////////
// $Id: DivxDecoder.h,v 1.10 2007-12-15 14:49:29 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// DivxVideo.dll - DirectShow filter for decoding Divx streams
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
	#ifdef  _MSC_VER
		#pragma warning(disable: 4244)
	#endif

	#include "avcodec.h"

	#ifdef  _MSC_VER
		#pragma warning(default: 4244)
	#endif
}

DEFINE_GUID(CLSID_CDivxDecoder, 0x4775acfd, 0x8fe4, 0x483d, 0x96, 0x2b, 0xaf, 0x4b, 0x5e, 0x74, 0xb3, 0xbf);

typedef struct
{
    DWORD FourCC;
    CodecID FFMpegCodecId;
} SCodecList;

class CDivxDecoder : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IAMDecoderCaps
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CDivxDecoder, "{4775ACFD-8FE4-483d-962B-AF4B5E74B3BF}", "MPEG4 Video Filter", "Filter.DivxVideo.1", "Filter.DivxVideo", "both")
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
    CDivxDecoder();
    ~CDivxDecoder();


BEGIN_PARAM_LIST()
    DEFINE_PARAM_ENUM(DIBob, DIAuto, L"Deinterlace Mode")
    DEFINE_PARAM_INT(0, 200, 0, L"ms", L"Video Delay")
    DEFINE_PARAM_BOOL(0, L"Use accurate aspect ratios")
    DEFINE_PARAM_ENUM(SPACE_NV12, SPACE_YUY2, L"Colour space to output")
END_PARAM_LIST()

    enum eDivxVideoParams
    {
        DEINTMODE,
        VIDEODELAY,
        DOACCURATEASPECT,
		OUTPUTSPACE,
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

    //  functions relating to the fourccs we support
    static SCodecList* getCodecList();

    static unsigned long UpperFourCC(unsigned long inFourCC);
    static unsigned long LowerFourCC(unsigned long inFourCC);
    static CodecID lookupCodec(unsigned long inFourCC);
    static int __cdecl GetBuffer(struct AVCodecContext *c, AVFrame *pic);
    int InternalGetBuffer(struct AVCodecContext *c, AVFrame *pic);
    static void __cdecl ReleaseBuffer(struct AVCodecContext *c, AVFrame *pic);
    void InternalReleaseBuffer(struct AVCodecContext *c, AVFrame *pic);
    static int __cdecl RegetBuffer(struct AVCodecContext *c, AVFrame *pic);
    int InternalRegetBuffer(struct AVCodecContext *c, AVFrame *pic);


protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText);

private:
    REFERENCE_TIME m_AvgTimePerFrame;
    REFERENCE_TIME m_LastOutputTime;
    REFERENCE_TIME m_LastInputTime;
    DWORD m_Rate;
    bool m_fWaitForKeyFrame;
    bool m_fFilm;
    CCanLock m_DeliverLock;
    CodecID m_CodecID;
	std::vector<BYTE> m_ExtraData;
    int m_ExtraSize;
    long m_NalSize;
    DWORD m_FourCC;


    AVCodec* m_Codec;
    AVCodecContext* m_CodecContext;

    class CFrameBuffer
    {
    public:
        CFrameBuffer();
        ~CFrameBuffer();
        REFERENCE_TIME m_rtStartCoded;
        REFERENCE_TIME m_rtStartDisplay;
        void Clear();
        void AddRef() {m_UseCount++;};
        void Release() {m_UseCount--;};
        bool NotInUse() {return (m_UseCount <= 0);};
    private:
        int m_UseCount;
    };
    std::vector<CFrameBuffer*> m_Buffers;

    int m_DivxWidth;
    int m_DivxHeight;
    long m_ARDivxX;
    long m_ARDivxY;
    
    typedef enum 
    {
        DIAuto,
        DIWeave,
        DIBob,
    } eDeintType;
    eDeintType m_NextFrameDeint;

    typedef enum
    {
        SPACE_YV12,
        SPACE_YUY2,
        SPACE_NV12,
    } eOutputSpace;

    HRESULT Deliver(AVFrame& NextFrame, CFrameBuffer* CurrentPicture);
    void FlushDivx();

    HRESULT AdjustRenderersMediaType();
    HRESULT ResetDivxDecoder();
    HRESULT GetEnumTextDeintMode(WCHAR **ppwchText);
    HRESULT GetEnumTextOutputSpace(WCHAR **ppwchText);

    CFrameBuffer* GetNextBuffer();
    void ResetBuffers();
    static void __cdecl avlog(void*,int,const char*,va_list);
    int (*m_OldGetBuffer)(struct AVCodecContext *c, AVFrame *pic);
    void (*m_OldReleaseBuffer)(struct AVCodecContext *c, AVFrame *pic);
    int (*m_OldRegetBuffer)(struct AVCodecContext *c, AVFrame *pic);

};

#define m_VideoInPin m_InputPins[0]
#define m_VideoOutPin ((CDSVideoOutPin*)m_OutputPins[0])
