///////////////////////////////////////////////////////////////////////////////
// $Id: EnumMediaTypes.cpp,v 1.3 2003-05-02 10:52:26 adcockj Exp $
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
// Revision 1.2  2003/05/02 07:03:13  adcockj
// Some minor changes most not really improvements
//
// Revision 1.1.1.1  2003/04/30 13:01:20  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnumMediaTypes.h"

CEnumMediaTypes::CEnumMediaTypes()
{
	m_Count = 0;
    m_NumTypes = 0;
    InitMediaType(&m_Types[0]);
    InitMediaType(&m_Types[1]);
}

CEnumMediaTypes::~CEnumMediaTypes()
{
	m_Count = 0;
    ClearMediaType(&m_Types[0]);
    ClearMediaType(&m_Types[1]);
}


STDMETHODIMP CEnumMediaTypes::Next(ULONG cTypes, AM_MEDIA_TYPE **ppTypes, ULONG *pcFetched)
{
    if(pcFetched != NULL)
    {
        *pcFetched = 0;
    }
    if(m_Update->HasChanged())
    {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }
    while(m_Count < m_NumTypes && cTypes)
    {
        // it's our job to allocate the MediaType Structures
        *ppTypes = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
        if(*ppTypes == NULL)
        {
            return E_OUTOFMEMORY;
        }
        HRESULT hr = CopyMediaType(*ppTypes, &m_Types[m_Count]);
        if(FAILED(hr))
        {
            return S_FALSE;
        }
        if(pcFetched != NULL)
        {
            ++(*pcFetched);
        }
        ++ppTypes;
        ++m_Count;
        --cTypes;
    }
    if(cTypes == 0)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CEnumMediaTypes::Skip(ULONG cTypes)
{
    m_Count += cTypes;
    if(m_Count < 1)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CEnumMediaTypes::Reset(void)
{
    m_Count = 0;
    m_Update->SetTypes(m_NumTypes, m_Types);
    return S_OK;
}

STDMETHODIMP CEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CComObject<CEnumMediaTypes>* NewEnum = new CComObject<CEnumMediaTypes>;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

	NewEnum->m_Count = m_Count;
    NewEnum->m_NumTypes = m_NumTypes;
    for(ULONG i(0); i < m_NumTypes; ++i)
    {
        CopyMediaType(&NewEnum->m_Types[i], &m_Types[i]);
    }
    
    NewEnum->AddRef();
    *ppEnum = NewEnum;

    return S_OK;
}

void CEnumMediaTypes::SetUpdate(IUpdateMediaTypes* Update)
{
    m_Update = Update;
    Reset();
}

