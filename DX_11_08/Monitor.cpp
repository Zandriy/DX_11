/*
* Monitor.cpp
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#include "Monitor.h"
#include <vector>
#include <DXGI1_2.h>
#include <DXGITYPE.h>
#include <gdiplus.h>

namespace ZDX
{

	Monitor::Monitor(CComPtr<IDXGIAdapter1> adapter1)
		: m_adapter1(adapter1)
	{
		init();
	}

	Monitor::~Monitor()
	{
	}

	bool Monitor::init()
	{
		using namespace std;

		// Iterating over all adapters to get all outputs
		vector<CComPtr<IDXGIOutput>> outputs;

		CComPtr<IDXGIOutput> DXGI_output;
		for (int i = 0; (m_adapter1)->EnumOutputs(i, &DXGI_output) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_OUTPUT_DESC outputDesc;
			DXGI_output->GetDesc(&outputDesc);

			if (outputDesc.AttachedToDesktop)
			{
				outputs.push_back(DXGI_output);
			}

			DXGI_output.Release();
		}

		if (outputs.size() == 0)
			return false;

		// Creating device for each adapter that has the output
		D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_9_1;
		HRESULT hr = D3D11CreateDevice((m_adapter1), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &m_D3D_device, &fl, &m_D3D_device_context);
		if (FAILED(hr))
		{
			return false;
		}

		for (std::vector<CComPtr<IDXGIOutput>>::iterator OutputIter = outputs.begin();
			OutputIter != outputs.end();
			OutputIter++)
		{
			m_DXGI_output1 = *OutputIter;
			CComQIPtr<IDXGIDevice1> spDXGIDevice = m_D3D_device;
			if (!m_DXGI_output1 || !spDXGIDevice)
				continue;

			hr = m_DXGI_output1->DuplicateOutput(spDXGIDevice, &m_DXGI_output_duplication);
			if (FAILED(hr))
				continue;
		}

		return true;
	}

} // namespace ZDX
