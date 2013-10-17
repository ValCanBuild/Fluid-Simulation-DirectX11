/***************************************************************
PerformanceMonitor.h: Monitors system resources and performance.

Author: Valentin Hinov
Date: 16/09/2013
**************************************************************/
#include "PerformanceMonitor.h"

PerformanceMonitor::PerformanceMonitor() {

}

PerformanceMonitor::~PerformanceMonitor() {
	if(mCanReadCpu) {
		PdhCloseQuery(mQueryHandle);
	}
}

bool PerformanceMonitor::Initialize() {
	PDH_STATUS status;

	// Initialize the flag indicating whether this object can read the system cpu usage or not.
	mCanReadCpu = true;

	// Create a query object to poll cpu usage.
	status = PdhOpenQuery(NULL, 0, &mQueryHandle);
	if(status != ERROR_SUCCESS) {
		mCanReadCpu = false;
	}

	// Set query object to poll all cpus in the system.
	status = PdhAddCounter(mQueryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &mCounterHandle);
	if(status != ERROR_SUCCESS) {
		mCanReadCpu = false;
	}

	// Initialize the start time and cpu usage.
	mLastSampleTime = GetTickCount(); 
	mCpuUsage = 0;

	return mCanReadCpu;
}

void PerformanceMonitor::Tick() {
	PDH_FMT_COUNTERVALUE value; 

	if(mCanReadCpu) {
		// If it has been 1 second then update the current cpu usage and reset the 1 second timer again.
		if((mLastSampleTime + 1000) < GetTickCount())
		{
			mLastSampleTime = GetTickCount(); 

			PdhCollectQueryData(mQueryHandle);
        
			PdhGetFormattedCounterValue(mCounterHandle, PDH_FMT_LONG, NULL, &value);

			mCpuUsage = value.longValue;
		}
	}
}

int PerformanceMonitor::GetCpuPercentage() const {
	int usage;

	// If the class can read the cpu from the operating system then return the current usage.  If not then return zero.
	if(mCanReadCpu) {
		usage = (int)mCpuUsage;
	}
	else {
		usage = 0;
	}
	return usage;
}