////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the
// DMO project wizard.  That code is (c) Microsoft Corporation
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

#pragma once

class DECLSPEC_UUID("2ebd5238-53a5-11d7-b843-0002a5623377") CGenericPropPage;

STDAPI DMODllRegisterVideoDMO(LPCWSTR Name, const CLSID& Clsid);
STDAPI DMODllRegisterAudioDMO(LPCWSTR Name, const CLSID& Clsid, bool IsFloat);
STDAPI DMODllRegisterDeintDMO(LPCWSTR Name, const CLSID& Clsid);
STDAPI DMODllUnregisterVideoDMO(const CLSID& Clsid);
STDAPI DMODllUnregisterAudioDMO(const CLSID& Clsid);
STDAPI DMODllUnregisterDeintDMO(const CLSID& Clsid);

#define countof(Array) (sizeof(Array)/sizeof(Array[0]))
