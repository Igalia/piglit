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
#define TEX_HALF (TEX_SIZE / 2)
#define TEX_LEVELS 6

#define DRAW_SIZE 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_width = 680;
	config.window_height = 620;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *fs_cube =
   "uniform samplerCube tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = textureCube(tex, gl_TexCoord[0].xyz); \n"
   "} \n";

static const char *fs_cube_array =
   "#version 130\n"
   "#extension GL_ARB_texture_cube_map_array : enable\n"
   "uniform samplerCubeArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture(tex, gl_TexCoord[0]); \n"
   "} \n";

static GLboolean test_array;
static GLenum target;
static GLenum num_layers;
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
};
#define NUM_COLORS ARRAY_SIZE(colors)

static void
load_texcube(void)
{
	float *p = malloc(TEX_SIZE * TEX_SIZE * num_layers * 4 * sizeof(float));
	int x,y,z;

	for (z = 0; z < num_layers; z++) {
		for (y = 0; y < TEX_SIZE; y++) {
			for (x = 0; x < TEX_SIZE; x++) {
				int quadrant = y < TEX_SIZE/2 ? (x < TEX_SIZE/2 ? 0 : 1) :
								(x < TEX_SIZE/2 ? 2 : 3);
				float *dest = &p[(z*TEX_SIZE*TEX_SIZE + y*TEX_SIZE + x)*4];

				memcpy(dest, colors[(z + quadrant*2) % NUM_COLORS],
				       4 * sizeof(float));
			}
		}
	}

	if (test_array) {
		glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0, 0,
				TEX_SIZE, TEX_SIZE, num_layers, GL_RGBA, GL_FLOAT, p);
	}
	else {
		int i;

		for (i = 0; i < 6; i++) {
			glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0,
					TEX_SIZE, TEX_SIZE, GL_RGBA, GL_FLOAT,
					p + i*TEX_SIZE*TEX_SIZE*4);
		}
	}
	free(p);
}

static GLuint
create_texcube(void)
{
	GLuint tex, fb;
	GLenum status;
	int layer;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	if (test_array)
		glTexStorage3D(target, TEX_LEVELS, format,
			       TEX_SIZE, TEX_SIZE, num_layers);
	else
		glTexStorage2D(target, TEX_LEVELS, format,
			       TEX_SIZE, TEX_SIZE);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	for (layer = 0; layer < num_layers; layer++) {
		if (test_array)
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						  tex, 0, layer);
		else
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					       GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
					       tex, 0);

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			load_texcube();
			goto done;
		}

		glViewport(0, 0, TEX_SIZE, TEX_SIZE);
		piglit_ortho_projection(TEX_SIZE, TEX_SIZE, GL_FALSE);

		glColor4fv(colors[(layer + 0) % NUM_COLORS]);
		piglit_draw_rect(0, 0, TEX_HALF, TEX_HALF);

		glColor4fv(colors[(layer + 2) % NUM_COLORS]);
		piglit_draw_rect(TEX_HALF, 0, TEX_HALF, TEX_HALF);

		glColor4fv(colors[(layer + 4) % NUM_COLORS]);
		piglit_draw_rect(0, TEX_HALF, TEX_HALF, TEX_HALF);

		glColor4fv(colors[(layer + 6) % NUM_COLORS]);
		piglit_draw_rect(TEX_HALF, TEX_HALF, TEX_HALF, TEX_HALF);
	}

done:
	glGenerateMipmap(target);
	glDeleteFramebuffers(1, &fb);
	return tex;
}

#define FACE(x) (GL_TEXTURE_CUBE_MAP_##x - GL_TEXTURE_CUBE_MAP_POSITIVE_X)

static void
piglit_draw_rect_face(float x, float y, float w, float h, int face, int cube_layer)
{
	float verts[4][4];
	float tex[4][4];
	float sign = face % 2 ? -1 : 1;

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	switch (face) {
	case FACE(POSITIVE_X):
	case FACE(NEGATIVE_X):
		tex[0][0] = sign;
		tex[1][0] = sign;
		tex[2][0] = sign;
		tex[3][0] = sign;

		tex[3][1] = -1 * sign;
		tex[3][2] = 1;

		tex[0][1] = -1 * sign;
		tex[0][2] = -1;

		tex[1][1] = 1 * sign;
		tex[1][2] = -1;

		tex[2][1] = 1 * sign;
		tex[2][2] = 1;
		break;

	case FACE(POSITIVE_Y):
	case FACE(NEGATIVE_Y):
		tex[0][1] = sign;
		tex[1][1] = sign;
		tex[2][1] = sign;
		tex[3][1] = sign;

		tex[0][0] = 1 * sign;
		tex[0][2] = -1;

		tex[1][0] = -1 * sign;
		tex[1][2] = -1;

		tex[2][0] = -1 * sign;
		tex[2][2] = 1;

		tex[3][0] = 1 * sign;
		tex[3][2] = 1;
		break;

	case FACE(POSITIVE_Z):
	case FACE(NEGATIVE_Z):
		tex[0][2] = sign;
		tex[1][2] = sign;
		tex[2][2] = sign;
		tex[3][2] = sign;

		tex[0][0] = 1;
		tex[0][1] = 1 * sign;

		tex[1][0] = -1;
		tex[1][1] = 1 * sign;

		tex[2][0] = -1;
		tex[2][1] = -1 * sign;

		tex[3][0] = 1;
		tex[3][1] = -1 * sign;
		break;

	default:
		assert(0);
	}

	tex[0][3] = cube_layer;
	tex[1][3] = cube_layer;
	tex[2][3] = cube_layer;
	tex[3][3] = cube_layer;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(4, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void
draw_face(int face, int cube_layer, int x, int y)
{
	int loc;

	glUseProgram(prog);
	loc = glGetUniformLocation(prog, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	piglit_draw_rect_face(x, y, DRAW_SIZE, DRAW_SIZE, face, cube_layer);
	glUseProgram(0);
}

static void
draw_cube(int x, int y, int level, int cube_layer)
{
	glTexParameteri(target, GL_TEXTURE_MIN_LOD, level);
	glTexParameteri(target, GL_TEXTURE_MAX_LOD, level);

	draw_face(FACE(POSITIVE_X), cube_layer, x,		y+DRAW_SIZE);
	draw_face(FACE(POSITIVE_Y), cube_layer, x+DRAW_SIZE,	y+DRAW_SIZE);
	draw_face(FACE(NEGATIVE_X), cube_layer, x+DRAW_SIZE*2,	y+DRAW_SIZE);
	draw_face(FACE(NEGATIVE_Y), cube_layer, x+DRAW_SIZE*3,	y+DRAW_SIZE);
	draw_face(FACE(POSITIVE_Z), cube_layer, x+DRAW_SIZE,	y+DRAW_SIZE*2);
	draw_face(FACE(NEGATIVE_Z), cube_layer, x+DRAW_SIZE,	y);
}

static void
draw_mipmap_tree(int x, int y)
{
	int i,j;

	for (i = 0; i < TEX_LEVELS; i++) {
		int cubes = num_layers / 6;

		for (j = 0; j < cubes; j++) {
			draw_cube(x + j*(DRAW_SIZE * 4 + 5),
				  y + i*(DRAW_SIZE * 3 + 5), i, j);
		}
	}
}

static bool
test_face(int face, int level, int cube, float *observed)
{
	int size = TEX_SIZE >> level;
	int layer = cube*6 + face;
	int x,y,i,c;
	bool pass = true;
	float *e;

	e = malloc(size * size * 4 * sizeof(float));

	if (size == 1) {
		memset(e, 0, 4 * sizeof(float));

		for (i = 0; i < 4; i++)
			for (c = 0; c < 4; c++)
				e[c] += colors[(layer + i*2) % NUM_COLORS][c] * 0.25;
	}
	else {
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				int quadrant = y < size/2 ? (x < size/2 ? 0 : 1) :
							    (x < size/2 ? 2 : 3);
				float *color = colors[(layer + quadrant*2) % NUM_COLORS];

				memcpy(&e[(y*size+x)*4], color, sizeof(float) * 4);
			}
		}
	}


	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			float *probe = &observed[(y*size+x)*4];
			float *expected = &e[(y*size+x)*4];

			for (i = 0; i < 4; ++i) {
				if (fabs(probe[i] - expected[i]) >= piglit_tolerance[i]) {
					printf("Probe color at (%i,%i)\n", x, y);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0], probe[1], probe[2], probe[3]);
					printf("  when testing face %i, level %i, cube %i\n",
					       face, level, cube);
					pass = false;
					goto done;
				}
			}
		}
	}

done:
	free(e);
	return pass;
}

static bool
test_mipmap_tree(void)
{
	bool pass = true;
	int layer, level;

	for (level = 0; level < TEX_LEVELS; level++) {
		int size = TEX_SIZE >> level;
		float *observed;

		/* With a compressed texture, skip checking the second and
		 * third last levels, because one DXTC block cannot contain
		 * more than 2 colors.
		 *
		 * However, always test the last level, which should only
		 * contain one color, which is the average of all 4.
		 */
		if (format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT &&
		    level >= TEX_LEVELS-3 && level <= TEX_LEVELS-2)
			continue;

		if (test_array) {
			observed = malloc(num_layers * size * size * 4 * sizeof(float));
			glGetTexImage(target, level, GL_RGBA, GL_FLOAT, observed);

			for (layer = 0; layer < num_layers; layer++) {
				pass = pass && test_face(layer % 6, level, layer / 6,
							 observed + layer * size * size * 4);
			}

			free(observed);
		}
		else {
			for (layer = 0; layer < num_layers; layer++) {
				observed = malloc(size * size * 4 * sizeof(float));
				glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
					      level, GL_RGBA, GL_FLOAT, observed);

				pass = pass && test_face(layer, level, 0, observed);

				free(observed);
			}
		}
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex;

	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_texcube();

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
	target = GL_TEXTURE_CUBE_MAP;
	num_layers = 6;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "array") == 0) {
			piglit_require_GLSL_version(130);
			piglit_require_extension("GL_ARB_texture_cube_map_array");
			test_array = GL_TRUE;
			target = GL_TEXTURE_CUBE_MAP_ARRAY;
			num_layers = 6 * 5;
		}
		else if (strcmp(argv[i], "RGB9_E5") == 0) {
			/* Test a non-renderable format. */
			piglit_require_extension("GL_EXT_texture_shared_exponent");
			format = GL_RGB9_E5;
		}
		else if (strcmp(argv[i], "S3TC_DXT1") == 0) {
			/* Test a compressed format. */
			piglit_require_extension("GL_EXT_texture_compression_s3tc");
			format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			piglit_set_tolerance_for_bits(5, 6, 5, 8);
		}
		else {
			assert(0);
		}
	}

	prog = piglit_build_simple_program(NULL, test_array ? fs_cube_array :
							      fs_cube);

	glClearColor(0.25, 0.25, 0.25, 0.25);
}
