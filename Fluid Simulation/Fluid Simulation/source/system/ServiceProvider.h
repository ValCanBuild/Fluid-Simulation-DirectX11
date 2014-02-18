/*************************************************************
ServiceProvider.h: Provides access to the main services in the
engine.

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/
#ifndef _SERVICEPROVIDER_H_
#define _SERVICEPROVIDER_H_

#include "InputSystem.h"
#include "GraphicsSystem.h"

class ServiceProvider {
public:		
	static ServiceProvider& Instance() {
		// Lazy initialize.
		if (mSingleton == nullptr)
			mSingleton = new ServiceProvider();

		return *mSingleton;
	}

	void Initialize(InputSystem* inputSystem, GraphicsSystem* graphicsSystem) {
		if (mInitialized)
			return;

		pInputSystem = inputSystem;
		pGraphicsSystem = graphicsSystem;

		mInitialized = true;
	}

	InputSystem* GetInputSystem() { return pInputSystem; };
	GraphicsSystem* GetGraphicsSystem() { return pGraphicsSystem; };

	~ServiceProvider() {
		pInputSystem = nullptr;
		pGraphicsSystem = nullptr;
	}

private:
	ServiceProvider() {mInitialized = false;}
	
private:
	static ServiceProvider *mSingleton;
	bool mInitialized;
	InputSystem* pInputSystem;
	GraphicsSystem* pGraphicsSystem;
};

#endif
