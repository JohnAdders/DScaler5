///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Gabest
///////////////////////////////////////////////////////////////////////////////
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSBasePin.h"
#include "DSCSSInputPin.h"
#include "DSOutputPin.h"
#include "DSBaseFilter.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"
#include "CSSauth.h"
#include "CSSscramble.h"


CDSCSSInputPin::CDSCSSInputPin() :
    CDSInputPin()
{
    LOG(DBGLOG_ALL, ("CDSCSSInputPin::CDSCSSInputPin\n"));
	m_varient = -1;
	memset(m_Challenge, 0, sizeof(m_Challenge));
	memset(m_KeyCheck, 0, sizeof(m_KeyCheck));
	memset(m_DiscKey, 0, sizeof(m_DiscKey));
	memset(m_TitleKey, 0, sizeof(m_TitleKey));
}

CDSCSSInputPin::~CDSCSSInputPin()
{
    LOG(DBGLOG_ALL, ("CDSCSSInputPin::~CDSCSSInputPin\n"));
}

HRESULT CDSCSSInputPin::GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    
    hr = CDSInputPin::GetSampleProperties(Sample, SampleProperties);
    CHECK(hr);

	if(GetMediaType()->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && SampleProperties->lActual == 2048)
    {
    	if(SampleProperties->pbBuffer[0x14] & 0x30)
    	{
    		CSSdescramble(SampleProperties->pbBuffer, m_TitleKey);
	    	SampleProperties->pbBuffer[0x14] &= ~0x30;
        }
	}
	SampleProperties->dwTypeSpecificFlags &= ~AM_UseNewCSSKey;
    return hr;
}


STDMETHODIMP CDSCSSInputPin::Set(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength)
{
	if(PropSet != AM_KSPROPSETID_CopyProt)
        return CDSInputPin::Set(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength);

	switch(Id)
	{
	case AM_PROPERTY_COPY_MACROVISION:
		break;
	case AM_PROPERTY_DVDCOPY_CHLG_KEY: // 3. auth: receive drive nonce word, also store and encrypt the buskey made up of the two nonce words
		{
			AM_DVDCOPY_CHLGKEY* pChlgKey = (AM_DVDCOPY_CHLGKEY*)pPropertyData;
			for(int i = 0; i < 10; i++)
				m_Challenge[i] = pChlgKey->ChlgKey[9-i];

			CSSkey2(m_varient, m_Challenge, &m_Key[5]);

			CSSbuskey(m_varient, m_Key, m_KeyCheck);
		}
		break;
	case AM_PROPERTY_DVDCOPY_DISC_KEY: // 5. receive the disckey
		{
			AM_DVDCOPY_DISCKEY* pDiscKey = (AM_DVDCOPY_DISCKEY*)pPropertyData; // pDiscKey->DiscKey holds the disckey encrypted with itself and the 408 disckeys encrypted with the playerkeys

			bool fSuccess = false;

			for(int j = 0; j < g_nPlayerKeys; j++)
			{
				for(int k = 1; k < 409; k++)
				{
                    int i;
					BYTE DiscKey[6];
					for(i = 0; i < 5; i++)
						DiscKey[i] = pDiscKey->DiscKey[k*5+i] ^ m_KeyCheck[4-i];
					DiscKey[5] = 0;

					CSSdisckey(DiscKey, g_PlayerKeys[j]);

					BYTE Hash[6];
					for(i = 0; i < 5; i++)
						Hash[i] = pDiscKey->DiscKey[i] ^ m_KeyCheck[4-i];
					Hash[5] = 0;

					CSSdisckey(Hash, DiscKey);

					if(!memcmp(Hash, DiscKey, 6))
					{
						memcpy(m_DiscKey, DiscKey, 6);
						j = g_nPlayerKeys;
						fSuccess = true;
						break;
					}
				}
			}

			if(!fSuccess)
				return E_FAIL;
		}
		break;
	case AM_PROPERTY_DVDCOPY_DVD_KEY1: // 2. auth: receive our drive-encrypted nonce word and decrypt it for verification
		{
            int i;
			AM_DVDCOPY_BUSKEY* pKey1 = (AM_DVDCOPY_BUSKEY*)pPropertyData;
			for(i = 0; i < 5; i++)
				m_Key[i] =  pKey1->BusKey[4-i];

			m_varient = -1;

			for(i = 31; i >= 0; i--)
			{
				CSSkey1(i, m_Challenge, m_KeyCheck);

				if(memcmp(m_KeyCheck, &m_Key[0], 5) == 0)
					m_varient = i;
			}
		}
		break;
	case AM_PROPERTY_DVDCOPY_REGION:
		break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
		break;
	case AM_PROPERTY_DVDCOPY_TITLE_KEY: // 6. receive the title key and decrypt it with the disc key
		{
			AM_DVDCOPY_TITLEKEY* pTitleKey = (AM_DVDCOPY_TITLEKEY*)pPropertyData;
			for(int i = 0; i < 5; i++)
				m_TitleKey[i] = pTitleKey->TitleKey[i] ^ m_KeyCheck[4-i];
			m_TitleKey[5] = 0;
			CSStitlekey(m_TitleKey, m_DiscKey);
		}
		break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP CDSCSSInputPin::Get(REFGUID PropSet, ULONG Id, LPVOID pInstanceData, ULONG InstanceLength, LPVOID pPropertyData, ULONG DataLength, ULONG* pBytesReturned)
{
	if(PropSet != AM_KSPROPSETID_CopyProt)
		return CDSInputPin::Get(PropSet, Id, pInstanceData, InstanceLength, pPropertyData, DataLength, pBytesReturned);

	switch(Id)
	{
	case AM_PROPERTY_DVDCOPY_CHLG_KEY: // 1. auth: send our nonce word
		{
			AM_DVDCOPY_CHLGKEY* pChlgKey = (AM_DVDCOPY_CHLGKEY*)pPropertyData;
			for(int i = 0; i < 10; i++)
				pChlgKey->ChlgKey[i] = 9 - (m_Challenge[i] = i);
			*pBytesReturned = sizeof(AM_DVDCOPY_CHLGKEY);
		}
		break;
	case AM_PROPERTY_DVDCOPY_DEC_KEY2: // 4. auth: send back the encrypted drive nonce word to finish the authentication
		{
			AM_DVDCOPY_BUSKEY* pKey2 = (AM_DVDCOPY_BUSKEY*)pPropertyData;
			for(int i = 0; i < 5; i++)
				pKey2->BusKey[4-i] = m_Key[5+i];
			*pBytesReturned = sizeof(AM_DVDCOPY_BUSKEY);
		}
		break;
	case AM_PROPERTY_DVDCOPY_REGION:
		{
			DVD_REGION* pRegion = (DVD_REGION*)pPropertyData;
			pRegion->RegionData = 0;
			pRegion->SystemRegion = 0;
			*pBytesReturned = sizeof(DVD_REGION);
		}
		break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
		{
			AM_DVDCOPY_SET_COPY_STATE* pState = (AM_DVDCOPY_SET_COPY_STATE*)pPropertyData;
			pState->DVDCopyState = AM_DVDCOPYSTATE_AUTHENTICATION_REQUIRED;
			*pBytesReturned = sizeof(AM_DVDCOPY_SET_COPY_STATE);
		}
		break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}

STDMETHODIMP CDSCSSInputPin::QuerySupported(REFGUID PropSet, ULONG Id, ULONG* pTypeSupport)
{
	if(PropSet != AM_KSPROPSETID_CopyProt)
		return CDSInputPin::QuerySupported(PropSet, Id, pTypeSupport);

	switch(Id)
	{
	case AM_PROPERTY_COPY_MACROVISION:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_CHLG_KEY:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_DEC_KEY2:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET;
		break;
	case AM_PROPERTY_DVDCOPY_DISC_KEY:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_DVD_KEY1:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_REGION:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_SET_COPY_STATE:
		*pTypeSupport = KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET;
		break;
	case AM_PROPERTY_DVDCOPY_TITLE_KEY:
		*pTypeSupport = KSPROPERTY_SUPPORT_SET;
		break;
	default:
		return E_PROP_ID_UNSUPPORTED;
	}

	return S_OK;
}