////////////////////////////////////////////////////////////////////////////
// $Id: CPUID.h,v 1.2 2003-09-30 16:59:26 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2003/07/28 08:35:53  adcockj
// Added missing files
//
/////////////////////////////////////////////////////////////////////////////

#pragma once 

#define FEATURE_MMX            0x00000020
#define FEATURE_3DNOW          0x00000080
#define FEATURE_3DNOWEXT       0x00000100
#define FEATURE_MMXEXT         0x00000200
#define FEATURE_SSE            0x00002000
#define FEATURE_SSE2           0x00004000

extern "C"
{
    void CPU_SetupFeatureFlag(void);
    extern UINT CpuFeatureFlags;        // TRB 12/20/00 Processor capability flags
}
