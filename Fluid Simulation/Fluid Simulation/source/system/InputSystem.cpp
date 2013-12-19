/*************************************************************
InputSystem.cpp: Handles all user input 

Author: Valentin Hinov
Date: 02/09/2013
**************************************************************/
// Preprocessing directive and linking
#define DIRECTINPUT_VERSION 0x0800
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#include <dinput.h>

#include "InputSystem.h"

InputSystem::InputSystem() {
}


InputSystem::InputSystem(const InputSystem& other) {
}


InputSystem::~InputSystem() {
	if (mMouse) {
		mMouse->Unacquire();
		mMouse->Release();
		mMouse = nullptr;
	}
	if (mMouseState) {
		delete mMouseState;
		mMouseState = nullptr;
	}
	if (mDirectInput) {
		mDirectInput->Release();
		mDirectInput = nullptr;
	}
}


bool InputSystem::Initialize(HINSTANCE hInstance, HWND hwnd) {
	mMouseX = mMouseY = 0;
	mLastKeyDown = -1;

	int i;		
	// Initialize all the keys to being released and not pressed.
	for(i=0; i<256; i++) {
		mKeys[i] = false;
	}

	mMouseLeft = mMouseRight = mMouseMid = mMouseLeftClicked = mMouseRightClicked = mMouseMidClicked = false;

	// Initialize the main direct input interface.
	HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&mDirectInput, NULL);
	if(FAILED(result)) {
		return false;
	}

	// Initialize the direct input interface for the mouse.
	result = mDirectInput->CreateDevice(GUID_SysMouse, &mMouse, NULL);
	if(FAILED(result)) {
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = mMouse->SetDataFormat(&c_dfDIMouse);
	if(FAILED(result)) {
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = mMouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(FAILED(result)) {
		return false;
	}

	// Acquire the mouse.
	result = mMouse->Acquire();
	if(FAILED(result)) {
		return false;
	}

	mMouseState = new DIMOUSESTATE();
	
	return true;
}

void InputSystem::Update(float delta) {
	// Read the mouse 
	HRESULT result = mMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)mMouseState);
	if(FAILED(result)) {
		// If the mouse lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED)) {
			mMouse->Acquire();
		}
		else {
			// could not reacquire mouse
		}
	}
}

void InputSystem::PostUpdate() {
	if (mLastKeyDown != -1) {
		mLastKeyDown = -1;
	}
	mMouseLeftClicked = mMouseRightClicked = mMouseMidClicked = false;
}

void InputSystem::KeyDown(unsigned int input) {
	// If a key is pressed then save that state in the key array.
	mKeys[input] = true;
	mLastKeyDown = input;
}

void InputSystem::KeyUp(unsigned int input) {
	// If a key is released then clear that state in the key array.
	mKeys[input] = false;
}

bool InputSystem::IsKeyDown(unsigned int key) const {
	// Return what state the key is in (pressed/not pressed).
	return mKeys[key];
}

bool InputSystem::IsKeyClicked(unsigned int key) const {
	return key == mLastKeyDown;
}

void InputSystem::OnMouseButtonAction(int key, bool status) {
	if (key == 0) {
		mMouseLeft = status;
		mMouseLeftClicked = status;
	}
	else if (key == 1) {
		mMouseRight = status;
		mMouseRightClicked = status;
	}
	else if (key == 2) {
		mMouseMid = status;
		mMouseMidClicked = status;
	}
}

void InputSystem::SetMousePos(int x, int y) {
	mMouseX = x;
	mMouseY = y;
}

bool InputSystem::IsMouseMidDown() const {
	return mMouseMid;
}

bool InputSystem::IsMouseLeftDown() const {
	return mMouseLeft;
}

bool InputSystem::IsMouseRightDown() const {
	return mMouseRight;
}

bool InputSystem::IsMouseLeftClicked() const {
	return mMouseLeftClicked;
}

bool InputSystem::IsMouseRightClicked() const {
	return mMouseRightClicked;
}

bool InputSystem::IsMouseMidClicked() const {
	return mMouseMidClicked;
}

void InputSystem::GetMousePos(int& xPos, int& yPos) const {
	xPos = mMouseX;
	yPos = mMouseY;
}

void InputSystem::GetMouseDelta(int& xDelta, int& yDelta) const {
	xDelta = mMouseState->lX;
	yDelta = mMouseState->lY;
}

void InputSystem::GetMouseScrollDelta(int &scrollDelta) const {
	scrollDelta = mMouseState->lZ;
}