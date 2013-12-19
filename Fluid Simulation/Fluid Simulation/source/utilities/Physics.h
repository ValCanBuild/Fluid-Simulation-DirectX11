/***************************************************************
Physics.h: Provides physics constants

Author: Valentin Hinov
Date: 01/12/2013
**************************************************************/

#ifndef _PHYSICS_H
#define _PHYSICS_H

class Physics {
public:
	static float fMaxSimulationTimestep;
	static float fGravity;
	static float fAirDensity;
	static float fDragCoefficient;
	static float fWindSpeed;
	static float fRestitution;
	static float fThrustForce;
	static float fLinearDragCoefficient;
	static int   iSolverIterationCount;
	static float fSleepAngularVelocity;
	static float fSleepVelocity;
};

#define DEFAULT_MAX_SIMULATION_TIMESTEP	0.02f	// 1/50th of a second
#define DEFAULT_GRAVITY	-9.81f
#define DEFAULT_AIR_DENSITY 1.23f // (1.23kg/m^3) standard air at 15 degrees celcius
#define DEFAULT_DRAG_COEFFICIENT 0.6f
#define DEFAULT_WIND_SPEED 10.0f // 10m/s, about 20 knots
#define DEFAULT_RESTITUTION 0.6f	// coefficient of restitution
#define DEFAULT_THRUST_FORCE 5.0f
#define DEFAULT_LINEAR_DRAG_COEFFICIENT 1.25f
#define DEFAULT_SOLVER_ITERATION_COUNT 7
#define DEFAULT_SLEEP_ANGULAR_VELOCITY 0.14f
#define DEFAULT_SLEEP_VELOCITY 0.15f


#endif