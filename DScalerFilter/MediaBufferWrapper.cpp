///////////////////////////////////////////////////////////////////////////////
// $Id: MediaBufferWrapper.cpp,v 1.1 2003-08-21 16:17:58 adcockj Exp $
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
#include "MediaBufferWrapper.h"

CMediaBufferWrapper::CMediaBufferWrapper()
{
}

CMediaBufferWrapper::~CMediaBufferWrapper()
{
}

STDMETHODIMP CMediaBufferWrapper::GetBufferAndLength(BYTE** ppBuffer, DWORD* pcbLength)
{
	*pcbLength = m_pSample->GetActualDataLength();
	return m_pSample->GetPointer(ppBuffer);
}

STDMETHODIMP CMediaBufferWrapper::GetMaxLength(DWORD* pcbMaxLength)
{
	*pcbMaxLength = m_pSample->GetSize();
	return S_OK;
}

STDMETHODIMP CMediaBufferWrapper::SetLength(DWORD cbLength)
{
	return m_pSample->SetActualDataLength(cbLength);;
}


IMediaBuffer* CMediaBufferWrapper::CreateBuffer(IMediaSample* Sample)
{
	CComObject<CMediaBufferWrapper>* NewBuffer = new CComObject<CMediaBufferWrapper>;
	NewBuffer->m_pSample = Sample;
    NewBuffer->AddRef();
    return (IMediaBuffer*)NewBuffer;
}
