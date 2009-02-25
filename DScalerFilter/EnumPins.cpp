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
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnumPins.h"

CEnumPins::CEnumPins()
{
	m_Count = 0;
}

STDMETHODIMP CEnumPins::Next(ULONG cPins,IPin **ppPins, ULONG *pcFetched)
{
    if(pcFetched != NULL)
    {
        *pcFetched = 0;
    }
    while(m_Count != -1 && cPins)
    {
        HRESULT hr = m_Filter->GetPin(m_Count, ppPins);
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
            ++ppPins;
            ++m_Count;
            --cPins;
        }
    }
    if(cPins == 0)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CEnumPins::Skip(ULONG cPins)
{
    m_Count += cPins;
    if(m_Count < 2)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

STDMETHODIMP CEnumPins::Reset(void)
{
    m_Count = 0;
    return S_OK;
}

STDMETHODIMP CEnumPins::Clone(IEnumPins **ppEnum)
{
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CComObject<CEnumPins>* NewEnum = new CComObject<CEnumPins>;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

	NewEnum->m_Count = m_Count;

    NewEnum->AddRef();
    *ppEnum = NewEnum;

    return S_OK;
}

HRESULT CEnumPins::SetFilter(IHavePins* Filter)
{
    m_Filter = Filter;
    return S_OK;
}
