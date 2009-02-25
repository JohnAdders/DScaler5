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

#define _USE_DECSS
#ifdef _USE_DECSS

/////////////////////////////////////////////////////////////////////////////
// CDSInputPin
class CDSCSSInputPin : public CDSInputPin
{
public:

IMPLEMENT_UNKNOWN(CDSCSSInputPin)

BEGIN_INTERFACE_TABLE(CDSCSSInputPin)
	IMPLEMENTS_INTERFACE(IPin)
	IMPLEMENTS_INTERFACE(IMemInputPin)
    IMPLEMENTS_INTERFACE(IQualityControl)
	IMPLEMENTS_INTERFACE(IPinConnection)
	IMPLEMENTS_INTERFACE(IKsPropertySet)
END_INTERFACE_TABLE()

public:
	CDSCSSInputPin();
	~CDSCSSInputPin();

    // IKsPropertySet
    STDMETHOD(Set)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData);
    STDMETHOD(Get)(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned);
    STDMETHOD(QuerySupported)(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);

protected:
    HRESULT GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties);
	int m_varient;
	BYTE m_Challenge[10], m_KeyCheck[5], m_Key[10];
	BYTE m_DiscKey[6], m_TitleKey[6];
};

#else

#define CDSCSSInputPin CDSInputPin

#endif