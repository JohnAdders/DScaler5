///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
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
#include "stdafx.h"
#include "DScaler.h"

CDScaler::CMap::CMap()
{
    m_Length = 0;
    m_Map = NULL;
}

CDScaler::CMap::~CMap()
{
    if(m_Map != NULL)
    {
        free(m_Map);
    }
}


STDMETHODIMP CDScaler::CMap::GetBufferAndLength(BYTE** ppBuffer, DWORD* pcbLength)
{
    *pcbLength = m_Length;
    *ppBuffer = m_Map;
    return S_OK;
}

STDMETHODIMP CDScaler::CMap::GetMaxLength(DWORD* pcbMaxLength)
{
    *pcbMaxLength = m_Length;
    return S_OK;
}

STDMETHODIMP CDScaler::CMap::SetLength(DWORD cbLength)
{
    if(cbLength > m_Length)
    {
        ReAlloc(cbLength);
    }
    return S_OK;
}

void CDScaler::CMap::ReAlloc(DWORD NewLength)
{
    if(NewLength > m_Length)
    {
        if(m_Map != NULL)
        {
            free(m_Map);
        }
        m_Map = (BYTE*)malloc(NewLength);
        m_Length = NewLength;
        Clear();
    }
}

void CDScaler::CMap::Clear()
{
    if(m_Map)
    {
        ZeroMemory(m_Map, m_Length);
    }
}