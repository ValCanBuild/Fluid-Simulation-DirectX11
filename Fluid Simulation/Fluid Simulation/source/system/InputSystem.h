/*************************************************************
InputSystem.h: Specific Implementation of the 
engine input system. 

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#ifndef _INPUTSYSTEM_H_
#define _INPUTSYSTEM_H_

#include "I_InputSystem.h"

class InputSystem : public I_InputSystem{
public:
	InputSystem();
	InputSystem(const InputSystem&);
	~InputSystem();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);
	
	bool IsKeyDown(unsigned int) const;

	void SetMousePos(int x, int y);
	// change in mouse key status
	// 0 - Left
	// 1 - Right
	void OnMouseButtonAction(int key, bool status);

	bool IsMouseLeftDown() const;
	bool IsMouseRightDown() const;

	void GetMousePos(int& xPos, int& yPos) const;
	void GetMouseDelta(int& xDelta, int& yDelta) const;


private:
	bool mKeys[256];
	bool mMouseLeft,mMouseRight;

	int mMouseX,mMouseY;
	int mMouseDeltaX,mMouseDeltaY;
};

#endif
