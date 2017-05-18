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

	SIZE Monitor::size() const
	{
		DXGI_OUTPUT_DESC outDesc;
		m_DXGI_output1->GetDesc(&outDesc);
		RECT rect = outDesc.DesktopCoordinates;
		return { rect.right - rect.left, rect.bottom - rect.top };
	}

	bool Monitor::copy_bits(BYTE* biffer) const
	{
		DXGI_OUTDUPL_FRAME_INFO fi;
		CComPtr<IDXGIResource> spDXGIResource;
		HRESULT hr = m_DXGI_output_duplication->AcquireNextFrame(20, &fi, &spDXGIResource);
		if (FAILED(hr))
		{
			return false;
		}

		CComQIPtr<ID3D11Texture2D> spTextureResource = spDXGIResource;

		D3D11_TEXTURE2D_DESC desc;
		spTextureResource->GetDesc(&desc);

		D3D11_TEXTURE2D_DESC texDesc;
		ZeroMemory(&texDesc, sizeof(texDesc));
		texDesc.Width = desc.Width;
		texDesc.Height = desc.Height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_STAGING;
		texDesc.Format = desc.Format;
		texDesc.BindFlags = 0;
		texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		texDesc.MiscFlags = 0;

		CComPtr<ID3D11Texture2D> spD3D11Texture2D = NULL;
		hr = m_D3D_device->CreateTexture2D(&texDesc, NULL, &spD3D11Texture2D);
		if (FAILED(hr))
			return false;

		m_D3D_device_context->CopyResource(spD3D11Texture2D, spTextureResource);

		CComQIPtr<IDXGISurface1> spDXGISurface = spD3D11Texture2D;

		DXGI_MAPPED_RECT map;
		spDXGISurface->Map(&map, DXGI_MAP_READ);

		switch (rotation())
		{
		case DXGI_MODE_ROTATION_IDENTITY:
		{
		}
		break;
		}
	}

	DXGI_MODE_ROTATION Monitor::rotation() const
	{
		DXGI_OUTPUT_DESC outDesc;
		m_DXGI_output1->GetDesc(&outDesc);
		return outDesc.Rotation;
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
