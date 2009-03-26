///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2004 Gabest
//  http://www.gabest.org
//
///////////////////////////////////////////////////////////////////////////////
//
//  This Program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//
//  This Program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with GNU Make; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//  http://www.gnu.org/copyleft/gpl.html
//
///////////////////////////////////////////////////////////////////////////////
//
// This file was based on the mpeg2dec filter which is part of the MPC
// program see http://sf.net/projects/guliverkli/ for more details
//
// Changes made to files by John Adcock 06/02/04
//  - Removed use of MFC
//  - Replaced use of ATL with YACL
//  - Replaced Baseclasses with FilterLib
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioDecoder.h"
#include "DSInputPin.h"

#define MAX_SPEED 4

HRESULT CAudioDecoder::SetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData)
{
    bool fRefresh = false;

    switch(dwPropID)
    {
    case AM_RATE_SimpleRateChange:
        {
            AM_SimpleRateChange* p = (AM_SimpleRateChange*)pPropertyData;
            if(!m_CorrectTS) return E_PROP_ID_UNSUPPORTED;
            m_ratechange.Rate = p->Rate;
            m_ratechange.StartTime = p->StartTime;
            LOG(DBGLOG_FLOW, ("Simple Rate Change StartTime=%I64d, Rate=%d\n", p->StartTime, p->Rate));
        }
        break;
    case AM_RATE_CorrectTS:
        {
            LONG* p = (LONG*)pPropertyData;
            m_CorrectTS = (*p != 0);
            LOG(DBGLOG_FLOW, ("Rate Change Correct TS =%d\n", m_CorrectTS));
        }
        break;
    case AM_RATE_UseRateVersion:
        {
            if(*(WORD*)pPropertyData == 0x0101)
            {
                LOG(DBGLOG_FLOW, ("Rate Change 1.1\n"));
                m_CorrectTS = true;
                // todo get 1.1 working properly
                return E_UNEXPECTED;
            }
            else if(*(WORD*)pPropertyData == 0x0100)
            {
                LOG(DBGLOG_FLOW, ("Rate Change 1.0\n"));
                m_CorrectTS = true;
            }
            else
            {
                return E_UNEXPECTED;
            }
        }
        break;
    default:
        return E_PROP_ID_UNSUPPORTED;
    }

    return S_OK;
}

HRESULT CAudioDecoder::GetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned)
{
    switch(dwPropID)
    {
    case AM_RATE_MaxFullDataRate:
        {
            LOG(DBGLOG_FLOW, ("MaxFullDataRate\n"));
            AM_MaxFullDataRate* p = (AM_MaxFullDataRate*)pPropertyData;
            // this is not what is says you're supposed to return in the documentation
            // but doing that seems to screw things up
            // Docs say 10000 / MAX_SPEED
            *p = 10000 * MAX_SPEED;
            *pcbReturned = sizeof(AM_MaxFullDataRate);
        }
        break;
    case AM_RATE_QueryFullFrameRate:
        {
            LOG(DBGLOG_FLOW, ("QueryFullFrameRate\n"));
            AM_QueryRate* p = (AM_QueryRate*)pPropertyData;
            // this is not what is says you're supposed to return in the documentation
            // but doing that seems to screw things up
            // Docs say 10000 / MAX_SPEED
            p->lMaxForwardFullFrame = 10000 * MAX_SPEED;
            p->lMaxReverseFullFrame = 0;
            *pcbReturned = sizeof(AM_QueryRate);
        }
        break;
    case AM_RATE_QueryLastRateSegPTS:
        {
            LOG(DBGLOG_FLOW, ("QueryLastRateSegPTS\n"));
            REFERENCE_TIME* LastPTS = (REFERENCE_TIME*)pPropertyData;
            *pcbReturned = sizeof(REFERENCE_TIME);
        }
        break;
    default:
        return E_PROP_ID_UNSUPPORTED;
    }
    return S_OK;
}

HRESULT CAudioDecoder::SupportPropSetRate(DWORD dwPropID, DWORD *pTypeSupport)
{
    switch(dwPropID)
    {
    case AM_RATE_SimpleRateChange:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    case AM_RATE_MaxFullDataRate:
        *pTypeSupport = KSPROPERTY_SUPPORT_GET;
        break;
    case AM_RATE_QueryFullFrameRate:
        *pTypeSupport = KSPROPERTY_SUPPORT_GET;
        break;
    case AM_RATE_CorrectTS:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    case AM_RATE_UseRateVersion:
        *pTypeSupport = KSPROPERTY_SUPPORT_SET;
        break;
    case AM_RATE_QueryLastRateSegPTS:
        *pTypeSupport = KSPROPERTY_SUPPORT_GET;
        break;
    }
    return S_OK;
}

