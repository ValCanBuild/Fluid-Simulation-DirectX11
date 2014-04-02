/***************************************************************
AppTimerImpl.h: Implementation of IAppTimer interface. Provides
additional methods for controlling time functionality

Author: Valentin Hinov, based on GameTimer from Frank Luna
Date: 10/09/2013
**************************************************************/

#ifndef _APPTIMERIMPL_H
#define _APPTIMERIMPL_H

#include "IAppTimer.h"

class AppTimerImpl : public IAppTimer
{
public:
	AppTimerImpl();

	float GetGameTime() const override;  // in seconds
	float GetDeltaTime() const override; // in seconds
	
	int	  GetFps() const override;
	long  GetCurrTime() const override;  // in millis

	void Reset(); // Call before message loop.
	void Start(); // Call when unpaused.
	void Stop();  // Call when paused.
	void Tick();  // Call every frame.

private:
	void Frame(); // Call every frame to compute fps

private:
	double mSecondsPerCount;
	double mDeltaTime;
	int	   mFps;

	__int64 mBaseTime;
	__int64 mPausedTime;
	__int64 mStopTime;
	__int64 mPrevTime;
	__int64 mCurrTime;

	bool mStopped;
};

#endif