/*************************************************************
I_InputSystem.h: Interface implementation of the engine input
system

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
#ifndef INPUTSYSTEM_H_
#define INPUTSYSTEM_H_

class InputSystem {
public:
	InputSystem(){};
	virtual ~InputSystem(){};

	virtual bool IsKeyDown(unsigned int vKey) const = 0;
	virtual bool IsKeyClicked(unsigned int) const = 0;

	virtual bool IsMouseLeftDown() const = 0;
	virtual bool IsMouseRightDown() const = 0;
	virtual bool IsMouseMidDown() const = 0;

	virtual bool IsMouseLeftClicked() const = 0;
	virtual bool IsMouseRightClicked() const = 0;
	virtual bool IsMouseMidClicked() const = 0;

	virtual void GetMousePos(int& xPos, int& yPos) const = 0;
	virtual void GetMouseDelta(int& xDelta, int& yDelta) const = 0;

	virtual void GetMouseScrollDelta(int &scrollDelta) const = 0;
};

#endif
