///////////////////////////////////////////////////////////////////////////////
// $Id: Statistics.h,v 1.1 2004-12-20 08:51:50 adcockj Exp $
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

#include "ProtectCode.h"
#include "GenDMOProp.h"

#define BEGIN_STATISTICS_LIST() private: \
    StatisticsInfo* _GetStatsList(DWORD* pCount = NULL) { \
        static StatisticsInfo _stats[] = { 

#define DEFINE_STATISTIC(name) {name},

#define END_STATISTICS_LIST()         {NULL}, \
                  }; if(pCount != NULL) *pCount = countof(_stats) - 1; return _stats; };

class CHaveStatistics : 
    public IHaveStatistics
{
public:
    CHaveStatistics(){};

protected:
    typedef struct
    {
        const wchar_t* Name;
    } StatisticsInfo;


// IHaveStatistics
public:
    STDMETHOD(get_NumStatistics)(DWORD* Number)
    {
        _GetStatsList(Number);
        return S_OK;
    }

	STDMETHOD(get_StatisticName)(DWORD Index, BSTR* StatName)
    {
        DWORD Count(0);
        StatisticsInfo* pInfo = _GetStatsList(&Count);
        if(Index < Count)
        {
            *StatName = SysAllocString(pInfo[Index].Name);
            return S_OK;
        }
        else
        {
            return E_INVALIDARG;
        }
    }

private:
    virtual StatisticsInfo* _GetStatsList(DWORD* pCount) = 0;
};

#define ASSERT(__x__) if(!(__x__)) DebugBreak();