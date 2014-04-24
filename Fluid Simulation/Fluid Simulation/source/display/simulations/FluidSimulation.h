/********************************************************************
FluidSimulation.h: Encapsulates an object that handles both
simulating and and applying LoD to a fluid

Author:	Valentin Hinov
Date: 3/3/2014
*********************************************************************/

#ifndef _FLUIDSIMULATOR_H
#define	_FLUIDSIMULATOR_H

#include <memory>
#include <vector>
#include "../../utilities/AtlInclude.h"
#include "../D3DGraphicsObject.h"
#include "../../utilities/FluidCalculation/FluidSettings.h"
//#include "LODController.h"

class VolumeRenderer;
class ICamera;
struct CTwBar;
class IAppTimer;

namespace Fluid3D {
	class Fluid3DCalculator;
}

class FluidSimulation {
public:
	// Creates a fluid simulation from fluid settings
	FluidSimulation(const FluidSettings &fluidSettings);
	~FluidSimulation();

	// Add a volume renderer who will use the fluid calculator for this simulation for rendering
	void AddVolumeRenderer(std::shared_ptr<VolumeRenderer> volumeRenderer);
	bool Initialize(_In_ D3DGraphicsObject * d3dGraphicsObj, HWND hwnd);

	// Returns true if this simulation is updated and false if it wasn't
	bool Update(float dt, const ICamera &camera);

	void DisplayInfoOnBar(CTwBar * const pBar);
	// Checks if this ray intersects any of the volume renderers associated with this 
	// simulation and returns the one hit or nullptr
	std::shared_ptr<VolumeRenderer> IntersectsRay(const Ray &ray, float &distance) const;
	void FluidInteraction(const Ray &ray);
private:
	static void __stdcall GetFluidSettings(void *value, void *clientData);
	static void __stdcall SetFluidSettings(const void *value, void *clientData);

	Vector3 GetLocalIntersectPosition(const Ray &ray, float distance) const;
private:
	std::shared_ptr<Fluid3D::Fluid3DCalculator>	mFluidCalculator;
	std::vector<std::shared_ptr<VolumeRenderer>> mVolumeRenderers;

	bool mUpdateEnabled;
	bool mRenderEnabled;
	bool mIsVisible;

// LOD Values
private:
	//LODController mLodController;
	long long mAvgUpdateTime;
	long long mUpdateTime;
	int mNumUpdates;
	int mFluidUpdatesSinceStart;
	int mFramesSinceLastProcess;

	IAppTimer *pAppTimer;
};

#endif