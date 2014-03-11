/*************************************************************
D3dIncludes.h: Contains necessary includes for using D3D

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DINCLUDES_H
#define _D3DINCLUDES_H

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(_DEBUG)
	#ifndef D3D_DEBUG_INFO
	#define D3D_DEBUG_INFO
	#endif
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment( lib, "d3d11.lib" )

#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_1.h>

#include "../utilities/math/MathUtils.h"

struct COMDeleter {
	template<typename T> void operator()(T* ptr) {
		ptr->Release();
	}
};

#endif