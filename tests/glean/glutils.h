// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999, 2000  Allen Akin   All Rights Reserved.
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




// glutils.h:  frequently-used OpenGL operations

#ifndef __glutils_h__
#define __glutils_h__

namespace GLEAN {

class Environment;		// Forward reference.

namespace GLUtils {

// Set up projection and modelview matrices so that first-quadrant
// object coordinates map directly to screen coordinates (using the
// normal Cartesian convention, with (0,0) at lower left).
void useScreenCoords(int windowW, int windowH);

// Check to see if the current rendering context supports a given
// extension or set of extensions.  (This is here, rather than in
// RenderingContext, because it can only be applied to the ``current''
// context rather than to any arbitrary context.)
bool haveExtensions(const char* required);
inline bool haveExtension(const char* name) {
	return haveExtensions(name);
}

// Get a pointer to a function (usually, an extension function).
// Like haveExtension, we have to do this here rather than in
// RenderingContext.
void (APIENTRY *getProcAddress(const char* name))();

// Return GL renderer version as a float (1.1, 2.0, etc)
float getVersion();

// Check for OpenGL errors and log any that have occurred:
void logGLErrors(Environment& env);

// Syntactic sugar for dealing with light source parameters:
class Light {
	GLenum lightNumber;
    public:
	Light(int l);
	void ambient(float r, float g, float b, float a);
	void diffuse(float r, float g, float b, float a);
	void specular(float r, float g, float b, float a);
	void position(float x, float y, float z, float w);
	void spotDirection(float x, float y, float z);
	void spotExponent(float e);
	void spotCutoff(float c);
	void constantAttenuation(float a);
	void linearAttenuation(float a);
	void quadraticAttenuation(float a);
	void enable();
	void disable();
}; // Light

// Syntactic sugar for dealing with light model:
class LightModel {
    public:
	LightModel();
	void ambient(float r, float g, float b, float a);
	void localViewer(bool v);
	void twoSide(bool v);
	void colorControl(GLenum e);
}; // LightModel

// Syntactic sugar for dealing with material properties:
class Material {
	GLenum face;
    public:
	Material(GLenum f = GL_FRONT_AND_BACK);
	void ambient(float r, float g, float b, float a);
	void diffuse(float r, float g, float b, float a);
	void ambientAndDiffuse(float r, float g, float b, float a);
	void specular(float r, float g, float b, float a);
	void emission(float r, float g, float b, float a);
	void shininess(float s);
}; // Material

} // namespace GLUtils

} // namespace GLEAN

#endif // __glutils_h__
