///////////////////////////////////////////////////////////////////////////////
// $Id$
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

DEFINE_GUID(CLSID_CConvolverWrapper, 0x6105e395, 0x1c5a, 0x4b3e, 0x97, 0x1, 0xfb, 0x71, 0xc2, 0x52, 0x3a, 0xb6);

// Guid of John Pavel's Convolver WMP add-in
// {47427372-7AED-4e37-ABEB-7BD64C4184BF}
DEFINE_GUID(CLSID_Convolver, 
0x47427372, 0x7aed, 0x4e37, 0xab, 0xeb, 0x7b, 0xd6, 0x4c, 0x41, 0x84, 0xbf);

DEFINE_GUID(IID_IConvolver, 
0x9B102F5D, 0x8e2c, 0x41f2, 0x92, 0x56, 0x2d, 0x3c, 0xa7, 0x6f, 0xbe, 0x35);


interface IConvolver : IUnknown
{
public:

	virtual HRESULT STDMETHODCALLTYPE get_wetmix(double *pVal) = 0;
	virtual HRESULT STDMETHODCALLTYPE put_wetmix(double newVal) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_filterfilename(TCHAR* *pVal) = 0;
	virtual HRESULT STDMETHODCALLTYPE put_filterfilename(TCHAR* newVal) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_attenuation(double *pVal) = 0;
	virtual HRESULT STDMETHODCALLTYPE put_attenuation(double newVal) = 0;

	virtual double	decode_Attenuationdb(const DWORD dwValue) = 0;
	virtual DWORD	encode_Attenuationdb(const double fValue) = 0;

};


class CConvolverWrapper : 
    public CDSBaseFilter,
    public IConvolver,
    public IAmFreeSoftwareLicensed
{
public:

IMPLEMENT_AGGREGATABLE_COCLASS(CConvolverWrapper, "{6105E395-1C5A-4b3e-9701-FB71C2523AB6}", "Convolver Wrapper Class", "Filter.ConvolverWrapper.1", "Filter.ConvolverWrapper", "both")
	IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
	IMPLEMENTS_INTERFACE(IBaseFilter)
	IMPLEMENTS_INTERFACE(IMediaFilter)
	IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE_AS(IPersist, IPersistStream)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
    IMPLEMENTS_INTERFACE(IConvolver)
END_INTERFACE_TABLE()

public:
	CConvolverWrapper();
    ~CConvolverWrapper();

BEGIN_PARAM_LIST()
END_PARAM_LIST()

// IBaseFilter
public:
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);


// IAmFreeSoftwareLicensed
public:
    STDMETHOD(get_Name)(BSTR* Name);
	STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

    STDMETHOD(GetPages)(CAUUID *pPages);

// IConvolver
public:
    HRESULT STDMETHODCALLTYPE get_wetmix(double *pVal);
	HRESULT STDMETHODCALLTYPE put_wetmix(double newVal);

	HRESULT STDMETHODCALLTYPE get_filterfilename(TCHAR* *pVal);
	HRESULT STDMETHODCALLTYPE put_filterfilename(TCHAR* newVal);

	HRESULT STDMETHODCALLTYPE get_attenuation(double *pVal);
	HRESULT STDMETHODCALLTYPE put_attenuation(double newVal);

	double	decode_Attenuationdb(const DWORD dwValue);
	DWORD	encode_Attenuationdb(const double fValue);

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
    
protected:
    HRESULT UpdateTypes(const AM_MEDIA_TYPE* MediaType);
    HRESULT CheckConvolver();
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText);
	void CreateInternalMediaType();
    SI(IMediaObject) m_Convolver;
    SI(IConvolver) m_ConvolverPrivate;

private:
};

#define m_AudioInPin m_InputPins[0]
#define m_AudioOutPin m_OutputPins[0]
