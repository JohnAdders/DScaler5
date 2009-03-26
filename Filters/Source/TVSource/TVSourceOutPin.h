///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
//
// You should have received a copy of the GNU General Public
// License along with this package; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////


#pragma once
#include "resource.h"       // main symbols
#include "DSOutputPin.h"


class CTVSource;

class CTVSourceOutPin : public CDSOutputPin,
    public IStreamBuilder
{
public:

IMPLEMENT_UNKNOWN(CTVSourceOutPin)

BEGIN_INTERFACE_TABLE(CTVSourceOutPin)
    IMPLEMENTS_INTERFACE(IPin)
    IMPLEMENTS_INTERFACE(IPinFlowControl)
    IMPLEMENTS_INTERFACE(IQualityControl)
    IMPLEMENTS_INTERFACE(IMediaSeeking)
    IMPLEMENTS_INTERFACE(IKsPropertySet)
    IMPLEMENTS_INTERFACE(IAMLatency)
    IMPLEMENTS_INTERFACE(IStreamBuilder)
END_INTERFACE_TABLE()

public:
    CTVSourceOutPin();
    ~CTVSourceOutPin();

// IMediaSeeking
public:
    STDMETHOD(GetCapabilities)(DWORD *pCapabilities);
    STDMETHOD(CheckCapabilities)(DWORD *pCapabilities);
    STDMETHOD(IsFormatSupported)(const GUID *pFormat);
    STDMETHOD(QueryPreferredFormat)(GUID *pFormat);
    STDMETHOD(GetTimeFormat)(GUID *pFormat);
    STDMETHOD(IsUsingTimeFormat)(const GUID *pFormat);
    STDMETHOD(SetTimeFormat)(const GUID *pFormat);
    STDMETHOD(GetDuration)(LONGLONG *pDuration);
    STDMETHOD(GetStopPosition)(LONGLONG *pStop);
    STDMETHOD(GetCurrentPosition)(LONGLONG *pCurrent);
    STDMETHOD(ConvertTimeFormat)(
                                    LONGLONG *pTarget,
                                    const GUID *pTargetFormat,
                                    LONGLONG Source,
                                    const GUID *pSourceFormat
                                );
    STDMETHOD(SetPositions)(
                            LONGLONG *pCurrent,
                            DWORD dwCurrentFlags,
                            LONGLONG *pStop,
                            DWORD dwStopFlags
                           );
    STDMETHOD(GetPositions)(LONGLONG *pCurrent, LONGLONG *pStop);
    STDMETHOD(GetAvailable)(LONGLONG *pEarliest, LONGLONG *pLatest);
    STDMETHOD(SetRate)(double dRate);
    STDMETHOD(GetRate)(double *pdRate);
    STDMETHOD(GetPreroll)(LONGLONG *pllPreroll);

// IStreamBuilder

    STDMETHOD(Render)(IPin *ppinOut,IGraphBuilder *pGraph);
    STDMETHOD(Backout)(IPin *ppinOut,IGraphBuilder *pGraph);
};
