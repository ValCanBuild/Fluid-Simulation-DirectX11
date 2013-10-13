/***************************************************************
AppTimer.h: Provides methods for interfacing with time

Author: Valentin Hinov, based on GameTimer from Frank Luna
Date: 10/09/2013
**************************************************************/

#ifndef _APPTIMER_H
#define _APPTIMER_H

class AppTimer
{
public:
	AppTimer();

	float GetGameTime() const;  // in seconds
	float GetDeltaTime() const; // in seconds
	
	int	  GetFps() const;
	long  GetCurrTime() const;  // in millis

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