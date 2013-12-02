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

#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "ServiceProvider.h"
#include "../utilities/AppTimer.h"
#include "../utilities/PerformanceMonitor.h"

using namespace std;

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
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	AppTimer			mAppTimer;
	PerformanceMonitor	mPerfMonitor;
	
	unique_ptr<InputSystem> mInput;
	unique_ptr<GraphicsSystem> mGraphics;

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