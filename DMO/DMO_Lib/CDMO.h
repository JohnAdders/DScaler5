////////////////////////////////////////////////////////////////////////////
// $Id: CDMO.h,v 1.1 2003-05-16 16:19:12 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the 
// DMO project wizard.  That code is (c) Microsoft Corporation
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#pragma once 

#include "param.h"
#include "..\..\GenDMOProp\GenDMOProp.h"

/////////////////////////////////////////////////////////////////////////////
// CDMO
class ATL_NO_VTABLE CDMO : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IMediaObjectImpl<CDMO,1,1>,				// DMO Template (1 input stream & 1 output stream)
	public CParamsManager,
	public CParamsManager::UpdateCallback,
	public IPersistStream,
    public ISpecifyPropertyPagesImpl<CDMO>,
    public IDScalerFilterPlugin
{
friend class IMediaObjectImpl<CDMO,1,1>;
friend class LockIt;

public:
    CDMO();	// Constructor
	~CDMO();	// Destructor

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDMO)
	COM_INTERFACE_ENTRY_IID(IID_IMediaObject, IMediaObject)
   	COM_INTERFACE_ENTRY(IMediaParams)
	COM_INTERFACE_ENTRY(IMediaParamInfo)
	COM_INTERFACE_ENTRY(IPersistStream)
	COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IDScalerFilterPlugin)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

BEGIN_PROP_MAP(CDMO)
	PROP_PAGE(CLSID_GenDMOPropPage)
	PROP_PAGE(CLSID_LicensePropPage)
END_PROP_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		FreeStreamingResources();  // In case client does not call this.
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

public:
	// SetParam handling
	STDMETHOD(SetParam)(DWORD dwParamIndex,MP_DATA value) { return SetParamInternal(dwParamIndex, value, false); }
	HRESULT SetParamUpdate(DWORD dwParamIndex, MP_DATA value) { return SetParamInternal(dwParamIndex, value, true); }

	// IPersist Methods
	STDMETHOD(GetClassID)(CLSID* pClassID) = 0;

	// IPersistStream Methods
	STDMETHOD(IsDirty)();
	STDMETHOD(Load)(IStream* pStream);
	STDMETHOD(Save)(IStream* pStream, BOOL fClearDirty);
	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize);

    // IDScalerFilterPlugin

	STDMETHOD(Attach)(IUnknown* pFilter);
	STDMETHOD(Detach)();

protected:
	//IMediaObjectImpl Methods   
    STDMETHOD(InternalAllocateStreamingResources)(void);
	STDMETHOD(InternalDiscontinuity)(DWORD dwInputStreamIndex);
    STDMETHOD(InternalGetInputStreamInfo)(DWORD dwInputStreamIndex, DWORD *pdwFlags);
    STDMETHOD(InternalGetOutputStreamInfo)(DWORD dwInputStreamIndex, DWORD *pdwFlags);
    STDMETHOD(InternalGetInputMaxLatency)(DWORD dwInputStreamIndex, REFERENCE_TIME *prtMaxLatency);
	STDMETHOD(InternalSetInputMaxLatency)(DWORD dwInputStreamIndex, REFERENCE_TIME rtMaxLatency);
	STDMETHOD(InternalFlush)(void) = 0;
	STDMETHOD(InternalFreeStreamingResources)(void) = 0;
    STDMETHOD(InternalGetInputSizeInfo)(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pcbMaxLookahead, DWORD *pcbAlignment) = 0;
	STDMETHOD(InternalGetOutputSizeInfo)(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pcbAlignment) = 0;
    virtual HRESULT InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt) = 0;
	virtual HRESULT InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt) = 0;
	STDMETHOD(InternalProcessInput)(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimestamp, REFERENCE_TIME rtTimelength) = 0;
	STDMETHOD(InternalProcessOutput)(DWORD dwFlags, DWORD cOutputBufferCount, DMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus) = 0;

	// IMediaObjectImpl Required overides
	STDMETHOD(InternalAcceptingInput)(DWORD dwInputStreamIndex) = 0;
	virtual HRESULT InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt) = 0;
	virtual HRESULT InternalCheckOutputType(DWORD dwOutputStreamIndex, const DMO_MEDIA_TYPE *pmt) = 0;
	void Lock(void);
	void Unlock(void);

	// Private functions
    /** UpdateStatesInternal
        Override this function if you need to do maintain
        any information based on the parameters
    */
	virtual HRESULT UpdateStatesInternal();

	virtual HRESULT SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager) = 0;

	// States of the DMO
	bool	m_fDirty;
	bool	m_fInitialized;

	REFERENCE_TIME			m_rtTimestamp;		// Most recent timestamp
	REFERENCE_TIME			m_rtTimelength;		// Most recent timelength
	bool					m_bValidTime;		// Controls whether timestamp is valid or not
	bool					m_bValidLength;		// Controls whether timelength is valid or not
    IUnknown*               m_ParentFilter;
};


