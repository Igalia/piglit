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




// geomutil.h:  frequently-used geometric operations

#ifndef __geomutil_h__
#define __geomutil_h__

#include <vector>

namespace GLEAN {

class RandomDouble;		// Forward reference.

class RandomMesh2D {
	float* m;
	int rowLength;
    public:
	RandomMesh2D(float minX, float maxX, int xPoints,
		   float minY, float maxY, int yPoints,
		   RandomDouble& rand);
	~RandomMesh2D();
	inline float* operator() (int y, int x)
		{ return m + 2 * (y * rowLength + x); }
}; // RandomMesh2D

class Sphere3D {
    std::vector<float> vertices;
    std::vector<float> normals;
    int numVertices;
    std::vector<unsigned int> indices;
    int numIndices;
 public:
    Sphere3D(float radius, int slices, int stacks);

    // This returns the vertices: 3 floats per vertex in a tightly packed array (no padding between vertices).
    const float* getVertices() const;
    int getNumVertices() const;

    // This returns the normals; same data format as the vertices.  And of course the number of normals is
    // the same as the number of vertices.
    const float* getNormals() const;    

    // This returns a series of vertices that form triangles from the vertices (the indices specify loose
    // triangles, not tristrips or fans or whatnot.  So each triplet of indices is an individual triangle.)
    const unsigned int* getIndices() const;
    int getNumIndices() const;
};

} // namespace GLEAN

#endif // __geomutil_h__
