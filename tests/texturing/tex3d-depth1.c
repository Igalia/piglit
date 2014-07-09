/*
 * Copyright (c) 2012 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS AND/OR THEIR
 * SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * Tests 3D texture with depth=1 (to make sure it's not errantly treated
 * as a 2D texture.
 *
 * Brian Paul
 * 28 Aug 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END


#define TEX_SIZE 8

static const float green[4]  = {0.0, 1.0, 0.0, 1.0};


enum piglit_result
piglit_display(void)
{
	/* texcoords with R=-1.0 to sample the border color */
	static const GLfloat texcoords[4][3] =
		{ { 0.0, 0.0, -1.0 },
		  { 1.0, 0.0, -1.0 },
		  { 1.0, 1.0, -1.0 },
		  { 0.0, 1.0, -1.0 } };
	GLfloat verts[4][2];
	bool pass;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);

	verts[0][0] = 0.0;  verts[0][1] = 0.0;
	verts[1][0] = piglit_width;  verts[1][1] = 0.0;
	verts[2][0] = piglit_width;  verts[2][1] = piglit_height;
	verts[3][0] = 0.0;  verts[3][1] = piglit_height;

	glVertexPointer(2, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, texcoords);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	/* Should have drawn solid green since we're sampling the
	 * texture border color for all fragments drawn.  If red is
	 * seen, it's probably because the 3D texture (with depth=1)
	 * is being treated as a 2D texture.
	 *
	 * BTW, another way the difference between 2D/3D textures
	 * could be detected would be with R-coordinate derivatives
	 * and LOD selection.
	 */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width - 1, piglit_height - 1,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLubyte pixels[TEX_SIZE][TEX_SIZE][4];
	GLuint tex;
	int i, j;

	piglit_require_gl_version(13);

	/* solid red texture */
	for (i = 0; i < TEX_SIZE; i++) {
		for (j = 0; j < TEX_SIZE; j++) {
			pixels[i][j][0] = 255;
			pixels[i][j][1] = 0;
			pixels[i][j][2] = 0;
			pixels[i][j][3] = 255;
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, green);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 1,
		     0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	glEnable(GL_TEXTURE_3D);
}
