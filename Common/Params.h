///////////////////////////////////////////////////////////////////////////////
// $Id: Params.h,v 1.9 2004-10-28 09:05:25 adcockj Exp $
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
#include "GenDMOProp.h"

#define BEGIN_PARAM_LIST() private: \
    ParamInfo* _GetParamList(DWORD* pCount = NULL) { \
        static ParamInfo _params[] = { 

#define DEFINE_PARAM_INT(min, max, neutral, unit, label) {{MPT_INT, 0, min, max, neutral, unit, label, }, neutral,},
#define DEFINE_PARAM_FLOAT(min, max, neutral, unit, label) {{MPT_FLOAT, 0, min, max, neutral, unit, label,}, neutral,},
#define DEFINE_PARAM_BOOL(neutral, label) {{MPT_BOOL, 0, 0, 1, neutral, L"None", label,}, neutral,},
#define DEFINE_PARAM_ENUM(max, neutral, label) {{MPT_ENUM, 0, 0, max, neutral, L"None", label,}, neutral,},

#define END_PARAM_LIST()         {{MPT_INT, 0, 0, 1, 0, L"", L"",}, 0, }, \
                  }; if(pCount != NULL) *pCount = countof(_params) - 1; return _params; }; \

#define GetParamFloat(Index) ((float)_GetParamList()[Index].Value)
#define GetParamInt(Index) ((long)_GetParamList()[Index].Value)
#define GetParamBool(Index) ((BOOL)_GetParamList()[Index].Value)
#define GetParamEnum(Index) ((long)_GetParamList()[Index].Value)

class CParams : 
    public IMediaParamInfo,
    public IMediaParams,
    public IPersistStream,
    public ISaveDefaults,
    public CCanLock
{
public:
    CParams(LPCWSTR RegistryKey):m_fDirty(false), m_RegKey(RegistryKey) {};

IMPLEMENT_UNKNOWN(CParams)

BEGIN_INTERFACE_TABLE(CParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE(IPersist)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
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
        HRESULT hr = S_OK;
        ParamInfo* Params = _GetParamList(&ParamCount);
        if(dwParamIndex != DWORD_ALLPARAMS)
        {
            // normal case: change a single parameter
            if(dwParamIndex < ParamCount)
            {
                CProtectCode WhileVarInScope(this);
                MP_DATA OldValue = Params[dwParamIndex].Value;
                if(value < Params[dwParamIndex].MParamInfo.mpdMinValue)
                {
                    Params[dwParamIndex].Value = Params[dwParamIndex].MParamInfo.mpdMinValue;
                    return E_INVALIDARG;
                }
                else if(value > Params[dwParamIndex].MParamInfo.mpdMaxValue)
                {
                    Params[dwParamIndex].Value = Params[dwParamIndex].MParamInfo.mpdMaxValue;
                    return E_INVALIDARG;
                }
                else
                {
                    Params[dwParamIndex].Value = value;
                }
                m_fDirty = TRUE;
                hr = ParamChanged(dwParamIndex);
                if(FAILED(hr))
                {
                    Params[dwParamIndex].Value = OldValue;
                }
                return hr;
            }
            else
            {
                return E_INVALIDARG;
            }
        }
        else
        {
            // special case: change all parameters
            for(DWORD i(0); i < ParamCount; ++i)
            {
                CProtectCode WhileVarInScope(this);
                HRESULT hr2 = SetParam(i, value);
                if(FAILED(hr2) || (SUCCEEDED(hr) && hr2 > 0))
                {
                    hr = hr2;
                }

            }
            return hr;
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
    // ISaveDefaults Methods
public:
    STDMETHOD(SaveDefaultsToRegistry)()
    {
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        HKEY MySubKey;
        if(FAILED(GetRegistryKey(MySubKey)))
        {
            return E_UNEXPECTED;
        }
         
        for(DWORD i(0); i < ParamCount; ++i)
        {
            if(Params[i].MParamInfo.mpType != MPT_FLOAT)
            {
                DWORD dwValue = (DWORD)Params[i].Value;
                if(RegSetValueExW(MySubKey, Params[i].MParamInfo.szLabel, 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD)) != ERROR_SUCCESS)
                {
                    RegCloseKey(MySubKey);
                    return E_UNEXPECTED;
                }
            }
            else
            {

            }
        }

        RegCloseKey(MySubKey);

        return S_OK;
    }

    STDMETHOD(SaveDefaultToRegistry)(DWORD ParamIndex)
    {
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        HKEY MySubKey;
        if(FAILED(GetRegistryKey(MySubKey)))
        {
            return E_UNEXPECTED;
        }
         
        if(ParamIndex < ParamCount)
        {
            if(Params[ParamIndex].MParamInfo.mpType != MPT_FLOAT)
            {
                DWORD dwValue = (DWORD)Params[ParamIndex].Value;
                if(RegSetValueExW(MySubKey, Params[ParamIndex].MParamInfo.szLabel, 0, REG_DWORD, (BYTE*)&dwValue, sizeof(DWORD)) != ERROR_SUCCESS)
                {
                    RegCloseKey(MySubKey);
                    return E_UNEXPECTED;
                }
            }
            else
            {

            }
        }

        RegCloseKey(MySubKey);

        return S_OK;
    }

    STDMETHOD(LoadDefaultsFromRegistry)()
    {
        DWORD ParamCount(0);
        const ParamInfo* Params = _GetParamList(&ParamCount);
        HKEY MySubKey;
        if(FAILED(GetRegistryKey(MySubKey)))
        {
            return E_UNEXPECTED;
        }
        
        for(DWORD i(0); i < ParamCount; ++i)
        {
            if(Params[i].MParamInfo.mpType != MPT_FLOAT)
            {
                DWORD dwValue = (DWORD)Params[i].MParamInfo.mpdNeutralValue;
                DWORD RegType;
                DWORD Size;
                if(!RegQueryValueExW(MySubKey, Params[i].MParamInfo.szLabel, 0, &RegType, (BYTE*)&dwValue, &Size) != ERROR_SUCCESS)
                {
                    if(RegType == REG_DWORD)
                    {
                        if(Params[i].MParamInfo.mpType == MPT_INT)
                        {
                            int IntValue = *(int*)&dwValue;
                            SetParam(i, (MP_DATA)IntValue);
                        }
                        else
                        {
                            SetParam(i, (MP_DATA)dwValue);
                        }
                    }
                    else
                    {
                        SetParam(i, Params[i].MParamInfo.mpdNeutralValue);
                    }
                }
            }
            else
            {
                SetParam(i, Params[i].MParamInfo.mpdNeutralValue);
            }
        }

        RegCloseKey(MySubKey);

        return S_OK;

    }
public:
    void Lock() {CCanLock::Lock();};
    void Unlock() {CCanLock::Unlock();};

private:
    virtual ParamInfo* _GetParamList(DWORD* pCount) = 0;
    virtual HRESULT ParamChanged(DWORD dwParamIndex) = 0;
    HRESULT GetRegistryKey(HKEY& RegKey)
    {
        HKEY SoftwareKey;
        if(RegOpenKeyExW(HKEY_CURRENT_USER, L"Software", 0, KEY_CREATE_SUB_KEY, &SoftwareKey) != ERROR_SUCCESS)
        {
            return E_UNEXPECTED;
        }

        HKEY DScaler5Key;
        if(RegCreateKeyExW(SoftwareKey, L"DScaler5", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &DScaler5Key, NULL) != ERROR_SUCCESS)
        {
            RegCloseKey(SoftwareKey);
            return E_UNEXPECTED;
        }

        if(RegCreateKeyExW(DScaler5Key, m_RegKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &RegKey, NULL) != ERROR_SUCCESS)
        {
            RegCloseKey(DScaler5Key);
            RegCloseKey(SoftwareKey);
            return E_UNEXPECTED;
        }
        RegCloseKey(DScaler5Key);
        RegCloseKey(SoftwareKey);
        return S_OK;
    }
protected:
    bool    m_fDirty;
    virtual HRESULT GetEnumText(DWORD dwParamIndex, WCHAR** ppwchText) {return E_NOTIMPL;};
    LPCWSTR m_RegKey;

};

#define ASSERT(__x__) if(!(__x__)) DebugBreak();