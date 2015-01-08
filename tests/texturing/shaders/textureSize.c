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
 * - All pipeline stages (VS, GS, FS)
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

void
parse_args(int argc, char **argv);
static enum shader_target test_stage = UNKNOWN;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 150;
	config.window_height = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	piglit_gl_process_args(&argc, argv, &config);

	parse_args(argc, argv);
	if (test_stage == GS) {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	} else {
		config.supports_gl_compat_version = 10;
		config.supports_gl_core_version = 31;
	}

PIGLIT_GL_TEST_CONFIG_END

static int lod_location;
static int vertex_location;

static char *extension = "";

/**
 * Returns the number of components expected from textureSize().
 */
int
sampler_size()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_BUFFER:
		return 1;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_2D_MULTISAMPLE:
		return 2;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		return 3;
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
	static const float verts[] = {
		-1, -1,
		-1,  1,
		 1,  1,
		 1, -1,
	};
	GLuint vbo;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* For GL core, we need to have a vertex array object bound.
	 * Otherwise, we don't particularly have to.  Always use a
	 * vertex buffer object, though.
	 */
	if (piglit_get_gl_version() >= 31) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STREAM_DRAW);

	glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vertex_location);

	/* Draw consecutive squares for each mipmap level */
	for (l = 0; l < miplevels; l++) {
		const int x = 10 + l * 20;
		float expected_color[4] = {0, 0, 0, 0};
		expected_color[0] = 0.01 * level_size[l][0];
		if (sampler.target == GL_TEXTURE_1D_ARRAY) {
			expected_color[1] = 0.01 * level_size[l][2];
		} else {
			for (i = 1; i < size; i++) {
				expected_color[i] = 0.01 * level_size[l][i];
				/* the ARB_texture_cube_map_array spec specifies we get number of layer cubes back not faces * layers */
				if (i == 2 && sampler.target == GL_TEXTURE_CUBE_MAP_ARRAY)
					expected_color[i] /= 6;
			}
		}
		expected_color[3] = 1.0;

		glUniform1i(lod_location, l);
		glViewport(x, 10, 10, 10);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

		pass &= piglit_probe_rect_rgba(x, 10, 10, 10, expected_color);
	}

	glDisableVertexAttribArray(vertex_location);
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
	} else if (sampler.target == GL_TEXTURE_CUBE_MAP_ARRAY) {
		base_size[0] = base_size[1] = 65;
		base_size[2] = 6;
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
	if (target == GL_TEXTURE_BUFFER ||
		target == GL_TEXTURE_2D_MULTISAMPLE ||
		target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		/* Texture buffers, multisample textures and multisample
		 * texture arrays only use texelFetch() and textureSize(),
		 * so setting the filter parameters on them is invalid.
		 */
	} else if (target == GL_TEXTURE_RECTANGLE) {
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	} else {
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (target != GL_TEXTURE_BUFFER &&
		target != GL_TEXTURE_2D_MULTISAMPLE &&
		target != GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

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

bool
has_lod(void)
{
	switch (sampler.target) {
		case GL_TEXTURE_RECTANGLE:
		case GL_TEXTURE_BUFFER:
		case GL_TEXTURE_2D_MULTISAMPLE:
		case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
			return false;
		default:
			return true;
	}
}

int
generate_GLSL(enum shader_target test_stage)
{
	int vs, gs = 0, fs;
	int prog;

	static char *vs_code;
	static char *gs_code = NULL;
	static char *fs_code;
	char *lod_arg;
	static const char *zeroes[3] = { "", "0, ", "0, 0, " };

	const int size = sampler_size();

	/* The GLSL 1.40 sampler2DRect/samplerBuffer samplers don't
	 * take a lod argument. Neither do ARB_texture_multisample's
	 * sampler2DMS/sampler2DMSArray samplers.
	 */
	lod_arg = has_lod() ? ", lod" : "";

	switch (test_stage) {
	case VS:
		asprintf(&vs_code,
			 "#version %d\n%s"
			 "#define ivec1 int\n"
			 "uniform int lod;\n"
			 "uniform %s tex;\n"
			 "in vec4 vertex;\n"
			 "flat out ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    size = textureSize(tex%s);\n"
			 "    gl_Position = vertex;\n"
			 "}\n",
			 shader_version, extension, sampler.name, size, lod_arg);
		asprintf(&fs_code,
			 "#version %d\n"
			 "#define ivec1 int\n"
			 "#define vec1 float\n"
			 "flat in ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_FragColor = vec4(0.01 * size,%s 1);\n"
			 "}\n",
			 shader_version, size, zeroes[3 - size]);
		break;
	case GS:
		asprintf(&vs_code,
			 "#version %d\n"
			 "in vec4 vertex;\n"
			 "out vec4 pos_to_gs;\n"
			 "void main()\n"
			 "{\n"
			 "    pos_to_gs = vertex;\n"
			 "}\n",
			 shader_version);
		asprintf(&gs_code,
			 "#version %d\n"
			 "%s\n"
			 "#define ivec1 int\n"
			 "layout(triangles) in;\n"
			 "layout(triangle_strip, max_vertices = 3) out;\n"
			 "uniform int lod;\n"
			 "uniform %s tex;\n"
			 "in vec4 pos_to_gs[3];\n"
			 "flat out ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    for (int i = 0; i < 3; i++) {\n"
			 "        size = textureSize(tex%s);\n"
			 "        gl_Position = pos_to_gs[i];\n"
			 "        EmitVertex();\n"
			 "    }\n"
			 "}\n",
			 shader_version, extension, sampler.name, size,
			 lod_arg);
		asprintf(&fs_code,
			 "#version %d\n"
			 "#define ivec1 int\n"
			 "#define vec1 float\n"
			 "flat in ivec%d size;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_FragColor = vec4(0.01 * size,%s 1);\n"
			 "}\n",
			 shader_version, size, zeroes[3 - size]);
		break;
	case FS:
		asprintf(&vs_code,
			 "#version %d\n"
			 "in vec4 vertex;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_Position = vertex;\n"
			 "}\n",
			 shader_version);
		asprintf(&fs_code,
			 "#version %d\n%s"
			 "#define ivec1 int\n"
			 "uniform int lod;\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    ivec%d size = textureSize(tex%s);\n"
			 "    gl_FragColor = vec4(0.01 * size,%s 1);\n"
			 "}\n",
			 shader_version, extension, sampler.name, size, lod_arg,
			 zeroes[3 - size]);
		break;
	default:
		assert(!"Should not get here.");
		break;
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	if (gs_code) {
		gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_code);
	}
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);

	if (!vs || (gs_code && !gs) || !fs)
		return 0;

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	if (gs_code)
		glAttachShader(prog, gs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	return prog;
}

void
fail_and_show_usage()
{
	printf("Usage: textureSize [140] <vs|gs|fs> <sampler type> [piglit args...]\n");
	piglit_report_result(PIGLIT_SKIP);
}


void
parse_args(int argc, char **argv)
{
	int i;
	bool sampler_found = false;

	for (i = 1; i < argc; i++) {
		if (test_stage == UNKNOWN) {
			/* Maybe it's the shader stage? */
			if (strcmp(argv[i], "vs") == 0) {
				test_stage = VS;
				continue;
			} else if (strcmp(argv[i], "gs") == 0) {
				test_stage = GS;
				continue;
			} else if (strcmp(argv[i], "fs") == 0) {
				test_stage = FS;
				continue;
			}
		}

		if (strcmp(argv[i], "140") == 0) {
			shader_version = 140;
			continue;
		}

		/* Maybe it's the sampler type? */
		if (!sampler_found && (sampler_found = select_sampler(argv[i])))
			continue;

		fail_and_show_usage();
	}

	if (test_stage == UNKNOWN || !sampler_found)
		fail_and_show_usage();

	if (test_stage == GS && shader_version < 150)
		shader_version = 150;
}


void
piglit_init(int argc, char **argv)
{
	int prog;
	int tex_location;

	require_GL_features(test_stage);

	if (sampler.target == GL_TEXTURE_CUBE_MAP_ARRAY)
		extension = "#extension GL_ARB_texture_cube_map_array : enable\n";
	if (sampler.target == GL_TEXTURE_2D_MULTISAMPLE
		|| sampler.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
		extension = "#extension GL_ARB_texture_multisample : enable\n";

	prog = generate_GLSL(test_stage);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	tex_location = glGetUniformLocation(prog, "tex");
	lod_location = glGetUniformLocation(prog, "lod");
	vertex_location = glGetAttribLocation(prog, "vertex");
	glUseProgram(prog);
	glUniform1i(tex_location, 0);

	/* Create textures and set miplevel info */
	set_base_size();
	compute_miplevel_info();
	generate_texture();
}
