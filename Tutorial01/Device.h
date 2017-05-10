/*
* Device.h
*
* Created on: May 10, 2017
* Author: Andrew Zhabura
*/

#ifndef zdxDevice_h
#define zxdDevice_h

#include <windows.h>
#include <d3d11_1.h>
#include <directxcolors.h>

namespace ZDX
{

	class Device
	{
	public:
		Device(HWND hWnd);
		~Device();
		void Render();
	private:
		D3D_DRIVER_TYPE         m_driverType{ D3D_DRIVER_TYPE_NULL };
		D3D_FEATURE_LEVEL       m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };
		ID3D11Device*           m_pd3dDevice{ nullptr };
		ID3D11Device1*          m_pd3dDevice1{ nullptr };
		ID3D11DeviceContext*    m_pImmediateContext{ nullptr };
		ID3D11DeviceContext1*   m_pImmediateContext1{ nullptr };
		IDXGISwapChain*         m_pSwapChain{ nullptr };
		IDXGISwapChain1*        m_pSwapChain1{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };


		HRESULT InitDevice(HWND hWnd);
	};

} // namespace ZDX


#endif // zdxDevice_h
