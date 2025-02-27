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

	Desktop::Desktop(size_t monitor_number)
		: m_active_monitor_number{ monitor_number }
	{
		CoInitialize(NULL);
		if (!init() || !prepare_buffer())
			throw std::domain_error("Desktop is not initialized.");
	}


	Desktop::~Desktop()
	{
	}

	bool Desktop::acquire_next_buffer()
	{
		if (m_monitors.size() <= m_active_monitor_number)
		{
			for (auto monitor : m_monitors)
			{
				if (!monitor.copy_bits(m_buffer.get(), m_capture_rect))
					return false;
			}
		}
		else
		{
			if(!m_monitors[m_active_monitor_number].copy_bits(m_buffer.get(), m_capture_rect))
				return false;
		}

		return true;
	}

	const BYTE* Desktop::get_buffer() const
{
		return m_buffer.get();
	}
	
	bool Desktop::save(const char* file_name) const
	{
		if (!m_buffer)
		{
			return false;
		}

		SIZE s{ m_capture_rect.right - m_capture_rect.left, m_capture_rect.bottom - m_capture_rect.top };
		CComPtr<IWICBitmap> IWIC_bitmap = NULL;
		if (FAILED(m_WIC_factory->CreateBitmapFromMemory(s.cx, s.cy, GUID_WICPixelFormat32bppBGRA, s.cx*4, m_buffer_size, m_buffer.get(), &IWIC_bitmap)))
		{
			return false;
		}
		CComPtr<IWICStream> IWIC_stream = NULL;
		if (FAILED(m_WIC_factory->CreateStream(&IWIC_stream)))
		{
			return false;
		}
		if (FAILED(IWIC_stream->InitializeFromFilename(L"capture.bmp", GENERIC_WRITE)))
		{
			return false;
		}
		CComPtr<IWICBitmapEncoder> IWIC_bitmap_encoder = NULL;		
		if (FAILED(m_WIC_factory->CreateEncoder(GUID_ContainerFormatBmp, NULL, &IWIC_bitmap_encoder)))
		{
			return false;
		}
		if (FAILED(IWIC_bitmap_encoder->Initialize(IWIC_stream, WICBitmapEncoderNoCache)))
		{
			return false;
		}
		CComPtr<IWICBitmapFrameEncode> IWIC_bitmap_frame_encode = NULL;		
		if (FAILED(IWIC_bitmap_encoder->CreateNewFrame(&IWIC_bitmap_frame_encode, NULL)))
		{
			return false;
		}		
		if (FAILED(IWIC_bitmap_frame_encode->Initialize(NULL)))
		{
			return false;
		}		
		if (FAILED(IWIC_bitmap_frame_encode->SetSize(s.cx, s.cy)))
		{
			return false;
		}
		WICPixelFormatGUID format;
		IWIC_bitmap->GetPixelFormat(&format);		
		if (FAILED(IWIC_bitmap_frame_encode->SetPixelFormat(&format)))
		{
			return false;
		}		
		if (FAILED(IWIC_bitmap_frame_encode->WriteSource(IWIC_bitmap, NULL)))
		{
			return false;
		}		
		if (FAILED(IWIC_bitmap_frame_encode->Commit()))
		{
			return false;
		}		
		if (FAILED(IWIC_bitmap_encoder->Commit()))
		{
			return false;
		}
		return true;
	}

	bool Desktop::prepare_buffer()
	{
		SetRect(&m_capture_rect, 0, 0, 0, 0);
		size_t pixel_size{};
		if (m_monitors.size() <= m_active_monitor_number)
		{
			for (auto monitor : m_monitors)
			{
				UnionRect(&m_capture_rect, &m_capture_rect, &monitor.rect());
				pixel_size = monitor.pixel_size();
			}
		}
		else
		{
			UnionRect(&m_capture_rect, &m_capture_rect, &m_monitors[m_active_monitor_number].rect());
			pixel_size = m_monitors[m_active_monitor_number].pixel_size();
		}

		SIZE rect_size{ m_capture_rect.right - m_capture_rect.left, m_capture_rect.bottom - m_capture_rect.top };
		m_buffer_size = rect_size.cx * rect_size.cy * pixel_size;
		m_buffer.reset(new BYTE[m_buffer_size]);

		return !!m_buffer;
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
		CComPtr<IDXGIAdapter1> adapter1;
		for (int i = 0; m_DXGI_factory1->EnumAdapters1(i, &adapter1) != DXGI_ERROR_NOT_FOUND; i++)
		{
			CComPtr<IDXGIOutput> DXGI_output;
			for (int i = 0; (adapter1)->EnumOutputs(i, &DXGI_output) != DXGI_ERROR_NOT_FOUND; i++)
			{
				DXGI_OUTPUT_DESC outputDesc;
				DXGI_output->GetDesc(&outputDesc);

				if (outputDesc.AttachedToDesktop)
				{
					CComQIPtr<IDXGIOutput1> DXGI_output1 = DXGI_output;
					m_monitors.emplace_back(adapter1, DXGI_output1);
				}

				DXGI_output.Release();
			}

			adapter1.Release();
		}
		
		hr = m_WIC_factory.CoCreateInstance(CLSID_WICImagingFactory);
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

} // namespace ZDX
