/********************************************************************
FluidSimulation.h: Encapsulates an object that handles both
simulating and rendering a 3D fluid

Author:	Valentin Hinov
Date: 3/3/2014
*********************************************************************/

#ifndef _FLUIDSIMULATOR_H
#define	_FLUIDSIMULATOR_H

#include <memory>
#include "../../utilities/AtlInclude.h"
#include "../D3DGraphicsObject.h"

class VolumeRenderer;
class ICamera;
struct CTwBar;

namespace Fluid3D {
	class Fluid3DCalculator;
}

class FluidSimulation {
public:
	// Creates a fluid simulation with a default fluid calculator and volume renderer
	FluidSimulation();
	FluidSimulation(std::unique_ptr<Fluid3D::Fluid3DCalculator> fluidCalculator, std::shared_ptr<VolumeRenderer> volumeRenderer);
	~FluidSimulation();

	bool Initialize(_In_ D3DGraphicsObject* d3dGraphicsObj, HWND hwnd);

	// Returns true if this simulation is rendered, and false if it is culled away
	bool Render(const ICamera &camera);

	// Returns true if this simulation is updated and false if it wasn't
	bool Update(float dt) const;

	void DisplayInfoOnBar(CTwBar * const pBar);
	bool IntersectsRay(Ray &ray, float &distance) const;

	std::shared_ptr<VolumeRenderer> GetVolumeRenderer() const;

private:
	static void __stdcall GetFluidSettings(void *value, void *clientData);
	static void __stdcall SetFluidSettings(const void *value, void *clientData);

private:
	std::unique_ptr<Fluid3D::Fluid3DCalculator>	mFluidCalculator;
	std::shared_ptr<VolumeRenderer> mVolumeRenderer;


	bool mUpdatePaused;
};

#endif