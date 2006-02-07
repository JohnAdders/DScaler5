///////////////////////////////////////////////////////////////////////////////
// $Id: DSBaseFilter.h,v 1.7 2006-02-07 17:39:12 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
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

#include "EnumPins.h"
#include "Params.h"
#include "DSInputPin.h"
#include "DSOutputPin.h"

class CDSBaseFilter : 
    public ISpecifyPropertyPages,
    public IBaseFilter,
    public CParams,
    public IHavePins
{
public:
    CDSBaseFilter(LPCWSTR Name, int NumberOfInputPins, int NumberOfOutputPins);
    virtual ~CDSBaseFilter();

public:
    // IBaseFilter
    STDMETHOD(EnumPins)(IEnumPins **ppEnum);
    STDMETHOD(FindPin)(LPCWSTR Id, IPin **ppPin);
    STDMETHOD(QueryFilterInfo)(FILTER_INFO *pInfo);
    STDMETHOD(JoinFilterGraph)(IFilterGraph *pGraph, LPCWSTR pName);
    STDMETHOD(QueryVendorInfo)(LPWSTR *pVendorInfo);
    STDMETHOD(Stop)(void);
    STDMETHOD(Pause)(void);
    STDMETHOD(Run)(REFERENCE_TIME tStart);
    STDMETHOD(GetState)(DWORD dwMilliSecsTimeout, FILTER_STATE *State);
    STDMETHOD(SetSyncSource)(IReferenceClock *pClock);
    STDMETHOD(GetSyncSource)(IReferenceClock **pClock);
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID) = 0;

    // ISpecifyPropertyPages
    STDMETHOD(GetPages)(CAUUID* pPages);

    // IHavePins
    HRESULT GetPin(ULONG PinNum, IPin** pPin);

public:
    void SetTypesChangedFlag();
    HRESULT CheckProcessingLine();
    long GetNumPins() {return m_NumInputPins + m_NumOutputPins;};
    CDSBasePin* GetPin(int PinIndex);
    virtual HRESULT NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin) = 0;
    virtual HRESULT NotifyConnected(CDSBasePin* pPin) = 0;
    virtual HRESULT ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin) = 0;
    /// return VFW_S_NO_MORE_ITEMS when you have no more types you can generate, TypeNum = 0 is the first
    virtual HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum) = 0;
    virtual bool IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin) = 0;
    virtual HRESULT SendOutLastSamples(CDSBasePin* pPin) = 0;
    virtual HRESULT Flush(CDSBasePin* pPin) = 0;
    /// return S_FALSE if you don't want this message propogated
    virtual HRESULT NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin) = 0;
    virtual HRESULT Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin) = 0;
    virtual HRESULT GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin) = 0;
    virtual HRESULT Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin) = 0;
    virtual HRESULT Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin) = 0;
    virtual HRESULT QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin) = 0;
    virtual HRESULT Activate() = 0;
    virtual HRESULT Deactivate() = 0;
    bool IsClockUpstream();

public:
    SI(IReferenceClock) m_RefClock;
    FILTER_STATE m_State;
    IFilterGraph2* m_Graph;
    WCHAR m_Name[MAX_FILTER_NAME];
    
protected:
    STDMETHOD(AddToRot)(IUnknown *pUnkGraph);
    void RemoveFromRot();
    bool IsClockUpstreamFromFilter(IBaseFilter* Filter);
    void LockAllPins();
    void UnlockAllPins();

    CDSInputPin** m_InputPins;
    CDSOutputPin** m_OutputPins;
    int m_NumInputPins;
    int m_NumOutputPins;
    bool m_IsDiscontinuity;
    REFERENCE_TIME m_rtStartTime;
    DWORD m_Register;
};


