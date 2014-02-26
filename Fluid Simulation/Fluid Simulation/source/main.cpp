/***************************************************************
main.cpp: Entry Point of Application

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/

#include "system\MainSystem.h"
#include "utilities\Console.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow) {

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