// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
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


// ttexgen.cpp:  Basic test of GL texture coordinate generation.
// This test does a basic test of the glTexGen functions, including
// object_linear, eye_linear, and sphere_map.  We use the Sphere3D with
// a GeomRenderer to draw a sphere, and map a check texture onto it.  We 
// use an ortho projection to keep it simple.  The result should be a 1:1
// mapping of the check texture for all three modes (sphere map maps 1:1 
// because mapping it onto a sphere inverts the spheremap math).
//
// Note that accuracy issues might cause this test to fail if the
// texcoords near the center are a little warped; I've specifically tried
// to keep the matrices as "pure" as possible (no rotations) to
// keep the numerical precision high.  So far it seems to work fine.
// Introducing a rotation by 90 degrees about the x axis resulted,
// on one driver, in a warping at the center of the sphere which caused
// the test to fail.
//
// For the second test of the three, we offset the texture by 0.5,
// so that each test's rendering is visually distinct from the 
// previous.
//
// To test for pass/fail we examine the color buffer for red and blue,
// (the check colors) in the appropriate places.
//
// Author: Brian Sharp (brian@maniacal.org) December 2000

#include "ttexgen.h"
#include <stdio.h>
#include "geomutil.h"


const GLuint viewSize=50;


namespace GLEAN {

void
TexgenTest::FailMessage(BasicResult &r, const std::string& texgenMode,
			GeomRenderer::DrawMethod method, bool arraysCompiled,
			int retainedMode,
			const std::string& colorMismatch) const {
	env->log << name << ":  FAIL "
             << r.config->conciseDescription() << '\n';
	env->log << "\t" << "during mode " << texgenMode << ", ";
    switch(method)
    {
        case GeomRenderer::GLVERTEX_MODE: env->log << "glVertex-style rendering, "; break;
        case GeomRenderer::GLARRAYELEMENT_MODE: env->log << "glArrayElement-style rendering, "; break;
        case GeomRenderer::GLDRAWELEMENTS_MODE: env->log << "glDrawElements-style rendering, "; break;
        case GeomRenderer::GLDRAWARRAYS_MODE: env->log << "glDrawArrays-style rendering, "; break;
    }
    if (arraysCompiled) env->log << "arrays locked, ";
    else env->log << "arrays not locked, ";

    if (retainedMode) env->log << "built into a display list, ";
    else env->log << "called immediately (not display listed), ";
    
    env->log << colorMismatch << std::endl; 
}

bool 
TexgenTest::compareColors(GLfloat* color0, GLfloat* color1, std::string& failureInfo) const {

	// Compare the colors; fail and report why if they don't match.
	if (color0[0] != color1[0] || color0[1] != color1[1] || color0[2] != color1[2])
	{
        // Assemble the error message into a C-string, then hand it back in the string.
        char failureOut[1024];
        sprintf(failureOut, "expected [%f,%f,%f], read back [%f,%f,%f]", 
                color0[0], color0[1], color0[2],
                color1[0], color1[1], color1[2]);

		failureInfo = std::string(failureOut);
		return false;
	}

	return true;
}

bool 
TexgenTest::verifyCheckers(GLfloat* pixels, GLfloat* upperLeftColor, GLfloat* upperRightColor, std::string& failureInfo) const {
    
    // My loop   control variable, since gcc and MSVC do things differently.
    GLint samp;
    
    // It's a viewSize x viewSize pixel block; since we drew a sphere that doesn't quite touch the 
	// edges, we need to be careful not to sample from what should be background.
	// These pairs are hand-picked coordinates on the image that fall on the bottom-left quadrant
	// of the sphere.
	// XXX FIX ME: these sample coordinates assume that viewSize == 50.
    GLuint samples[6][2] = {{13,13}, {4,22}, {22,4}, {20,20}, {20,10}, {10,20}};
    
    // Run through those sample points in the bottom-left corner and make sure they're all the right color.
	for (samp=0; samp<6; samp++)
	{
        GLuint sampleOffset = (samples[samp][0] + (viewSize*samples[samp][1]))*3;
        if (!compareColors(upperRightColor, pixels + sampleOffset, failureInfo))
        {
            return false;
        }
	}
    
    // Run through those sample points in the bottom-right corner and make sure they're all the right color.
	// Note the "viewSize - samples[samp][0]" to flip it to the bottom-right quadrant.
	for (samp=0; samp<6; samp++)
	{
        GLuint sampleOffset = ((viewSize - samples[samp][0]) + (viewSize*samples[samp][1]))*3;
        if (!compareColors(upperLeftColor, pixels + sampleOffset, failureInfo))
        {
            return false;
        }
	}
    
    // Run through those sample points in the upper-right corner and make sure they're all the right color.
	for (samp=0; samp<6; samp++)
	{
        GLuint sampleOffset = ((viewSize - samples[samp][0]) + (viewSize*(viewSize - samples[samp][1])))*3;
        if (!compareColors(upperRightColor, pixels + sampleOffset, failureInfo))
        {
            return false;
        }
	}
    
    // Run through those sample points in the upper-left corner and make sure they're all the right color.
	for (samp=0; samp<6; samp++)
	{
        GLuint sampleOffset = (samples[samp][0] + (viewSize*(viewSize - samples[samp][1])))*3;
        if (!compareColors(upperLeftColor, pixels + sampleOffset, failureInfo))
        {
            return false;
        }
	}
    
	return true;
}
    
    
///////////////////////////////////////////////////////////////////////////////
// runOne:  Run a single test case
///////////////////////////////////////////////////////////////////////////////
void
TexgenTest::runOne(BasicResult& r, Window&) {
    
    // Temporary buffer to store pixels we've read back for verification.
    GLfloat pixels[50*50*3];
	
	// Colors for matching against when we readback pixels.
	GLfloat matchBlue[3] = {0,0,1};
	GLfloat matchRed[3] = {1,0,0};

    // A sphere to draw.
    Sphere3D theSphere(9.9, 32, 16);

    // A GeomRenderer to draw it with.
    GeomRenderer sphereRenderer;
    sphereRenderer.setDrawMethod(GeomRenderer::GLVERTEX_MODE);
    sphereRenderer.setParameterBits(GeomRenderer::NORMAL_BIT);
    sphereRenderer.setVArrayIndices(theSphere.getNumIndices(),GL_UNSIGNED_INT,theSphere.getIndices());
    sphereRenderer.setVertexPointer(theSphere.getNumVertices(), 3, GL_FLOAT, 0, theSphere.getVertices());
    sphereRenderer.setNormalPointer(GL_FLOAT, 0, theSphere.getNormals());
    
	// draw the sphere in a 50x50 pixel window for some precision.
	glViewport(0, 0, 50, 50);
    
	// Basic GL setup.
	glDisable(GL_DITHER);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor3f(1,1,1);
    
	// Setup the projection.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-10,10,-10,10,-10,10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
    
	// Set up our texture.
	glEnable(GL_TEXTURE_2D);
	GLuint checkerTextureHandle;
	glGenTextures(1, &checkerTextureHandle);
	glBindTexture(GL_TEXTURE_2D, checkerTextureHandle);
    
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
    
	// Make a little checker texture.
	unsigned char redBlueCheck[256*256*3];
	for (int x=0; x<256; x++)
	{
        for (int y=0; y<256; y++)
		{
            bool xPastHalf = x >= 128;
			bool yPastHalf = y >= 128;
            
			redBlueCheck[(x+(256*y))*3 + 0] = ((xPastHalf && yPastHalf) || (!xPastHalf && !yPastHalf)) ? 255 : 0;
			redBlueCheck[(x+(256*y))*3 + 1] = 0;
			redBlueCheck[(x+(256*y))*3 + 2] = ((xPastHalf && !yPastHalf) || (!xPastHalf && yPastHalf)) ? 255 : 0;
		}
	}
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, 256, 256, GL_RGB, GL_UNSIGNED_BYTE, redBlueCheck);
    
    // Setup our arrays of configuration info; we loop over the rendering pass a number of times,
    // using a different GL primitive path each time.
    GeomRenderer::DrawMethod drawMethods[] = {GeomRenderer::GLVERTEX_MODE, GeomRenderer::GLARRAYELEMENT_MODE,
                                              GeomRenderer::GLDRAWELEMENTS_MODE, GeomRenderer::GLARRAYELEMENT_MODE,
                                              GeomRenderer::GLDRAWELEMENTS_MODE};

    bool arraysCompiled[] = {false, false, false, true, true};
    
    // Iterate once for all immediate mode styles, then once for retained mode styles.
    for (int retainedMode=0; retainedMode<2; retainedMode++)
    {
        for (int testIteration=0; testIteration<5; testIteration++)
        {
            sphereRenderer.setDrawMethod(drawMethods[testIteration]);
            if (!sphereRenderer.setArraysCompiled(arraysCompiled[testIteration]))
            {
                // We don't have the extension... not sure what we should do.
                // May as well just keep going, it's no big deal (it should still
                // yield correct results, of course, it's just redundant).
            }

            // GL_SPHERE_MAP: with spheremap, the UL corner is blue
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            renderSphere(retainedMode, sphereRenderer);
            glReadPixels(0,0,50,50, GL_RGB, GL_FLOAT, pixels);
            
            // Validate it.
            std::string sphereMapResult;
            if (!verifyCheckers(pixels, matchBlue, matchRed, sphereMapResult))
            {
                FailMessage(r, std::string("GL_SPHERE_MAP"), drawMethods[testIteration], 
                            arraysCompiled[testIteration], retainedMode, sphereMapResult);
                r.pass = false;
                glDeleteTextures(1, &checkerTextureHandle);
                return;
            }
            
            // GL_OBJECT_LINEAR: with object linear and the below planes, the UL corner is red.
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
            float sObjPlane[4] = {0,0.05,0,1.5};  // We flip the checker by setting W to 1.5 (phases by half a period)
            float tObjPlane[4] = {0.05,0,0,1};
            glTexGenfv(GL_S, GL_OBJECT_PLANE, sObjPlane);
            glTexGenfv(GL_T, GL_OBJECT_PLANE, tObjPlane);
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            renderSphere(retainedMode, sphereRenderer);
            glReadPixels(0,0,50,50, GL_RGB, GL_FLOAT, pixels);
            
            // Validate it.
            std::string objectLinearResult;
            if (!verifyCheckers(pixels, matchRed, matchBlue, objectLinearResult))
            {
                FailMessage(r, std::string("GL_OBJECT_LINEAR"), drawMethods[testIteration], 
                            arraysCompiled[testIteration], retainedMode, objectLinearResult);
                r.pass = false;
                glDeleteTextures(1, &checkerTextureHandle);
                return;
            }
            
            // GL_EYE_LINEAR: with eye linear and the below planes, the UL corner is blue.
            glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
            float sEyePlane[4] = {0,0.05,0,1};
            float tEyePlane[4] = {0.05,0,0,1};
            glTexGenfv(GL_S, GL_EYE_PLANE, sEyePlane);
            glTexGenfv(GL_T, GL_EYE_PLANE, tEyePlane);
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            renderSphere(retainedMode, sphereRenderer);
            glReadPixels(0,0,50,50, GL_RGB, GL_FLOAT, pixels);
            
            // Validate it.
            std::string eyeLinearResult;
            if (!verifyCheckers(pixels, matchBlue, matchRed, eyeLinearResult))
            {
                FailMessage(r, std::string("GL_EYE_LINEAR"), drawMethods[testIteration], 
                            arraysCompiled[testIteration], retainedMode, eyeLinearResult);
                r.pass = false;
                glDeleteTextures(1, &checkerTextureHandle);
                return;
            }
        }
    }

	// success
	r.pass = true;
} // TexgenTest::runOne

///////////////////////////////////////////////////////////////////////////////
// logOne:  Log a single test case
///////////////////////////////////////////////////////////////////////////////
void
TexgenTest::logOne(BasicResult& r) {
	if (r.pass) {
		logPassFail(r);
		logConcise(r);
	}
} // TexgenTest::logOne

void
TexgenTest::renderSphere(int retainedMode, GeomRenderer& sphereRenderer)
{
    if (retainedMode)
    {
        GLint displayList;
        assert(sphereRenderer.generateDisplayList(GL_TRIANGLES, displayList));
        glCallList(displayList);
        glDeleteLists(displayList, 1);
    }
    else
    {
        assert(sphereRenderer.renderPrimitives(GL_TRIANGLES)); 
    }
} // TexgenTest::renderSphere
    
    
///////////////////////////////////////////////////////////////////////////////
// The test object itself:
///////////////////////////////////////////////////////////////////////////////
TexgenTest texgenTest("texgen", "window, rgb",

	"This test verifies that the three basic OpenGL texture coordinate\n"
	"modes: object_linear, eye_linear, and sphere_map, work for a simple\n"
	"case.\n");


} // namespace GLEAN





