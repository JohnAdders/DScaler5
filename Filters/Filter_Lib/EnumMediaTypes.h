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


class IUpdateMediaTypes : public IUnknown
{
public:
    virtual ULONG FormatVersion() = 0;
    virtual HRESULT GetType(ULONG TypeNum, AM_MEDIA_TYPE* Type) = 0;
};


class CEnumMediaTypes : 
    public IEnumMediaTypes
{
public:
    CEnumMediaTypes();
    ~CEnumMediaTypes();

IMPLEMENT_UNKNOWN(CEnumMediaTypes);

BEGIN_INTERFACE_TABLE(CEnumMediaTypes)
    IMPLEMENTS_INTERFACE(IEnumMediaTypes)
END_INTERFACE_TABLE()

// IEnumMediaTypes
public:
    STDMETHOD(Next)(ULONG cTypes,AM_MEDIA_TYPE **ppTypes, ULONG *pcFetched);
    STDMETHOD(Skip)(ULONG cTypes);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumMediaTypes **ppEnum);
    
    HRESULT SetUpdate(IUpdateMediaTypes* Update);
protected:
    IUpdateMediaTypes* m_Update;
    LONG m_Count;
    ULONG m_Version;
};
