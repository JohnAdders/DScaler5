///////////////////////////////////////////////////////////////////////////////
// $Id: InputMemAlloc.cpp,v 1.1 2003-08-21 16:17:58 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputMemAlloc.h"

CInputMemAlloc::CInputMemAlloc()
{
	m_MemAlloc.CoCreateInstance(CLSID_MemoryAllocator);
}

CInputMemAlloc::~CInputMemAlloc()
{
	m_MemAlloc.Release();
}

STDMETHODIMP CInputMemAlloc::SetProperties(ALLOCATOR_PROPERTIES *pRequest, ALLOCATOR_PROPERTIES *pActual)
{
	if(pRequest->cBuffers < 4)
	{
		pRequest->cBuffers = 4;
	}
	if(pRequest->cbBuffer < 768*576*2)
	{
		pRequest->cbBuffer = 768*576*2;
	}
	if(pRequest->cbAlign < 16)
	{
		pRequest->cbAlign = 16;
	}
	return m_MemAlloc->SetProperties(pRequest, pActual);
}

STDMETHODIMP CInputMemAlloc::GetProperties(ALLOCATOR_PROPERTIES *pProps)
{
	return m_MemAlloc->GetProperties(pProps);
}

STDMETHODIMP CInputMemAlloc::Commit(void)
{
	return m_MemAlloc->Commit();
}

STDMETHODIMP CInputMemAlloc::Decommit(void)
{
	return m_MemAlloc->Decommit();
}

STDMETHODIMP CInputMemAlloc::GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
	return m_MemAlloc->GetBuffer(ppBuffer, pStartTime, pEndTime, dwFlags);
}

STDMETHODIMP CInputMemAlloc::ReleaseBuffer(IMediaSample *pBuffer)
{
	return m_MemAlloc->ReleaseBuffer(pBuffer);
}
