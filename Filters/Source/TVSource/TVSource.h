///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
//
// You should have received a copy of the GNU General Public
// License along with this package; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "resource.h"       // main symbols
#include "DSBaseFilter.h"

// {1E3EAB17-274F-45a5-8E45-F70B3D97AB26}
DEFINE_GUID(CLSID_CTVSource, 0x1e3eab17, 0x274f, 0x45a5, 0x8e, 0x45, 0xf7, 0xb, 0x3d, 0x97, 0xab, 0x26);

class CTVSource :
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IFileSourceFilter
{
public:
// expand out the usual entries so that we can register out file extensions
BEGIN_CLASS_REGISTRY_TABLE(CTVSource)
    DEFAULT_CLASS_REGISTRY_ENTRIES("{1E3EAB17-274F-45a5-8E45-F70B3D97AB26}", "TVSource Filter Class", "Filter.TVSource.1", "Filter.TVSource", "both")
    REGISTRY_KEY(HKEY_CLASSES_ROOT, "Media Type\\Extensions\\.tv", 0, NULL, REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "Source Filter", "{1E3EAB17-274F-45a5-8E45-F70B3D97AB26}", REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "PerceivedType", "video", REGFLAG_NORMAL)
    REGISTRY_KEY(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Multimedia\\WMPlayer\\Extensions\\.tv", 0, NULL, REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "Runtime", "7", REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "Permissions", "15", REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "UserApprovedOwning", "yes", REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "PerceivedType", "video", REGFLAG_NORMAL)
END_CLASS_REGISTRY_TABLE()
IMPLEMENT_AGGREGATABLE_UNKNOWN(CTVSource)
IMPLEMENT_GENERIC_CLASS_FACTORY(CTVSource)
IMPLEMENT_CREATE_AGGREGATABLE_INSTANCE(CTVSource)
BEGIN_INTERFACE_TABLE(CTVSource)
    IMPLEMENTS_INTERFACE(IAmFreeSoftwareLicensed)
    IMPLEMENTS_INTERFACE(IBaseFilter)
    IMPLEMENTS_INTERFACE(IMediaFilter)
    IMPLEMENTS_INTERFACE(ISpecifyPropertyPages)
    IMPLEMENTS_INTERFACE(IMediaParams)
    IMPLEMENTS_INTERFACE(IMediaParamInfo)
    IMPLEMENTS_INTERFACE(IPersistStream)
    IMPLEMENTS_INTERFACE_AS(IPersist, IPersistStream)
    IMPLEMENTS_INTERFACE(ISaveDefaults)
    IMPLEMENTS_INTERFACE(IFileSourceFilter)
END_INTERFACE_TABLE()

public:
    CTVSource();
    ~CTVSource();


BEGIN_PARAM_LIST()
END_PARAM_LIST()

    enum eTVSourceFilterParams
    {
        PARAMS_LASTONE,
    };

// IBaseFilter
public:
    STDMETHOD(GetClassID)(CLSID __RPC_FAR *pClassID);


// IAmFreeSoftwareLicensed
public:
    STDMETHOD(get_Name)(BSTR* Name);
    STDMETHOD(get_License)(eFreeLicense* License);
    STDMETHOD(get_Authors)(BSTR* Authors);

// IFileSourceFilter
public:
    STDMETHOD(Load)(LPCOLESTR pszFileName,const AM_MEDIA_TYPE *pmt);
    STDMETHOD(GetCurFile)(LPOLESTR *ppszFileName, AM_MEDIA_TYPE *pmt);

public:
    HRESULT NotifyFormatChange(const AM_MEDIA_TYPE* pMediaType, CDSBasePin* pPin);
    HRESULT NotifyConnected(CDSBasePin* pPin);
    HRESULT ProcessSample(IMediaSample* InSample, AM_SAMPLE2_PROPERTIES* pSampleProperties, CDSBasePin* pPin);
    HRESULT CreateSuitableMediaType(AM_MEDIA_TYPE* pmt, CDSBasePin* pPin, int TypeNum);
    bool IsThisATypeWeCanWorkWith(const AM_MEDIA_TYPE* pmt, CDSBasePin* pPin);
    HRESULT SendOutLastSamples(CDSBasePin* pPin);
    HRESULT Flush(CDSBasePin* pPin);
    HRESULT NewSegmentInternal(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate, CDSBasePin* pPin);
    HRESULT Notify(IBaseFilter *pSelf, Quality q, CDSBasePin* pPin);
    HRESULT GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps, CDSBasePin* pPin);
    HRESULT Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, CDSBasePin* pPin);
    HRESULT Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned, CDSBasePin* pPin);
    HRESULT QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport, CDSBasePin* pPin);
    HRESULT Activate();
    HRESULT Deactivate();

public:
    HRESULT Render(IPin* pPin, IGraphBuilder* pGraphBuilder);
    HRESULT Backout(IPin* pPin, IGraphBuilder* pGraphBuilder);

private:

protected:
    HRESULT ParamChanged(DWORD dwParamIndex);
    HRESULT GetEnumText(DWORD dwParamIndex, WCHAR **ppwchText);

private:
    HRESULT PushFile();
    HRESULT BuildGraph(IGraphBuilder* pGraphBuilder);
    long getInteger(LPCWSTR settingName, long defaultValue);
    std::wstring getString(LPCWSTR settingName, LPCWSTR defaultValue);

    SI(IBaseFilter) m_NullFilter;
    bool m_bNotYetSetPins;

    std::wstring m_FileName;
    CCanLock m_WorkerThreadLock;
    static void ProcessingThread(void* pParam);
    HRESULT PushFile(HANDLE hFile);
    HANDLE m_ThreadStopEvent;
    HANDLE m_WorkerThread;
    HRESULT m_ThreadRetCode;
    DWORD m_ThreadId;
};

#define m_OutPin m_OutputPins[0]
