///////////////////////////////////////////////////////////////////////////////
// $Id: TSReaderOutPin.cpp,v 1.1 2004-10-26 16:26:54 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
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
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TSReaderOutPin.h"
#include "TSReader.h"

CTSReaderOutPin::CTSReaderOutPin() :
    CDSOutputPin()
{
    LOG(DBGLOG_ALL, ("CTSReaderOutPin::CTSReaderOutPin\n"));
}

CTSReaderOutPin::~CTSReaderOutPin()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::~CDSOutputPin\n"));
}


STDMETHODIMP CTSReaderOutPin::GetCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::CheckCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::IsFormatSupported(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::QueryPreferredFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetTimeFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::IsUsingTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetDuration(LONGLONG *pDuration)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetStopPosition(LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::ConvertTimeFormat(
                                LONGLONG *pTarget,
                                const GUID *pTargetFormat,
                                LONGLONG Source,
                                const GUID *pSourceFormat
                            )
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetPositions( 
                        LONGLONG *pCurrent,
                        DWORD dwCurrentFlags,
                        LONGLONG *pStop,
                        DWORD dwStopFlags
                       )
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetRate(double dRate)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetRate(double *pdRate)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetPreroll(LONGLONG *pllPreroll)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::Render(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTSReader*)m_Filter)->Render(this, pGraph);
}

STDMETHODIMP CTSReaderOutPin::Backout(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTSReader*)m_Filter)->Backout(this, pGraph);
}
