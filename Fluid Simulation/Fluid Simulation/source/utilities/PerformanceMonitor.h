/***************************************************************
PerformanceMonitor.h: Monitors system resources and performance.

Author: Valentin Hinov
Date: 16/09/2013
**************************************************************/
#ifndef _PERFORMANCEMONITOR_H_
#define _PERFORMANCEMONITOR_H_

#pragma comment(lib, "pdh.lib")

#include <pdh.h>

class PerformanceMonitor {
public:
	PerformanceMonitor();
	~PerformanceMonitor();

	bool Initialize();
	void Tick();

	int GetCpuPercentage() const;

private:
	bool			mCanReadCpu;
	HQUERY			mQueryHandle;
	HCOUNTER		mCounterHandle;
	ULONGLONG		mLastSampleTime;
	long			mCpuUsage;
};

#endif