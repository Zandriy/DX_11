/*
* Desktop.h
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#ifndef zdxDesktop_h
#define zxdDesktop_h

#include <vector>
#include <windows.h>
#include <atlbase.h>
#include <Wincodec.h>
#include "Monitor.h"

namespace ZDX
{
	class Desktop
	{
	public:
		Desktop(int monitor_number = 0);
		~Desktop();

		bool acquire_next_buffer();
		const BYTE* get_buffer();
		bool save(const char* file_name);
	private:
		std::vector<Monitor> m_monitors;
		int m_active_monitor_number{};
		BYTE* m_buffer{};
		size_t m_buffer_size{};
		ULONG_PTR m_gdi_plus_token{};
		CComPtr<IDXGIFactory1> m_DXGI_factory1;
		CComPtr<IWICImagingFactory> m_WIC_factory;

		void prepare_buffer();
		bool init();
	};


} // namespace ZDX


#endif // zdxDesktop_h
