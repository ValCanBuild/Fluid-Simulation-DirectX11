/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#ifndef _COLLISIONMANAGER_H
#define _COLLISIONMANAGER_H

#include <vector>
#include "../objects/BoxCollider.h"
#include "GeometricPrimitive.h"

class BaseD3DBody; 

struct ContactPoint {
	Vector3 normal; // Normal of the contact point
	Vector3 hitPoint;
	float   penetration;

	ContactPoint() : normal(0.0f), hitPoint(0.0f), penetration(0.0f) {}
};

struct ContactManifold {
	const Collider *pBody1;
	const Collider *pBody2;

	int numContacts;
	std::vector<ContactPoint> contactPoints;

	ContactManifold() : pBody1(nullptr), pBody2(nullptr), numContacts(0) {}
};

class CollisionManager {
public:
	CollisionManager(std::vector<BaseD3DBody*> *sceneObjects);
	~CollisionManager();

	void DebugRender(const Matrix* viewMatrix, const Matrix* projMatrix) const;

	void PerformCollisionResponse(float dt) const;
	void PerformCollisionCheck();

private:
	void CheckForCollision(const BoxCollider * pBox0, const BoxCollider * pBox1); // check for a collision and write result to contact manifold collection if there is one

	// Box Collision Functions
	bool SpanIntersect(const BoxCollider* pBox0, const BoxCollider* pBox1, const Vector3& axisc, _Inout_ float* minPenetration, _Inout_ Vector3* axisPenetration, _Inout_ float* penetration = nullptr) const;
	void CalculateInterval(const BoxCollider* pBox, const Vector3& axis, float& min, float& max) const;
	void CalculateHitPoint(const BoxCollider* pBox0, const BoxCollider* pBox1, const float penetration, Vector3& hitNormal, _Out_ std::vector<ContactPoint> &contactPoints) const;
	int  GetNumHitPoints(const BoxCollider* pBox, const Vector3& hitNormal, const float penetration, Vector3 verts[8], int* vertIndexs) const;
	void ClipFaceFaceVerts(Vector3* verts0, int* vertIndexs0, Vector3* verts1, int* vertIndexs1, Vector3* vertsX, int* numVertsX) const;
	void SortVertices(Vector3* verts0, int* vertIndexs0) const;
	bool VertInsideFace(Vector3* verts0, Vector3& p0, float planeErr = 0.0f) const;
	void ClosestPointLineLine(const Vector3* verts0, const Vector3* verts1, _Out_ Vector3* vertsX, _Out_ int* numVertX) const;
	void ClipLinePlane(const Vector3* verts0, const BoxCollider* pBox1, _Inout_ Vector3* vertsX, _Out_ int* numVertX) const;
	void ClosestPtPointOBB(const Vector3& point, const BoxCollider* pBox, _Out_ Vector3* closestPoint) const;

private:
	const std::vector<BaseD3DBody*> *	pSceneObjects;
	std::vector<ContactManifold>		mContactManifolds; // accumulates contact results each frame

	std::vector<DirectX::GeometricPrimitive*>    mRenderContactPoints;
};

#endif