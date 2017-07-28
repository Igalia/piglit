// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyrigth (C) 2007  Intel Corporation
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
//
// Authors:
//  Shuang He <shuang.he@intel.com>
//
// tfbo.cpp:  Test OpenGL Extension GL_EXT_framebuffer_object


#define GL_GLEXT_PROTOTYPES

#include "tfbo.h"
#include <cassert>
#include <math.h>
#include <cstring>
#include "piglit-util-gl.h"

namespace GLEAN
{

static int useFramebuffer;

bool
FBOTest::setup(void)
{
        // setup vertex transform (we'll draw a quad in middle of window)
        glMatrixMode(GL_PROJECTION);

        glLoadIdentity();
        glOrtho(0, 100, 0, 100, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDrawBuffer(GL_FRONT);
        glReadBuffer(GL_FRONT);

        glDisable(GL_DITHER);

        // compute error tolerances (may need fine-tuning)
        int bufferBits[5];

        glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
        glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
        glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
        glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
        glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);

        tolerance[0] = 3.0 / (1 << bufferBits[0]);
        tolerance[1] = 3.0 / (1 << bufferBits[1]);
        tolerance[2] = 3.0 / (1 << bufferBits[2]);
        if (bufferBits[3])
                tolerance[3] = 3.0 / (1 << bufferBits[3]);
        else
                tolerance[3] = 1.0;
        if (bufferBits[4])
                tolerance[4] = 16.0 / (1 << bufferBits[4]);
        else
                tolerance[4] = 1.0;

        // Check if GL_EXT_framebuffer_object is supported
        if (GLUtils::haveExtension("GL_EXT_framebuffer_object")) {
                printf("GL_EXT_framebuffer_object is supported\n");
                useFramebuffer = 1;
        }
        else {
                printf("GL_EXT_framebuffer_object is not supported\n");
                useFramebuffer = 0;
 		return false;
        }

	haveARBfbo = GLUtils::haveExtension("GL_ARB_framebuffer_object");
	if (haveARBfbo)
		printf("GL_ARB_framebuffer_object is supported\n");
	else
		printf("GL_ARB_framebuffer_object is not supported\n");

        return true;
}


void
FBOTest::reportFailure(const char *msg, const int line) const
{
        env->log << "FAILURE: " << msg << " (at tfbo.cpp:" << line
                << ")\n";
}

void
FBOTest::reportFailure(const char *msg, const GLenum target, const int line) const
{
        env->log << "FAILURE: " << msg;
        if (target == GL_FRAGMENT_SHADER)
                env->log << " (fragment)";
        else
                env->log << " (vertex)";
        env->log << " (at tfbo.cpp:" << line << ")\n";
}

#define REPORT_FAILURE(MSG) reportFailure(MSG, __LINE__)
#define REPORT_FAILURE_T(MSG, TARGET) reportFailure(MSG, TARGET, __LINE__)
// Compare actual and expected colors 
bool
FBOTest::equalColors(const GLfloat act[3], const GLfloat exp[3]) const
{
        if ((fabsf(act[0] - exp[0]) > tolerance[0])
            || (fabsf(act[1] - exp[1]) > tolerance[1])
            || (fabsf(act[2] - exp[2]) > tolerance[2])) {
                return false;
        }
        else
                return true;
}


#define TEXSIZE 64

/*
|--------------------|
   |---depth---|
     |---stencil---|
*/
bool
FBOTest::checkResult(const GLfloat color[4], const int depth,
                     const int stencil) const
{
        GLfloat buf[TEXSIZE * TEXSIZE * 3];
        int i, j;
        const GLfloat black[4] = { 0.0, 0.0, 0.0, 0.0 };
        const GLfloat *exp;

        glReadPixels(0, 0, TEXSIZE, TEXSIZE, GL_RGB, GL_FLOAT, buf);

        for (j = 0; j < TEXSIZE; j++) {
                for (i = 0; i < TEXSIZE; i++) {
                        exp = color;

                        if (i * 4 >= TEXSIZE && i * 8 < TEXSIZE * 5
                            && depth)
                                exp = black;
                        if (i * 2 >= TEXSIZE && i * 8 < TEXSIZE * 7
                            && stencil)
                                exp = black;


                        if (!equalColors(buf + (j * TEXSIZE + i) * 3, exp)) {
                                printf("  depth = %d, stencil = %d\n",
                                       depth, stencil);
                                printf("  (%d, %d) = [%f, %f, %f], is expected to be[%f, %f, %f]\n", i, j, buf[(j * TEXSIZE + i) * 3], buf[(j * TEXSIZE + i) * 3 + 1], buf[(j * TEXSIZE + i) * 3 + 2], exp[0], exp[1], exp[2]);
                                return false;
                        }
                }
        }
        return true;
}


// Check FB status, print unexpected results to stdout.
static GLenum
CheckFramebufferStatus(const char *func, int line)
{
	GLenum status;
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE_EXT:
		/*printf("  (%s:%d)GL_FRAMEBUFFER_COMPLETE_EXT\n", func, line);*/
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_UNSUPPORTED_EXT\n", func, line);
		/* choose different formats */
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT\n", func, line);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT\n", func, line);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT\n", func, line);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT\n", func, line);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
		printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT\n", func, line);
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
	    printf("  (%s:%d)GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT\n", func, line);
	    break;
	default:
		/* programming error; will fail on all hardware */
		printf("  (%s:%d)programming error\n", func, line);
		break;
	}
	return status;
}


enum
{ BLACK, RED, GREEN, BLUE, WHITE };

GLfloat colors[][4] = {
        {0.0, 0.0, 0.0, 0.0},
        {1.0, 0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 1.0},
        {1.0, 1.0, 1.0, 1.0}
};



bool
FBOTest::testSanity(void)
{
        GLuint fbs[2];
        GLuint maxColorAttachment;
        GLuint fb_binding;

        if (!useFramebuffer)
                return true;

        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT,
                      (GLint *) & maxColorAttachment);
        if (maxColorAttachment < 1) {
                REPORT_FAILURE
                        ("Failed to get max color attachment points");
                return false;
        }


        glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) & fb_binding);
        if (fb_binding != 0) {
                printf("  fb_binding = %d\n", fb_binding);
                REPORT_FAILURE
                        ("The default framebuffer binding should be 0");
                return false;
        }

        glGenFramebuffersEXT(1, fbs);


        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, (GLint *) & fb_binding);
        if (fb_binding != fbs[0]) {
                printf("  fb_binding = %d\n", fb_binding);
                REPORT_FAILURE("Binding framebuffer failed");
                return false;
        }
	if (glIsFramebufferEXT(fbs[0]) != GL_TRUE)
	{
                REPORT_FAILURE("Call glIsFramebufferEXT failed");
                return false;
	}

        glDeleteFramebuffersEXT(1, fbs);

        GLint maxRenderbufferSize;

        glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE_EXT, &maxRenderbufferSize);
        if (maxRenderbufferSize < 1) {
                printf("  maxRenderbufferSize = %d\n",
                       maxRenderbufferSize);
                REPORT_FAILURE("Get max Renderbuffer Size failed");
                return false;
        }

        return true;
}


void
FBOTest::reset(void)
{                                        
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
}

GLenum textureModes[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP};

bool
FBOTest::testRender2SingleTexture(void)
{
        GLint depthBuffer = 0;
        GLint stencilBuffer = 0;
        GLuint fbs[1];
        GLuint depth_rb[1];
        GLuint stencil_rb[1];
        GLuint textures[1];
        int mode;
	int maxzoffset = -1;
	GLenum status;

	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxzoffset);
	if (maxzoffset > 16)
		maxzoffset = 16;

        for (depthBuffer = 0; depthBuffer < 2; depthBuffer++) {
                for (stencilBuffer = 0; stencilBuffer < 2; stencilBuffer++) {
                        for (mode = 0; mode < 4; mode++) {

				//
				// Setup state to test
				//
				if (mode == 2 && maxzoffset <= 0)
					continue;

                                if (useFramebuffer)
                                        glGenFramebuffersEXT(1, fbs);
                                glGenTextures(1, textures);

                                glBindTexture(textureModes[mode],
                                              textures[0]);
                                glTexParameteri(textureModes[mode],
                                                GL_TEXTURE_MIN_FILTER,
                                                GL_NEAREST);
                                glTexParameteri(textureModes[mode],
                                                GL_TEXTURE_MAG_FILTER,
                                                GL_NEAREST);

                                switch (textureModes[mode]) {
                                case GL_TEXTURE_1D:
                                        glTexImage1D(GL_TEXTURE_1D,
                                                     0, GL_RGB,
                                                     TEXSIZE, 0,
                                                     GL_RGB, GL_INT, NULL);
                                        break;
                                case GL_TEXTURE_2D:
                                        glTexImage2D(GL_TEXTURE_2D,
                                                     0, GL_RGB,
                                                     TEXSIZE,
                                                     TEXSIZE, 0,
                                                     GL_RGB, GL_INT, NULL);
                                        break;
                                case GL_TEXTURE_3D:
                                        glTexImage3D(GL_TEXTURE_3D,
                                                     0, GL_RGB,
                                                     TEXSIZE,
                                                     TEXSIZE,
                                                     maxzoffset, 0,
                                                     GL_RGB, GL_INT, NULL);
                                        break;
                                case GL_TEXTURE_CUBE_MAP:
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        glTexImage2D
                                                (GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                                                 0, GL_RGB,
                                                 TEXSIZE, TEXSIZE,
                                                 0, GL_RGB, GL_INT, NULL);
                                        break;
                                }


                                if (useFramebuffer) {
                                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                                        int height = TEXSIZE;

                                        if (textureModes[mode] == GL_TEXTURE_1D)
                                                height = 1;

                                        if (depthBuffer) {
                                                int params;

                                                glGenRenderbuffersEXT(1, depth_rb);

					
                                                glBindRenderbufferEXT
                                                        (GL_RENDERBUFFER_EXT,
                                                         depth_rb[0]);
						if (glIsRenderbufferEXT(depth_rb[0]) != GL_TRUE)
						{
                					REPORT_FAILURE("Call glIsRenderbufferEXT failed\n");
                					return false;
						}

                                                glRenderbufferStorageEXT
                                                        (GL_RENDERBUFFER_EXT,
                                                         GL_DEPTH_COMPONENT,
                                                         TEXSIZE, height);
                                                glFramebufferRenderbufferEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_DEPTH_ATTACHMENT_EXT,
                                                         GL_RENDERBUFFER_EXT,
                                                         depth_rb[0]);
                                                glGetRenderbufferParameterivEXT
                                                        (GL_RENDERBUFFER_EXT,
                                                         GL_RENDERBUFFER_WIDTH_EXT,
                                                         &params);
                                                if (params != TEXSIZE) {
                                                        REPORT_FAILURE("Get Renderbuffer width failed");
                                                        printf("glGetRenderbufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("width = %d\n", params);
                                                        return false;
                                                }
                                                glGetRenderbufferParameterivEXT
                                                        (GL_RENDERBUFFER_EXT,
                                                         GL_RENDERBUFFER_HEIGHT_EXT,
                                                         &params);
                                                if (params != height) {
                                                        REPORT_FAILURE("Get Renderbuffer height failed");
                                                        printf("glGetRenderbufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        return false;
                                                }
                                        }

                                        if (stencilBuffer) {
                                                int type;

                                                type = -1;
                                                glGenRenderbuffersEXT(1, stencil_rb);
                                                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, stencil_rb[0]);
                                                glRenderbufferStorageEXT
                                                        (GL_RENDERBUFFER_EXT,
                                                         GL_STENCIL_INDEX,
                                                         TEXSIZE, height);
                                                glFramebufferRenderbufferEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_STENCIL_ATTACHMENT_EXT,
                                                         GL_RENDERBUFFER_EXT,
                                                         stencil_rb[0]);
                                                glGetFramebufferAttachmentParameterivEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_STENCIL_ATTACHMENT_EXT,
                                                         GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT,
                                                         &type);
                                                if (type != GL_RENDERBUFFER_EXT) {
                                                        REPORT_FAILURE("Get Framebuffer attached object type failed");
                                                        printf("glGetFramebufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("type = %d\n", type);
                                                        return false;
                                                }
                                        }

                                        switch (textureModes[mode]) {
                                        case GL_TEXTURE_1D:
                                                int name;

                                                name = -1;
                                                glFramebufferTexture1DEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_TEXTURE_1D,
                                                         textures[0], 0);
                                                glGetFramebufferAttachmentParameterivEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT,
                                                         &name);
                                                if ((GLuint)name != textures[0]) {
                                                        REPORT_FAILURE("Get Framebuffer attached texture name failed");
                                                        printf("glGetFramebufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("name = %d\n", name);
                                                        return false;
                                                }

                                                break;
                                        case GL_TEXTURE_2D:
                                                int level;

                                                level = -1;
                                                glFramebufferTexture2DEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_TEXTURE_2D,
                                                         textures[0], 0);
                                                glGetFramebufferAttachmentParameterivEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT,
                                                         &level);
                                                if (level != 0) {
                                                        REPORT_FAILURE("Get Framebuffer attached texture level failed");
                                                        printf("glGetFramebufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("level = %d\n", level);
                                                        return false;
                                                }

                                                break;
                                        case GL_TEXTURE_3D:
                                                int zoffset;

                                                zoffset = -1;
                                                glFramebufferTexture3DEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_TEXTURE_3D,
                                                         textures[0], 
							 0,
                                                         maxzoffset-1);

                                                glGetFramebufferAttachmentParameterivEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT,
                                                         &zoffset);

                                                if (zoffset != maxzoffset-1) {
                                                        REPORT_FAILURE("Get Framebuffer attached 3D texture z-offset failed");
                                                        printf("glGetFramebufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("zoffset = %d\n", zoffset);
                                                        return false;
                                                }
                                                break;
                                        case GL_TEXTURE_CUBE_MAP:
                                                int face = 0;

                                                glFramebufferTexture2DEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                                         textures[0], 0);
                                                glGetFramebufferAttachmentParameterivEXT
                                                        (GL_FRAMEBUFFER_EXT,
                                                         GL_COLOR_ATTACHMENT0_EXT,
                                                         GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT,
                                                         &face);
                                                if (face !=
                                                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z)
                                                {
                                                        REPORT_FAILURE("Get Framebuffer attached cube map face failed");
                                                        printf("glGetFramebufferParameterivEXT: %s\n", piglit_get_gl_error_name(glGetError()));
                                                        printf("face = %d\n", face);
                                                       return false;
                                                }

                                                break;
                                        }

                                        status = CheckFramebufferStatus("FBOTest::testRender2SingleTexture", __LINE__);
                                }
				else {
					status = GL_FRAMEBUFFER_COMPLETE_EXT;
				}


				if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
					glDeleteTextures(1, textures);
					if (useFramebuffer)
						glDeleteFramebuffersEXT(1, fbs);
					if (depthBuffer)
						glDeleteRenderbuffersEXT(1, depth_rb);
					if (stencilBuffer)
						glDeleteRenderbuffersEXT(1, stencil_rb);
					continue;
				}

				//
				// Render, test the results
				//

                                if (depthBuffer) {
                                        glClear(GL_DEPTH_BUFFER_BIT);
                                        // Init depth buffer
                                        glEnable(GL_DEPTH_TEST);
                                        glDepthFunc(GL_ALWAYS);
                                        switch (textureModes[mode]) {
                                        case GL_TEXTURE_1D:
                                        case GL_TEXTURE_2D:
                                        case GL_TEXTURE_3D:
                                        case GL_TEXTURE_CUBE_MAP:
                                                glBegin(GL_POLYGON);
                                                glVertex3f(TEXSIZE / 4, 0, 0.3);
                                                glVertex3f(TEXSIZE * 5 / 8, 0, 0.3);
                                                glVertex3f(TEXSIZE * 5 / 8, TEXSIZE, 0.3);
                                                glVertex3f(TEXSIZE / 4, TEXSIZE, 0.3);
                                                glEnd();
                                                break;
                                        default:
                                                break;
                                        }
                                        glDepthFunc(GL_LESS);
                                }

                                if (stencilBuffer) {
                                        glClear(GL_STENCIL_BUFFER_BIT);
                                        // Init stencil buffer
                                        glEnable(GL_STENCIL_TEST);
                                        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
                                        glStencilOp(GL_KEEP,
                                                    GL_KEEP, GL_REPLACE);
                                        switch (textureModes[mode]) {
                                        case GL_TEXTURE_1D:
                                        case GL_TEXTURE_2D:
                                        case GL_TEXTURE_3D:
                                        case GL_TEXTURE_CUBE_MAP:
                                                glBegin(GL_POLYGON);
                                                glVertex3f(TEXSIZE / 2, 0, 0.3);
                                                glVertex3f(TEXSIZE * 7 / 8, 0, 0.3);
                                                glVertex3f(TEXSIZE * 7 / 8, TEXSIZE, 0.3);
                                                glVertex3f(TEXSIZE / 2, TEXSIZE, 0.3);
                                                glEnd();
                                                break;
                                        default:
                                                break;
                                        }

                                        glStencilFunc(GL_NOTEQUAL, 0x1, 0x1);
                                }

                                // Render to the texture
                                glBindTexture(textureModes[mode], 0);
                                glDisable(textureModes[mode]);
                                glColor4fv(colors[RED]);
                                glClearColor(0.0, 0.0, 0.0, 0.0);
                                glClear(GL_COLOR_BUFFER_BIT);

                                switch (textureModes[mode]) {
                                case GL_TEXTURE_1D:
                                case GL_TEXTURE_2D:
                                case GL_TEXTURE_3D:
                                case GL_TEXTURE_CUBE_MAP:
                                        glBegin(GL_POLYGON);
                                        glVertex3f(0, 0, 0.2);
                                        glVertex3f(TEXSIZE, 0, 0.2);
                                        glVertex3f(TEXSIZE, TEXSIZE, 0.2);
                                        glVertex3f(0, TEXSIZE, 0.2);
                                        glEnd();
                                        break;
                                }

                                // Render to the window
                                glEnable(textureModes[mode]);
                                glBindTexture(textureModes[mode],
                                              textures[0]);
                                if (useFramebuffer) {
                                        glBindFramebufferEXT
                                                (GL_FRAMEBUFFER_EXT, 0);
                                        glBindTexture(textureModes
                                                      [mode], textures[0]);
                                }
                                else {
                                        switch (textureModes[mode]) {
                                        case GL_TEXTURE_1D:
                                                glCopyTexImage1D
                                                        (GL_TEXTURE_1D,
                                                         0, GL_RGB,
                                                         0, 0, TEXSIZE, 0);
                                                break;
                                        case GL_TEXTURE_2D:
                                                glCopyTexImage2D
                                                        (GL_TEXTURE_2D,
                                                         0, GL_RGB,
                                                         0, 0,
                                                         TEXSIZE,
                                                         TEXSIZE, 0);
                                                break;
                                        case GL_TEXTURE_3D:
                                                glCopyTexSubImage3D
                                                        (GL_TEXTURE_3D,
                                                         0, 0, 0,
                                                         0, 0, 0,
                                                         TEXSIZE, TEXSIZE);
                                                break;
                                        case GL_TEXTURE_CUBE_MAP:
                                                glCopyTexImage2D
                                                        (GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                                                         0, GL_RGB,
                                                         0, 0,
                                                         TEXSIZE,
                                                         TEXSIZE, 0);
                                        default:
                                                break;
                                        }
                                }
                                if (depthBuffer)
                                        glDisable(GL_DEPTH_TEST);
                                if (stencilBuffer)
                                        glDisable(GL_STENCIL_TEST);

                                glEnable(textureModes[mode]);
                                glColor4fv(colors[WHITE]);
                                glClearColor(0.0, 0.0, 0.0, 0.0);
                                glClear(GL_COLOR_BUFFER_BIT);

				glTexParameteri (textureModes[mode], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri (textureModes[mode], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri (textureModes[mode], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

                                if (textureModes[mode] !=
                                    GL_TEXTURE_CUBE_MAP) {
					GLfloat depth = 0.99+0.01;
                                        glBegin(GL_POLYGON);
                                        glTexCoord3f(0.0, 0.0, depth);
                                        glVertex2f(0, 0);
                                        glTexCoord3f(1.0, 0.0, depth);
                                        glVertex2f(TEXSIZE, 0);
                                        glTexCoord3f(1.0, 1.0, depth);
                                        glVertex2f(TEXSIZE, TEXSIZE);
                                        glTexCoord3f(0.0, 1.0, depth);
                                        glVertex2f(0, TEXSIZE);
                                        glEnd();
                                }
                                else {
                                        glBegin(GL_POLYGON);
                                        glTexCoord3f(-1.0, 1.0, 1.0);
                                        glVertex2f(0, 0);
                                        glTexCoord3f(1.0, 1.0, 1.0);
                                        glVertex2f(TEXSIZE, 0);
                                        glTexCoord3f(1.0, -1.0, 1.0);
                                        glVertex2f(TEXSIZE, TEXSIZE);
                                        glTexCoord3f(-1.0, -1.0, 1.0);
                                        glVertex2f(0, TEXSIZE);
                                        glEnd();
                                }

                                glDeleteTextures(1, textures);
                                if (useFramebuffer)
                                        glDeleteFramebuffersEXT(1, fbs);
                                if (depthBuffer)
                                        glDeleteRenderbuffersEXT(1, depth_rb);
                                if (stencilBuffer)
                                        glDeleteRenderbuffersEXT(1, stencil_rb);

//					getchar();
                                if (checkResult(colors[RED], depthBuffer, stencilBuffer) == false) {
                                        REPORT_FAILURE("Render to single texture failed");
                                        printf("  mode = %d\n", mode);
                                        return false;
                                }
                        }
                }
        }

        return true;
}


bool
FBOTest::testRender2MultiTexture(void)
{
        int i;
        GLuint fbs[8];
        GLuint textures[8];
        GLint maxColorAttachment = 8;


        enum { MULTI_FBO, SINGLE_COLOR_ATTACH, MULTI_COLOR_ATTACH };
        int numRender;
        int numFBO;
        int numColorAttach;
        int mode;

	reset();
        for (mode = MULTI_FBO; mode < MULTI_COLOR_ATTACH + 1; mode++) {
                if (useFramebuffer) {
                        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT,
                                      &maxColorAttachment);
                        if (maxColorAttachment < 1) {
                                REPORT_FAILURE("Failed to get max color attachment points");
                                return false;
                        }
                }

                numRender = maxColorAttachment;
                numColorAttach = maxColorAttachment;
                if (mode == MULTI_FBO)
                        numFBO = maxColorAttachment;
                else
                        numFBO = 1;

                if (useFramebuffer)
                        glGenFramebuffersEXT(numFBO, fbs);

                GLint maxTexUnits;

                glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTexUnits);
                glGenTextures(maxTexUnits, textures);


                for (i = 0; i < numColorAttach; i++) {
                        int idx;

                        if (i > maxTexUnits - 1)
                                idx = maxTexUnits - 1;
                        else
                                idx = i;

                        glActiveTexture(GL_TEXTURE0 + idx);
                        glBindTexture(GL_TEXTURE_2D, textures[idx]);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                                     TEXSIZE, TEXSIZE, 0, GL_RGB,
                                     GL_INT, NULL);

                        if (useFramebuffer) {
                                if (mode == MULTI_FBO)
                                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[i]);
                                else
                                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);

                                if (mode != SINGLE_COLOR_ATTACH)
                                        glFramebufferTexture2DEXT
                                                (GL_FRAMEBUFFER_EXT,
                                                 GL_COLOR_ATTACHMENT0_EXT + i,
                                                 GL_TEXTURE_2D,
                                                 textures[idx], 0);
                                else
                                        glFramebufferTexture2DEXT
                                                (GL_FRAMEBUFFER_EXT,
                                                 GL_COLOR_ATTACHMENT0_EXT,
                                                 GL_TEXTURE_2D,
                                                 textures[idx], 0);
                                if (mode != SINGLE_COLOR_ATTACH) {
                                        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
                                        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
                                }
                                else {
                                        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
                                        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
                                }
                                CheckFramebufferStatus("FBOTest::testRender2MultiTexture", __LINE__);
                        }
                }



                for (i = 0; i < numRender; i++) {
                        int idx;

                        if (i > maxTexUnits - 1)
                                idx = maxTexUnits - 1;
                        else
                                idx = i;


                        if (useFramebuffer) {
                                if (mode == MULTI_FBO)
                                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[i]);
                                else
                                        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);

                                if (mode == MULTI_COLOR_ATTACH) {
                                        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT + idx);
                                        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + idx);
                                }

                                CheckFramebufferStatus("FBOTest::testRender2MultiTexture", __LINE__);
                                if (mode == SINGLE_COLOR_ATTACH) {
                                        glFramebufferTexture2DEXT
                                                (GL_FRAMEBUFFER_EXT,
                                                 GL_COLOR_ATTACHMENT0_EXT,
                                                 GL_TEXTURE_2D,
                                                 textures[idx], 0);
                                }
                        }

                        glDisable(GL_TEXTURE_2D);

                        // Render to the texture 
                        glColor4fv(colors[RED + i % (WHITE - RED)]);

                        glClearColor(0.0, 0.0, 0.0, 0.0);
                        glClear(GL_COLOR_BUFFER_BIT);

                        glBegin(GL_POLYGON);
                        glVertex3f(0, 0, 1);
                        glVertex3f(TEXSIZE, 0, 1);
                        glVertex3f(TEXSIZE, TEXSIZE, 1);
                        glVertex3f(0, TEXSIZE, 1);
                        glEnd();


                        glEnable(GL_TEXTURE_2D);
                        if (useFramebuffer) {
                                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                                glBindTexture(GL_TEXTURE_2D, textures[idx]);
                        }
                        else {
                                glBindTexture(GL_TEXTURE_2D, textures[idx]);
                                glCopyTexImage2D(GL_TEXTURE_2D, 0,
                                                 GL_RGB, 0, 0,
                                                 TEXSIZE, TEXSIZE, 0);
                        }

                }
                // Clean up
                if (useFramebuffer)
                        glDeleteFramebuffersEXT(numFBO, fbs);


                // Render to the window
                for (i = 0; i < numRender; i++) {
                        int idx;

                        if (i > maxTexUnits - 1)
                                idx = maxTexUnits - 1;
                        else
                                idx = i;

                        glActiveTexture(GL_TEXTURE0 + idx);
                        glEnable(GL_TEXTURE_2D);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                        glTexParameteri(GL_TEXTURE_2D,
                                        GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                        glColor4fv(colors[WHITE]);
                        glClearColor(0.0, 0.0, 0.0, 0.0);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glBegin(GL_POLYGON);
                        glMultiTexCoord2f(GL_TEXTURE0 + idx, 0, 0);
                        glVertex3f(0, 0, 1);
                        glMultiTexCoord2f(GL_TEXTURE0 + idx, 1, 0);
                        glVertex3f(TEXSIZE, 0, 1);
                        glMultiTexCoord2f(GL_TEXTURE0 + idx, 1, 1);
                        glVertex3f(TEXSIZE, TEXSIZE, 1);
                        glMultiTexCoord2f(GL_TEXTURE0 + idx, 0, 1);
                        glVertex3f(0, TEXSIZE, 1);
                        glEnd();

                        //Check result
                        int exp = (i >= maxTexUnits - 1) ? maxColorAttachment - 1 : i;

                        if (checkResult(colors[RED + (exp % (WHITE - RED))], 0, 0) == false) {
                                glDeleteTextures(maxTexUnits, textures);

                                REPORT_FAILURE("Render to multi texture failed");
                                return false;
                        }

                        glDisable(GL_TEXTURE_2D);
                        glActiveTexture(GL_TEXTURE0);
                }

                glDeleteTextures(maxTexUnits, textures);
        }

        return true;
}


bool
FBOTest::testRender2depthTexture(void)
{
        GLuint fbs[2];
        GLuint textures[8];

	reset();
        if (useFramebuffer)
                glGenFramebuffersEXT(1, fbs);

        glGenTextures(1, textures);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEXSIZE,
                     TEXSIZE, 0, GL_DEPTH_COMPONENT, GL_INT, NULL);

        if (useFramebuffer) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_DEPTH_ATTACHMENT_EXT,
                                          GL_TEXTURE_2D, textures[0], 0);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);

                CheckFramebufferStatus("FBOTest::testRender2depthTexture", __LINE__);
        }
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_TEXTURE_2D);

        // Render to the texture 
        glColor4fv(colors[RED]);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_POLYGON);
        glVertex3f(TEXSIZE / 4, 0, 0.5);
        glVertex3f(TEXSIZE * 5 / 8, 0, 0.5);
        glVertex3f(TEXSIZE * 5 / 8, TEXSIZE, 0.5);
        glVertex3f(TEXSIZE / 4, TEXSIZE, 0.5);
        glEnd();

        if (useFramebuffer) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
        }
        else {
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glCopyTexImage2D(GL_TEXTURE_2D, 0,
                                 GL_DEPTH_COMPONENT, 0, 0, TEXSIZE,
                                 TEXSIZE, 0);
        }

        glClear(GL_DEPTH_BUFFER_BIT);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                        GL_COMPARE_R_TO_TEXTURE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glTexParameterf(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE,
                        GL_LUMINANCE);
        glEnable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);

        // Render to the window
        glColor4fv(colors[GREEN]);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBegin(GL_POLYGON);
        glTexCoord3f(0, 0, 0.75);
        glVertex2f(0, 0);
        glTexCoord3f(1, 0, 0.75);
        glVertex2f(TEXSIZE, 0);
        glTexCoord3f(1, 1, 0.75);
        glVertex2f(TEXSIZE, TEXSIZE);
        glTexCoord3f(0, 1, 0.75);
        glVertex2f(0, TEXSIZE);
        glEnd();
        glFlush();

        // Clean up
        if (useFramebuffer)
                glDeleteFramebuffersEXT(1, fbs);
        glDeleteTextures(1, textures);

        // Check result
        if (checkResult(colors[WHITE], 1, 0) == false) {
                REPORT_FAILURE("Render to depth texture failed");
                return false;
        }



        return true;
}


bool
FBOTest::testRender2MipmapTexture(void)
{
        int i;
        GLuint fbs[1];
        GLuint textures[1];

	reset();
        if (useFramebuffer)
                glGenFramebuffersEXT(1, fbs);

        glGenTextures(1, textures);
        glBindTexture(GL_TEXTURE_2D, textures[0]);

        if (useFramebuffer)
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);

        glDisable(GL_TEXTURE_2D);

        GLint level = 0;

        for (i = TEXSIZE; i > 0; i /= 2, level++) {
                if (useFramebuffer) {
                        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB,
                                     i, i, 0, GL_RGB, GL_INT, NULL);
                        glFramebufferTexture2DEXT
                                (GL_FRAMEBUFFER_EXT,
                                 GL_COLOR_ATTACHMENT0_EXT,
                                 GL_TEXTURE_2D, textures[0], level);
                        CheckFramebufferStatus("FBOTest::testRender2MipmapTexture", __LINE__);

                        glColor4fv(colors[RED + (level % (WHITE - RED))]);
                        glClearColor(0.0, 0.0, 0.0, 0.0);
                        glClear(GL_COLOR_BUFFER_BIT);

                        glBegin(GL_POLYGON);
                        glVertex3f(0, 0, 1);
                        glVertex3f(TEXSIZE, 0, 1);
                        glVertex3f(TEXSIZE, TEXSIZE, 1);
                        glVertex3f(0, TEXSIZE, 1);
                        glEnd();
                }
                else {
                        glColor4fv(colors[RED + (level % (WHITE - RED))]);
                        glClearColor(0.0, 0.0, 0.0, 0.0);
                        glClear(GL_COLOR_BUFFER_BIT);

                        glBegin(GL_POLYGON);
                        glVertex3f(0, 0, 1);
                        glVertex3f(TEXSIZE, 0, 1);
                        glVertex3f(TEXSIZE, TEXSIZE, 1);
                        glVertex3f(0, TEXSIZE, 1);
                        glEnd();


                        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB,
                                     i, i, 0, GL_RGB, GL_INT, NULL);
                        glCopyTexImage2D(GL_TEXTURE_2D, level,
                                         GL_RGB, 0, 0, i, i, 0);
                }
        }

        if (useFramebuffer) {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
        }
        glEnable(GL_TEXTURE_2D);

        // Render to the window
        glColor4fv(colors[GREEN]);
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_NEAREST);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        for (i = TEXSIZE; i > 0; i /= 2) {
                glBegin(GL_POLYGON);
                glTexCoord2f(0, 0);
                glVertex3f(windowSize / 2 - i / 2,
                           windowSize / 2 - i / 2, 1);
                glTexCoord2f(1, 0);
                glVertex3f(windowSize / 2 + i / 2,
                           windowSize / 2 - i / 2, 1);
                glTexCoord2f(1, 1);
                glVertex3f(windowSize / 2 + i / 2,
                           windowSize / 2 + i / 2, 1);
                glTexCoord2f(0, 1);
                glVertex3f(windowSize / 2 - i / 2,
                           windowSize / 2 + i / 2, 1);
                glEnd();
        }
        glFlush();

        // Clean up
        if (useFramebuffer)
                glDeleteFramebuffersEXT(1, fbs);
        glDeleteTextures(1, textures);

        // Check result
        level = 0;
        for (i = TEXSIZE; i > 1; i /= 2, level++) {
                GLfloat pixel[3];

                glReadPixels(windowSize / 2 - i / 2,
                             windowSize / 2 - i / 2, 1, 1, GL_RGB,
                             GL_FLOAT, pixel);
                if (!equalColors
                    (pixel, colors[RED + (level % (WHITE - RED))])) {
                        REPORT_FAILURE("Render to mipmap texture failed");
                        printf("  level = %d\n", level);
                        return false;
                }
        }


        return true;
}


bool
FBOTest::testErrorHandling(void)
{
        GLuint fbs[1];
        GLuint textures[2];
	GLuint renderbuffer;
        GLenum status;
	bool have_ARB_ES2 = GLUtils::haveExtension("GL_ARB_ES2_compatibility");

        if (useFramebuffer) {
                GLuint maxColorAttachment;

                glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, (GLint *) & maxColorAttachment);
                if (maxColorAttachment < 1) {
                        REPORT_FAILURE("Failed to get max color attachment points");
                        return false;
                }


                // At least one image attached to the framebuffer
                glGenFramebuffersEXT(1, fbs);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_NONE);
                status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glDeleteFramebuffersEXT(1, fbs);
                if (status !=
                    GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT) {
                        REPORT_FAILURE
                                ("If no image is attached to framebuffer, status should be GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
                        return false;
                }

                // All attached images have the same width and height,
		// unless GL_ARB_framebuffer object is supported.
                glGenFramebuffersEXT(1, fbs);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glGenTextures(2, textures);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE,
                             TEXSIZE, 0, GL_RGB, GL_INT, NULL);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_COLOR_ATTACHMENT0_EXT,
                                          GL_TEXTURE_2D, textures[0], 0);
                glBindTexture(GL_TEXTURE_2D, textures[1]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE / 2,
                             TEXSIZE / 2, 0, GL_RGB, GL_INT, NULL);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_COLOR_ATTACHMENT0_EXT
                                          + maxColorAttachment - 1,
                                          GL_TEXTURE_2D, textures[1], 0);
                status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glDeleteFramebuffersEXT(1, fbs);
                glDeleteTextures(2, textures);
                if (!haveARBfbo &&
		    status != GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT) {
                        REPORT_FAILURE
                                ("If renderbuffer sizes don't all match, status should be GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
                        return false;
                }

                // All images attached to the attachment points
                // COLOR_ATTACHMENT0_EXT through COLOR_ATTACHMENTn_EXT must
                // have the same internal format, unless ARB_fbo is supported.
                glGenFramebuffersEXT(1, fbs);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glGenTextures(2, textures);
                glBindTexture(GL_TEXTURE_2D, textures[0]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE,
                             TEXSIZE, 0, GL_RGB, GL_INT, NULL);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_COLOR_ATTACHMENT0_EXT,
                                          GL_TEXTURE_2D, textures[0], 0);
                glBindTexture(GL_TEXTURE_2D, textures[1]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXSIZE,
                             TEXSIZE, 0, GL_RGBA, GL_INT, NULL);
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                          GL_COLOR_ATTACHMENT0_EXT
                                          + maxColorAttachment - 1,
                                          GL_TEXTURE_2D, textures[1], 0);
                status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glDeleteFramebuffersEXT(1, fbs);
                glDeleteTextures(2, textures);
                if (!haveARBfbo &&
		    status != GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT) {
                        REPORT_FAILURE
                                ("All color renderbuffers must be of same format, status should be GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
                        return false;
                }


                // The value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not
                // be NONE for any color attachment point(s) named by
		// DRAW_BUFFERi.
		// [Note: to avoid being caught by the no-attachments
		// case above, we attach a depth renderbuffer.]
                glGenFramebuffersEXT(1, fbs);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT +
                             maxColorAttachment - 1);
		glGenRenderbuffers(1, &renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
				      TEXSIZE, TEXSIZE);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
					  GL_DEPTH_ATTACHMENT_EXT,
					  GL_RENDERBUFFER_EXT,
					  renderbuffer);
                status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glDeleteFramebuffersEXT(1, fbs);
		glDeleteTextures(1, textures);
		glDeleteRenderbuffers(1, &renderbuffer);
                if (status != GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT &&
		    !have_ARB_ES2) {
                        REPORT_FAILURE
                                ("All any buffer named by glDrawBuffers is missing, status should be GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
                        return false;
                }

                // If READ_BUFFER is not NONE, then the value of
                // FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT must not be NONE for
                // the color attachment point named by READ_BUFFER.
		// [Note: to avoid being caught by the no-attachments
		// case above, we attach a depth renderbuffer.]
                glGenFramebuffersEXT(1, fbs);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbs[0]);
                glDrawBuffer(GL_NONE);
                glReadBuffer(GL_COLOR_ATTACHMENT0_EXT +
                             maxColorAttachment - 1);
		glGenRenderbuffers(1, &renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER_EXT, renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24,
				      TEXSIZE, TEXSIZE);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT,
					  GL_DEPTH_ATTACHMENT_EXT,
					  GL_RENDERBUFFER_EXT,
					  renderbuffer);
                status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
                glDeleteFramebuffersEXT(1, fbs);
		glDeleteRenderbuffers(1, &renderbuffer);
                if (status != GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT &&
		    !have_ARB_ES2) {
                        REPORT_FAILURE
                                ("If buffer named by glReadBuffers is missing, status should be GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
                        return false;
                }
        }
        return true;
}

void
FBOTest::runOne(MultiTestResult & r, Window & w)
{
        (void) w;

        if (!setup()) {
                r.pass = false;
                return;
        }

        static SubTestFunc funcs[] = {
                &GLEAN::FBOTest::testSanity,
                &GLEAN::FBOTest::testRender2SingleTexture,
                &GLEAN::FBOTest::testRender2MultiTexture,
                &GLEAN::FBOTest::testRender2depthTexture,
                &GLEAN::FBOTest::testRender2MipmapTexture,
                &GLEAN::FBOTest::testErrorHandling,
                NULL
        };

        for (int i = 0; funcs[i]; i++)
                if ((this->*funcs[i]) ())
                        r.numPassed++;
                else
                        r.numFailed++;

        r.pass = (r.numFailed == 0);
}


// The test object itself:
FBOTest fboTest("fbo", "window, rgb, z", "", // no extension filter 
                "fbo test: Test OpenGL Extension GL_EXT_framebuffer_object\n");



}  // namespace GLEAN
