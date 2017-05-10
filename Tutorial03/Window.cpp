/*
* Window.cpp
*
* Created on: May 10, 2017
* Author: Andrew Zhabura
*/

#include "Window.h"

namespace ZDX
{
	static const char s_wndObj[] = "winObject";
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		ZDX::Window* wnd = (ZDX::Window*)GetProp(hWnd, LPCWSTR(s_wndObj));
		if (wnd)
			return wnd->WindowLoop(hWnd, message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}


	Window::Window(HINSTANCE hInstance, LPCWSTR name, LPCWSTR caption, RECT r)
	{
		HRESULT res = InitWindow(hInstance, name, caption, r);
		if (FAILED(res))
		{
			throw res;
		}
	}

	Window::~Window()
	{
		::DestroyWindow(m_hWnd);
	}

	void Window::ShowWindow(int nCmdShow)
	{
		::ShowWindow(m_hWnd, nCmdShow);
	}

	HRESULT Window::InitWindow(HINSTANCE hInstance, LPCWSTR name, LPCWSTR caption, RECT r)
	{
		// Register class
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon(hInstance, name);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = name;
		wcex.hIconSm = LoadIcon(wcex.hInstance, name);
		if (!RegisterClassEx(&wcex))
			return E_FAIL;

		// Create window
		m_hInst = hInstance;
		AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
		m_hWnd = CreateWindow(name, caption,
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
			CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance,
			nullptr);
		if (!m_hWnd)
			return E_FAIL;

		SetProp(m_hWnd, (LPCWSTR)s_wndObj, this);

		return S_OK;
	}

	LRESULT Window::WindowLoop(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hdc;

		switch (message)
		{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

}