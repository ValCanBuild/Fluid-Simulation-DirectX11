/********************************************************************
LODData: Contains Level of Detail data for the fluid simulation

Author:	Valentin Hinov
Date: 31/3/2014
*********************************************************************/

#ifndef _LODDATA_H
#define	_LODDATA_H

#include <memory>

namespace DirectX 
{
	struct BoundingBox;
}

class ICamera;
enum ETwType;

struct LODData {
	float overallLOD;

	float distanceLOD;
	float minDistance;
	float maxDistance;
	
	float partOfScreen;

	int maxFramesToSkip;
	int framesToSkip;

	int maxSamples;
	int numSamples;

	void SetObjectBoundingBox(const DirectX::BoundingBox* objectBox);
	void CalculateOverallLOD(const ICamera &camera);
	LODData();
	~LODData();

	static ETwType GetLODDataTwType(); // for use on an AntTweakBar

private:
	// Calculates what percentage of the total screen is occupied by the simulation
	void CalculateScreenPercentage(const ICamera &camera);
	void CalculatedDistanceLOD(const ICamera &camera);

private:
	const DirectX::BoundingBox* pObjectBox;
};



#endif 
