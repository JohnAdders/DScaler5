///////////////////////////////////////////////////////////////////////////////
// $Id: TSReader.h,v 1.3 2004-10-27 09:56:49 adcockj Exp $
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

extern "C"
{
#include "./libdvbpsi/vc++/inttypes.h"
#include "./libdvbpsi/vc++/config.h"
#include "./libdvbpsi/src/dvbpsi.h"
#include "./libdvbpsi/src/psi.h"
#include "./libdvbpsi/src/tables/pat.h"
#include "./libdvbpsi/src/descriptor.h"
#include "./libdvbpsi/src/tables/pmt.h"
}



DEFINE_GUID(CLSID_CTSReader, 0xe59f5154, 0xdac7, 0x44f0, 0x99, 0x7e, 0x9a, 0xc1, 0x9a, 0x17, 0x66, 0x92);

class CTSReader : 
    public CDSBaseFilter,
    public IAmFreeSoftwareLicensed,
    public IFileSourceFilter
{
public:
// expand out the usual entries so that we can register out file extensions
BEGIN_CLASS_REGISTRY_TABLE(CTSReader)
    DEFAULT_CLASS_REGISTRY_ENTRIES("{E59F5154-DAC7-44f0-997E-9AC19A176692}", "TSReader Filter Class", "Filter.TSReader.1", "Filter.TSReader", "both")
    REGISTRY_KEY(HKEY_CLASSES_ROOT, "Media Type\\Extensions\\.ts", 0, NULL, REGFLAG_NORMAL)
    REGISTRY_SUBKEY(0, "Source Filter", "{E59F5154-DAC7-44f0-997E-9AC19A176692}", REGFLAG_NORMAL)
END_CLASS_REGISTRY_TABLE()
IMPLEMENT_AGGREGATABLE_UNKNOWN(CTSReader)
IMPLEMENT_GENERIC_CLASS_FACTORY(CTSReader)
IMPLEMENT_CREATE_AGGREGATABLE_INSTANCE(CTSReader)
BEGIN_INTERFACE_TABLE(CTSReader)
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
	CTSReader();
    ~CTSReader();


BEGIN_PARAM_LIST()
END_PARAM_LIST()

    enum eTSReaderFilterParams
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

// fudge so that VC6 can compile
public:
    int m_GotPMTS;


private:

    SI(IMpeg2Demultiplexer) m_MpegDemux;
    bool m_bNotYetSetPins;

    wchar_t m_FileName[MAX_PATH * 2 + 1];
    CCanLock m_WorkerThreadLock;
    static void ProcessingThread(void* pParam);
    HRESULT PushFile(HANDLE hFile);
    HANDLE m_ThreadStopEvent;
    HANDLE m_WorkerThread;
	HRESULT m_ThreadRetCode;
	DWORD m_ThreadId;
    static void StaticUpdatePAT(void* p_zero, dvbpsi_pat_t* p_pat);
    void UpdatePAT(dvbpsi_pat_t* p_pat);
    HRESULT AnalyseFile();
    void ClearPrograms();
    AM_MEDIA_TYPE* GetVideoMediaType();
    AM_MEDIA_TYPE* GetAC3MediaType();
    AM_MEDIA_TYPE* GetMPAMediaType();

    class CPSIData
    {
    public:
        CPSIData(dvbpsi_pat_program_t* pProgram, CTSReader* Parent);
        ~CPSIData();
        static void StaticUpdatePMT(void* pThis, dvbpsi_pmt_t* p_pmt);
        void UpdatePMT(dvbpsi_pmt_t* p_pmt);

        dvbpsi_handle m_PMTHandle;
        int m_pid;
        int m_program;
        int m_VideoPid;
        int m_AudioPid;
        CTSReader* m_Parent;
        int m_AudioType;
    };

    CPSIData** m_PSIDataArray;
    int m_NumPrograms;

};

#define m_OutPin m_OutputPins[0]
