///////////////////////////////////////////////////////////////////////////////
// $Id: EnumMediaTypes.cpp,v 1.1 2004-02-06 12:17:17 adcockj Exp $
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
// Revision 1.7  2003/12/09 11:45:55  adcockj
// Improved implementation of EnumPins
//
// Revision 1.6  2003/05/08 15:58:38  adcockj
// Better error handling, threading and format support
//
// Revision 1.5  2003/05/06 16:38:00  adcockj
// Changed to fixed size output buffer and changed connection handling
//
// Revision 1.4  2003/05/02 19:15:39  adcockj
// Futher corrections to Next
//
// Revision 1.3  2003/05/02 10:52:26  adcockj
// Fixed memory allocation bug in next
//
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
    m_Version = 0;
}

CEnumMediaTypes::~CEnumMediaTypes()
{
	m_Count = 0;
}


STDMETHODIMP CEnumMediaTypes::Next(ULONG cTypes, AM_MEDIA_TYPE **ppTypes, ULONG *pcFetched)
{
    if(pcFetched != NULL)
    {
        *pcFetched = 0;
    }
    if(m_Version != m_Update->FormatVersion())
    {
        m_Version = m_Update->FormatVersion();
        return VFW_E_ENUM_OUT_OF_SYNC;
    }
    while(m_Count != -1 && cTypes)
    {
        // it's our job to allocate the MediaType Structures
        *ppTypes = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
        if(*ppTypes == NULL)
        {
            return E_OUTOFMEMORY;
        }
        InitMediaType(*ppTypes);
        HRESULT hr = m_Update->GetType(m_Count, *ppTypes);

        CHECK(hr);
        if(hr == S_FALSE)
        {
            m_Count = -1;
        }
        else
        {
            if(pcFetched != NULL)
            {
                ++(*pcFetched);
            }
            ++ppTypes;
            ++m_Count;
            --cTypes;
        }
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
    m_Version = m_Update->FormatVersion();
    return S_OK;
}

STDMETHODIMP CEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CEnumMediaTypes* NewEnum = new CEnumMediaTypes;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

	NewEnum->m_Count = m_Count;
    NewEnum->m_Version = m_Version;
    
    NewEnum->AddRef();
    *ppEnum = NewEnum;

    return S_OK;
}

HRESULT CEnumMediaTypes::SetUpdate(IUpdateMediaTypes* Update)
{
    m_Update = Update;
    return Reset();
}

