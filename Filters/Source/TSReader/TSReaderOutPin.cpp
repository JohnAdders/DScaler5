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
#include "TSReaderOutPin.h"
#include "TSReader.h"

CTSReaderOutPin::CTSReaderOutPin() :
    CDSOutputPin()
{
    LOG(DBGLOG_ALL, ("CTSReaderOutPin::CTSReaderOutPin\n"));
}

CTSReaderOutPin::~CTSReaderOutPin()
{
    LOG(DBGLOG_ALL, ("CDSOutputPin::~CDSOutputPin\n"));
}


STDMETHODIMP CTSReaderOutPin::GetCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::CheckCapabilities(DWORD *pCapabilities)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::IsFormatSupported(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::QueryPreferredFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetTimeFormat(GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::IsUsingTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetTimeFormat(const GUID *pFormat)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetDuration(LONGLONG *pDuration)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetStopPosition(LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetCurrentPosition(LONGLONG *pCurrent)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::ConvertTimeFormat(
                                LONGLONG *pTarget,
                                const GUID *pTargetFormat,
                                LONGLONG Source,
                                const GUID *pSourceFormat
                            )
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetPositions(
                        LONGLONG *pCurrent,
                        DWORD dwCurrentFlags,
                        LONGLONG *pStop,
                        DWORD dwStopFlags
                       )
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::SetRate(double dRate)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetRate(double *pdRate)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::GetPreroll(LONGLONG *pllPreroll)
{
    return E_FAIL;
}

STDMETHODIMP CTSReaderOutPin::Render(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTSReader*)m_Filter)->Render(this, pGraph);
}

STDMETHODIMP CTSReaderOutPin::Backout(IPin *ppinOut,IGraphBuilder *pGraph)
{
    return ((CTSReader*)m_Filter)->Backout(this, pGraph);
}
