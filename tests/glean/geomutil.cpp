// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999,2000  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT



// geomutil.cpp:  frequently-used geometric operations

#include "geomutil.h"
#include "rand.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <float.h>
#include <stdio.h>

using namespace std;

namespace GLEAN {

///////////////////////////////////////////////////////////////////////////////
// RandomMesh2D:  Generate 2D array with fixed boundaries but interior points
//	that have been perturbed randomly.
///////////////////////////////////////////////////////////////////////////////
RandomMesh2D::RandomMesh2D(float minX, float maxX, int xPoints,
    float minY, float maxY, int yPoints,
    RandomDouble& rand) {
    	m = new float[xPoints * yPoints * 2];
	rowLength = xPoints;

	// Loop var; we declare it here and not in the for loop because
	// different compilers scope variables differently when
	// declared in a for loop.
	int iy;

	// Drop each point squarely into the center of its grid cell:
	for (iy = 0; iy < yPoints; ++iy)
		for (int ix = 0; ix < xPoints; ++ix) {
			float* v = (*this)(iy, ix);
			v[0] = minX + (ix * (maxX - minX)) / (xPoints - 1);
			v[1] = minY + (iy * (maxY - minY)) / (yPoints - 1);
		}
	// Now perturb each interior point, but only within its cell:
	double deltaX = 0.9 * (maxX - minX) / (xPoints - 1);
	double deltaY = 0.9 * (maxY - minY) / (yPoints - 1);
	for (iy = 1; iy < yPoints - 1; ++iy)
		for (int ix = 1; ix < xPoints - 1; ++ix) {
			float* v = (*this)(iy, ix);
			v[0] += deltaX * (rand.next() - 0.5);
			v[1] += deltaY * (rand.next() - 0.5);
		}
} // RandomMesh2D::RandomMesh2D

RandomMesh2D::~RandomMesh2D() {
	delete[] m;
} // RandomMesh2D::~RandomMesh2D


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Sphere3D: Forms a stacks/slices sphere and can return the vertices and index list for drawing it.
//////////////////////////////////////////////////////////////////////////////////////////////////////
Sphere3D::Sphere3D(float radius, int slices, int stacks)
{
    // Loop vars.
    int curStack, curSlice;

    // Can't have a sphere of less than 2 slices or stacks.
    assert(slices >= 2 && stacks >= 2);

    // We have 2 verts for the top and bottom point, and then slices*(stacks-1) more for the
    // middle rings (it's stacks-1 since the top and bottom points each count in the stack count).
    numVertices = 2 + (slices*(stacks-1));
    vertices.reserve(numVertices*3);
    normals.reserve(numVertices*3);

    // The top and bottom slices have <slices> tris in them, and the ones in the middle (since they're
    // made of quads) have 2*<slices> each.
    numIndices = 3*(2*slices + 2*(stacks-2)*slices);
    indices.reserve(numIndices);

#define VX(i) vertices[3*(i)+0]
#define VY(i) vertices[3*(i)+1]
#define VZ(i) vertices[3*(i)+2]
#define VINDEX(st,sl) (1 + (((st)-1)*slices) + (sl))
#ifndef M_PI
#define M_PI 3.14159
#endif

    // Generate the verts.  The bottom and top verts are kind of special cases (they
    // occupy the first and last vertex slots, respectively).
    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(-radius);
    normals.push_back(0);
    normals.push_back(0);
    normals.push_back(-1);

    // Now the inner rings; I can't decide whether it spreads the tri area out better to do this by
    // increments in the spherical coordinate phi or in the cartesian z, but I think phi is a better bet.
    for (curStack=1; curStack<stacks; curStack++)
    {
        float phi = M_PI - ((curStack / (float)stacks) * M_PI);
        float zVal = radius * cos(phi);
        float sliceRadius = sqrt(radius*radius - zVal*zVal);
        for (curSlice = 0; curSlice < slices; curSlice++)
        {
            float theta = 2*M_PI*((float)curSlice / slices);

            float xVal = sliceRadius*cos(theta);
            float yVal = sliceRadius*sin(theta);

            vertices.push_back(xVal);
            vertices.push_back(yVal);
            vertices.push_back(zVal);
            normals.push_back(xVal/radius);
            normals.push_back(yVal/radius);
            normals.push_back(zVal/radius);
        }
    }

    vertices.push_back(0);
    vertices.push_back(0);
    vertices.push_back(radius);
    normals.push_back(0);
    normals.push_back(0);
    normals.push_back(1);

    // Now to assemble them into triangles.  Do the top and bottom slices first.
    for (curSlice=0; curSlice<slices; curSlice++)
    {
        indices.push_back(0);
        indices.push_back((curSlice+1)%slices + 1);
        indices.push_back(curSlice+1);

        indices.push_back(numVertices - 1);
        indices.push_back(numVertices - 2 - ((curSlice+1)%slices));
        indices.push_back(numVertices - 2 - curSlice);
    }

    // Now for the inner rings.  We're already done with 2*slices triangles, so start after that.
    for (curStack=1; curStack<stacks-1; curStack++)
    {
        for (curSlice=0; curSlice<slices; curSlice++)
        {
            int nextStack = curStack+1;
            int nextSlice = (curSlice+1)%slices;
            indices.push_back(VINDEX(curStack, curSlice));
            indices.push_back(VINDEX(curStack, nextSlice));
            indices.push_back(VINDEX(nextStack, nextSlice));

            indices.push_back(VINDEX(curStack, curSlice));
            indices.push_back(VINDEX(nextStack, nextSlice));
            indices.push_back(VINDEX(nextStack, curSlice));
        }
    }

    assert(static_cast<int>(vertices.size()) == numVertices*3);
    assert(static_cast<int>(indices.size()) == numIndices);

#undef VX
#undef VY
#undef VZ
#undef VINDEX
}

// This returns the vertices: 3 floats per vertex in a tightly packed array (no padding between vertices).
const float* Sphere3D::getVertices() const { return &(vertices[0]); }
int Sphere3D::getNumVertices() const { return numVertices; }

// This returns the normals; same data format as the vertices.  And of course the number of normals is
// the same as the number of vertices.
const float* Sphere3D::getNormals() const { return &(normals[0]); }

// This returns a series of vertices that form triangles from the vertices (the indices specify loose
// triangles, not tristrips or fans or whatnot.  So each triplet of indices is an individual triangle.)
const unsigned int* Sphere3D::getIndices() const { return &(indices[0]); }
int Sphere3D::getNumIndices() const { return numIndices; }


} // namespace GLEAN
