///////////////////////////////////////////////////////////////////////////////
// $Id: EnumTwoPins.h,v 1.1.1.1 2003-04-30 13:01:20 adcockj Exp $
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

/////////////////////////////////////////////////////////////////////////////
// CDScaler
class ATL_NO_VTABLE CEnumTwoPins : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumPins
{
public:
	CEnumTwoPins();

BEGIN_COM_MAP(CEnumTwoPins)
	COM_INTERFACE_ENTRY(IEnumPins)
END_COM_MAP()

// IBaseFilter
public:
    STDMETHOD(Next)(ULONG cPins,IPin **ppPins, ULONG *pcFetched);
    STDMETHOD(Skip)(ULONG cPins);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumPins **ppEnum);
    
    void SetPins(IPin* Pin1, IPin* Pin2);
protected:
    ULONG m_Count;
    CComPtr<IPin> m_Pins[2];
};

