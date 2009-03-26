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

class CInputMemAlloc :
    public CComObjectRootEx<CComMultiThreadModel>,
    public IMemAllocator
{
public:
    CInputMemAlloc();
    ~CInputMemAlloc();

BEGIN_COM_MAP(CInputMemAlloc)
    COM_INTERFACE_ENTRY(IMemAllocator)
END_COM_MAP()

    STDMETHOD(SetProperties)(ALLOCATOR_PROPERTIES *pRequest, ALLOCATOR_PROPERTIES *pActual);
    STDMETHOD(GetProperties)(ALLOCATOR_PROPERTIES *pProps);
    STDMETHOD(Commit)(void);
    STDMETHOD(Decommit)(void);
    STDMETHOD(GetBuffer)(IMediaSample **ppBuffer, REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime, DWORD dwFlags);
    STDMETHOD(ReleaseBuffer)(IMediaSample *pBuffer);
private:
    CComPtr<IMemAllocator> m_MemAlloc;
};
