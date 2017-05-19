/*
* Monitor.h
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#ifndef zdxMonitor_h
#define zxdMonitor_h

#include <atlbase.h>
#include <DXGI1_2.h>
#include <d3d11.h>

namespace ZDX
{

	class Monitor
	{
	public:
		Monitor(CComPtr<IDXGIAdapter1> adapter1, CComPtr<IDXGIOutput1> DXGI_output1);
		~Monitor();
		RECT rect() const;
		SIZE size() const;
		size_t pixel_size() const;
		bool copy_bits(BYTE* buffer, RECT buffer_rect);
	private:
		CComPtr<IDXGIAdapter1> m_adapter1;
		CComPtr<IDXGIOutput1> m_DXGI_output1;
		CComPtr<ID3D11Device> m_D3D_device;
		CComPtr<ID3D11DeviceContext> m_D3D_device_context;
		CComPtr<IDXGIOutputDuplication> m_DXGI_output_duplication;
		CComPtr<ID3D11Texture2D> m_D3D11_texture;
		const size_t m_pixel_size{4};

		bool init();
		bool get_texture();
		DXGI_MODE_ROTATION rotation() const;
	};

} // namespace ZDX


#endif // zdxMonitor_h
