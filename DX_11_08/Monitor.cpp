/*
* Monitor.cpp
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#include "Monitor.h"
#include <stdexcept>

namespace ZDX
{

	Monitor::Monitor(CComPtr<IDXGIAdapter1> adapter1, CComPtr<IDXGIOutput1> DXGI_output1)
		: m_adapter1{ adapter1 }
		, m_DXGI_output1{ DXGI_output1 }
	{
		if(!init())
			throw std::domain_error("Monitor is not initialized.");
	}

	Monitor::~Monitor()
	{
	}

	RECT Monitor::rect() const
	{
		DXGI_OUTPUT_DESC outDesc;
		m_DXGI_output1->GetDesc(&outDesc);
		return outDesc.DesktopCoordinates;
	}


	DXGI_MODE_ROTATION Monitor::rotation() const
	{
		return DXGI_MODE_ROTATION_UNSPECIFIED;
	}

	bool Monitor::init()
	{
		D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_9_1;
		HRESULT hr = D3D11CreateDevice((m_adapter1), D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &m_D3D_device, &fl, &m_D3D_device_context);
		if (FAILED(hr))
		{
			return false;
		}

		CComQIPtr<IDXGIDevice1> spDXGIDevice = m_D3D_device;
		if (!m_DXGI_output1 || !spDXGIDevice)
			return false;
		hr = m_DXGI_output1->DuplicateOutput(spDXGIDevice, &m_DXGI_output_duplication);
		if (FAILED(hr))
			return false;

		return true;
	}

} // namespace ZDX
