/*
* DX_11_08.cpp
*
* Created on: May 18, 2017
* Author: Andrew Zhabura
*/

#include "stdafx.h"
#include "Desktop.h"

using namespace ZDX;

int main()
{
	Desktop desktop{0};
	desktop.acquire_next_buffer();
	const BYTE* buffer = desktop.get_buffer();
	desktop.save("test.png");
	
    return 0;
}

