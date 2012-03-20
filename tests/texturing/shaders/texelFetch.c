/*
 * Copyright © 2011 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file texelFetch.c
 *
 * Tests the GLSL 1.30+ texelFetch() built-in function.
 *
 * The "texelFetch" binary takes two arguments: shader stage and sampler type.
 *
 * For example:
 * ./bin/texelFetch fs sampler1DArray
 * ./bin/texelFetch usampler3D vs
 *
 * The test covers:
 * - All pipeline stages (VS, FS)
 * - Integer and floating point texture formats
 * - Sampler dimensionality (1D, 2D, 3D, 1DArray, 2DArray)
 * - Mipmapping
 * - Non-power-of-two textures
 *
 * Draws a series of "rectangles" which display each miplevel and array slice,
 * at full size.  They are layed out as follows:
 *
 * miplevel 3 +          +          +          +          +
 *
 * miplevel 2 +-+        +-+        +-+        +-+        +-+
 *            +-+        +-+        +-+        +-+        +-+
 *
 * miplevel 1 +---+      +---+      +---+      +---+      +---+
 *            |   |      |   |      |   |      |   |      |   |
 *            +---+      +---+      +---+      +---+      +---+
 *
 *            +------+   +------+   +------+   +------+   +------+
 * miplevel 0 |      |   |      |   |      |   |      |   |      |
 *            |      |   |      |   |      |   |      |   |      |
 *            +------+   +------+   +------+   +------+   +------+
 *            slice #0   slice #1   slice #2   slice #3   slice #4
 *
 * Normally, we could draw each rectangle as a single quad (or two triangles),
 * interpolate the texture coordinates across the primitive, and let the
 * fragment shader look up the color values from the texture.
 *
 * However, this fails miserably for vertex shader texturing: a quad only has
 * four vertices, which means we could only fetch/display at most 4 texels.
 * If we used a simple RGBW checkerboard, as in other Piglit tests, this would
 * only tell us that we sampled somewhere in the right 1/4 of the texture.
 *
 * Instead, we take a clever approach: draw each "rectangle" via a series of
 * 1-pixel wide GL_POINT primitives.  This gives us one vertex per pixel; by
 * drawing the texture at full size, each pixel and vertex also correspond to
 * exactly one texel.  So every texel is sampled and verified for correctness.
 *
 * In other words: "One pixel, one texel, one vertex."
 *
 * For convenience, we take the same approach for fragment shader testing
 * as well, since it allows us to reuse the same VBO setup and drawing code.
 */
#include "common.h"

int piglit_width = 355, piglit_height = 250;
int piglit_window_mode = GLUT_RGBA | GLUT_DOUBLE;

/** Vertex shader attribute locations */
const int pos_loc = 0;
const int texcoord_loc = 1;

/** Uniform locations */
int divisor_loc;

/**
 * Expected color data for each rectangle drawn, indexed by miplevel and slice.
 * For example, expected_colors[1][3] contains the data for miplevel 1 slice 3.
 */
float ***expected_colors;

/**
 * Return the divisors necessary to scale the unnormalized texture data to
 * a floating point color value in the range [0, 1].
 */
static void
compute_divisors(int lod, float *divisors)
{
	divisors[0] = max2(level_size[lod][0] - 1, 1);
	divisors[1] = max2(level_size[lod][1] - 1, 1);
	divisors[2] = max2(level_size[lod][2] - 1, 1);
	divisors[3] = 1.0;

	if (sampler.data_type != GL_UNSIGNED_INT)
		divisors[0] = -divisors[0];
}

enum piglit_result
piglit_display()
{
	int i, l, z;
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, false);

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(1.0);

	i = 0;
	for (l = 0; l < miplevels; ++l) {
		for (z = 0; z < level_size[l][2]; z++) {
			/* Draw the "rectangle" for this miplevel/slice. */
			int points = level_size[l][0] * level_size[l][1];
			float divisors[4];

			compute_divisors(l, divisors);
			swizzle(divisors);
			glUniform4fv(divisor_loc, 1, divisors);

			glDrawArrays(GL_POINTS, i, points);

			i += points;

			/* Compare results against reference image. */
			pass &= piglit_probe_image_rgba(5+(5+base_size[0]) * z,
							5+(5+base_size[1]) * l,
							level_size[l][0],
							level_size[l][1],
							expected_colors[l][z]);
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

/**
 * Generate two VBOs for our vertex attributes:
 * 1. Pixel position (in window coordinates).
 * 2. Texture coordinates.
 *
 * The VBOs contain the data for every rectangle being drawn (as opposed to
 * creating and binding a separate VBO per miplevel/slice.)
 */
void
generate_VBOs()
{
	int x, y, z, l;
	GLuint pos_vbo, tc_vbo;
	float *pos, *pos_data;
	int *tc, *tc_data;
	bool array_1D = sampler.target == GL_TEXTURE_1D_ARRAY;
	int num_texels = 0;

	/* Calculate the # of texels a.k.a. size of the VBOs */
	for (l = 0; l < miplevels; l++) {
		num_texels += level_size[l][0] * level_size[l][1] * level_size[l][2];
	}

	pos = pos_data = malloc(num_texels * 4 * sizeof(float));
	tc = tc_data = malloc(num_texels * 4 * sizeof(int));

	for (l = 0; l < miplevels; l++) {
		for (z = 0; z < level_size[l][2]; z++) {
			for (y = 0; y < level_size[l][1]; y++) {
				for (x = 0; x < level_size[l][0]; x++) {
					/* Assign pixel positions: */
					pos[0] = 5.5 + (5 + base_size[0])*z + x;
					pos[1] = 5.5 + (5 + base_size[1])*l + y;
					pos[2] = 0.0;
					pos[3] = 1.0;
					pos += 4;

					/* Assign texture coordinates:
					 * 1D:      x _ _ l
					 * 2D:      x y _ l
					 * 3D:      x y z l
					 * 1DArray: x z _ l
					 * 2DArray: x y z l
					 */
					tc[0] = x;
					tc[1] = array_1D ? z : y;
					tc[2] = z;
					tc[3] = l;
					tc += 4;
				}
			}
		}
	}

	/* Create VBO for pixel positions in screen-space: */
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     num_texels * 4 * sizeof(float),
		     pos_data, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_loc, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(pos_loc);

	/* Create VBO for texture coordinates: */
	glGenBuffers(1, &tc_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tc_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     num_texels * 4 * sizeof(int),
		     tc_data, GL_STATIC_DRAW);
	glVertexAttribIPointer(texcoord_loc, 4, GL_INT, 0, 0);
	glEnableVertexAttribArray(texcoord_loc);
}

/**
 * Create texel data.
 */
void
generate_texture()
{
	int l, x, y, z;
	unsigned *u_level, *u_ptr;
	int      *i_level, *i_ptr;
	float    *f_level, *f_ptr;
	float    *expected_ptr;
	void *level_image;
	float divisors[4];
	GLuint tex;
	GLenum target = sampler.target;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (swizzling)
		glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA,
				 (GLint *) sampler.swizzle);

	expected_colors = calloc(miplevels, sizeof(float **));

	for (l = 0; l < miplevels; l++) {
		expected_colors[l] = calloc(level_size[l][2], sizeof(float **));

		i_ptr = i_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(int));

		u_ptr = u_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(unsigned));

		f_ptr = f_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(float));

		for (z = 0; z < level_size[l][2]; z++) {
			expected_ptr = expected_colors[l][z] =
				malloc(level_size[l][0] * level_size[l][1] *
				       level_size[l][2] * 4 * sizeof(float));
			for (y = 0; y < level_size[l][1]; y++) {
				for (x = 0; x < level_size[l][0]; x++) {
					int nx = sampler.data_type == GL_UNSIGNED_INT ?  x : -x;
					f_ptr[0] = nx;
					f_ptr[1] = y;
					f_ptr[2] = z;
					f_ptr[3] = 1.0;

					i_ptr[0] = nx;
					i_ptr[1] = y;
					i_ptr[2] = z;
					i_ptr[3] = 1;

					u_ptr[0] = nx;
					u_ptr[1] = y;
					u_ptr[2] = z;
					u_ptr[3] = 1;

					compute_divisors(l, divisors);

					expected_ptr[0] = f_ptr[0]/divisors[0];
					expected_ptr[1] = f_ptr[1]/divisors[1];
					expected_ptr[2] = f_ptr[2]/divisors[2];
					expected_ptr[3] = 1.0;
					swizzle(expected_ptr);

					f_ptr += 4;
					i_ptr += 4;
					u_ptr += 4;
					expected_ptr += 4;
				}
			}
		}

		switch (sampler.data_type) {
		case GL_FLOAT:
			level_image = f_level;
			break;
		case GL_UNSIGNED_INT:
			level_image = u_level;
			break;
		case GL_INT:
			level_image = i_level;
			break;
		}

		upload_miplevel_data(target, l, level_image);
	}
}

/**
 * How many components are in the coordinate?
 */
static int
coordinate_size()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
		return 1;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_RECTANGLE:
		return 2;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		return 3;
	default:
		assert(!"Should not get here.");
		return 0;
	}
}

/**
 * Generate, compile, and link the GLSL shaders.
 */
int
generate_GLSL(enum shader_target test_stage)
{
	int vs, fs, prog;

	static char *vs_code;
	static char *fs_code;

	switch (test_stage) {
	case VS:
		asprintf(&vs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "flat out %s color;\n"
			 "in vec4 pos;\n"
			 "in ivec4 texcoord;\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    color = texelFetch(tex, ivec%d(texcoord),\n"
			 "                       texcoord.w);\n"
			 "    gl_Position = gl_ModelViewProjectionMatrix*pos;\n"
			 "}\n",
			 sampler.return_type, sampler.name, coordinate_size());
		asprintf(&fs_code,
			 "#version 130\n"
			 "flat in %s color;\n"
			 "uniform vec4 divisor;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_FragColor = vec4(color)/divisor;\n"
			 "}\n",
			 sampler.return_type);
		break;
	case FS:
		asprintf(&vs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "in vec4 pos;\n"
			 "in ivec4 texcoord;\n"
			 "flat out ivec4 tc;\n"
			 "void main()\n"
			 "{\n"
			 "    tc = texcoord;\n"
			 "    gl_Position = gl_ModelViewProjectionMatrix*pos;\n"
			 "}\n");
		asprintf(&fs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "flat in ivec4 tc;\n"
			 "uniform vec4 divisor;\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    vec4 color = texelFetch(tex, ivec%d(tc), tc.w);\n"
			 "    gl_FragColor = color/divisor;\n"
			 "}\n",
			 sampler.name, coordinate_size());
		break;
	default:
		assert(!"Should not get here.");
		break;
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	piglit_AttachShader(prog, fs);

	glBindAttribLocation(prog, pos_loc, "pos");
	glBindAttribLocation(prog, texcoord_loc, "texcoord");

	piglit_LinkProgram(prog);
	piglit_link_check_status(prog);

	return prog;
}

/**
 * Set the size of the texture's base level.
 */
void
set_base_size()
{
	base_size[0] = 65;
	base_size[1] = has_height() ? 32 : 1;
	base_size[2] = has_slices() ?  5 : 1;
}

/**
 * Is this sampler supported by texelFetch?
 */
static bool
supported_sampler()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_3D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	/* case GL_TEXTURE_RECTANGLE: not implemented yet */
		return true;
	}
	return false;
}

void
fail_and_show_usage()
{
	printf("Usage: texelFetch <vs|fs> <sampler type> [piglit args...]\n");
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int prog;
	int tex_location;
	int i;
	enum shader_target test_stage = UNKNOWN;
	bool sampler_found = false;

	for (i = 1; i < argc; i++) {
		if (test_stage == UNKNOWN) {
			/* Maybe it's the shader stage? */
			if (strcmp(argv[i], "vs") == 0) {
				test_stage = VS;
				continue;
			} else if (strcmp(argv[i], "fs") == 0) {
				test_stage = FS;
				continue;
			}
		}

		/* Maybe it's the sampler type? */
		if (!sampler_found && (sampler_found = select_sampler(argv[i])))
			continue;

		if (!swizzling && (swizzling = parse_swizzle(argv[i])))
			continue;

		fail_and_show_usage();
	}

	if (test_stage == UNKNOWN || !sampler_found)
		fail_and_show_usage();

	if (!supported_sampler()) {
		printf("%s unsupported\n", sampler.name);
		piglit_report_result(PIGLIT_FAIL);
	}

	require_GL_features(test_stage);

	prog = generate_GLSL(test_stage);

	tex_location = piglit_GetUniformLocation(prog, "tex");
	divisor_loc = piglit_GetUniformLocation(prog, "divisor");

	piglit_UseProgram(prog);

	piglit_Uniform1i(tex_location, 0);

	/* Create textures and set miplevel info */
	set_base_size();
	compute_miplevel_info();
	generate_texture();

	generate_VBOs();
}
