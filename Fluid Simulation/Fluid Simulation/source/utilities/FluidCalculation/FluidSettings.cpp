#include "FluidSettings.h"

#include <AntTweakBar.h>

TwType fluidSettingsType;
TwType fluidSettingsTypeFire;

void InitTypes() {
	TwEnumVal advectionTypeEV[] = { {SystemAdvectionType_t::NORMAL, "Normal"}, 
									{SystemAdvectionType_t::MACCORMARCK, "MacCormack"} };
	TwType advectionTwType = TwDefineEnum("AdvectionType", advectionTypeEV, 2);

	TwStructMember fluidSettingsStructMembers[] = {
		{ "Advection", advectionTwType, offsetof(FluidSettings, advectionType), "" },
		{ "Jacobi Iterations", TW_TYPE_INT32, offsetof(FluidSettings, jacobiIterations), "min=1 max=50 step=1" },
		{ "Time Step", TW_TYPE_FLOAT, offsetof(FluidSettings, timeStep), "min=0.0 max=1.0 step=0.001" },
		{ "Vorticity Strength", TW_TYPE_FLOAT, offsetof(FluidSettings, vorticityStrength), "min=0.0 max=1.0 step=0.01" },
		{ "Velocity Dissipation", TW_TYPE_FLOAT, offsetof(FluidSettings, velocityDissipation), "min=0.0 max=1.0 step=0.001" },
		{ "Temperature Dissipation", TW_TYPE_FLOAT, offsetof(FluidSettings, temperatureDissipation), "min=0.0 max=1.0 step=0.001" },
		{ "Constant Temperature", TW_TYPE_FLOAT, offsetof(FluidSettings, constantTemperature), "min=0.0 max=100.0 step=0.01" },
		{ "Density Dissipation", TW_TYPE_FLOAT, offsetof(FluidSettings, densityDissipation), "min=0.0 max=1.0 step=0.001" },
		{ "Constant Density", TW_TYPE_FLOAT, offsetof(FluidSettings, constantDensityAmount), "min=0.0 max=100.0 step=0.01" },
		{ "Density Weight", TW_TYPE_FLOAT, offsetof(FluidSettings, densityWeight), "min=0.001 max=10.0 step=0.001" },
		{ "Density Buoyancy", TW_TYPE_FLOAT, offsetof(FluidSettings, densityBuoyancy), "min=0.0 max=100.0 step=0.001" },
		{ "Input Radius", TW_TYPE_FLOAT, offsetof(FluidSettings, constantInputRadius), "min=0.005 max=1.0 step=0.01" },
		{ "Constant Reaction", TW_TYPE_FLOAT, offsetof(FluidSettings, constantReactionAmount), "min=0.0 max=100.0 step=0.01" },
		{ "Reaction Decay", TW_TYPE_FLOAT, offsetof(FluidSettings, reactionDecay), "min=0.0 max=10.0 step=0.001" },
		{ "Reaction Extinguishment", TW_TYPE_FLOAT, offsetof(FluidSettings, reactionExtinguishment), "min=0.001 max=1.0 step=0.001" }
	};

	int numMembers = sizeof(fluidSettingsStructMembers)/sizeof(fluidSettingsStructMembers[0]);

	fluidSettingsType = TwDefineStruct("FluidSettingsType", fluidSettingsStructMembers, numMembers-3, sizeof(FluidSettings), nullptr, nullptr);
	fluidSettingsTypeFire = TwDefineStruct("FluidSettingsTypeFire", fluidSettingsStructMembers, numMembers, sizeof(FluidSettings), nullptr, nullptr);
}

TwType FluidSettings::GetFluidSettingsTwType() {
	if (fluidSettingsType == TW_TYPE_UNDEF) {
		InitTypes();
	}
	switch (mFluidType) {
	case SMOKE:
		return fluidSettingsType;
	case FIRE:
		return fluidSettingsTypeFire;
	default:
		return fluidSettingsType;
	}

}

FluidSettings::FluidSettings(FluidType_t fluidType) : mFluidType(fluidType) {
	jacobiIterations = JACOBI_ITERATIONS;
	timeStep = TIME_STEP;
	advectionType = MACCORMARCK;
	velocityDissipation = VEL_DISSIPATION;
	temperatureDissipation = TEMPERATURE_DISSIPATION;
	constantTemperature = CONSTANT_TEMPERATURE;
	densityDissipation = DENSITY_DISSIPATION;
	constantDensityAmount = CONSTANT_DENSITY;
	densityWeight = SMOKE_WEIGHT;
	densityBuoyancy = SMOKE_BUOYANCY;
	dimensions = Vector3(DIMENSION);
	constantInputRadius = CONSTANT_INPUT_RADIUS;
	vorticityStrength = VORTICITY_STRENGTH;
	constantInputPosition = Vector3(0.5f,0.0f,0.5f);
	buoyancyDirection = Vector3(0,1,0);

	// fire only settings
	constantReactionAmount = CONSTANT_REACTION;
	reactionDecay = REACTION_DECAY;
	reactionExtinguishment = REACTION_EXTINGUISHMENT;
}