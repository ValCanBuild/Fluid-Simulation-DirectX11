/***************************************************************
IAppTimer.h: Provides methods for interfacing with time functions

Author: Valentin Hinov, based on GameTimer from Frank Luna
Date: 19/03/2014
**************************************************************/

#ifndef _IAPPTIMER_H
#define _IAPPTIMER_H

class IAppTimer
{
public:
	virtual float GetGameTime() const = 0;  // in seconds
	virtual float GetDeltaTime() const = 0; // in seconds
	
	virtual int	  GetFps() const = 0;
	virtual long  GetCurrTime() const = 0;  // in milliseconds
};

#endif