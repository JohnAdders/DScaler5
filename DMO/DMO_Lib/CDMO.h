////////////////////////////////////////////////////////////////////////////
// $Id$
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

#pragma once

#include "params.h"
#include "..\..\GenDMOProp\GenDMOProp.h"

/////////////////////////////////////////////////////////////////////////////
// CDMO
class CDMO :
    public IMediaObjectImpl<CDMO,1,1>,                // DMO Template (1 input stream & 1 output stream)
    public CParams,
    public ISpecifyPropertyPages,
    public IDScalerFilterPlugin
{
friend class IMediaObjectImpl<CDMO,1,1>;
friend class LockIt;

public:
    CDMO(LPCWSTR Name);
    virtual ~CDMO();

public:
    // IPersist Methods
    STDMETHOD(GetClassID)(CLSID* pClassID) = 0;

    // IDScalerFilterPlugin
    STDMETHOD(Attach)(IUnknown* pFilter);
    STDMETHOD(Detach)();

    // ISpecifyPropertyPages
    STDMETHOD(GetPages)(CAUUID* pPages);


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

    // States of the DMO
    bool    m_fInitialized;

    REFERENCE_TIME            m_rtTimestamp;        // Most recent timestamp
    REFERENCE_TIME            m_rtTimelength;        // Most recent timelength
    bool                    m_bValidTime;        // Controls whether timestamp is valid or not
    bool                    m_bValidLength;        // Controls whether timelength is valid or not
    IUnknown*               m_ParentFilter;
};


