// ts2dvr.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "initguid.h"
DEFINE_GUID(CLSID_CAudioDecoder, 0xd2ca75c2, 0x5a1, 0x4915, 0x88, 0xa8, 0xd4, 0x33, 0xf8, 0x76, 0xd1, 0x86);

#include "yacl\include\combook.cpp"

using namespace std;

void SaveGraphFile(SI(IGraphBuilder)& graph, WCHAR* wszPath) ;

__inline void check(HRESULT hr)
{
    if(FAILED(hr))
    {
        throw logic_error("Call failed");
    }
}
class RegisterInRotROT
{
public:
    RegisterInRotROT(IUnknown *pUnkGraph)
    {
        SI(IRunningObjectTable) rot;
        check(GetRunningObjectTable(0, rot.GetReleasedInterfaceReference()));

        WCHAR wsz[256];
        wsprintfW(wsz, L"FilterGraph %08x pid %08x", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

        SI(IMoniker) moniker;
        check(CreateItemMoniker(L"!", wsz, moniker.GetReleasedInterfaceReference()));

        check(rot->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, moniker.GetNonAddRefedInterface(), &m_Register));
    }

    ~RegisterInRotROT()
    {
        SI(IRunningObjectTable) rot;
        check(GetRunningObjectTable(0, rot.GetReleasedInterfaceReference()));
        rot->Revoke(m_Register);
    }
private:
    DWORD m_Register;
};

void SaveGraphFile(SI(IGraphBuilder)& graph, WCHAR* wszPath)
{
    const WCHAR wszStreamName[] = L"ActiveMovieGraph";

    SI(IStorage) storage;
    check(StgCreateDocfile(
        wszPath,
        STGM_CREATE | STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
        0, storage.GetReleasedInterfaceReference()));

    SI(IStream) stream;
    check(storage->CreateStream(
        wszStreamName,
        STGM_WRITE | STGM_CREATE | STGM_SHARE_EXCLUSIVE,
        0, 0, stream.GetReleasedInterfaceReference()));

    SI(IPersistStream) persist(graph);

    persist->Save(stream.GetNonAddRefedInterface(), TRUE);

    check(storage->Commit(STGC_DEFAULT));
}



void convertTs2Dvr(string& tsfilename)
{
    SI(IGraphBuilder) graph;

    check(graph.CreateInstance(CLSID_FilterGraph, CLSCTX_INPROC_SERVER));

    RegisterInRotROT rot(graph.GetNonAddRefedInterface());

    SI(IMediaControl) control(graph);

    SI(IFilterGraph2) graph2(graph);
    SI(IMediaEvent) mediaEvent(graph);

    SI(IBaseFilter)    inputFileFilter;
    check(graph->AddSourceFilter(_U(tsfilename.c_str()), _U(tsfilename.c_str()), inputFileFilter.GetReleasedInterfaceReference()));

    SI(IBaseFilter)    sink;

    check(sink.CreateInstance(CLSID_StreamBufferSink, CLSCTX_INPROC_SERVER));

    check(graph->AddFilter(sink.GetNonAddRefedInterface(), L"Sink"));

    // Now we need to connect the output pin of the source
    // to the input pin of the parser.
    // obtain the output pin of the source filter
    // We use the filter's member function GetPin to do this.
    SI(IPin) outPin;

    check(inputFileFilter->FindPin(L"Output", outPin.GetReleasedInterfaceReference()));

    check(graph2->RenderEx(outPin.GetNonAddRefedInterface(), AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL));

    SI(IBaseFilter) demux;
    check(graph->FindFilterByName(L"MPEG-2 Demultiplexer", demux.GetReleasedInterfaceReference()));

    SI(IPin) inPin;

    check(demux->FindPin(L"Audio", outPin.GetReleasedInterfaceReference()));
    check(outPin->ConnectedTo(inPin.GetReleasedInterfaceReference()));

    check(graph->Disconnect(outPin.GetNonAddRefedInterface()));
    check(graph->Disconnect(inPin.GetNonAddRefedInterface()));

    SI(IBaseFilter)    infTee;
    check(infTee.CreateInstance(CLSID_InfTee, CLSCTX_INPROC_SERVER));
    check(graph->AddFilter(infTee.GetNonAddRefedInterface(), L"Inf Tee"));

    check(infTee->FindPin(L"Input", inPin.GetReleasedInterfaceReference()));
    check(graph2->Connect(outPin.GetNonAddRefedInterface(), inPin.GetNonAddRefedInterface()));

    check(infTee->FindPin(L"Output1", outPin.GetReleasedInterfaceReference()));

    SI(IBaseFilter)    audioDecoder;
    check(audioDecoder.CreateInstance(CLSID_CAudioDecoder, CLSCTX_INPROC_SERVER));
    check(graph->AddFilter(audioDecoder.GetNonAddRefedInterface(), L"DScaler Audio"));

    check(audioDecoder->FindPin(L"Audio In", inPin.GetReleasedInterfaceReference()));
    check(graph2->Connect(outPin.GetNonAddRefedInterface(), inPin.GetNonAddRefedInterface()));

    check(audioDecoder->FindPin(L"Audio Out", outPin.GetReleasedInterfaceReference()));

    SI(IBaseFilter)    audioRender;
    check(audioRender.CreateInstance(CLSID_DSoundRender, CLSCTX_INPROC_SERVER));
    check(graph->AddFilter(audioRender.GetNonAddRefedInterface(), L"DShow Audio Renderer"));

    check(audioRender->FindPin(L"Audio Input pin (rendered)", inPin.GetReleasedInterfaceReference()));

    check(graph2->Connect(outPin.GetNonAddRefedInterface(), inPin.GetNonAddRefedInterface()));

    check(infTee->FindPin(L"Output2", outPin.GetReleasedInterfaceReference()));

    check(graph2->RenderEx(outPin.GetNonAddRefedInterface(), AM_RENDEREX_RENDERTOEXISTINGRENDERERS, NULL));

    check(graph->SetDefaultSyncSource());

    SaveGraphFile(graph, L"Graph.grf");

    SI(IStreamBufferSink) sinkControl(sink);

    check(sinkControl->LockProfile(NULL));
    SI(IUnknown) rec;
    check(sinkControl->CreateRecorder(L"C:\\MyRecording.dvr-ms",
                                            RECORDING_TYPE_CONTENT,
                                            rec.GetReleasedInterfaceReference()));

    SI(IStreamBufferRecordControl) recControl(rec);

    REFERENCE_TIME rtStart = 0;
    check(recControl->Start(&rtStart));

    check(control->Run());

    long evCode;
    check(mediaEvent->WaitForCompletion(INFINITE, &evCode));

    check(recControl->Stop(0));
    check(control->Stop());
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        cout << "Usage : " << endl;
        cout << "ts2dvr tsfilename " << endl;
        cout << endl;
        cout << "(c) 2006 John Adcock" << endl;
    }

    string tsfilename(argv[1]);

    try
    {
        check(CoInitializeEx(NULL, COINIT_MULTITHREADED));
        convertTs2Dvr(tsfilename);
    }
    catch(exception &e)
    {
        cout << "Failed with message " << e.what() << endl;
    }

    CoUninitialize();

    return 0;
}

