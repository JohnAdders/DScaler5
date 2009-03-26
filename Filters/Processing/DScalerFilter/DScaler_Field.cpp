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
#include "stdafx.h"
#include "DScaler.h"

STDMETHODIMP CDScaler::CField::GetBufferAndLength(BYTE** ppBuffer, DWORD* pcbLength)
{
    *pcbLength = m_Sample->GetActualDataLength();
    return  m_Sample->GetPointer(ppBuffer);
}

STDMETHODIMP CDScaler::CField::GetMaxLength(DWORD* pcbMaxLength)
{
    *pcbMaxLength = m_Sample->GetSize();
    return S_OK;
}

STDMETHODIMP CDScaler::CField::SetLength(DWORD cbLength)
{
    return m_Sample->SetActualDataLength(cbLength);
}

STDMETHODIMP CDScaler::CField::get_TopFieldFirst(BOOLEAN* TopFieldFirst)
{
    if(m_IsTopLine)
    {
        *TopFieldFirst = TRUE;
    }
    else
    {
        *TopFieldFirst = FALSE;
    }
    return S_OK;
}

STDMETHODIMP CDScaler::CField::get_Hint(eDetectionHint *HintValue)
{
    *HintValue = m_Hint;
    return S_OK;
}

STDMETHODIMP CDScaler::CField::get_FieldNumber(DWORD* FieldNumber)
{
    *FieldNumber = m_FieldNumber;
    return S_OK;
}
