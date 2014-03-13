#include "FluidSettings.h"

#include <AntTweakBar.h>

TwType fluidSettingsType;

void InitType() {
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
		{ "Input Radius", TW_TYPE_FLOAT, offsetof(FluidSettings, constantInputRadius), "min=0.05 max=50.0 step=0.1" }
	};

	int numMembers = sizeof(fluidSettingsStructMembers)/sizeof(fluidSettingsStructMembers[0]);

	fluidSettingsType = TwDefineStruct("FluidSettingsType", fluidSettingsStructMembers, numMembers, sizeof(FluidSettings), nullptr, nullptr);
}

TwType GetFluidSettingsTwType() {
	if (fluidSettingsType == TW_TYPE_UNDEF) {
		InitType();
	}
	return fluidSettingsType;
}

