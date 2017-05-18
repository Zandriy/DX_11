/*
* Desktop.cpp
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#include "Desktop.h"
#include <d3d11.h>
#include <DXGI1_2.h>
#include <DXGITYPE.h>
#include <gdiplus.h>

namespace ZDX
{

	Desktop::Desktop(int monitor_number)
		: m_active_monitor_number{ monitor_number }
	{
		CoInitialize(NULL);
		init();
	}


	Desktop::~Desktop()
	{
	}

	bool Desktop::acquire_next_buffer()
	{
		return false;
	}

	const BYTE* Desktop::get_buffer()
	{
		return m_buffer;
	}
	
	bool Desktop::save(const char* file_name)
	{
		return false;
	}

	void Desktop::prepare_buffer()
	{

	}

	bool Desktop::init()
	{
		using namespace Gdiplus;
		using namespace std;

		GdiplusStartupInput gdi_plus_startup_input;
		GdiplusStartup(&m_gdi_plus_token, &gdi_plus_startup_input, NULL);

		HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&m_DXGI_factory1));
		if (FAILED(hr))
		{	
			return false;
		}

		// Getting all adapters
		vector<CComPtr<IDXGIAdapter1>> DXGI_adapters;

		CComPtr<IDXGIAdapter1> adapter1;
		for (int i = 0; m_DXGI_factory1->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_adapters.push_back(adapter1);
			adapter1.Release();
		}

		for (auto adapter : DXGI_adapters)
		{
			m_monitors.emplace_back(adapter);
		}
		
		hr = m_WIC_factory.CoCreateInstance(CLSID_WICImagingFactory);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

} // namespace ZDX
