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

using namespace std;

#include "geomutil.h"
#include "rand.h"
#include <cassert>
#include <algorithm>
#include <cmath>
#include <float.h>
#include <stdio.h>

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



///////////////////////////////////////////////////////////////////////////////
// SpiralStrip2D: Generate (x,y) vertices for a triangle strip of arbitrary
//	length.  The triangles are of approximately equal size, and arranged
//	in a spiral so that a reasonably large number of triangles can be
//	packed into a small screen area.
///////////////////////////////////////////////////////////////////////////////
SpiralStrip2D::SpiralStrip2D(int nPoints, float minX, float maxX,
    float minY, float maxY) {

	// Most of the complexity of this code results from attempting
	// to keep the triangles approximately equal in area.
	//
	// Conceptually, we construct concentric rings whose inner and
	// outer radii differ by a constant.  We then split each ring
	// (at theta == 0), and starting from the point of the split
	// gradually increase both the inner and outer radii so that
	// when we've wrapped all the way around the ring (to theta ==
	// 2*pi), the inner radius now matches the original outer
	// radius.  We then repeat the process with the next ring
	// (working from innermost to outermost) until we've
	// accumulated enough vertices to satisfy the caller's
	// requirements.
	//
	// Finally, we scale and offset all the points so that the
	// resulting spiral fits comfortably within the rectangle
	// provided by the caller.

	// Set up the array of vertices:
	v = new float[2 * nPoints];
	float* lastV = v + 2 * nPoints;

	// Initialize the ring parameters:
	double innerRadius = 4.0;
	double ringWidth = 1.0;
	double segLength = 1.0;

	float* pV = v;
	while (pV < lastV) {
		// Each ring consists of segments.  We'll make the arc
		// length of each segment that lies on the inner
		// radius approximately equal to segLength, but we'll
		// adjust it to ensure there are an integral number of
		// equal-sized segments in the ring.
		int nSegments = static_cast<int>
			(6.2831853 * innerRadius / segLength + 0.5);

		double dTheta = 6.2831853 / nSegments;
		double dRadius = ringWidth / nSegments;

		double theta = 0.0;
		for (int i = 0; i < nSegments; ++i) {
			double c = cos(theta);
			double s = sin(theta);

			*pV++ = innerRadius * c;
			*pV++ = innerRadius * s;
			if (pV >= lastV)
				break;

			*pV++ = (innerRadius + ringWidth) * c;
			*pV++ = (innerRadius + ringWidth) * s;
			if (pV >= lastV)
				break;

			theta += dTheta;
			innerRadius += dRadius;
		}
	}

	// Find the bounding box for the spiral:
	float lowX = FLT_MAX;
	float highX = - FLT_MAX;
	float lowY = FLT_MAX;
	float highY = -FLT_MAX;
	for (pV = v; pV < lastV; pV += 2) {
		lowX = min(lowX, pV[0]);
		highX = max(highX, pV[0]);
		lowY = min(lowY, pV[1]);
		highY = max(highY, pV[1]);
	}

	// Find scale and offset to map the spiral into the bounds supplied
	// by our caller, with a little bit of margin around the edges:
	lowX -= ringWidth;
	highX += ringWidth;
	lowY -= ringWidth;
	highY += ringWidth;
	float scaleX = (maxX - minX) / (highX - lowX);
	float offsetX = minX - scaleX * lowX;
	float scaleY = (maxY - minY) / (highY - lowY);
	float offsetY = minY - scaleY * lowY;

	// Finally scale and offset the constructed vertices so that
	// they fit in the caller-supplied rectangle:
	for (pV = v; pV < lastV; pV += 2) {
		pV[0] = scaleX * pV[0] + offsetX;
		pV[1] = scaleY * pV[1] + offsetY;
	}
} // SpiralStrip2D::SpiralStrip2D

SpiralStrip2D::~SpiralStrip2D() {
	delete[] v;
} // SpiralStrip2D::~SpiralStrip2D




///////////////////////////////////////////////////////////////////////////////
// SpiralTri2D:  Generate (x,y) vertices for a set of independent triangles,
//	arranged in spiral fashion exactly as in SpiralStrip2D.
//	One may rely on the fact that SpiralTri2D generates exactly the
//	same triangles as SpiralStrip2D, so that comparison of images
//	using the two primitives is meaningful.
///////////////////////////////////////////////////////////////////////////////
SpiralTri2D::SpiralTri2D(int nTris, float minX, float maxX,
    float minY, float maxY) {
	SpiralStrip2D ts(nTris + 2, minX, maxX, minY, maxY);
	const int nVertices = 3 * nTris;
	v = new float[2 * nVertices];

	float* pTris = v;
	float* pStrip = ts(0);
	bool front = true;	// winding order alternates in strip
	for (int i = 0; i < nTris; ++i) {
		if (front) {
			pTris[0] = pStrip[0];
			pTris[1] = pStrip[1];

			pTris[2] = pStrip[2];
			pTris[3] = pStrip[3];

			pTris[4] = pStrip[4];
			pTris[5] = pStrip[5];
		} else {
			pTris[0] = pStrip[0];
			pTris[1] = pStrip[1];

			pTris[2] = pStrip[4];
			pTris[3] = pStrip[5];

			pTris[4] = pStrip[2];
			pTris[5] = pStrip[3];
		}

		front = !front;
		pTris += 6;
		pStrip += 2;
	}
} // SpiralTri2D::SpiralTri2D

SpiralTri2D::~SpiralTri2D() {
	delete[] v;
} // SpiralTri2D::~SpiralTri2D


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
