/*
 * Copyright © 2015 Ilia Mirkin
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
 *
 * Heavily based on textureSize.c
 */

/**
 * \file textureSamples.c
 *
 * Tests the ARB_shader_texture_image_samples textureSamples() built-in.
 *
 * The test covers:
 * - All pipeline stages (VS, GS, FS)
 * - Sampler data types (floating point, signed integer, unsigned integer)
 * - Sampler dimensionality (2DMS, 2DMSArray)
 *
 * The "textureSamples" binary takes three arguments:
 *   shader stage
 *   sampler type
 *   number of samples
 *
 * For example:
 * ./bin/textureSamples fs sampler2DMS 4
 * ./bin/textureSamples vs usampler2DMSArray 2
 */
#include "common.h"

void
parse_args(int argc, char **argv);
static enum shader_target test_stage = UNKNOWN;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	piglit_gl_process_args(&argc, argv, &config);

	parse_args(argc, argv);
	if (test_stage == GS) {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	} else {
		config.supports_gl_compat_version = 30;
		config.supports_gl_core_version = 31;
	}

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display()
{
	bool pass = true;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	float expected_color[4] = {0, 1, 0};
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_draw_rect(-1, -1, 2, 2);

	pass &= piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				      expected_color);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
generate_texture()
{
	GLuint tex;
	GLint samples = 0;
	GLenum target = sampler.target;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	if (target == GL_TEXTURE_2D_MULTISAMPLE)
		glTexImage2DMultisample(target, sample_count, GL_RGBA8,
					32, 32, GL_TRUE);
	else
		glTexImage3DMultisample(target, sample_count, GL_RGBA8,
					32, 32, 32, GL_TRUE);
	glGetTexLevelParameteriv(
		target, 0, GL_TEXTURE_SAMPLES, &samples);
	if (samples != sample_count) {
		printf("Sample count of %d not supported, "
		       "got %d samples\n",
		       sample_count, samples);
		piglit_report_result(PIGLIT_SKIP);
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

	switch (test_stage) {
	case VS:
		asprintf(&vs_code,
			 "#version %d\n"
			 "#extension GL_ARB_texture_multisample: enable\n"
			 "#extension GL_ARB_shader_texture_image_samples: enable\n"
			 "uniform %s tex;\n"
			 "in vec4 piglit_vertex;\n"
			 "flat out int samples;\n"
			 "void main()\n"
			 "{\n"
			 "    samples = textureSamples(tex);\n"
			 "    gl_Position = piglit_vertex;\n"
			 "}\n",
			 shader_version, sampler.name);
		break;
	case GS:
		asprintf(&vs_code,
			 "#version %d\n"
			 "in vec4 piglit_vertex;\n"
			 "out vec4 pos_to_gs;\n"
			 "void main()\n"
			 "{\n"
			 "    pos_to_gs = piglit_vertex;\n"
			 "}\n",
			 shader_version);
		asprintf(&gs_code,
			 "#version %d\n"
			 "#extension GL_ARB_texture_multisample: enable\n"
			 "#extension GL_ARB_shader_texture_image_samples: enable\n"
			 "layout(triangles) in;\n"
			 "layout(triangle_strip, max_vertices = 3) out;\n"
			 "uniform %s tex;\n"
			 "in vec4 pos_to_gs[3];\n"
			 "flat out int samples;\n"
			 "void main()\n"
			 "{\n"
			 "    for (int i = 0; i < 3; i++) {\n"
			 "	  samples = textureSamples(tex);\n"
			 "	  gl_Position = pos_to_gs[i];\n"
			 "	  EmitVertex();\n"
			 "    }\n"
			 "}\n",
			 shader_version, sampler.name);
		break;
	case FS:
		asprintf(&vs_code,
			 "#version %d\n"
			 "in vec4 piglit_vertex;\n"
			 "void main()\n"
			 "{\n"
			 "    gl_Position = piglit_vertex;\n"
			 "}\n",
			 shader_version);
		asprintf(&fs_code,
			 "#version %d\n"
			 "#extension GL_ARB_texture_multisample: enable\n"
			 "#extension GL_ARB_shader_texture_image_samples: enable\n"
			 "uniform %s tex;\n"
			 "out vec4 color;\n"
			 "void main()\n"
			 "{\n"
			 "  if (textureSamples(tex) == %d) color = vec4(0,1,0,1);\n"
			 "  else color = vec4(1,0,0,1);\n"
			 "}\n",
			 shader_version, sampler.name, sample_count);
		break;
	default:
		assert(!"Should not get here.");
		break;
	}

	switch (test_stage) {
	case VS:
	case GS:
		asprintf(&fs_code,
			 "#version %d\n"
			 "flat in int samples;\n"
			 "out vec4 color;\n"
			 "void main()\n"
			 "{\n"
			 "  if (samples == %d) color = vec4(0,1,0,1);\n"
			 "  else color = vec4(1,0,0,1);\n"
			 "}\n",
			 shader_version, sample_count);
		break;
	default:
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
	printf("Usage: textureSize <vs|gs|fs> <sampler type> <sample_count> [piglit args...]\n");
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

		/* Maybe it's the sampler type? */
		if (!sampler_found && (sampler_found = select_sampler(argv[i])))
			continue;

		/* Maybe it's the sample count? */
		if (sampler_found && !sample_count) {
			sample_count = atoi(argv[i]);
			continue;
		}

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

	piglit_require_extension("GL_ARB_shader_texture_image_samples");
	require_GL_features(test_stage);

	if (sample_count > 1) {
		/* check it */
		GLint max_samples;

		if (sampler.data_type == GL_INT ||
		    sampler.data_type == GL_UNSIGNED_INT) {
			glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &max_samples);
			if (sample_count > max_samples) {
				printf("Sample count of %d not supported,"
				       " >MAX_INTEGER_SAMPLES\n",
				       sample_count);
				piglit_report_result(PIGLIT_SKIP);
			}
		} else {
			glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
			if (sample_count > max_samples) {
				printf("Sample count of %d not supported,"
				       " >MAX_SAMPLES\n",
				       sample_count);
				piglit_report_result(PIGLIT_SKIP);
			}
		}
	} else {
		sample_count = 1;
	}

	prog = generate_GLSL(test_stage);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	generate_texture();
}
