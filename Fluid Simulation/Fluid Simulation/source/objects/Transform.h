/***************************************************************
Transform Class: Provides a transform component to objects.

Author: Valentin Hinov
Date: 06/09/2013
Version: 1.0
**************************************************************/

#ifndef _TRANSFORM_H
#define _TRANSFORM_H

#include "../utilities/math/MathUtils.h"

class Transform {
public:
	Transform(): position(0,0,0),rotation(0,0,0),scale(1,1,1){
		
	}

	Vector3f position;
	Vector3f rotation;
	Vector3f scale;

	void GetTransformMatrix(D3DXMATRIX &matrix) {
		D3DXMATRIX m;
		D3DXMatrixIdentity(&matrix);
		D3DXMatrixScaling(&m, scale.x, scale.y, scale.z);
		matrix*=m;
		D3DXMatrixRotationX(&m, rotation.x);
		matrix*=m;
		D3DXMatrixRotationY(&m, rotation.y);
		matrix*=m;
		D3DXMatrixRotationZ(&m, rotation.z);	
		matrix*=m;
		D3DXMatrixTranslation(&m, position.x, position.y, position.z);
		matrix*=m;
	}

	~Transform(){}	
};

#endif