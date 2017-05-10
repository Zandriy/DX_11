/*
* Window.h
*
* Created on: May 10, 2017
* Author: Andrew Zhabura
*/

#ifndef zdxWindow_h
#define zxdWindow_h

#include <windows.h>

namespace ZDX
{

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	class Window
	{
	public:
		Window(HINSTANCE hInstance, LPCWSTR name, LPCWSTR caption, RECT r);
		~Window();

		void ShowWindow(int nCmdShow);
		inline HWND WinId() const { return m_hWnd; }

		LRESULT WindowLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	private:
		HINSTANCE  m_hInst{ nullptr };
		HWND m_hWnd{ nullptr };

		HRESULT InitWindow(HINSTANCE hInstance, LPCWSTR name, LPCWSTR caption, RECT r);
	};

} // namespace ZDX


#endif // zdxWindow_h