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

class IHavePins : public IUnknown
{
public:
    virtual HRESULT GetPin(ULONG PinNum, IPin** pPin) = 0;
};

/////////////////////////////////////////////////////////////////////////////
// CDScaler
class CEnumPins : 
    public IEnumPins
{
public:

IMPLEMENT_UNKNOWN(CEnumPins)

BEGIN_INTERFACE_TABLE(CEnumPins)
    IMPLEMENTS_INTERFACE(IEnumPins)
END_INTERFACE_TABLE()

public:
    CEnumPins();
    ~CEnumPins();

// IBaseFilter
public:
    STDMETHOD(Next)(ULONG cPins,IPin **ppPins, ULONG *pcFetched);
    STDMETHOD(Skip)(ULONG cPins);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumPins **ppEnum);

    HRESULT SetFilter(IHavePins* Filter);
    
protected:
    IHavePins* m_Filter;
    LONG m_Count;
};

