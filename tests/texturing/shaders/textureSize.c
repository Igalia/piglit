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
 * \file textureSize.c
 *
 * Tests the GLSL 1.30+ textureSize() built-in function.
 *
 * The test covers:
 * - All pipeline stages (VS, FS)
 * - Sampler data types (floating point, signed integer, unsigned integer)
 * - Sampler dimensionality (1D, 2D, 3D, Cube, 1DArray, 2DArray)
 * - Color and shadow samplers
 * - Mipmapped textures
 * - Non-power-of-two textures
 *
 * It doesn't cover texture format variations.  In fact, the test never
 * actually provides any content for the textures, because it should be
 * irrelevant for textureSize(), is easier to program, and also extra mean.
 *
 * The "textureSize" binary takes two arguments: shader stage and sampler type.
 *
 * For example:
 * ./bin/textureSize fs sampler1DArrayShadow
 * ./bin/textureSize vs usamplerCube
 */
#include "common.h"

int piglit_width = 150, piglit_height = 30;
int piglit_window_mode = GLUT_RGBA | GLUT_DOUBLE;

static int lod_location;

/**
 * Returns the number of components expected from textureSize().
 */
int
sampler_size()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
		return 1;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_RECTANGLE:
		return 2;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		return 3;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		assert(!"Not implemented yet.");
	default:
		assert(!"Should not get here.");
		return 0;
	}
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	int i, l;
	const int size = sampler_size();

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw consecutive squares for each mipmap level */
	for (l = 0; l < miplevels; l++) {
		const int x = 10 + l * 20;
		float expected_color[4] = {0, 0, 0, 0};
		expected_color[0] = 0.01 * level_size[l][0];
		if (sampler.target == GL_TEXTURE_1D_ARRAY) {
			expected_color[1] = 0.01 * level_size[l][2];
		} else {
			for (i = 1; i < size; i++)
				expected_color[i] = 0.01 * level_size[l][i];
		}
		expected_color[3] = 1.0;

		piglit_Uniform1i(lod_location, l);
		glViewport(x, 10, 10, 10);
		piglit_draw_rect(-1, -1, 2, 2);

		pass &= piglit_probe_rect_rgba(x, 10, 10, 10, expected_color);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

/**
 * Set the size of the texture's base level.
 */
void
set_base_size()
{
	if (sampler.target == GL_TEXTURE_CUBE_MAP) {
		/* Cube face width/height must be the same size. */
		base_size[0] = base_size[1] = 65;
		base_size[2] = 1;
	} else {
		base_size[0] = 65;
		base_size[1] = has_height() ? 32 : 1;
		base_size[2] = has_slices() ? 40 : 1;
	}
}

void
generate_texture()
{
	int l, i;
	GLuint tex;
	const GLenum target = sampler.target;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	for (l = 0; l < miplevels; l++) {
		if (target == GL_TEXTURE_CUBE_MAP) {
			for (i = 0; i < 6; i++) {
				GLenum f = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
				upload_miplevel_data(f, l, NULL);
			}
		} else {
			upload_miplevel_data(sampler.target, l, NULL);
		}
	}
}


int
generate_GLSL(enum shader_target test_stage)
{
	int vs, fs;

	static char *vs_code;
	static char *fs_code;

	static const char *zeroes[3] = { "", "0, ", "0, 0, " };

	const int size = sampler_size();

	switch (test_stage) {
	case VS:
		asprintf(&vs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "uniform int lod;\n"
			 "uniform %s tex;\n"
			 "flat out ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    size = textureSize(tex, lod);\n"
			 "    gl_Position = gl_Vertex;\n"
			 "}\n",
			 sampler.name, size);
		asprintf(&fs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "#define vec1 float\n"
			 "flat in ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_FragColor = vec4(0.01 * size,%s 1);\n"
			 "}\n",
			 size, zeroes[3 - size]);
		break;
	case FS:
		asprintf(&vs_code,
			 "#version 130\n"
			 "void main()\n"
			 "{\n"
			 "    gl_Position = gl_Vertex;\n"
			 "}\n");
		asprintf(&fs_code,
			 "#version 130\n"
			 "#define ivec1 int\n"
			 "uniform int lod;\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    ivec%d size = textureSize(tex, lod);\n"
			 "    gl_FragColor = vec4(0.01 * size,%s 1);\n"
			 "}\n",
			 sampler.name, size, zeroes[3 - size]);
		break;
	default:
		assert(!"Should not get here.");
		break;
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
	return piglit_link_simple_program(vs, fs);
}

void
fail_and_show_usage()
{
	printf("Usage: textureSize <vs|fs> <sampler type> [piglit args...]\n");
	piglit_report_result(PIGLIT_SKIP);
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

		fail_and_show_usage();
	}

	if (test_stage == UNKNOWN || !sampler_found)
		fail_and_show_usage();

	/* Not implemented yet */
	assert(sampler.target != GL_TEXTURE_CUBE_MAP_ARRAY &&
	       sampler.target != GL_TEXTURE_RECTANGLE);

	require_GL_features(test_stage);

	prog = generate_GLSL(test_stage);

	tex_location = piglit_GetUniformLocation(prog, "tex");
	lod_location = piglit_GetUniformLocation(prog, "lod");
	piglit_UseProgram(prog);
	piglit_Uniform1i(tex_location, 0);

	/* Create textures and set miplevel info */
	set_base_size();
	compute_miplevel_info();
	generate_texture();
}
