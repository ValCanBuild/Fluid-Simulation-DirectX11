/*************************************************************
GeometryBuilder: Provides utility functions for creating 
geometrical objects and populating the renderer for it

Author: Valentin Hinov
Date: 09/04/2013
**************************************************************/
#ifndef _GEOMETRY_BUILDER_H
#define _GEOMETRY_BUILDER_H

#include "D3dIncludes.h"

class D3DRenderer;

// Creates a unit cube with the with normal and texture coordinates
bool BuildCubeNormalTexture(ID3D11Device* device, D3DRenderer *targetRenderer);

#endif