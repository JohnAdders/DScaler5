///////////////////////////////////////////////////////////////////////////////
// $Id: DivxDecoder.h,v 1.2 2004-11-09 17:21:37 adcockj Exp $
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

#include "avcodec.h"

#define NUM_BUFFERS 4

DEFINE_GUID(CLSID_CDivxDecoder, 0x4775acfd, 0x8fe4, 0x483d, 0x96, 0x2b, 0xaf, 0x4b, 0x5e, 0x74, 0xb3, 0xbf);

class CDivxDecoder : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IAMDecoderCaps
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CDivxDecoder, "{4775ACFD-8FE4-483d-962B-AF4B5E74B3BF}", "DivxVideo Filter", "Filter.DivxVideo.1", "Filter.DivxVideo", "both")
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
    DEFINE_PARAM_ENUM(SPACE_YUY2, SPACE_YUY2, L"Colour space to output")
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

protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText);

private:
    REFERENCE_TIME m_AvgTimePerFrame;
    REFERENCE_TIME m_LastOutputTime;
    DWORD m_Rate;
    bool m_fWaitForKeyFrame;
    bool m_fFilm;
    CCanLock m_DeliverLock;
    CodecID m_CodecID;
    void* m_ExtraData;
    int m_ExtraSize;
    DWORD m_FourCC;

    AVCodec* m_Codec;
    AVCodecContext* m_CodecContext;

    class CFrameBuffer
    {
    public:
        CFrameBuffer();
        ~CFrameBuffer();
        AVFrame m_Picture;
        REFERENCE_TIME m_rtStart;
        REFERENCE_TIME m_rtStop;
        unsigned int m_NumFields;
        int m_UseCount;
        void Clear();
        void AddRef() {m_UseCount++;};
        void Release() {m_UseCount--;};
        bool NotInUse() {return (m_UseCount <= 0);};
    private:
    };
    CFrameBuffer m_Buffers[NUM_BUFFERS];
    CFrameBuffer* m_CurrentPicture;

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
    } eOutputSpace;

    HRESULT Deliver();
    void FlushDivx();

    HRESULT AdjustRenderersMediaType();
    void ResetDivxDecoder();
    HRESULT GetEnumTextDeintMode(WCHAR **ppwchText);
    HRESULT GetEnumTextOutputSpace(WCHAR **ppwchText);

    CFrameBuffer* GetNextBuffer();
    static void __cdecl avlog(void*,int,const char*,va_list);

};

#define m_VideoInPin m_InputPins[0]
#define m_VideoOutPin ((CDSVideoOutPin*)m_OutputPins[0])
