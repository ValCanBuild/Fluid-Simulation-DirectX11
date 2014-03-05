/********************************************************************
FluidSettings.h: A struct encompassing the settings required for a 
fluid calculator. Provides a method for getting a TwType for placing
on a Tweak Bar

Author:	Valentin Hinov
Date: 3/3/2014
*********************************************************************/

#ifndef _FLUIDSETTINGS_H
#define _FLUIDSETTINGS_H

#include "../math/MathUtils.h"

// Default parameters
#define DIMENSION 64
#define TIME_STEP 0.125f
#define IMPULSE_RADIUS 3.0f
#define INTERACTION_IMPULSE_RADIUS 7.0f
#define OBSTACLES_IMPULSE_RADIUS 5.0f
#define JACOBI_ITERATIONS 15
#define VEL_DISSIPATION 0.999f
#define DENSITY_DISSIPATION 0.999f
#define TEMPERATURE_DISSIPATION 0.99f
#define SMOKE_BUOYANCY 1.0f
#define SMOKE_WEIGHT 0.05f
#define AMBIENT_TEMPERATURE 0.0f
#define IMPULSE_TEMPERATURE 1.5f
#define IMPULSE_DENSITY 1.0f

struct FluidSettings {
	int jacobiIterations;
	float timeStep;
	bool macCormackEnabled;
	float velocityDissipation;
	float temperatureDissipation;
	float constantTemperature;
	float densityDissipation;
	float constantDensityAmount;
	float densityWeight;
	float densityBuoyancy;
	float constantInputRadius;
	Vector3 dimensions;	
	Vector3 constantInputPosition;	// location of permanent density and temperature input as % of dimensions

	FluidSettings() {
		jacobiIterations = JACOBI_ITERATIONS;
		timeStep = TIME_STEP;
		macCormackEnabled = true;
		velocityDissipation = VEL_DISSIPATION;
		temperatureDissipation = TEMPERATURE_DISSIPATION;
		constantTemperature = IMPULSE_TEMPERATURE;
		densityDissipation = DENSITY_DISSIPATION;
		constantDensityAmount = IMPULSE_DENSITY;
		densityWeight = SMOKE_WEIGHT;
		densityBuoyancy = SMOKE_BUOYANCY;
		dimensions = Vector3(DIMENSION);
		constantInputRadius = IMPULSE_RADIUS;
		constantInputPosition = Vector3(0.5f,0.0f,0.5f);
	}
};

enum ETwType;
ETwType GetFluidSettingsTwType(); // for use on an AntTweakBar

#endif