/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
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
 *
 * Authors:
 *     Marek Olšák <maraeo@gmail.com>
 */

#include "piglit-util-gl.h"

#define TEX_SIZE 32
#define TEX_LEVELS 6

#define DRAW_SIZE 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_width = 600;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *fs_3d =
   "uniform sampler3D tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture3D(tex, gl_TexCoord[0].xyz); \n"
   "} \n";

static GLuint prog;
static GLenum format;

static float colors[][4] = {
	{0.0, 0.0, 0.0, 1.0},
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{1.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
	{0.0, 1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
};

static void
gen_level(int level, float *p)
{
	int size = TEX_SIZE >> level;
	int x,y,z,i,c;

	if (size == 1) {
		memset(p, 0, 4 * sizeof(float));

		/* Average all colors. */
		for (i = 0; i < 8; i++)
			for (c = 0; c < 4; c++)
				p[c] += colors[i][c] * 0.125;
		return;
	}

	for (z = 0; z < size; z++) {
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				int octant = (z >= size/2)*4 +
					     (y >= size/2)*2 +
					     (x >= size/2);
				float *dest = &p[(z*size*size + y*size + x)*4];

				memcpy(dest, colors[octant], 4 * sizeof(float));
			}
		}
	}
}

static GLuint
create_tex3d(void)
{
	GLuint tex;
	float *p;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage3D(GL_TEXTURE_3D, TEX_LEVELS, format,
		       TEX_SIZE, TEX_SIZE, TEX_SIZE);

	p = malloc(TEX_SIZE * TEX_SIZE * TEX_SIZE * 4 * sizeof(float));
	gen_level(0, p);

	glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, TEX_SIZE, TEX_SIZE, TEX_SIZE,
			GL_RGBA, GL_FLOAT, p);
	free(p);

	glGenerateMipmap(GL_TEXTURE_3D);
	return tex;
}

static void
piglit_draw_rect_tex3(float x, float y, float w, float h,
		      float tx, float ty, float tw, float th, float td)
{
	float verts[4][4];
	float tex[4][3];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	tex[0][2] = td;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	tex[1][2] = td;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	tex[2][2] = td;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;
	tex[3][2] = td;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void
draw_slice(int x, int y, float z)
{
	int loc;

	glUseProgram(prog);
	loc = glGetUniformLocation(prog, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	piglit_draw_rect_tex3(x, y, DRAW_SIZE, DRAW_SIZE, 0, 0, 1, 1, z);
	glUseProgram(0);
}

static void
draw_mipmap_tree(int x, int y)
{
	int z, level;

	for (level = 0; level < TEX_LEVELS; level++) {
		int size = TEX_SIZE >> level;

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_LOD, level);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LOD, level);

		for (z = 0; z < size; z++) {
			if (level == 0) {
				draw_slice(x + (z % (TEX_SIZE/2)) * (DRAW_SIZE + 5),
					   y + (z / (TEX_SIZE/2)) * (DRAW_SIZE + 5),
					   z / (float)size);
			}
			else {
				draw_slice(x + z * (DRAW_SIZE + 5),
					   y + (level + 1) * (DRAW_SIZE + 10),
					   z / (float)size);
			}
		}
	}
}

static bool
test_box(int level)
{
	int size = TEX_SIZE >> level;
	int z,x,y,i;
	bool pass = true;
	float *e, *observed;

	observed = malloc(size * size * size * 4 * sizeof(float));
	glGetTexImage(GL_TEXTURE_3D, level, GL_RGBA, GL_FLOAT, observed);

	e = malloc(size * size * size * 4 * sizeof(float));
	gen_level(level, e);

	for (z = 0; z < size; z++) {
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				float *probe = &observed[(y*size+x)*4];
				float *expected = &e[(y*size+x)*4];

				for (i = 0; i < 4; ++i) {
					if (fabs(probe[i] - expected[i]) >= piglit_tolerance[i]) {
						printf("Probe color at (%i,%i,%i)\n", x, y, z);
						printf("  Expected: %f %f %f %f\n",
						       expected[0], expected[1], expected[2], expected[3]);
						printf("  Observed: %f %f %f %f\n",
						       probe[0], probe[1], probe[2], probe[3]);
						printf("  when testing level %i\n", level);
						pass = false;
						goto done;
					}
				}
			}
		}
	}

done:
	free(e);
	free(observed);
	return pass;
}

static bool
test_mipmap_tree(void)
{
	bool pass = true;
	int level;

	for (level = 0; level < TEX_LEVELS; level++) {
		pass = pass && test_box(level);
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;

	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_tex3d();

	draw_mipmap_tree(5, 5);

	pass = test_mipmap_tree();

	glDeleteTextures(1, &tex);
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_GLSL_version(120);

	format = GL_RGBA8;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "RGB9_E5") == 0) {
			/* Test a non-renderable format. */
			piglit_require_extension("GL_EXT_texture_shared_exponent");
			format = GL_RGB9_E5;
		}
		else {
			assert(0);
		}
	}

	prog = piglit_build_simple_program(NULL, fs_3d);

	glClearColor(0.25, 0.25, 0.25, 0.25);
}
