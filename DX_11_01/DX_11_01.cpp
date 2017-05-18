/*
* DX_11_01.cpp
*
* Created on: May 10, 2017
* Author: Andrew Zhabura
*/

#include "Window.h"
#include "Device.h"


int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
try
{
	ZDX::Window window{ hinstance, L"DXWindow", L"DX11 Sample", {0,0,800,600} };
	window.ShowWindow(nCmdShow);

	ZDX::Device device(window.WinId());

	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			device.Render();
		}
	}

	return 0;
}
catch (HRESULT)
{
	MessageBox(
		NULL,
		L"Problem in Windows functionality",
		L"Fatal Error",
		MB_ICONERROR
	);
	return 1;
}
catch (...)
{
	MessageBox(
		NULL,
		L"Unknown problem",
		L"Fatal Error",
		MB_ICONERROR
	);
	return 2;
}