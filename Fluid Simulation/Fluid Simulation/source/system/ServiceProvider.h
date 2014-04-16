/*************************************************************
ServiceProvider.h: Provides access to the main services in the
engine.

Author: Valentin Hinov
Date: 04/09/2013
Version: 1.0
**************************************************************/
#ifndef _SERVICEPROVIDER_H_
#define _SERVICEPROVIDER_H_

#include <map>
#include <typeinfo>
//#include <memory>
//#include "InputSystem.h"
//#include "IGraphicsSystem.h"
//#include "../utilities/AppTimer/IAppTimer.h"

struct TypeInfoLess {
	bool operator() (const std::type_info* lhs, const std::type_info* rhs) const {
		return lhs->before(*rhs) != 0;
	}
};

class ServiceProvider {
public:		
	static ServiceProvider& Instance() {
		// Lazy initialize.
		static ServiceProvider provider;

		return provider;
	}

	~ServiceProvider() {}

	template <typename T>
	void RegisterService(T * service) {
		mServiceMap[&typeid(T)] = reinterpret_cast<void *>(service);
	}

	template <typename T>
	T *GetService() {
		auto it = mServiceMap.find(&typeid(T));
		if (it == mServiceMap.end()) {
			return nullptr;
		}
		return reinterpret_cast<T *>(it->second);
	}

private:
	ServiceProvider() {mServiceMap.clear();}
	
private:
	typedef std::map<const std::type_info *, void *, TypeInfoLess> ServiceMap;
	ServiceMap mServiceMap;
};

#endif
