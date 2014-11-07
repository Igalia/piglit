/*
 * Copyright © 2012 Red Hat
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
 *
 */

/*
 * arb_texture_cube_map_array-cubemap
 *
 * This constructs a two layer mipmapped cube array, and tests
 * that sampling from each level of both layers produces the correct
 * results.
 */

#include "piglit-util-gl.h"

#define PAD 5

#define NUM_LAYERS 2

#define COORD_SCALE 0.5

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_width =  (64 * 6 + PAD * 9) * 2;
    config.window_height = 400*NUM_LAYERS;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

int max_size;

static const char *prog = "arb_texture_cube_map_array-cubemap";

static const GLfloat colors[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{0.0, 1.0, 0.0},
};

/* we need a larger size than the common code so just copy them in */
GLfloat a_cube_face_texcoords[6][4][4];

void setup_texcoords(void)
{
	int i, j, k;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 3; k++) {
				a_cube_face_texcoords[i][j][k] = COORD_SCALE * cube_face_texcoords[i][j][k];
			}
		}
	}
}

static const char *frag_shader = 
 "#extension GL_ARB_texture_cube_map_array : enable\n"
 "uniform samplerCubeArray tex; \n"
 "void main()\n"
 "{\n"
 " gl_FragColor = texture(tex, gl_TexCoord[0]);\n"
  "}\n";

static GLuint program_cube_array;

static void
set_image(int level, int size, int *color)
{
	const GLfloat *color1;
	const GLfloat *color2;
	GLfloat *tex;
	int x, y;
	int face;
	//	int color = 0;
	int face_size;
	face_size = size * size * 3;
	tex = malloc(NUM_LAYERS * 6 * face_size * sizeof(GLfloat)); 

	for (face = 0; face < NUM_LAYERS * 6; face++) {
		if (face % 6 == 0)
			*color = face / 6;
		color1 = colors[*color];
		color2 = colors[(*color + 1) % ARRAY_SIZE(colors)];
		/* Set the texture for this face to one corner being color2 and the
		 * rest color1.  If the texture is 1x1, then it's all color1.
		 */
		for (y = 0; y < size; y++) {
			for (x = 0; x < size; x++) {
				const GLfloat *chosen_color;

				if (y >= (size / 2) || x >= (size / 2))
					chosen_color = color1;
				else
					chosen_color = color2;

				tex[(face * face_size) + (y * size + x) * 3 + 0] = chosen_color[0];
				tex[(face * face_size) + (y * size + x) * 3 + 1] = chosen_color[1];
				tex[(face * face_size) + (y * size + x) * 3 + 2] = chosen_color[2];
			}
		}
		*color = (*color + 1) % ARRAY_SIZE(colors);
	}
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, level, GL_RGB, size, size, 6 * NUM_LAYERS, 0, GL_RGB, GL_FLOAT, tex);

	free(tex);
}

/**
 * Tests that the mipmap drawn at (x,y)-(x+size,y+size) has the majority color,
 * with color+1 in bottom left.
 */
static bool
test_results(int x, int y, int size, int level, int face, bool mipmapped,
	     int color, int layer)
{
	const GLfloat *color1 = colors[color];
	const GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	bool pass = true;
	int x1 = x + size / 4, x2 = x + size * 3 / 4;
	int y1 = y + size / 4, y2 = y + size * 3 / 4;

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
		printf("%s: Cube map failed at size %dx%d, level %d (%dx%d), face %s%s\n",
		       prog, base_size, base_size, level, size, size,
		       cube_face_names[face],
		       mipmapped ? ", mipmapped" : "");
	}

	return pass;
}

static bool
draw_at_size(int size, int x_offset, int y_offset,
	     bool mipmapped)
{
	GLfloat row_y = PAD + y_offset;
	int dim, face;
	int color = 0, level = 0;
	GLuint texname;
	bool pass = true;
	GLint loc;
	
	glUseProgram(program_cube_array);
	loc = glGetUniformLocation(program_cube_array, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	/* Create the texture. */
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, texname);

	/* For each face drawing, we want to only see that face's contents at
	 * that mipmap level.
	 */
	if (mipmapped) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
			GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in faces on each level */
	for (dim = size; dim > 0; dim /= 2) {
	        set_image(level, dim, &color);
		if (!mipmapped)
			break;

		level++;
	}

	color = 0;
	level = 0;
	for (dim = size; dim > 0; dim /= 2) {
		GLfloat row_x = PAD + x_offset;

		for (face = 0; face < 6 * NUM_LAYERS; face++) {
			GLint realface = face % 6;
			GLint layer = face / 6;
			GLfloat base_x = row_x + realface * (max_size + PAD);
			GLfloat base_y = row_y + (400 * layer);

			if (realface == 0)
			    color = layer;
			glBegin(GL_QUADS);

			a_cube_face_texcoords[realface][0][3] = layer;
			a_cube_face_texcoords[realface][1][3] = layer;
			a_cube_face_texcoords[realface][2][3] = layer;
			a_cube_face_texcoords[realface][3][3] = layer;

			glTexCoord4fv(a_cube_face_texcoords[realface][0]);
			glVertex2f(base_x, base_y);

			glTexCoord4fv(a_cube_face_texcoords[realface][1]);
			glVertex2f(base_x + dim, base_y);

			glTexCoord4fv(a_cube_face_texcoords[realface][2]);
			glVertex2f(base_x + dim, base_y + dim);

			glTexCoord4fv(a_cube_face_texcoords[realface][3]);
			glVertex2f(base_x, base_y + dim);

			glEnd();

			if (dim > 2) {
				pass = test_results(base_x, base_y,
						    dim, level, realface,
						    mipmapped,
						    color, layer) && pass;
			}

			color = (color + 1) % ARRAY_SIZE(colors);
		}

		if (!mipmapped)
			break;

		row_y += dim + PAD;
		level++;
	}

	glDeleteTextures(1, &texname);

	glUseProgram(0);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	int dim;
	bool pass = true;
	int i = 0, y_offset = 0;
	int row_dim = 0;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* First, do each size from MAX_SIZExMAX_SIZE to 1x1 as a
	 * single texture level.
	 */
	y_offset = 0;
	for (dim = max_size; dim > 0; dim /= 2) {
		pass = draw_at_size(dim, 0, y_offset, false) && pass;
		y_offset += dim + PAD;
	}

	/* Next, do each size with mipmaps from MAX_SIZExMAX_SIZE
	 * to 1x1.
	 */
	y_offset = 0;
	for (dim = max_size; dim > 0; dim /= 2) {
		int x_offset = (i % 2 == 1) ? 0 : piglit_width / 2;

		row_dim = (row_dim < dim) ? dim : row_dim;

		pass &= draw_at_size(dim, x_offset, y_offset, true);
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

	piglit_require_extension("GL_ARB_texture_cube_map_array");

	max_size = 64;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "npot") == 0) {
			piglit_require_extension("GL_ARB_texture_non_power_of_two");
			max_size = 50;
			break;
		}
	}

	program_cube_array =
		piglit_build_simple_program(NULL, frag_shader);

	setup_texcoords();
}
