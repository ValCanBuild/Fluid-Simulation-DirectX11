/*************************************************************
InputSystem.cpp: Handles all user input 

Author: Valentin Hinov
Date: 02/09/2013
Version: 1.0
**************************************************************/
#include "InputSystem.h"


InputSystem::InputSystem() {
	mMouseX = mMouseY = mMouseDeltaX = mMouseDeltaY = 0;
}


InputSystem::InputSystem(const InputSystem& other) {
}


InputSystem::~InputSystem() {
}


void InputSystem::Initialize() {
	int i;	

	// Initialize all the keys to being released and not pressed.
	for(i=0; i<256; i++) {
		mKeys[i] = false;
	}

	mMouseLeft = mMouseRight = false;
}

void InputSystem::KeyDown(unsigned int input) {
	// If a key is pressed then save that state in the key array.
	mKeys[input] = true;
}

void InputSystem::KeyUp(unsigned int input) {
	// If a key is released then clear that state in the key array.
	mKeys[input] = false;
}

bool InputSystem::IsKeyDown(unsigned int key) const {
	// Return what state the key is in (pressed/not pressed).
	return mKeys[key];
}

void InputSystem::OnMouseButtonAction(int key, bool status) {
	if (key == 0) {
		mMouseLeft = status;
	}
	else if (key == 1) {
		mMouseRight = status;
	}
}

void InputSystem::SetMousePos(int x, int y) {
	mMouseDeltaX = x - mMouseX;
	mMouseDeltaY = y - mMouseY;
	mMouseX = x;
	mMouseY = y;
}

bool InputSystem::IsMouseLeftDown() const {
	return mMouseLeft;
}

bool InputSystem::IsMouseRightDown() const {
	return mMouseRight;
}

void InputSystem::GetMousePos(int& xPos, int& yPos) const {
	xPos = mMouseX;
	yPos = mMouseY;
}

void InputSystem::GetMouseDelta(int& xDelta, int& yDelta) const {
	xDelta = mMouseDeltaX;
	yDelta = mMouseDeltaY;
}