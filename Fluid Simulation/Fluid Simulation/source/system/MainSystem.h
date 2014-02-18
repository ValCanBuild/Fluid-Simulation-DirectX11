/***************************************************************
MainSystem.h:

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#ifndef _MAINSYSTEM_H_
#define _MAINSYSTEM_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <memory>

#include "ServiceProvider.h"
#include "../utilities/AppTimer.h"
#include "../utilities/PerformanceMonitor.h"

using namespace std;

class InputSystemImpl;
class GraphicsSystemImpl;

class MainSystem
{
public:
	MainSystem();
	MainSystem(const MainSystem&);
	~MainSystem();

	bool Initialize();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR mApplicationName;
	HINSTANCE mHInstance;
	HWND mHwnd;

	AppTimer			mAppTimer;
	PerformanceMonitor	mPerfMonitor;
	
	unique_ptr<InputSystemImpl> mInput;
	unique_ptr<GraphicsSystemImpl> mGraphics;

	float	mTimeLag;
};


/////////////////////////
// FUNCTION PROTOTYPES //
/////////////////////////
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


/////////////
// GLOBALS //
/////////////
static MainSystem* ApplicationHandle = 0;


#endif