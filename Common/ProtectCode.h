///////////////////////////////////////////////////////////////////////////////
// $Id$
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

class ICanLock
{
public:
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
};

class CCanLock :
    public ICanLock
{
public:
    CCanLock() 
	{
		InitializeCriticalSection(&m_Crit);
	}
    
	~CCanLock()
	{
		DeleteCriticalSection(&m_Crit);
	}

    void Lock()
	{
		EnterCriticalSection(&m_Crit);
	}
    
	void Unlock() 
	{
		LeaveCriticalSection(&m_Crit);
	}
private:
    CRITICAL_SECTION m_Crit;
};


class CProtectCode
{
public:
    CProtectCode(ICanLock* MasterObject)
    {
        m_Master = MasterObject;
        m_Master->Lock();
    }
    ~CProtectCode()
    {
        m_Master->Unlock();
    }
private:
    ICanLock* m_Master;
};
