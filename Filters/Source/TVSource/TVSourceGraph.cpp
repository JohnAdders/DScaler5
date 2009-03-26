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

#include "stdafx.h"
#include "TVSource.h"
#include "MoreUuids.h"

HRESULT getCaptureCardFilter(std::wstring cardName, SI(IBaseFilter)& filter)
{
    SI(ICreateDevEnum) devEnum;
    SI(IEnumMoniker) enumMoniker;
    HRESULT hr;

    hr = devEnum.CreateInstance(CLSID_SystemDeviceEnum, CLSCTX_INPROC_SERVER);
    if(FAILED(hr)) return hr;

    // Create an enumerator for the video capture category.
    hr = devEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, enumMoniker.GetReleasedInterfaceReference(), 0);
    if(FAILED(hr)) return hr;

    // we get false for empty which means that there are no capture cards
    if(hr == S_FALSE)
    {
        return E_POINTER;
    }

    SI(IMoniker) moniker;
    while (enumMoniker->Next(1, moniker.GetReleasedInterfaceReference(), NULL) == S_OK)
    {
        SI(IPropertyBag) propBag;
        hr = moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)propBag.GetReleasedInterfaceReference());
        if (FAILED(hr))
        {
            continue;  // Skip this one, maybe the next one will work.
        }

        // Find the description or friendly name.
        VARIANT varName;
        VariantInit(&varName);
        hr = propBag->Read(L"Description", &varName, 0);
        if (FAILED(hr))
        {
            hr = propBag->Read(L"FriendlyName", &varName, 0);
        }
        if (SUCCEEDED(hr))
        {
            // Add it to the application's list box.
            if(cardName == varName.bstrVal)
            {
                return moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)filter.GetReleasedInterfaceReference());
            }
            VariantClear(&varName);
        }
    }
    return E_POINTER;
}

HRESULT CTVSource::BuildGraph(IGraphBuilder* pGraphBuilder)
{
    std::wstring captureName(getString(L"CaptureCard", L""));
    if(captureName.empty())
    {
        return E_POINTER;
    }

    SI(IBaseFilter) capFilter;
    HRESULT hr = getCaptureCardFilter(captureName, capFilter);
    if(FAILED(hr)) return hr;

    hr = pGraphBuilder->AddFilter(capFilter.GetNonAddRefedInterface() , L"Capture Filter");

    // TODO: setup TV format and output pin formats

    SI(ICaptureGraphBuilder2) captureBuilder;

    // Create the Capture Graph Builder.
    hr = captureBuilder.CreateInstance(CLSID_CaptureGraphBuilder2, CLSCTX_INPROC_SERVER);
    if(FAILED(hr)) return hr;

    hr = captureBuilder->SetFiltergraph(pGraphBuilder);
    if(FAILED(hr)) return hr;

    hr = captureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, capFilter.GetNonAddRefedInterface(), NULL, NULL);
    if(FAILED(hr)) return hr;

    // TODO: setup tuning and crossbar

    return S_OK;
}
