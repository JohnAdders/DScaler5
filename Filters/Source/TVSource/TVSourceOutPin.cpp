///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock
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

#include "stdafx.h"
#include "TVSourceOutPin.h"
#include "TVSource.h"

CTVSourceOutPin::CTVSourceOutPin() :
    CDSOutputPin()
{
    LOG(DBGLOG_ALL, ("CTVSourceOutPin::CTVSourceOutPin\n"));
}

CTVSourceOutPin::~CTVSourceOutPin()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::~CDSOutputPin\n"));
}


STDMETHODIMP CTVSourceOutPin::GetCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::CheckCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::IsFormatSupported(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::QueryPreferredFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetTimeFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::IsUsingTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::SetTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetDuration(LONGLONG *pDuration)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetStopPosition(LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::ConvertTimeFormat(
                                LONGLONG *pTarget,
                                const GUID *pTargetFormat,
                                LONGLONG Source,
                                const GUID *pSourceFormat
                            )
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::SetPositions(
                        LONGLONG *pCurrent,
                        DWORD dwCurrentFlags,
                        LONGLONG *pStop,
                        DWORD dwStopFlags
                       )
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::SetRate(double dRate)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetRate(double *pdRate)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::GetPreroll(LONGLONG *pllPreroll)
{
    return E_FAIL;
}

STDMETHODIMP CTVSourceOutPin::Render(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTVSource*)m_Filter)->Render(this, pGraph);
}

STDMETHODIMP CTVSourceOutPin::Backout(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTVSource*)m_Filter)->Backout(this, pGraph);
}
