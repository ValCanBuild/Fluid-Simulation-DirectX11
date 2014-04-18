/********************************************************************
LODController.cpp:  Implementation of LODController

Author:	Valentin Hinov
Date: 31/3/2014
*********************************************************************/

#include "LODController.h"
#include <AntTweakBar.h>
#include "../../utilities/math/MathUtils.h"
#include "../../utilities/ICamera.h"
#include "../../utilities/Screen.h"

using namespace DirectX;

#define MIN_DISTANCE 6.0f
#define MAX_DISTANCE 20.0f
#define MAX_FRAMES_TO_SKIP 2
#define NUM_SAMPLES 64
#define MAX_SAMPLES 128

TwType lodTwType;

LODController::LODController() : 
	overallLOD(1.0f), distanceLOD(1.0f), framesToSkip(0), minDistance(MIN_DISTANCE), maxDistance(MAX_DISTANCE),
	maxFramesToSkip(MAX_FRAMES_TO_SKIP), partOfScreen(0.0f), pObjectBox(nullptr), numSamples(NUM_SAMPLES), maxSamples(MAX_SAMPLES)
{

}

LODController::~LODController() {
	pObjectBox = nullptr;
}

void LODController::CalculateOverallLOD(const ICamera &camera) {
	// calculate an LOD value depending on the current state.
	// 0 = no detail
	// 1 = full detail

	// view percentage check
	CalculateScreenPercentage(camera);
	// view distance check
	CalculatedDistanceLOD(camera);

	overallLOD = distanceLOD + partOfScreen;
	overallLOD = Clamp(overallLOD, 0.0f, 1.0f);	

	if (partOfScreen == 0.0f) {
		framesToSkip = maxFramesToSkip;
		//numSamples = 0;
	}
	else {
		framesToSkip = 0;
	}

	numSamples = Clamp(numSamples, 0, maxSamples);
	framesToSkip = Clamp(framesToSkip, 0, maxFramesToSkip);
}

void LODController::CalculatedDistanceLOD(const ICamera &camera) {
	// distance check
	Vector3 camPos;
	camera.GetPosition(camPos);
	XMFLOAT3 boxCenter = pObjectBox->Center;
	Vector3 position(boxCenter.x, boxCenter.y, boxCenter.z);
	float distance = Vector3::Distance(camPos, position);

	distanceLOD = MapValue(distance, minDistance, maxDistance, 1.0f, 0.0f);
	distanceLOD = Clamp(distanceLOD, 0.0f, 1.0f);
}

void LODController::CalculateScreenPercentage(const ICamera &camera) {
	Matrix viewProjMat = camera.GetViewProjectionMatrix();
	float minX, maxX;
	float minY, maxY;
	maxX = maxY = 0.0f;
	minX = minY = FLT_MAX;
	Vector3 corners[BoundingBox::CORNER_COUNT];
	pObjectBox->GetCorners(corners);

	float halfScreenWidth = Screen::width * 0.5f;
	float halfScreenHeight = Screen::height * 0.5f;
	int numInvalidCorners = 0;
	for (Vector3 &corner : corners) {
		Vector3 localSpace;
		Vector3::Transform(corner, viewProjMat, localSpace);
		if (localSpace.z < 0.0f || localSpace.z > 1.0f) {
			++numInvalidCorners;
			continue;
		}
		float screenX = ((localSpace.x / localSpace.z) * halfScreenWidth) + halfScreenWidth;
		float screenY = -((localSpace.y / localSpace.z) * halfScreenHeight) + halfScreenHeight;
		minX = max(0, min(minX, screenX));
		maxX = max(maxX, screenX);
		minY = max(0, min(minY, screenY));
		maxY = max(maxY, screenY);
	}

	if (numInvalidCorners == BoundingBox::CORNER_COUNT) {
		partOfScreen = 0.0f;
	}
	else {
		float objectScreenArea = (maxX - minX) * (maxY - minY);
		int screenArea = Screen::width * Screen::height;

		partOfScreen = Clamp(objectScreenArea / screenArea, 0.0f, 1.0f);
	}
}

void LODController::SetObjectBoundingBox(const DirectX::BoundingBox* objectBox) {
	pObjectBox = objectBox;
}

//////////////////////////////////////////////////////////////////////////
// TWEAK BAR LOGIC
//////////////////////////////////////////////////////////////////////////
void InitType() {
	TwStructMember structMembers[] = {
		{ "Overall LOD", TW_TYPE_FLOAT, offsetof(LODController, overallLOD), "readonly=true" },
		{ "Distance LOD", TW_TYPE_FLOAT, offsetof(LODController, distanceLOD), "readonly=true" },
		{ "View LOD", TW_TYPE_FLOAT, offsetof(LODController, partOfScreen), "readonly=true" },
		{ "Frames To Skip", TW_TYPE_INT32, offsetof(LODController, framesToSkip), "readonly=true" },
		{ "Num Samples", TW_TYPE_INT32, offsetof(LODController, numSamples), "readonly=true" },
		{ "Min Distance", TW_TYPE_FLOAT, offsetof(LODController, minDistance), "min=0.0 step=0.5" },
		{ "Max Distance", TW_TYPE_FLOAT, offsetof(LODController, maxDistance), "min=0.0 step=0.5" },
		{ "Max Skip Frames", TW_TYPE_INT32, offsetof(LODController, maxFramesToSkip), "min=0 step=1" },
		{ "Max Samples", TW_TYPE_INT32, offsetof(LODController, maxSamples), "min=16 step=1" },

	};

	int numMembers = sizeof(structMembers)/sizeof(structMembers[0]);

	lodTwType = TwDefineStruct("LODType", structMembers, numMembers, sizeof(LODController), nullptr, nullptr);
}

ETwType LODController::GetLODDataTwType() {
	if (lodTwType == TW_TYPE_UNDEF) {
		InitType();
	}
	return lodTwType;
}