/*
 * Copyright © 2014 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * With AMD OpenGL drivers, when we draw a point sprite and use
 * gl_PointCoord in the fragment (pixel) shader, the buggy host driver
 * will wrongly put gl_PointCoord value into a fragment shader input
 * variable, e.g., fs_color0, so that the rendering results are all wrong.
 * We will NOT see this issue if there is no vertex attribute for the
 * vertex position.
 *
 * Known to be
 *      -- Present in : ATI HD 6770M on Mac OS X 10.8.4
 *      -- Fixed in   : Mac OS 10.9
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define WIDTH 32
#define HEIGHT 32
#define LEVELS 6
#define COLOR_GRAY       0x7F7F7FFF
#define CLEAR_COLOR      0x000033FF
#define NUM_VERTICES     4
#define NUM_ATTRS        2
#define ATTR_SIZE        4

static GLuint prog;

static bool
test_pointsprite_ps(void)
{
        static const float vertArray[ATTR_SIZE * NUM_ATTRS] = {
                0.0f, 0.0f, 0.0f, 1.0f,
                1.0f, 1.0f, 1.0f, 1.0f,
        };
        const unsigned int numPixels = WIDTH * HEIGHT;
        GLuint texData[WIDTH * HEIGHT];
        GLuint i, texFbo, fbo, vertexArray, vertexBuf;
        GLint attrLoc;
        const float pointSize = WIDTH;
        const unsigned int  expectedTexelColor = 0xFFFFFFFF;

        for (i = 0; i < numPixels; ++i) {
	        texData[i] = COLOR_GRAY;
        }

        /* Create 2D textures */
        glGenTextures(1, &texFbo);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texFbo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0,
                     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        /* Setup the vertex attributes */
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
        glGenBuffers(1, &vertexBuf);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertArray), vertArray,
                     GL_STATIC_DRAW);

        for (i = 0; i < NUM_ATTRS; i++) {
                const GLvoid *offset = 
                        (const GLvoid *)(i * ATTR_SIZE * sizeof(float));
                GLchar name[8];

                snprintf(name, sizeof name, "Attr%d", i);
                attrLoc = glGetAttribLocation(prog, name);
                glEnableVertexAttribArray(attrLoc);
                glVertexAttribPointer(attrLoc, ATTR_SIZE, GL_FLOAT, GL_FALSE,
                                      ATTR_SIZE * sizeof(float), offset);
        }

        if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

        /* Setup the FBO */
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D, texFbo, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
                printf("incomplete framebuffer at line %d\n", __LINE__);
                return false;
        }

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
                printf("incomplete framebuffer at line %d\n", __LINE__);
                return false;
        }

        /* Clear and draw */
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(((CLEAR_COLOR >> 24) & 0xFF) / 255.0f,
                     ((CLEAR_COLOR >> 16) & 0xFF) / 255.0f,
                     ((CLEAR_COLOR >> 8)  & 0xFF) / 255.0f,
                     ((CLEAR_COLOR) & 0xFF) / 255.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(pointSize);
        glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);
        glDrawArrays(GL_POINTS, 0, 1);

        /* Read back */
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE) {
                printf("incomplete framebuffer at line %d\n", __LINE__);
                return false;
        }

	/* read color buffer */
        glPixelStorei(GL_PACK_ROW_LENGTH, WIDTH);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        memset(texData, 0, sizeof(texData));
        glReadPixels(0, 0, WIDTH, HEIGHT,
                     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, texData);

        if (texData[0] != expectedTexelColor) {
                printf("At pixel (0,0) expected 0x%x but found 0x%x\n",
                       expectedTexelColor, texData[0]);
                /* clean up */
                glDeleteTextures(1, &texFbo);
                glDeleteFramebuffers(1, &fbo);
                return false;
        }

        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        glDeleteTextures(1, &texFbo);
        glDeleteFramebuffers(1, &fbo);
        return true;
}


static void
setup_shaders(void)
{
        static const char *vsSrc =
                "#version 150\n"
                "in vec4 Attr0;"
                "in vec4 Attr1;"
                "smooth out vec4 fs_color0;"
                "void main(void) {"
                "   gl_Position = Attr0;"
                "   fs_color0 = Attr1;"
                "}";
        static const char *fsSrc =
                "#version 150\n"
                "smooth in vec4 fs_color0;"
                "out vec4 fragColor0;"
                "void main(void) {"
                "   vec2 psCoords = gl_PointCoord;"
                "   fragColor0 = fs_color0;"
                "}";

        prog = piglit_build_simple_program(vsSrc, fsSrc);
        glBindFragDataLocation(prog, 0, "fragColor0");
        glLinkProgram(prog);
        glUseProgram(prog);
}


enum piglit_result
piglit_display(void)
{
        bool pass = test_pointsprite_ps();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	setup_shaders();
}
