///////////////////////////////////////////////////////////////////////////////
// $Id: Params.h,v 1.2 2004-02-12 17:06:43 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
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

#include <medparam.h>
#include "ProtectCode.h"

#define BEGIN_PARAM_LIST() private: \
    ParamInfo* _GetParamList(DWORD* pCount = NULL) { \
        static ParamInfo _params[] = { 

#define DEFINE_PARAM_INT(min, max, neutral, unit, label) {{MPT_INT, 0, min, max, neutral, unit, label, }, neutral,},
#define DEFINE_PARAM_FLOAT(min, max, neutral, unit, label) {{MPT_FLOAT, 0, min, max, neutral, unit, label,}, neutral,},
#define DEFINE_PARAM_BOOL(neutral, label) {{MPT_BOOL, 0, 0, 1, neutral, L"None", label,}, neutral,},
#define DEFINE_PARAM_ENUM(max, neutral, label) {{MPT_ENUM, 0, 0, max, neutral, L"None", label,}, neutral,},

#define END_PARAM_LIST()         {{MPT_INT, 0, 0, 1, 0, L"", L"",}, 0, }, \
                  }; if(pCount != NULL) *pCount = sizeof(_params)/sizeof(ParamInfo) - 1; return _params; }; \

#define GetParamFloat(Index) ((float)_GetParamList()[Index].Value)
#define GetParamInt(Index) ((long)_GetParamList()[Index].Value)
#define GetParamBool(Index) ((BOOL)_GetParamList()[Index].Value)
#define GetParamEnum(Index) ((long)_GetParamList()[Index].Value)

class CParams : 
    public IMediaParamInfo,
    public IMediaParams,
	public IPersistStream,
    public CCanLock
{
public:
    CParams():m_fDirty(false) {};

IMPLEMENT_UNKNOWN(CParams)

BEGIN_INTERFACE_TABLE(CParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IPersistStream)
	IMPLEMENTS_INTERFACE(IPersist)
END_INTERFACE_TABLE()

protected:
    typedef HRESULT LPFN_GETENUMTEXT(WCHAR **ppwchText);

    typedef struct
    {
        MP_PARAMINFO      MParamInfo;
        MP_DATA           Value;
    } ParamInfo;


// IMediaParams
public:
    STDMETHOD(GetParam)(DWORD dwParamIndex, MP_DATA *pValue)
    {
        if(pValue == NULL)
        {
            return E_POINTER;
        }
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        if(dwParamIndex < ParamCount)
        {
            CProtectCode WhileVarInScope(this);
            *pValue = Params[dwParamIndex].Value;
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

    STDMETHOD(SetParam)(DWORD dwParamIndex,MP_DATA value)
    {
        DWORD ParamCount(0);
        ParamInfo* Params = _GetParamList(&ParamCount);
        if(dwParamIndex < ParamCount)
        {
            CProtectCode WhileVarInScope(this);
            Params[dwParamIndex].Value = value;
            m_fDirty = TRUE;
            ParamChanged(dwParamIndex);
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    STDMETHOD(AddEnvelope)(DWORD dwParamIndex,DWORD cPoints,MP_ENVELOPE_SEGMENT *ppEnvelope)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }
    STDMETHOD(FlushEnvelope)(DWORD dwParamIndex,REFERENCE_TIME refTimeStart,REFERENCE_TIME refTimeEnd)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }
    STDMETHOD(SetTimeFormat)(GUID guidTimeFormat,MP_TIMEDATA mpTimeData)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }

// IMediaParamInfo
public:
    STDMETHOD(GetParamCount)(DWORD* pdwParams)
    {
        if(pdwParams == NULL)
        {
            return E_POINTER;
        }
        _GetParamList(pdwParams);
        return S_OK;
    };
    STDMETHOD(GetParamInfo)(DWORD dwParamIndex,MP_PARAMINFO* pInfo)
    {
        if(pInfo == NULL)
        {
            return E_POINTER;
        }
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        if(dwParamIndex >= ParamCount)
        {
            return E_INVALIDARG;
        }
        memcpy(pInfo, &(Params[dwParamIndex].MParamInfo), sizeof(MP_PARAMINFO));
        return S_OK;
    };
    STDMETHOD(GetParamText)(DWORD dwParamIndex,WCHAR **ppwchText)
    {
        if(ppwchText == NULL)
        {
            return E_POINTER;
        }
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        if(dwParamIndex >= ParamCount)
        {
            return E_INVALIDARG;
        }
        if(Params[dwParamIndex].MParamInfo.mpType != MPT_ENUM)
        {
            *ppwchText = (WCHAR*)CoTaskMemAlloc((wcslen(Params[dwParamIndex].MParamInfo.szLabel) + wcslen(Params[dwParamIndex].MParamInfo.szUnitText) + 3) * sizeof(WCHAR));
            if(*ppwchText == NULL) return E_OUTOFMEMORY;
            wcscpy(*ppwchText, Params[dwParamIndex].MParamInfo.szLabel);
            wcscpy(*ppwchText + wcslen(Params[dwParamIndex].MParamInfo.szLabel) + 1,
                    Params[dwParamIndex].MParamInfo.szUnitText);
            *(*ppwchText + wcslen(Params[dwParamIndex].MParamInfo.szLabel) + wcslen(Params[dwParamIndex].MParamInfo.szUnitText) + 1) = L'\0';
            return S_OK;
        }
        else
        {
            return GetEnumText(dwParamIndex, ppwchText);
        }
        return S_OK;
    }
    STDMETHOD(GetNumTimeFormats)(DWORD *pdwNumTimeFormats)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }
    STDMETHOD(GetSupportedTimeFormat)(DWORD dwFormatIndex,GUID *pguidTimeFormat)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }
    STDMETHOD(GetCurrentTimeFormat)( GUID *pguidTimeFormat,MP_TIMEDATA *pTimeData)
    {
        // \todo get working if required
        return E_NOTIMPL;
    }
	
    // IPersistStream Methods
	STDMETHOD(IsDirty)()
    {
	    return m_fDirty ? S_OK : S_FALSE;
    };
	STDMETHOD(Load)(IStream* pStm)
    {
	    ULONG ulSizeRead = 0;
	    HRESULT hr = S_OK;

	    if (NULL == pStm)
	    {
		    return E_POINTER;
	    }

	    DWORD NumParams;
	    hr = pStm->Read((void *)&NumParams, sizeof(NumParams), &ulSizeRead);
	    if (hr != S_OK || ulSizeRead < sizeof(NumParams))
	    {
		    return E_FAIL;
	    }

        DWORD ParamCount(0);
        _GetParamList(&ParamCount);

        for(DWORD i(0); i < NumParams && i < ParamCount; ++i)
        {
            MP_DATA Param;
	        hr = pStm->Read((void *)&Param, sizeof(Param), &ulSizeRead);
	        if (hr != S_OK || ulSizeRead < sizeof(Param))
	        {
		        return E_FAIL;
	        }
            SetParam(i, Param);
        }

	    m_fDirty = FALSE;

	    return hr;
    };

	STDMETHOD(Save)(IStream* pStm, BOOL fClearDirty)
    {
	    HRESULT hr = S_OK;
	    ULONG ulSizeWritten = 0;

	    if (NULL == pStm)
	    {
		    return E_POINTER;
	    }

        DWORD ParamCount(0);
         _GetParamList(&ParamCount);

	    hr = pStm->Write((void *)&ParamCount, sizeof(ParamCount), &ulSizeWritten);
	    if (hr != S_OK || ulSizeWritten < sizeof(ParamCount))
	    {
		    return E_FAIL;
	    }

        for(DWORD i(0); i < ParamCount; ++i)
        {
            MP_DATA Param;
            GetParam(i, &Param);
	        hr = pStm->Write((void *)&Param, sizeof(Param), &ulSizeWritten);
	        if (hr != S_OK || ulSizeWritten < sizeof(Param))
	        {
		        return E_FAIL;
	        }
        }

	    if (fClearDirty)
	    {
		    m_fDirty = FALSE;
	    }

	    return S_OK;
    };

	STDMETHOD(GetSizeMax)(ULARGE_INTEGER* pcbSize)
    {
	    if ( NULL == pcbSize )
		    return E_POINTER;

        DWORD ParamCount(0);
        _GetParamList(&ParamCount);

	    pcbSize->QuadPart = sizeof(ParamCount) + ParamCount * sizeof(MP_DATA);
	    return S_OK;
    };
public:
    void Lock() {CCanLock::Lock();};
    void Unlock() {CCanLock::Unlock();};

private:
    virtual ParamInfo* _GetParamList(DWORD* pCount) = 0;
    virtual HRESULT ParamChanged(DWORD dwParamIndex) = 0;

protected:
	bool	m_fDirty;
    virtual HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText) {return E_NOTIMPL;};

};

#define ASSERT(__x__) if(!(__x__)) DebugBreak();