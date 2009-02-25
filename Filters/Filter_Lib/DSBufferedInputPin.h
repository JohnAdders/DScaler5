///////////////////////////////////////////////////////////////////////////////
// $Id$
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

#include "EnumMediaTypes.h"
#include "InputMemAlloc.h"
#include "DSInputPin.h"
#include "queue"


/////////////////////////////////////////////////////////////////////////////
// CDSInputPin
class CDSBufferedInputPin : 
	public CDSInputPin
{
public:

IMPLEMENT_UNKNOWN(CDSBufferedInputPin)

BEGIN_INTERFACE_TABLE(CDSBufferedInputPin)
	IMPLEMENTS_INTERFACE(IPin)
	IMPLEMENTS_INTERFACE(IMemInputPin)
    IMPLEMENTS_INTERFACE(IQualityControl)
	IMPLEMENTS_INTERFACE(IPinConnection)
	IMPLEMENTS_INTERFACE(IKsPropertySet)
END_INTERFACE_TABLE()

public:
	CDSBufferedInputPin();
	~CDSBufferedInputPin();

// IPin
public:
    STDMETHOD(EndOfStream)(void);
    STDMETHOD(BeginFlush)(void);
    STDMETHOD(EndFlush)(void);
	STDMETHOD(NewSegment)(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

// IMemInputPin
public:
    STDMETHOD(Receive)(IMediaSample *pSample);
    STDMETHOD(ReceiveCanBlock)(void);

public:
    HRESULT Activate();
    HRESULT Deactivate();

protected:
    std::queue<SI(IMediaSample)> m_Samples;
    CCanLock m_SamplesLock;
    CCanLock m_WorkerThreadLock;
    static void ProcessingThread(void* pParam);
    HRESULT ProcessBufferedSamples();
    HRESULT ProcessBufferedSample(IMediaSample* InSample);
    HANDLE m_SamplesReadyEvent;
    HANDLE m_ThreadStopEvent;
    HANDLE m_WorkerThread;
	HRESULT m_ThreadRetCode;
	DWORD m_ThreadId;
};

