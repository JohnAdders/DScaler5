//------------------------------------------------------------------------------
// File: stdafx.h
//
// Desc: Standard include file.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

// stdafx.h : include file for standard system include files,
//      or project specific include files that are used frequently,
//      but are changed infrequently

#if !defined(AFX_STDAFX_H__537C8F51_48A5_4215_98BB_E2DE3E9FCEDE__INCLUDED_)
#define AFX_STDAFX_H__537C8F51_48A5_4215_98BB_E2DE3E9FCEDE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#pragma warning(disable:4786)       
#include <deque>
#pragma warning(default:4786)       

#define _ATL_FREE_THREADED
#define _ATL_STATIC_REGISTRY

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <statreg.h>
#include <statreg.cpp>
#include <atlimpl.cpp>

#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmoimpl.h>
#include <mmreg.h>
#include <uuids.h>
#include <dvdmedia.h>
#include <amvideo.h>
#include <medparam.h>

#endif // !defined(AFX_STDAFX_H__537C8F51_48A5_4215_98BB_E2DE3E9FCEDE__INCLUDED)
