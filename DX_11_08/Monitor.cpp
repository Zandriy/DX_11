/*
* Monitor.cpp
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#include "Monitor.h"
#include <stdexcept>
#include <algorithm>

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


	size_t Monitor::pixel_size() const
	{
		return m_pixel_size;
	}

	bool Monitor::copy_bits(BYTE* buffer, RECT buffer_rect)
	{
		if (!get_texture())
		{
			return false;
		}

		CComQIPtr<IDXGISurface1> IDXGI_surface1 = m_D3D11_texture;

		DXGI_MAPPED_RECT map;
		IDXGI_surface1->Map(&map, DXGI_MAP_READ);

		SIZE s = size();
		RECT r = rect();
		SIZE buffer_offset_corner{ r.left - buffer_rect.left, r.top - buffer_rect.top };
		size_t monitor_line_size = s.cx * m_pixel_size;
		size_t buffer_line_size = (buffer_rect.right - buffer_rect.left) * m_pixel_size;
		size_t buffer_offset = (buffer_line_size*buffer_offset_corner.cy + buffer_offset_corner.cx) * m_pixel_size;

		switch (rotation())
		{
		case DXGI_MODE_ROTATION_IDENTITY:
		{
			for (LONG i = 0; i < s.cy; ++i)
			{
				std::copy_n(map.pBits + monitor_line_size * i, monitor_line_size, buffer + (buffer_offset + buffer_line_size * i));
			}
		}
		break;
		default:
			return false;
			break;
		}

		return true;
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
		HRESULT hr = D3D11CreateDevice(m_adapter1, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &m_D3D_device, &fl, &m_D3D_device_context);
		if (FAILED(hr))
		{
			return false;
		}

		CComQIPtr<IDXGIDevice1> DXGI_device = m_D3D_device;
		if (!m_DXGI_output1 || !DXGI_device)
			return false;
		hr = m_DXGI_output1->DuplicateOutput(DXGI_device, &m_DXGI_output_duplication);
		if (FAILED(hr))
			return false;

		return true;
	}

	bool Monitor::get_texture()
	{
		DXGI_OUTDUPL_FRAME_INFO fi;
		CComPtr<IDXGIResource> DXGI_resource;
		HRESULT hr = m_DXGI_output_duplication->AcquireNextFrame(20, &fi, &DXGI_resource);
		if (FAILED(hr))
		{
			return false;
		}

		CComQIPtr<ID3D11Texture2D> ID3D11_texture_resource = DXGI_resource;

		D3D11_TEXTURE2D_DESC desc;
		ID3D11_texture_resource->GetDesc(&desc);

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

		hr = m_D3D_device->CreateTexture2D(&texDesc, NULL, &m_D3D11_texture);
		if (FAILED(hr))
			return false;

		m_D3D_device_context->CopyResource(m_D3D11_texture, ID3D11_texture_resource);

		return true;
	}

} // namespace ZDX
