/*
 * Copyright © 2013 Intel Corporation
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

/**
 * \file 400-combinations.c
 * Combine 20 vertex shaders and 20 fragment shaders in various ways.
 *
 * Verify that the right shaders are used in the right combinations several
 * ways.
 *
 * * The vertex shader has information baked-in that determines the X position
 *   of the block on the screen.
 *
 * * The fragment shader has information baked-in that determines how the
 *   block is colored.  This is combined with data passed from the vertex
 *   shader.
 *
 * Since data is passed from the vertex shader to the fragment shader, the
 * test can use either rendezvous-by-name (default) or rendezvous-by-location
 * (with --by-location command line parameter).
 */
#include "piglit-util-gl.h"

/**
 * Size of each square that will be drawn.
 */
static const unsigned tile_size = 5;

/**
 * Size of the gap between the squares.
 */
static const unsigned border_size = 2;

static GLuint vs_programs[20];
static GLuint fs_programs[20];

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_width = (tile_size + border_size)
		* ARRAY_SIZE(vs_programs);
	config.window_height = (tile_size + border_size)
		* ARRAY_SIZE(fs_programs);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipe;

static GLuint vao = 0;
static GLuint bo = 0;

struct combination {
	unsigned char row;
	unsigned char col;
};

static struct combination combinations[ARRAY_SIZE(vs_programs)
				       * ARRAY_SIZE(fs_programs)];

static const char *vs_code =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"layout(location = 1) in vec3 vertex_color;\n"
	"\n"
	"%s out vec3 %s;\n"
	"\n"
	"const vec4 offset = vec4(%d, 0, 0, 0);\n"
	"\n"
	"uniform mat4 transform;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = transform * (piglit_vertex + offset);\n"
	"    %s = vertex_color;\n"
	"}\n"
	;

static const char *fs_code =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: enable\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0) out vec4 out_color;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"%s in vec3 %s;\n"
	"\n"
	"const vec3 color_offset = vec3(%d, %d, %d);\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(%s + color_offset, 1.);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	unsigned i;
	unsigned j;

	static const float expected[] = {
		0.0f, 1.0f, 0.0f, 1.0f
	};

	/* This is stored in row-major order.  Note the GL_TRUE parameter to
	 * the glProgramUniformMatrix4fv call below.
	 */
	const float transform[16] = {
		2.f / piglit_width, 0.0f, 0.0f, -1.0f,
		0.0f, 2.f / piglit_height, 0.0f, -1.0f,
		0.0f, 0.0f, 0.0f,  0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	bool pass = true;

	glClearColor(.5f, .5f, .5f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < ARRAY_SIZE(vs_programs); i++) {
		const GLint loc =
			glGetUniformLocation(vs_programs[i], "transform");

		glProgramUniformMatrix4fv(vs_programs[i], loc, 1, GL_TRUE,
					  transform);
	}

	glBindProgramPipeline(pipe);

	for (i = 0; i < ARRAY_SIZE(combinations); i++) {
		const unsigned row = combinations[i].row;
		const unsigned col = combinations[i].col;

		glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT,
				   vs_programs[col]);
		glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT,
				   fs_programs[row]);
		glDrawArrays(GL_TRIANGLE_FAN, row * 4, 4);
	}

	glBindProgramPipeline(0);

	for (i = 0; i < ARRAY_SIZE(vs_programs); i++) {
		for (j = 0; j < ARRAY_SIZE(fs_programs); j++) {
			const unsigned x = (i * tile_size)
				+ ((i + 1) * border_size);
			const unsigned y = (j * tile_size)
				+ ((j + 1) * border_size);

			pass = piglit_probe_rect_rgba(x, y,
						      tile_size, tile_size,
						      expected)
				&& pass;
		}
	}

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

#define RED(x)    ((int) (x / 2))
#define GREEN(x)  (-(int) x)
#define BLUE(x)   ((int) (x * 7))

void
piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	unsigned i;
	unsigned j;
	unsigned idx;
	bool es;
	int glsl_major;
	int glsl_minor;
	const char *location;
	const char *vertex_name;
	const char *fragment_name;

	struct vertex {
		float x;
		float y;
		float r;
		float g;
		float b;
	} *vert;

	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	if (argc > 1 && strcmp(argv[1], "--by-location") == 0) {
		location = "layout(location = 3)";
		vertex_name = "a";
		fragment_name = "b";
	} else {
		location = "";
		vertex_name = "in_color";
		fragment_name = "in_color";
	}

	/* Some NVIDIA drivers have issues with layout qualifiers, 'in'
	 * keywords, and 'out' keywords in "lower" GLSL versions.  If the
	 * driver supports GLSL >= 1.40, use 1.40.  Otherwise, pick the
	 * highest version that the driver supports.
	 */
	piglit_get_glsl_version(&es, &glsl_major, &glsl_minor);
	glsl_version = ((glsl_major * 100) + glsl_minor) >= 140
		? 140 : ((glsl_major * 100) + glsl_minor);

	/* Generate the vertex shader programs.  Each vertex shader is
	 * hardcoded to select a specific column on the display.
	 */
	printf("Generating vertex shaders...\n");
	for (i = 0; i < ARRAY_SIZE(vs_programs); i++) {
		const unsigned base_x = (i * tile_size)
			+ ((i + 1) * border_size);

		char *source = NULL;

		asprintf(&source, vs_code,
			 glsl_version,
			 location,
			 vertex_name,
			 base_x,
			 vertex_name);

		vs_programs[i] =
			glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					       (const GLchar *const *) &source);
		piglit_link_check_status(vs_programs[i]);

		if (i == 0)
			puts(source);

		free(source);
	}

	printf("Generating fragment shaders...\n");
	for (i = 0; i < ARRAY_SIZE(fs_programs); i++) {
		char *source = NULL;

		asprintf(&source, fs_code,
			 glsl_version,
			 location,
			 fragment_name,
			 RED(i), GREEN(i), BLUE(i),
			 fragment_name);

		fs_programs[i] =
			glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
					       (const GLchar *const *) &source);
		piglit_link_check_status(fs_programs[i]);

		if (i == 3)
			puts(source);

		free(source);
	}

	glGenProgramPipelines(1, &pipe);

	/* Generate vertex data for the tests.  The row of each block is
	 * determined by the vertex data.  The color data for the block comes
	 * from the vertex data and the data baked into the fragment shader.
	 */
	if (piglit_get_gl_version() >= 30
	    || piglit_is_extension_supported("GL_ARB_vertex_array_object")) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER,
		     sizeof(vert[0]) * 4 * ARRAY_SIZE(fs_programs),
		     NULL, GL_STATIC_DRAW);

	vert = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	for (i = 0; i < ARRAY_SIZE(fs_programs); i++) {
		const unsigned base_y = (i * tile_size)
			+ ((i + 1) * border_size);

		vert[(i * 4) + 0].x = 0.f;
		vert[(i * 4) + 0].y = (float) base_y;
		vert[(i * 4) + 0].r = (float) -RED(i);
		vert[(i * 4) + 0].g = (float) 1 - GREEN(i);
		vert[(i * 4) + 0].b = (float) -BLUE(i);

		vert[(i * 4) + 1].x = (float) tile_size;
		vert[(i * 4) + 1].y = (float) base_y;
		vert[(i * 4) + 1].r = (float) -RED(i);
		vert[(i * 4) + 1].g = (float) 1 - GREEN(i);
		vert[(i * 4) + 1].b = (float) -BLUE(i);

		vert[(i * 4) + 2].x = (float) tile_size;
		vert[(i * 4) + 2].y = (float) (base_y + tile_size);
		vert[(i * 4) + 2].r = (float) -RED(i);
		vert[(i * 4) + 2].g = (float) 1 - GREEN(i);
		vert[(i * 4) + 2].b = (float) -BLUE(i);

		vert[(i * 4) + 3].x = 0.f;
		vert[(i * 4) + 3].y = (float) (base_y + tile_size);
		vert[(i * 4) + 3].r = (float) -RED(i);
		vert[(i * 4) + 3].g = (float) 1 - GREEN(i);
		vert[(i * 4) + 3].b = (float) -BLUE(i);
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vert[0]),
			      (void *)(intptr_t) offsetof(struct vertex, x));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vert[0]),
			      (void *)(intptr_t) offsetof(struct vertex, r));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Generate the set of combinations of vertex shader programs and
	 * fragment shader programs that will be used together.  This is all
	 * the possible combinations.  The next step is to shuffle list so
	 * that there's (hopefully) no pattern to the access combination... to
	 * uncover driver bugs.
	 */
	idx = 0;
	for (i = 0; i < ARRAY_SIZE(vs_programs); i++) {
		for (j = 0; j < ARRAY_SIZE(fs_programs); j++) {
			combinations[idx].row = j;
			combinations[idx].col = i;
			idx++;
		}
	}

	for (i = ARRAY_SIZE(combinations); i > 1; i--) {
		const unsigned j = rand() % i;

		if (j != i - 1) {
			const struct combination temp = combinations[j];
			combinations[j] = combinations[i - 1];
			combinations[i - 1] = temp;
		}
	}
}
