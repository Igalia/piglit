/*
 * Copyright © 2020 Google LLC
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

/** @file glsl-uniform-interstage-limits.c
 *
 * Tests that arrays of vec4 uniforms are fully correct in both the VS and the
 * FS.
 *
 * On adreno HW, the const file (which we move uniforms to if we can) is
 * shared between the stages, and you need to allocate between them.  Failure
 * to limit your stages by just a little bit seems to lead to corruption as
 * one shader writes over another's memory, while larger failure leads to GPU
 * hangs.
 */

#include <getopt.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_width = 64;
	config.window_height = 64;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static int statechanges;
static int subdivide;
static int vs_arg, fs_arg;
static int max_vs_vec4, max_fs_vec4;

/* Sets up the uniform arrays in the program for the given array sizes.  The
 * contents vary slightly between the stages to make sure you don't mix those
 * up, and the "delta" argument is used in the statechanges case to make sure
 * you don't use a stale uniform buffer's contents.
 */
static void
fill_uniform_arrays(int delta, int vs_array_size, int fs_array_size)
{
	GLuint unifvs = glGetUniformLocation(prog, "vsu");
	assert(unifvs != -1);
	for (int i = delta; i < delta + vs_array_size; i++)
		glUniform4f(unifvs + i - delta, i, i + 1, i + 2, i + 3);

	GLuint uniffs = glGetUniformLocation(prog, "fsu");
	assert(uniffs != -1);
	for (int i = delta; i < delta + fs_array_size; i++)
		glUniform4f(uniffs + i - delta, i, i + 1, i + 2, i + 4);

	glUniform1f(glGetUniformLocation(prog, "delta"), delta);
}

/* Creates the shader program for the given array sizes and initially fills
 * its uniforms.
 */
static void
setup_program(int vs_array_size, int fs_array_size)
{
	const char *vs_source =
		"uniform vec4 vsu[%d];\n"
		"uniform int vslen;\n"
		"uniform float delta;\n"
		"varying float result;\n"
		"void main()\n"
		"{\n"
		"	result = 0.75;\n"
		"	for (int i = 0; i < vslen; i++) {\n"
		"		if (vsu[i] - delta != vec4(i, i + 1, i + 2, i + 3))\n"
		"			result = 0.25;\n"
		"	}\n"
		"	gl_Position = gl_Vertex;\n"
		"}\n";

	const char *fs_source =
		"uniform vec4 fsu[%d];\n"
		"uniform int fslen;\n"
		"uniform float delta;\n"
		"varying float result;\n"
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(0.0, result, 0.75, 0.0);\n"
		"	for (int i = 0; i < fslen; i++) {\n"
		"		if (fsu[i] - delta != vec4(i, i + 1, i + 2, i + 4))\n"
		"			gl_FragColor.z = 0.25;\n"
		"	}\n"
		"}\n";

	piglit_require_gl_version(20);

	GLuint vs = piglit_compile_shader_formatted(GL_VERTEX_SHADER, vs_source,
						    vs_array_size);
	GLuint fs = piglit_compile_shader_formatted(GL_FRAGMENT_SHADER, fs_source,
						    fs_array_size);
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	glUniform1i(glGetUniformLocation(prog, "vslen"), vs_array_size);
	glUniform1i(glGetUniformLocation(prog, "fslen"), fs_array_size);
	fill_uniform_arrays(0, vs_array_size, fs_array_size);
}

/* Generates a program for the given uniform array sizes, draws, and checks
 * the results.
 */
static bool
test(int vs_array_size, int fs_array_size)
{
	printf("Testing %d VS vec4, %d fs vec4\n",
	       vs_array_size, fs_array_size);

	setup_program(vs_array_size, fs_array_size);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Emit several draws, so if the FS state overwrites the VS, we
	 * hopefully catch it on the next draw.
	 */
	for (int i = 0; i < 256; i++) {
		int xi = i % 8;
		int yi = i / 8;

		if (statechanges)
			fill_uniform_arrays(i, vs_array_size, fs_array_size);

		piglit_draw_rect(-1.0 + 2.0 * xi / 8.0,
				 -1.0 + 2.0 * yi / 8.0,
				 2.0 / 8,
				 2.0 / 8);
	}


	float expected[4] = {0.0, 0.75, 0.75, 0.0};
	bool pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
					   expected);

	glDeleteProgram(prog);

	return pass;
}

/* Returns an array of array sizes to test in a stage, terminated by a 0. */
static int *
pick_sizes(int arg, int max)
{
	int count = ((!arg && subdivide) ? subdivide : 1) + 1;

	int *sizes = calloc(count, sizeof(int));
	if (arg) {
		sizes[0] = arg;
	} else if (!subdivide) {
		sizes[0] = max;
	} else {
		sizes[0] = 1;

		for (int i = 1; i < subdivide; i++) {
			/* Be careful about overflow if the driver exposes a
			 * large max count.
			 */
			sizes[i] = (uint64_t)i * max / (subdivide - 1);
		}
	}

	return sizes;
}

/* Iterates over the sizes to test, returns the overall result. */
enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;

	int *vs_sizes = pick_sizes(vs_arg, max_vs_vec4);
	int *fs_sizes = pick_sizes(fs_arg, max_fs_vec4);

	for (int v = 0; vs_sizes[v] != 0; v++) {
		for (int f = 0; fs_sizes[f] != 0; f++) {
			piglit_merge_result(&result,
					    test(vs_sizes[v], fs_sizes[f]) ?
					    PIGLIT_PASS : PIGLIT_FAIL);
		}
	}

	free(vs_sizes);
	free(fs_sizes);

	piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	while (1) {
		static struct option long_options[] = {
			{"vs",            required_argument, 0, 'v' },
			{"fs",            required_argument, 0, 'f' },
			{"subdivide",     required_argument, 0, 's' },
			{"statechanges",  no_argument,       &statechanges, 1 },
			{0,               0,                 0,  0  }
		};

		int c = getopt_long(argc, argv, "v:f:", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			/* long opt with value */
			break;
		case 'v':
			vs_arg = (int)strtol(optarg, NULL, 10);
			break;
		case 'f':
			fs_arg = (int)strtol(optarg, NULL, 10);
			break;
		case 's':
			subdivide = (int)strtol(optarg, NULL, 10);
			break;
		default:
			fprintf(stderr, "usage: glsl-uniform-count "
				"[--vs vec4_count] "
				"[--fs vec4_count] "
				"[--subdivide divisions] "
				"[--statechanges]\n");
			exit(1);
		}
	}

	/* Check against HW limits.
	 */
	GLint max_vs, max_fs;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &max_vs);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &max_fs);
	/* Note: less than a vec4 worth of other uniforms in each of our
	 * shaders besides the array.
	 */
	max_vs_vec4 = max_vs / 4 - 1;
	max_fs_vec4 = max_fs / 4 - 1;

	if (vs_arg > max_vs_vec4) {
		fprintf(stderr, "VS vec4 count too large for HW limits "
			"(%d dwords)\n", max_vs);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (fs_arg > max_fs_vec4) {
		fprintf(stderr, "FS vec4 count too large for HW limits "
			"(%d dwords)\n", max_fs);
		piglit_report_result(PIGLIT_SKIP);
	}
}
