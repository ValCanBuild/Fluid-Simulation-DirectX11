/*************************************************************
I_InputSystem.h: Interface implementation of the engine input
system

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#ifndef _I_INPUTSYSTEM_H_
#define _I_INPUTSYSTEM_H_

class I_InputSystem {
public:
	I_InputSystem(){};
	virtual ~I_InputSystem(){};

	virtual bool IsKeyDown(unsigned int vKey) const = 0;
	virtual bool IsKeyClicked(unsigned int) const = 0;

	virtual bool IsMouseLeftDown() const = 0;
	virtual bool IsMouseRightDown() const = 0;
	virtual bool IsMouseMidDown() const = 0;

	virtual void GetMousePos(int& xPos, int& yPos) const = 0;
	virtual void GetMouseDelta(int& xDelta, int& yDelta) const = 0;
};

#endif
