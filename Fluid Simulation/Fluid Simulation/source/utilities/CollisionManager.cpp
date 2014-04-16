/*************************************************************
CollisionManager Class: Detects and manages collisions in a
scene

Author: Valentin Hinov
Date: 07/12/2013
**************************************************************/

#include <iostream>
#include "CollisionManager.h"
#include "../objects/BaseD3DBody.h"
#include "Physics.h"
#include "../system/ServiceProvider.h"
#include "../display/D3DGraphicsObject.h"
#include "../system/IGraphicsSystem.h"

using namespace DirectX;
using namespace std;

const int MAX_DEBUG_SPHERES = 20;

CollisionManager::CollisionManager(vector<BaseD3DBody*> *sceneObjects ) : pSceneObjects(sceneObjects) {
	// initialize an array of debug contact points
	auto graphicsSystem = ServiceProvider::Instance().GetService<IGraphicsSystem>();
	const IGraphicsObject* object = graphicsSystem->GetGraphicsObject();
	const D3DGraphicsObject *d3dObject = dynamic_cast<const D3DGraphicsObject*>(object);
	for (int i = 0; i < MAX_DEBUG_SPHERES; ++i) {
		unique_ptr<GeometricPrimitive> sphere = GeometricPrimitive::CreateSphere(d3dObject->GetDeviceContext(), 0.05f);
		mRenderContactPoints.push_back(sphere.release());
	}
}

CollisionManager::~CollisionManager() {
	pSceneObjects = nullptr;
	mContactManifolds.clear();
	for (GeometricPrimitive *sphere : mRenderContactPoints) {
		if (sphere) {
			delete sphere;
			sphere = nullptr;
		}
	}
	mRenderContactPoints.clear();
}

void CollisionManager::DebugRender(const Matrix* viewMatrix, const Matrix* projMatrix) const {
	int usedSpheres = 0;
	int numManifolds = mContactManifolds.size();
	for (int i = 0; i < numManifolds && usedSpheres < MAX_DEBUG_SPHERES; ++i) {
		vector<ContactPoint> contactPoints = mContactManifolds[i].contactPoints;
		int numContacts = contactPoints.size();
		for (int j = 0; j < numContacts && usedSpheres < MAX_DEBUG_SPHERES; ++j) {
			ContactPoint cPoint = contactPoints[j];

			//Matrix worldMat = Matrix::CreateScale(1.0f);
			Matrix worldMat = Matrix::CreateTranslation(cPoint.hitPoint);

			mRenderContactPoints[usedSpheres]->Draw(worldMat,*viewMatrix,*projMatrix,Colors::Yellow);

			usedSpheres++;
		}
	}
}

void CollisionManager::PerformCollisionCheck() {
	mContactManifolds.clear();

	int numObjects = pSceneObjects->size();
	for (int i = 0; i < numObjects; ++i) {
		BaseD3DBody * body = pSceneObjects->at(i);
		shared_ptr<BoxCollider> firstCollider = body->boxCollider;
		if (firstCollider == nullptr) {
			continue;
		}
		// only check for collision if collider has a rigid body and it is enabled is awake and collider is enabled
		shared_ptr<RigidBody3D> rigidBody0 = firstCollider->GetAttachedRigidBody();
		if (firstCollider->IsEnabled() && rigidBody0 != nullptr) {
			if (rigidBody0->GetIsSleeping() || rigidBody0->GetIsImmovable()) {
				continue;
			}
			// check against other enabled colliders - they don't need to have rigid bodies
			for (int j = 0; j < numObjects; ++j) {
				BaseD3DBody * otherBody = pSceneObjects->at(j);
				if (body == otherBody) {
					continue;
				}
				shared_ptr<BoxCollider> secondCollider = otherBody->boxCollider;
				if (secondCollider == nullptr) {
					continue;
				}
				if (secondCollider->IsEnabled()) {
					CheckForCollision(firstCollider.get(),secondCollider.get());
				}
			}
		}
	}
}

void CollisionManager::PerformCollisionResponse(float dt) const {
	int numCollisions = mContactManifolds.size();
	/*if (mContactManifolds.size() > 0) {
		cout << mContactManifolds.size() << " Contact Manifolds detected" << endl;
		int contactPoints = 0;
		for (int i = 0; i < mContactManifolds.size(); i++) {
			contactPoints += mContactManifolds[i].contactPoints.size();
		}
		if (contactPoints > 0) { 
			cout << "with " << contactPoints << " total Contact Points" << endl;
		}
	}*/

	if (numCollisions < 1) {
		return;
	}
	int i;
	for (i = 0; i < Physics::iSolverIterationCount; ++i) {
		for (ContactManifold cManifold : mContactManifolds) {
			const Collider *collider0 = cManifold.pBody1;
			const Collider *collider1 = cManifold.pBody2;

			shared_ptr<RigidBody3D> rigidBody0 = collider0->GetAttachedRigidBody();
			shared_ptr<RigidBody3D> rigidBody1 = collider1->GetAttachedRigidBody();

			if (rigidBody0) {
				rigidBody0->WakeUp();
				rigidBody0->InContact();
			}
			if (rigidBody1) {				
				rigidBody1->WakeUp();
				rigidBody1->InContact();
			}

			shared_ptr<Transform> transform0 = collider0->GetGameObject()->transform;
			shared_ptr<Transform> transform1 = collider1->GetGameObject()->transform;

			for (ContactPoint cPoint : cManifold.contactPoints) {
				Vector3 hitPoint = cPoint.hitPoint;
				Vector3 normal = cPoint.normal;
				float penDepth = cPoint.penetration;

				//----------------------------------------------------------
				// 
				// j =			                           -(1+Cor)(relv.norm)
				//	     -----------------------------------------------------------------------------------
				//	     norm.norm(1/Mass0 + 1/Mass1) + (sqr(r0 x norm) / Inertia0) + (sqr(r1 x norm) / Inertia1)
				//
				//----------------------------------------------------------
				
				// Step 0: Pre-calculate inverse mass
				float mass0 = rigidBody0 != nullptr ? rigidBody0->GetMass() : 1001.0f;
				float mass1 = rigidBody1 != nullptr ? rigidBody1->GetMass() : 1001.0f;

				float invMass0 = (mass0>1000.0f) ? 0.0f : (1.0f/mass0);
				float invMass1 = (mass1>1000.0f) ? 0.0f : (1.0f/mass1);

				Matrix inertiaMatrix0 = rigidBody0 != nullptr ? rigidBody0->GetInertiaMatrix() : Matrix::Identity();
				Matrix inertiaMatrix1 = rigidBody1 != nullptr ? rigidBody1->GetInertiaMatrix() : Matrix::Identity();

				Matrix inverseInertiaMatrix0 = rigidBody0 != nullptr ? rigidBody0->GetInverseInertiaMatrix() : Matrix::Identity();
				Matrix inverseInertiaMatrix1 = rigidBody1 != nullptr ? rigidBody1->GetInverseInertiaMatrix() : Matrix::Identity();


				// Step 1: Calculate relative velocity
				Vector3 r0 = hitPoint - transform0->position;
				Vector3 r1 = hitPoint - transform1->position;

				Vector3 v0 = rigidBody0 != nullptr ? rigidBody0->GetLinearVelocity() + rigidBody0->GetAngularVelocity().Cross(r0) : Vector3(0.0f);
				Vector3 v1 = rigidBody1 != nullptr ? rigidBody1->GetLinearVelocity() + rigidBody1->GetAngularVelocity().Cross(r1) : Vector3(0.0f);

				Vector3 relVel = v0 - v1;

				// Step 2: Calculate normal impulse
				float vn = relVel.Dot(normal);

				// work out bias to prevent sinking
				const float allowedPenetration = Physics::fAllowedPenetration;
				const float biasFactor = Physics::fContactBiasFactor; // 0.1 to 0.3
				bool positionCorrection = false;
				float biasFactorValue = positionCorrection ? biasFactor : 0.0f;

				float invDt = 1.0f / dt;
				float bias = biasFactorValue * invDt * max(0.0f, penDepth - allowedPenetration);

				float kNormal = invMass0 + invMass1;
				Vector3 r0CrossNorm = r0.Cross(normal);
				Vector3 r1CrossNorm = r1.Cross(normal);

				r0CrossNorm = Vector3::Transform(r0CrossNorm,inverseInertiaMatrix0);
				r0CrossNorm = r0CrossNorm.Cross(r0);

				r1CrossNorm = Vector3::Transform(r1CrossNorm,inverseInertiaMatrix1);
				r1CrossNorm = r1CrossNorm.Cross(r1);

				kNormal += normal.Dot(r0CrossNorm + r1CrossNorm);

				float massNormal = 1.0f / kNormal;

				float dPn = massNormal * (-vn + bias);

				dPn = max(dPn, 0.0f);

				// Step 3: Apply normal contact impulse
				Vector3 normalImpulse = dPn * normal;

				if (rigidBody0) {
					rigidBody0->AddLinearVelocity(invMass0 * normalImpulse);
					
					rigidBody0->AddAngularVelocity(Vector3::Transform(r0.Cross(normalImpulse), inverseInertiaMatrix0));
					//rigidBody0->AddForce(normalImpulse * mass0);
					//rigidBody0->AddTorque(Vector3::Transform(r0.Cross(normalImpulse), inertiaMatrix0));
				}

				if (rigidBody1) {
					rigidBody1->AddLinearVelocity(-invMass1 * normalImpulse);
					rigidBody1->AddAngularVelocity(-Vector3::Transform(r1.Cross(normalImpulse), inverseInertiaMatrix1));
					//rigidBody1->AddForce(-normalImpulse * mass1);
					//rigidBody1->AddTorque(-Vector3::Transform(r1.Cross(normalImpulse), inertiaMatrix1));
				}

				// Step 4: Apply tangent impulse			
				Vector3 tangent = relVel - (vn * normal);
				tangent.Normalize();

				float kTangent = invMass0 + invMass1;

				Vector3 r0CrossTangent = r0.Cross(tangent);
				Vector3 r1CrossTangent = r1.Cross(tangent);

				r0CrossTangent = Vector3::Transform(r0CrossTangent,inverseInertiaMatrix0);
				r0CrossTangent = r0CrossTangent.Cross(r0);

				r1CrossTangent = Vector3::Transform(r0CrossTangent,inverseInertiaMatrix1);
				r1CrossTangent = r1CrossTangent.Cross(r1);

				kTangent += tangent.Dot(r0CrossTangent + r1CrossTangent);

				float massTangent = 1.0f / kTangent;

				float vt = relVel.Dot(tangent);
				float dPt = massTangent * (-vt);

				float maxPt = (Physics::fRestitution) * dPn;
				dPt = Clamp(dPt, -maxPt, maxPt);

				// Apply contact impulse
				Vector3 tangentImpulse = dPt * tangent;

				if (rigidBody0) {
					rigidBody0->AddLinearVelocity(invMass0 * tangentImpulse);
					rigidBody0->AddAngularVelocity(Vector3::Transform(r0.Cross(tangentImpulse), inverseInertiaMatrix0));
					//rigidBody0->AddForce(tangentImpulse * mass0);
					//rigidBody0->AddTorque(Vector3::Transform(r0.Cross(tangentImpulse), inertiaMatrix0));
				}

				if (rigidBody1) {
					rigidBody1->AddLinearVelocity(-invMass1 * tangentImpulse);
					rigidBody1->AddAngularVelocity(-Vector3::Transform(r1.Cross(tangentImpulse), inverseInertiaMatrix1));
					//rigidBody1->AddForce(-tangentImpulse * mass1);
					//rigidBody1->AddTorque(-Vector3::Transform(r1.Cross(tangentImpulse), inertiaMatrix1));
				}
			}
		}
	}
}

void CollisionManager::CheckForCollision(const BoxCollider * pBox0, const BoxCollider * pBox1) {
	// Simple bounding sphere check first
	float combinedRadii = (pBox0->GetRadius() + pBox1->GetRadius());
	float combinedCenterDist = Vector3::DistanceSquared(pBox1->GetCenter(),pBox0->GetCenter());

	if (combinedCenterDist > combinedRadii*combinedRadii) {
		return; // too far apart
	}

	// do not add, if already exists
	for (ContactManifold existing : mContactManifolds) {
		if (pBox0 == existing.pBody1 || pBox0 == existing.pBody2) {
			if (pBox1 == existing.pBody1 || pBox1 == existing.pBody2) {
				return;
			}
		}
	}

	bool hit = true;
	float minPenetration = 10000.0f;
	Vector3 axisPenetration;

	Vector3 box0LocalRot[3];
	Vector3 box1LocalRot[3];

	pBox0->GetLocalRotationVectors(box0LocalRot);
	pBox1->GetLocalRotationVectors(box1LocalRot);

	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[0], &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[1], &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[2], &minPenetration, &axisPenetration);
						 
	hit &= SpanIntersect(pBox0, pBox1, box1LocalRot[0], &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box1LocalRot[1], &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box1LocalRot[2], &minPenetration, &axisPenetration);
						 
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[0].Cross(box1LocalRot[0]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[0].Cross(box1LocalRot[1]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[0].Cross(box1LocalRot[2]), &minPenetration, &axisPenetration);
													  
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[1].Cross(box1LocalRot[0]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[1].Cross(box1LocalRot[1]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[1].Cross(box1LocalRot[2]), &minPenetration, &axisPenetration);
													  
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[2].Cross(box1LocalRot[0]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[2].Cross(box1LocalRot[1]), &minPenetration, &axisPenetration);
	hit &= SpanIntersect(pBox0, pBox1, box0LocalRot[2].Cross(box1LocalRot[2]), &minPenetration, &axisPenetration);

	// collision occured
	if (hit) {
		ContactManifold contactManifold;

		contactManifold.pBody1 = pBox0;
		contactManifold.pBody2 = pBox1;

		CalculateHitPoint(pBox0, pBox1, minPenetration, axisPenetration, contactManifold.contactPoints);

		// set normal of contact points
		// set penetration of contact points
		contactManifold.numContacts = contactManifold.contactPoints.size();
		mContactManifolds.push_back(contactManifold);
	}
}

bool CollisionManager::SpanIntersect(const BoxCollider* pBox0, const BoxCollider* pBox1, 
									 const Vector3& axisc, _Inout_ float* minPenetration, _Inout_ Vector3* axisPenetration, 
									 _Inout_ float* penetration) const {

	Vector3 axis = axisc;

	float lengthAxis = axis.LengthSquared();

	if (lengthAxis <= 0.02f) {
		if (penetration) {
			*penetration = 100000.0f;			
		}
		return true;
	}

	axis.Normalize();

	float mina, maxa;
	float minb, maxb;
	CalculateInterval(pBox0,axis,mina,maxa);
	CalculateInterval(pBox1,axis,minb,maxb);

	float lena = maxa - mina;
	float lenb = maxb - minb;

	float minv = min(mina,minb);
	float maxv = max(maxa,maxb);

	float lenv = maxv - minv;

	if ( lenv > (lena + lenb) ) {
		// collision has not occured
		return false;
	}

	/*
	// This test also works as a substitute for the above one
	if (mina>maxb || minb>maxa) {
		return false;
	}
	*/

	float pen = (lena + lenb) - lenv;

	if (penetration) {
		*penetration = pen;
	}

	if (minPenetration && axisPenetration) {
		if (pen < *minPenetration) {
			*minPenetration = pen;
			*axisPenetration = axis;

			// BoxA pushes BoxB away in the correct Direction
			if (minb < mina) {
				*axisPenetration *= -1.0f;
			}
		}
	}

	return true;
}

void CollisionManager::CalculateInterval(const BoxCollider* pBox, const Vector3& axis, float& min, float& max) const {
	Vector3 extents = pBox->GetExtents();
	float x = extents.x;
	float y = extents.y;
	float z = extents.z;

	Vector3 vertex[8] = 
	{
		Vector3(   x,    y,   z	),
		Vector3(  -x,    y,	  z	),
		Vector3(   x,   -y,	  z	),
		Vector3(  -x,   -y,	  z	),

		Vector3(   x,    y,	 -z	),
		Vector3(  -x,    y,	 -z	),
		Vector3(   x,   -y,	 -z	),
		Vector3(  -x,   -y,	 -z	)
	};

	int i;

	Matrix boxWorldMat;

	pBox->GetGameObject()->transform->GetTransformMatrixQuaternion(boxWorldMat, false);

	Vector3::Transform(vertex,8,boxWorldMat,vertex);

	min = max = vertex[0].Dot(axis);

	for (i = 0; i < 8; ++i) {
		float d = vertex[i].Dot(axis);

		if (d < min) 
			min = d;
		if (d > max) 
			max = d;
	}
}

void CollisionManager::CalculateHitPoint(const BoxCollider* pBox0, const BoxCollider* pBox1, const float penetration, Vector3& hitNormal, _Out_ std::vector<ContactPoint> &contactPoints) const {
	Vector3 verts0[8];
	int vertIndeces0[8];

	Vector3 norm0 = hitNormal;

	int numVerts0 = GetNumHitPoints(pBox0, norm0, penetration, verts0, vertIndeces0);

	Vector3 verts1[8];
	int vertIndeces1[8];

	Vector3 norm1 = -hitNormal;

	int numVerts1 = GetNumHitPoints(pBox1, norm1, penetration, verts1, vertIndeces1);

	if (numVerts0 == 0 || numVerts1 == 0) {
		// This should never happen!
		cout << "WARNING: 0 hit points calculated in CollisionManager::CalculateHitPoint" << endl;
		return;
	}

	int numVertsX = numVerts0;
	Vector3 *vertsX = verts0;

	// Face-face collision
	if (numVerts0 >= 4 && numVerts1 >= 4) {
		static Vector3 clipVerts[50];
		
		ClipFaceFaceVerts(	verts0, vertIndeces0, 
							verts1, vertIndeces1,
							clipVerts, &numVertsX);

		vertsX = clipVerts;
	}

	if (numVerts1 < numVerts0) {
		numVertsX = numVerts1;
		vertsX    = verts1;
		hitNormal = -norm1;
	}

	if (numVerts1==2 && numVerts0==2) {
		static Vector3 V[2];
		static int numV = 0;

		ClosestPointLineLine(verts0,
			verts1,
			V,		&numV);

		vertsX = V;
		numVertsX = numV;
	}

	if (numVerts0==2 && numVerts1==4) {
		ClipLinePlane(verts0, pBox0, vertsX, &numVertsX);
	}

	if (numVerts0==4 && numVerts1==2) {
		ClipLinePlane(verts1, pBox1, vertsX, &numVertsX);
	}

	// Write results
	for (int i = 0; i < numVertsX; ++i) {
		ContactPoint cPoint;
		cPoint.hitPoint = vertsX[i];
		cPoint.normal = -hitNormal;
		cPoint.penetration = penetration;
		contactPoints.push_back(cPoint);
	}
}

int CollisionManager::GetNumHitPoints(const BoxCollider* pBox, const Vector3& hitNormal, const float penetration, Vector3 verts[8], int* vertIndexs) const {
	Vector3 extents = pBox->GetExtents();
	float x = extents.x;
	float y = extents.y;
	float z = extents.z;

	Vector3 vertex[8] = 
	{
		Vector3(  -x,   -y,   -z	),
		Vector3(   x,   -y,	  -z	),
		Vector3(   x,   -y,	   z	),
		Vector3(  -x,   -y,	   z	),

		Vector3(  -x,    y,	  -z	),
		Vector3(   x,    y,	  -z	),
		Vector3(   x,    y,	   z	),
		Vector3(  -x,    y,	   z	)
	};

	Matrix boxWorldMat;
	pBox->GetGameObject()->transform->GetTransformMatrixQuaternion(boxWorldMat, false);
	Vector3::Transform(vertex,8,boxWorldMat,vertex);

	// find the farthest vertex in
	// the polygon along the separation normal
	float maxDist = vertex[0].Dot(hitNormal);
	Vector3 planePoint = vertex[0];

	int i;
	for (i = 0; i < 8; ++i) {
		float d = vertex[i].Dot(hitNormal);

		if (d > maxDist) {
			maxDist = d;
			planePoint = vertex[i];
		}		
	}

	// Plane Equation (A dot N) - d = 0;
	float d = planePoint.Dot(hitNormal);
	d -= penetration + Physics::fAllowedPenetration;

	int numVerts = 0;

	for (i = 0; i < 8; ++i) {
		float side = vertex[i].Dot(hitNormal) - d;

		// points intersect
		if (side > 0.0f) {
			verts[numVerts] = vertex[i];
			vertIndexs[numVerts] = i;
			numVerts++;
		}
	}

	return numVerts;
}

void CollisionManager::ClipFaceFaceVerts(Vector3* verts0, int* vertIndexs0, Vector3* verts1, int* vertIndexs1, Vector3* vertsX, int* numVertsX) const {
	SortVertices(verts0, vertIndexs0);
	SortVertices(verts1, vertIndexs1);

	// Work out the normal for the face
	Vector3 v0 = verts0[1] - verts0[0];
	Vector3 v1 = verts0[2] - verts0[0];

	Vector3 faceNormal = v1.Cross(v0);
	faceNormal.Normalize();

	// Project all the vertices onto a shared plane, plane0
	Vector3 vertsTemp1[4];
	for (int i = 0; i < 4; ++i) {
		vertsTemp1[i] = verts1[i] + (faceNormal * faceNormal.Dot(verts0[0]-verts1[i]));
	}

	static Vector3 temp[50];
	int numVerts = 0;

	for (int c = 0; c < 2; ++c) {
		Vector3* vertA = vertsTemp1;
		Vector3* vertB = verts0;

		if (c==1) {
			vertA = verts0;
			vertB = vertsTemp1;
		}

		// Work out the normal for the face
		Vector3 v0 = vertA[1] - vertA[0];
		Vector3 v1 = vertA[2] - vertA[0];

		Vector3 n  = v1.Cross(v0);
		n.Normalize();

		for (int i = 0; i < 4; ++i) {
			Vector3 s0 = vertA[i];
			Vector3 s1 = vertA[(i+1)%4];
			Vector3 sx = s0 + n * 10.0f;

			// Work out the normal for the face
			Vector3 sv0 = s1 - s0;
			Vector3 sv1 = sx - s0;
			Vector3 sn  = sv1.Cross(sv0);
			sn.Normalize();

			float d = s0.Dot(sn);


			for (int j = 0; j < 4; ++j) {
				Vector3 p0 = vertB[j];
				Vector3 p1 = vertB[(j+1)%4]; // Loops back to the 0th for the last one

				float d0 = p0.Dot(sn) - d;
				float d1 = p1.Dot(sn) - d;

				// Check there on opposite sides of the plane
				if ( (d0 * d1) < 0.0f) {
					Vector3 pX = p0 + ( (sn.Dot(s0-p0)) / (sn.Dot(p1-p0)) ) * (p1 - p0);

					if (VertInsideFace(vertA, pX, 0.1f)) {
						temp[numVerts] = pX;
						numVerts++;
					}
				}
				
				if (VertInsideFace(vertA, p0)) {
					temp[numVerts] = p0;
					numVerts++;
				}				

				if (numVerts > 40) {
					// We have a fixed array we pass in, which has a max size of 50
					// so if we go past this we'll have memory corruption
					cout << "WARNING: CollisionManager::ClipFaceFaceVerts numVerts > 40" << endl;
					return;
				}
			}
		}
	}

	// Remove verts which are very close to each other
	for (int i = 0; i < numVerts; ++i) {
		for (int j = i; j < numVerts; ++j) {
			if (i!=j) {
				
				float dist = (temp[i] - temp[j]).LengthSquared();

				if (dist < 6.5f) {
					for (int k = j; k < numVerts; ++k) {
						temp[k] = temp[k+1];
					}
					numVerts--;
				}
			}
		}
	}

	*numVertsX = numVerts;
	for (int i = 0; i < numVerts; ++i) {
		vertsX[i] = temp[i];
	}
}

void CollisionManager::SortVertices(Vector3* verts0, int* vertIndexs0) const {
	int faces[6][4] =
	{
		{4,0,3,7},
		{1,5,6,2},
		{0,1,2,3},
		{7,6,5,4},
		{5,1,0,4},
		{6,7,3,2}
	};

	int faceSet = -1;
	Vector3 temp[4]; // New correct clockwise order

	// Work out which face to use
	for (int i = 0; i < 6 && faceSet == -1; ++i) {
		int numFound = 0;
		for (int j = 0; j < 4; ++j) {
			if (vertIndexs0[j] == faces[i][j]) {
				temp[numFound] = verts0[j];
				numFound++;

				if (numFound==4) {
					faceSet = i;
					break;
				}
			}
		}
	}

	if (faceSet < 0) {
		int errorOccured = 1;
		//cout << "WARNING: Error occured at CollisionManager::SortVertices, faceSet < 0" << endl;
	}
	else {
		for (int i = 0; i < 4; ++i) {
			verts0[i] = temp[i];
		}
	}
}

bool CollisionManager::VertInsideFace(Vector3* verts0, Vector3& p0, float planeErr) const {
	// Work out the normal for the face
	Vector3 v0 = verts0[1] - verts0[0];
	Vector3 v1 = verts0[2] - verts0[0];
	Vector3 faceNormal  = v1.Cross(v0);
	faceNormal.Normalize();

	for (int i = 0; i < 4; ++i) {
		Vector3 s0 = verts0[i];
		Vector3 s1 = verts0[(i+1)%4];
		Vector3 sx = s0 + faceNormal*10.0f;

		// Work out the normal for the face
		Vector3 sv0 = s1 - s0;
		Vector3 sv1 = sx - s0;
		Vector3 sn  = sv1.Cross(sv0);
		sn.Normalize();

		float d  = s0.Dot(sn);
		float d0 = p0.Dot(sn) - d;

		// Outside the plane
		if (d0 > planeErr) {
			return false;
		}
	}

	return true;
}

											// 2 verts			   // 2 verts
void CollisionManager::ClosestPointLineLine(const Vector3* verts0, const Vector3* verts1, _Out_ Vector3* vertsX, _Out_ int* numVertX) const {
	Vector3 p1 = verts0[0];
	Vector3 q1 = verts0[1];
	Vector3 p2 = verts1[0];
	Vector3 q2 = verts1[1];

	Vector3 d1 = q1 - p1;
	Vector3 d2 = q2 - p2;
	Vector3 r  = p1 - p2;
	float a = d1.Dot(d1);
	float e = d2.Dot(d2);
	float f = d2.Dot(r);

	const float epsilon = 0.00001f;

	float s, t;
	Vector3 c1, c2;

	if (a <= epsilon && e <= epsilon) {
		s = t = 0.0f;
		c1 = p1;
		c2 = p2;

		vertsX[0] = c1;
		*numVertX = 1;
		return;
	}

	if (a <= epsilon) {
		s = 0.0f;
		t = f / e;
		t = Clamp(t, 0.0f, 1.0f);
	}
	else {
		float c = d1.Dot(r);
		if (e <= epsilon) {
			t = 0.0f;
			s = Clamp(-c/a, 0.0f, 1.0f);
		}
		else {
			float b = d1.Dot(d2);
			float denom = a*e - b*b;

			if (denom != 0.0f) {
				s = Clamp((b*f - c*e) / denom, 0.0f, 1.0f);
			}
			else{
				s = 0.0f;
			}

			t = (b*s + f) / e;

			if (t < 0.0f) {
				t = 0.0f;
				s = Clamp(-c / a , 0.0f, 1.0f);
			}
			else if (t > 1.0f) {
				t = 1.0f;
				s = Clamp((b-c) / a, 0.0f, 1.0f);
			}
		}
	}

	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;

	vertsX[0] = (c1 + c2) * 0.5f;
	*numVertX = 1;
}

void CollisionManager::ClipLinePlane( const Vector3* verts0, const BoxCollider* pBox1, _Inout_ Vector3* vertsX, _Out_ int* numVertX) const {
	ClosestPtPointOBB(verts0[0], pBox1, &vertsX[0]);
	ClosestPtPointOBB(verts0[1], pBox1, &vertsX[1]);
	*numVertX = 2;
}

void CollisionManager::ClosestPtPointOBB(const Vector3& point, const BoxCollider* pBox, _Out_ Vector3* closestPoint) const {
	Vector3 d = point - pBox->GetCenter();

	Vector3 q = pBox->GetCenter();

	Vector3 localAxes[3];
	pBox->GetLocalRotationVectors(localAxes);

	Vector3 extentsV = pBox->GetExtents();

	float extents[3] = {extentsV.x, extentsV.y, extentsV.z};

	for (int i = 0; i < 3; ++i) {
		float dist = d.Dot(localAxes[i]);

		if (dist > extents[i]) {
			dist = extents[i];
		}

		if (dist < -extents[i]) {
			dist = -extents[i];
		}

		q += dist * localAxes[i];
	}

	*closestPoint = q;
}