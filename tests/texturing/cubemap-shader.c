/*
 * Copyright © 2008-2009 Intel Corporation
 * Copyright © 2012 Red Hat Inc.
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
 *    Dave Airlie
 *    Eric Anholt <eric@anholt.net>
 *
 */

/*
 * test cubemap with shaders enabled - accept lod (with glsl 1.30) and
 * bias parameters to test different sampling modes.
 * This test is for a bug on r600g where for cubemaps the explicit lod
 * and lod bias weren't being routed correctly to the texture instruction
 * in the fragment shader.
 */

#include "piglit-util-gl.h"

#define PAD		5

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (64*6+PAD*9)*2;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

int max_size;

#define STATE_PLAIN_SHADER 0
#define STATE_LOD_SHADER 1
#define STATE_LOD_BIAS_SHADER 2

int test_state;

static GLfloat colors[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{0.0, 1.0, 0.0},
};

static const char *frag_shader = 
 "uniform samplerCube tex; \n"
 "void main()\n"
 "{\n"
 " gl_FragColor = textureCube(tex, gl_TexCoord[0].xyz);\n"
  "}\n";

static const char *frag_shader_lod_bias =
 "uniform samplerCube tex; \n"
 "void main()\n"
 "{\n"
 " gl_FragColor = textureCube(tex, gl_TexCoord[0].xyz, 3.0);\n"
  "}\n";

static const char *frag_shader_lod =
 "#version 130\n"
 "uniform samplerCube tex; \n"
 "void main()\n"
 "{\n"
 " gl_FragColor = textureLod(tex, gl_TexCoord[0].xyz, 3.0);\n"
  "}\n";



static GLuint frag_shader_cube;
static GLuint program_cube;

#if defined(_MSC_VER)
/**
 * Find the first bit set in i and return the index set of that bit.
 */
static int
ffs(int i)
{
	int bit;

	if (i == 0) {
		return 0;
	}

	for (bit = 1; !(i & 1); bit++) {
		i = i >> 1;
	}

	return bit;
}
#endif

static void
set_face_image(int level, GLenum face, int size, int color)
{
	GLfloat *color1 = colors[color];
	GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	GLfloat *tex;
	int x, y;

	tex = malloc(size * size * 3 * sizeof(GLfloat));

	/* Set the texture for this face to one corner being color2 and the
	 * rest color1.  If the texture is 1x1, then it's all color1.
	 */
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			GLfloat *chosen_color;

			if (y >= (size / 2) || x >= (size / 2))
				chosen_color = color1;
			else
				chosen_color = color2;

			tex[(y * size + x) * 3 + 0] = chosen_color[0];
			tex[(y * size + x) * 3 + 1] = chosen_color[1];
			tex[(y * size + x) * 3 + 2] = chosen_color[2];
		}
	}

	glTexImage2D(face, level, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, tex);

	free(tex);
}

/**
 * Tests that the mipmap drawn at (x,y)-(x+size,y+size) has the majority color,
 * with color+1 in bottom left.
 */
static GLboolean
test_results(int x, int y, int size, int level, int face, GLboolean mipmapped,
	     int color, int maxlevel)
{
	GLfloat *color1 = colors[color];
	GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	GLboolean pass = GL_TRUE;
	int x1 = x + size / 4, x2 = x + size * 3 / 4;
	int y1 = y + size / 4, y2 = y + size * 3 / 4;

	if (test_state != STATE_PLAIN_SHADER)
		if (level >= maxlevel)
			color2 = color1;

	if (size == 1) {
		pass = pass && piglit_probe_pixel_rgb(x1, y1, color1);
	} else {
		pass = pass && piglit_probe_pixel_rgb(x1, y1, color2);
		pass = pass && piglit_probe_pixel_rgb(x2, y1, color1);
		pass = pass && piglit_probe_pixel_rgb(x2, y2, color1);
		pass = pass && piglit_probe_pixel_rgb(x1, y2, color1);
	}

	if (!pass) {
		int base_size = size * (1 << level);
		printf("Cube map failed at size %dx%d, level %d (%dx%d), face %s%s\n",
		       base_size, base_size, level, size, size,
		       cube_face_names[face],
		       mipmapped ? ", mipmapped" : "");
	}

	return pass;
}

static GLboolean
draw_at_size(int size, int x_offset, int y_offset, GLboolean mipmapped)
{
	GLfloat row_y = PAD + y_offset;
	int dim, face;
	int color = 0, level = 0, maxlevel, baselevel = 3;
	GLuint texname;
	GLboolean pass = GL_TRUE;
	GLint loc;
	
	glUseProgram(program_cube);
	loc = glGetUniformLocation(program_cube, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	/* Create the texture. */
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texname);

	/* For each face drawing, we want to only see that face's contents at
	 * that mipmap level.
	 */
	if (mipmapped) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in faces on each level */
	for (dim = size; dim > 0; dim /= 2) {
		if (test_state != STATE_PLAIN_SHADER)
			color = (level) % ARRAY_SIZE(colors);
		for (face = 0; face < 6; face++) {
			set_face_image(level, cube_face_targets[face],
				       dim, color);
			if (test_state == STATE_PLAIN_SHADER)
				color = (color + 1) % ARRAY_SIZE(colors);
		}
		if (!mipmapped)
			break;

		level++;
	}
	maxlevel = level;
	if (maxlevel >= ARRAY_SIZE(colors))
		maxlevel = ARRAY_SIZE(colors) - 1;

	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	color = 0;
	level = 0;
	if (test_state == STATE_LOD_BIAS_SHADER)
		level = baselevel;
	for (dim = size; dim > 0; dim /= 2) {
		GLfloat row_x = PAD + x_offset;

		if (test_state == STATE_LOD_SHADER)
			level = baselevel;

		for (face = 0; face < 6; face++) {
			GLfloat base_x = row_x + face * (max_size + PAD);
			GLfloat base_y = row_y;
			
			if (test_state != STATE_PLAIN_SHADER) {
				color = level;
				if (color >= ARRAY_SIZE(colors))
					color = ARRAY_SIZE(colors) - 1;
			}
			glBegin(GL_QUADS);

			glTexCoord3fv(cube_face_texcoords[face][0]);
			glVertex2f(base_x, base_y);

			glTexCoord3fv(cube_face_texcoords[face][1]);
			glVertex2f(base_x + dim, base_y);

			glTexCoord3fv(cube_face_texcoords[face][2]);
			glVertex2f(base_x + dim, base_y + dim);

			glTexCoord3fv(cube_face_texcoords[face][3]);
			glVertex2f(base_x, base_y + dim);

			glEnd();

			if (dim > 2) {
				pass = test_results(base_x, base_y,
						    dim, level, face,
						    mipmapped,
						    color, maxlevel) && pass;
			}

			if (test_state == STATE_PLAIN_SHADER)
				color = (color + 1) % ARRAY_SIZE(colors);
		}

		if (!mipmapped)
			break;

		row_y += dim + PAD;
		level++;
		if (test_state != STATE_PLAIN_SHADER)
			if (level > maxlevel)
				level = maxlevel;
	}

	glUseProgram(0);
	glDeleteTextures(1, &texname);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int dim;
	GLboolean pass = GL_TRUE;
	int i = 0, y_offset = 0;
	int row_dim = 0;
	int xc = 0;
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	if (test_state == STATE_PLAIN_SHADER) {
		/* First, do each size from MAX_SIZExMAX_SIZE to 1x1 as a
		 * single texture level.
		 */
		y_offset = 0;
		for (dim = max_size; dim > 0; dim /= 2) {
			pass = draw_at_size(dim, 0, y_offset, GL_FALSE) && pass;
			y_offset += dim + PAD;
		}
		xc = 1;
	}

	/* Next, do each size with mipmaps from MAX_SIZExMAX_SIZE
	 * to 1x1.
	 */
	y_offset = 0;
	for (dim = max_size; dim > max_size / 2; dim /= 2) {
		int x_offset = (i % 2 == xc) ? 0 : piglit_width / 2;

		row_dim = (row_dim < dim) ? dim : row_dim;

		pass &= draw_at_size(dim, x_offset, y_offset, GL_TRUE);
		if (i % 2 == 0) {
			y_offset += row_dim * 2 + (ffs(dim) + 3) * PAD;
			row_dim = 0;
		}
		i++;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_extension("GL_ARB_texture_cube_map");

	max_size = 64;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "npot") == 0) {
			piglit_require_extension("GL_ARB_texture_non_power_of_two");
			max_size = 50;
			break;
		}
		if (strcmp(argv[i], "lod") == 0) {
			piglit_require_GLSL_version(130);
			test_state = STATE_LOD_SHADER;
			break;
		}
		if (strcmp(argv[i], "bias") == 0) {
			test_state = STATE_LOD_BIAS_SHADER;
			break;
		}
	}

	switch (test_state) {
	default:
	case STATE_PLAIN_SHADER:
		frag_shader_cube = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader);
		break;
	case STATE_LOD_SHADER:
		frag_shader_cube = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader_lod);
		break;
	case STATE_LOD_BIAS_SHADER:
		frag_shader_cube = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader_lod_bias);
		break;
	}
	piglit_check_gl_error(GL_NO_ERROR);
	program_cube = piglit_link_simple_program(0, frag_shader_cube);
	piglit_check_gl_error(GL_NO_ERROR);
}
