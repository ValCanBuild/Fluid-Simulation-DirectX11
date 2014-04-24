/***************************************************************
AppTimerImpl.cpp: Implementation of AppTimerImpl

Author: Valentin Hinov, based on GameTimer from Frank Luna
Date: 10/09/2013
**************************************************************/

#include "AppTimerImpl.h"
#include <windows.h>


AppTimerImpl::AppTimerImpl()
: mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0), 
  mPausedTime(0), mPrevTime(0), mCurrTime(0), mStopped(false) {
	__int64 countsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	mSecondsPerCount = 1.0 / (double)countsPerSec;
}

// Returns the total time elapsed since reset() was called, NOT counting any
// time when the clock is stopped.
float AppTimerImpl::GetGameTime() const {
	// If we are stopped, do not count the time that has passed since we stopped.
	//
	// ----*---------------*------------------------------*------> time
	//  mBaseTime       mStopTime                      mCurrTime

	if( mStopped ) {
		return (float)((mStopTime - mBaseTime)*mSecondsPerCount);
	}

	// The distance mCurrTime - mBaseTime includes paused time,
	// which we do not want to count.  To correct this, we can subtract 
	// the paused time from mCurrTime:  
	//
	//  (mCurrTime - mPausedTime) - mBaseTime 
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------*------> time
	//  mBaseTime       mStopTime        startTime     mCurrTime
	
	else {
		return (float)(((mCurrTime-mPausedTime)-mBaseTime)*mSecondsPerCount);
	}
}

int AppTimerImpl::GetFps() const {
	return mFps;
}

float AppTimerImpl::GetDeltaTime() const {
	return (float)mDeltaTime;
}

void AppTimerImpl::Reset() {
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped  = false;
}

void AppTimerImpl::Start() {
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);


	// Accumulate the time elapsed between stop and start pairs.
	//
	//                     |<-------d------->|
	// ----*---------------*-----------------*------------> time
	//  mBaseTime       mStopTime        startTime     

	if( mStopped ) {
		mPausedTime += (startTime - mStopTime);	

		mPrevTime = startTime;
		mStopTime = 0;
		mStopped  = false;
	}
}

void AppTimerImpl::Stop() {
	if( !mStopped ) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		mStopped  = true;
	}
}

long AppTimerImpl::GetCurrTime() const {
	return (long)(GetCurrTimePrecise()*mSecondsPerCount);
}

long long AppTimerImpl::GetCurrTimePrecise() const {
	if( !mStopped ) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		return currTime;
	}
	return 0;
}

void AppTimerImpl::Tick() {
	if( mStopped ) {
		mDeltaTime = 0.0;
		return;
	}

	Frame();

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	// Time difference between this frame and the previous.
	mDeltaTime = (mCurrTime - mPrevTime)*mSecondsPerCount;

	// Prepare for next frame.
	mPrevTime = mCurrTime;

	// Force nonnegative.  The DXSDK's CDXUTTimer mentions that if the 
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.
	if(mDeltaTime < 0.0)
	{
		mDeltaTime = 0.0;
	}
}

void AppTimerImpl::Frame() {
	static int frameCnt = 0;
	static float t_base = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if( (GetGameTime() - t_base) >= 1.0f ) {
		mFps = frameCnt; // fps = frameCnt / 1
		
		// Reset for next average.
		frameCnt = 0;
		t_base  += 1.0f;
	}
}