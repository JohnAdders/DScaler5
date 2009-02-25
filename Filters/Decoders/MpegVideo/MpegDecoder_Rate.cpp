///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2003 Gabest
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
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.8  2005/02/17 09:31:48  adcockj
// Added analog blanking option
// Removed force Dscaler filter option
// Another proposed fix for menus
//
// Revision 1.7  2005/02/03 13:40:55  adcockj
// Improved seek support
//
// Revision 1.6  2004/07/21 15:05:25  adcockj
// Fixed some issues with ff & rew
//
// Revision 1.5  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.4  2004/05/25 16:59:29  adcockj
// fixed issues with new buffered pin
//
// Revision 1.3  2004/04/08 16:41:57  adcockj
// Tidy up subpicture support
//
// Revision 1.2  2004/03/06 20:50:29  adcockj
// Put in correct ff rate supported
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MpegDecoder.h"
#include "DSInputPin.h"

HRESULT CMpegDecoder::SetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData)
{
    bool fRefresh = false;

    switch(dwPropID)
    {
    case AM_RATE_SimpleRateChange:
        {
            CProtectCode WhileVarInScope(&m_RateLock);
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

HRESULT CMpegDecoder::GetPropSetRate(DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropertyData, DWORD cbPropData, DWORD *pcbReturned)
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

HRESULT CMpegDecoder::SupportPropSetRate(DWORD dwPropID, DWORD *pTypeSupport)
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

