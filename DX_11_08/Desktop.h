/*
* Desktop.h
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#ifndef zdxDesktop_h
#define zxdDesktop_h

#include <vector>
#include <memory>
#include <limits>
#include <windows.h>
#include <atlbase.h>
#include <Wincodec.h>
#include "Monitor.h"

namespace ZDX
{
	class Desktop
	{
	public:
		explicit Desktop(size_t monitor_number = UINT_MAX/* -1 is the whole desktop */);
		~Desktop();

		bool acquire_next_buffer();
		const BYTE* get_buffer() const;
		bool save(const char* file_name) const;
	private:
		std::vector<Monitor> m_monitors;
		std::unique_ptr<BYTE> m_buffer{};
		size_t m_buffer_size{};
		ULONG_PTR m_gdi_plus_token{};
		CComPtr<IDXGIFactory1> m_DXGI_factory1;
		CComPtr<IWICImagingFactory> m_WIC_factory;
		RECT m_capture_rect{ 0, 0, 0, 0 };
		const size_t m_active_monitor_number;

		bool prepare_buffer();
		bool init();
	};


} // namespace ZDX


#endif // zdxDesktop_h
