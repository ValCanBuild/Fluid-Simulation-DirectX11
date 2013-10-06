/*************************************************************
D3dIncludes.h: Contains necessary includes for using D3D

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#ifndef _D3DINCLUDES_H
#define _D3DINCLUDES_H

#define DEBUG

//redefine new operator to track memory leaks
/*#if defined(DEBUG) || defined(_DEBUG)
	#ifndef DBG_NEW      
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )      
		#define new DBG_NEW   
	#endif
#endif  */

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) || defined(_DEBUG)
	#ifndef D3D_DEBUG_INFO
	#define D3D_DEBUG_INFO
	#endif
#endif


#if defined(DEBUG) || defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <crtdbg.h>
#endif

#pragma comment(lib, "dxgi.lib")
#pragma comment( lib, "d3d11.lib" )
#pragma comment(lib, "d3dx10.lib")

#if defined(DEBUG) || defined(_DEBUG)
	#pragma comment( lib, "d3dx11d.lib" )
#else
	#pragma comment( lib, "d3dx11.lib" )
#endif

#include <dxgi.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#include <dxerr.h>
#include "../utilities/math/MathUtils.h"

struct COMDeleter {
    template<typename T> void operator()(T* ptr) {
        ptr->Release();
    }
};

#endif