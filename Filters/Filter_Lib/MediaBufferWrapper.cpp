///////////////////////////////////////////////////////////////////////////////
// $Id: MediaBufferWrapper.cpp,v 1.3 2004-07-07 14:09:01 adcockj Exp $
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
// Revision 1.2  2004/03/05 15:56:29  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.2  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
// Revision 1.1  2003/08/21 16:17:58  adcockj
// Changed filter to wrap the deinterlacing DMO, fixed many bugs
//
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
    return m_pSample->SetActualDataLength(cbLength);
}


IMediaBuffer* CMediaBufferWrapper::CreateBuffer(IMediaSample* Sample)
{
    CMediaBufferWrapper* NewBuffer = new CMediaBufferWrapper;
    NewBuffer->m_pSample = Sample;
    return (IMediaBuffer*)NewBuffer;
}
