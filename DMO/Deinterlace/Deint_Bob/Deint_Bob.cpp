#include "stdafx.h"
#include "Deint_Bob.h"

static ParamInfo* g_params = NULL;

// Bob method requires :
//  1 field to operate on
//  Doesn't introduce any delay
//  And has the lowest complexity possible
CDeint_Bob::CDeint_Bob() :
    CDeintDMO(1, 0, 0)
{
	// Initialize parameters in IMediaParam
	InitParams(1, &GUID_TIME_REFERENCE, 0, 0, 0, g_params);

}

CDeint_Bob::~CDeint_Bob()
{
}

void CDeint_Bob::DoDeinterlacingMethod(DMO_OUTPUT_DATA_BUFFER *pOutputBuffer)
{
    ProcessSingleFrame(pOutputBuffer);
}


HRESULT CDeint_Bob::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    return S_OK;
}

HRESULT CDeint_Bob::GetClassID(CLSID *pClsid)
{
	// Check for valid pointer
	if( NULL == pClsid )
	{
		return E_POINTER;
	}

	*pClsid = __uuidof(CDeint_Bob);
	return S_OK;

} // GetClassID

STDMETHODIMP CDeint_Bob::GetName(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_NAME))
    {
        return Result.CopyTo(Name);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CDeint_Bob::GetLicense(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CDeint_Bob::GetAuthors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_AUTHORS))
    {
        return Result.CopyTo(Authors);
    }
    else
    {
        return E_UNEXPECTED;
    }
}


///////////////////////
//
// Required ATL COM stuff
//
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(__uuidof(CDeint_Bob), CDeint_Bob)
END_OBJECT_MAP()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (DLL_PROCESS_DETACH == dwReason)
    {
		_Module.Term();
    }
	return TRUE;    // ok
}

STDAPI DllRegisterServer()
{
    return DMODllRegisterDeintDMO(L"Bob", __uuidof(CDeint_Bob));
}

STDAPI DllUnregisterServer()
{
    return DMODllUnregisterDeintDMO(__uuidof(CDeint_Bob));
}
