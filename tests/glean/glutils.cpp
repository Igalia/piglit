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




// glutils.cpp:  frequently-used OpenGL operations

#define GLX_GLXEXT_PROTOTYPES
#include <stdlib.h>
#include "glwrap.h"
#include "environ.h"
#include "lex.h"
#include "glutils.h"
#if defined(__X11__) || defined(__AGL__)
#   include <dlfcn.h>
#endif
#if defined(__AGL__)
#   include <cstring>
#endif

namespace GLEAN {

namespace GLUtils {

///////////////////////////////////////////////////////////////////////////////
// useScreenCoords:  Map object coords directly to screen coords.
///////////////////////////////////////////////////////////////////////////////
void
useScreenCoords(int windowW, int windowH) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowW, 0, windowH, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, windowW, windowH);
	glTranslatef(0.375, 0.375, 0.0);
} // useScreenCoords

///////////////////////////////////////////////////////////////////////////////
// haveExtensions:  See if the current rendering context supports a given
//	set of extensions.
///////////////////////////////////////////////////////////////////////////////
bool
haveExtensions(const char* required) {
	const char* available = reinterpret_cast<const char*>
		(glGetString(GL_EXTENSIONS));

	if (!required)
		return true;
	if (!available)
		return false;

	bool haveAll = true;
	Lex lRequired(required);
	for (lRequired.next(); lRequired.token != Lex::END; lRequired.next()) {
		if (lRequired.token != Lex::ID)
			continue;
		bool haveOne = false;
		Lex lAvailable(available);
		for (lAvailable.next(); lAvailable.token != Lex::END;
		    lAvailable.next())
			if (lAvailable.token == Lex::ID
			  && lAvailable.id == lRequired.id) {
				haveOne = true;
				break;
			}
		haveAll &= haveOne;
		if (!haveAll)
			break;
	}

	return haveAll;
} // haveExtensions


float
getVersion()
{
   const GLubyte *version = glGetString(GL_VERSION);
   // we rely on atof() stopping parsing at whitespace
   return atof((const char *) version);
}


///////////////////////////////////////////////////////////////////////////////
// getProcAddress: Get address of an OpenGL or window-system-binding function.
//	This belongs here, rather than as a member of RenderingContext, because
//	on Windows it must only be applied to the *current* context.  (The
//	return value on Windows is context-dependent, and wglGetProcAddress
//	doesn't take a rendering context as an argument.)
///////////////////////////////////////////////////////////////////////////////
void
(*getProcAddress(const char* name))() {
#if defined(__X11__)
#   if defined(GLX_ARB_get_proc_address)
	return glXGetProcAddressARB(reinterpret_cast<const GLubyte*>(name));
#   else
	// XXX This isn't guaranteed to work, but it may be the best option
	// we have at the moment.
	void* libHandle = dlopen("libGL.so", RTLD_LAZY);
	if (libHandle) {
		void* funcPointer = dlsym(libHandle, name);
		dlclose(libHandle);
		return funcPointer;
	} else
		return 0;
#   endif
#elif defined(__WIN__)
	// Gotta be a little more explicit about the cast to please MSVC.
	typedef void (__cdecl* VOID_FUNC_VOID) ();
	return reinterpret_cast<VOID_FUNC_VOID>(wglGetProcAddress(name));
#elif defined(__BEWIN__)
#	error "Need GetProcAddress (or equivalent) for BeOS"
	return 0;
#elif defined(__AGL__)
	return reinterpret_cast<void (*)()>(dlsym(RTLD_DEFAULT, name));
#endif
} // getProcAddress

///////////////////////////////////////////////////////////////////////////////
// logGLErrors: Check for OpenGL errors and log any that have occurred.
///////////////////////////////////////////////////////////////////////////////
void
logGLErrors(Environment& env) {
	GLenum err;
	while ((err = glGetError()))
		env.log << "\tOpenGL error: " << gluErrorString(err) << '\n';
} // logGLErrors


///////////////////////////////////////////////////////////////////////////////
// Syntactic sugar for light sources
///////////////////////////////////////////////////////////////////////////////

Light::Light(int l) {
	lightNumber = static_cast<GLenum>(GL_LIGHT0 + l);
} // Light::Light

void
Light::ambient(float r, float g, float b, float a){
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glLightfv(lightNumber, GL_AMBIENT, v);
} // Light::ambient

void
Light::diffuse(float r, float g, float b, float a){
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glLightfv(lightNumber, GL_DIFFUSE, v);
} // Light::diffuse

void
Light::specular(float r, float g, float b, float a){
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glLightfv(lightNumber, GL_SPECULAR, v);
} // Light::specular

void
Light::position(float x, float y, float z, float w){
	GLfloat v[4];
	v[0] = x; v[1] = y; v[2] = z; v[3] = w;
	glLightfv(lightNumber, GL_POSITION, v);
} // Light::position

void
Light::spotDirection(float x, float y, float z){
	GLfloat v[3];
	v[0] = x; v[1] = y; v[2] = z;
	glLightfv(lightNumber, GL_SPOT_DIRECTION, v);
} // Light::spotDirection

void
Light::spotExponent(float e){
	glLightf(lightNumber, GL_SPOT_EXPONENT, e);
} // Light::spotExponent

void
Light::spotCutoff(float c){
	glLightf(lightNumber, GL_SPOT_CUTOFF, c);
} // Light::spotCutoff

void
Light::constantAttenuation(float a){
	glLightf(lightNumber, GL_CONSTANT_ATTENUATION, a);
} // Light::constantAttenuation

void
Light::linearAttenuation(float a){
	glLightf(lightNumber, GL_LINEAR_ATTENUATION, a);
} // Light::linearAttenuation

void
Light::quadraticAttenuation(float a){
	glLightf(lightNumber, GL_QUADRATIC_ATTENUATION, a);
} // Light::quadraticAttenuation

void
Light::enable() {
	glEnable(lightNumber);
} // Light::enable

void
Light::disable() {
	glDisable(lightNumber);
} // Light::disable

///////////////////////////////////////////////////////////////////////////////
// Syntactic sugar for light model
///////////////////////////////////////////////////////////////////////////////

LightModel::LightModel() {
} // LightModel::LightModel

void
LightModel::ambient(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, v);
} // LightModel::ambient

void
LightModel::localViewer(bool v) {
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, static_cast<GLint>(v));
} // LightModel::localViewer

void
LightModel::twoSide(bool v) {
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, static_cast<GLint>(v));
} // LightModel::twoSide

void
LightModel::colorControl(GLenum e) {
	glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, e);
} // LightModel::colorControl

///////////////////////////////////////////////////////////////////////////////
// Syntactic sugar for material properties
///////////////////////////////////////////////////////////////////////////////

Material::Material(GLenum f) {
	face = f;
} // Material::Material

void
Material::ambient(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glMaterialfv(face, GL_AMBIENT, v);
} // Material::ambient

void
Material::diffuse(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glMaterialfv(face, GL_DIFFUSE, v);
} // Material::diffuse

void
Material::ambientAndDiffuse(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glMaterialfv(face, GL_AMBIENT_AND_DIFFUSE, v);
} // Material::ambientAndDiffuse

void
Material::specular(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glMaterialfv(face, GL_SPECULAR, v);
} // Material::specular

void
Material::emission(float r, float g, float b, float a) {
	GLfloat v[4];
	v[0] = r; v[1] = g; v[2] = b; v[3] = a;
	glMaterialfv(face, GL_EMISSION, v);
} // Material::emission

void
Material::shininess(float s) {
	glMaterialf(face, GL_SHININESS, static_cast<GLfloat>(s));
} // Material::shininess


} // namespace GLUtils

} // namespace GLEAN
