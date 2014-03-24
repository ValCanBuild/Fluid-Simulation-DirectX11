/***************************************************************
main.cpp: Entry Point of Application

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#if defined(_DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	//redefine new operator to track memory leaks
	#ifndef DBG_NEW      
		#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )      
		#define new DBG_NEW   
	#endif
#endif

#include "system\MainSystem.h"
#include "utilities\Console.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

	#if defined(_DEBUG)
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	std::unique_ptr<MainSystem> mainSystem(new MainSystem());
	
	//ShowWin32Console();

	if (!mainSystem)
		return 0;

	// Initialize and run the system object.
	bool result = mainSystem->Initialize();
	if(result) {
		mainSystem->Run();
	}

	return 0;
}