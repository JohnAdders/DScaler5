///////////////////////////////////////////////////////////////////////////////
// $Id: EnumTwoPins.cpp,v 1.2 2003-05-08 15:58:38 adcockj Exp $
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
// Revision 1.1.1.1  2003/04/30 13:01:20  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EnumTwoPins.h"

CEnumTwoPins::CEnumTwoPins()
{
	m_Count = 0;
}

STDMETHODIMP CEnumTwoPins::Next(ULONG cPins,IPin **ppPins, ULONG *pcFetched)
{
    if(pcFetched != NULL)
    {
        *pcFetched = 0;
    }
    while(m_Count < 2 && cPins)
    {
        HRESULT hr = m_Pins[m_Count].CopyTo(ppPins);
        CHECK(hr);
        if(pcFetched != NULL)
        {
            ++(*pcFetched);
        }
        ++ppPins;
        ++m_Count;
        --cPins;
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

STDMETHODIMP CEnumTwoPins::Skip(ULONG cPins)
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

STDMETHODIMP CEnumTwoPins::Reset(void)
{
    m_Count = 0;
    return S_OK;
}

STDMETHODIMP CEnumTwoPins::Clone(IEnumPins **ppEnum)
{
    if(ppEnum == NULL)
    {
        return E_POINTER;
    }
    CComObject<CEnumTwoPins>* NewEnum = new CComObject<CEnumTwoPins>;
    if(NewEnum == NULL)
    {
        return E_OUTOFMEMORY;
    }

	NewEnum->m_Count = m_Count;
    NewEnum->m_Pins[0] = m_Pins[0];
    NewEnum->m_Pins[1] = m_Pins[1];


    NewEnum->AddRef();
    *ppEnum = NewEnum;

    return S_OK;
}

void CEnumTwoPins::SetPins(IPin* Pin1, IPin* Pin2)
{
    m_Pins[0] = Pin1;
    m_Pins[1] = Pin2;
}
