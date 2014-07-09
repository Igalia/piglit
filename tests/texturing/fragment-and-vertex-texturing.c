/*
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
 *  Julian Adams <joolsa@gmail.com>
 */

/**
 * @file fragment-and-vertex-texturing.c
 *
 * Tests that sampling from both vertex and fragment textures each read
 * from the correct texture.
 */

#include <string.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_source =
	"uniform sampler2D vertex_tex; \n"
	"varying vec3 vertex_tex_color; \n"
/*	"uniform sampler2D fragment_tex; \n"
	"varying vec3 fragment_tex_color; \n" */
	"void main() \n"
	"{ \n"
	"	gl_Position = gl_Vertex;\n"
	"	vertex_tex_color = texture2DLod(vertex_tex, vec2(0.5), 0.0).xyz; \n"
/*	"	fragment_tex_color = texture2DLod(fragment_tex, vec2(0.5), 0.0).xyz; \n" */
	"} \n";

static const char *fs_source =
	"uniform sampler2D fragment_tex; \n"
	"varying vec3 vertex_tex_color; \n"
	"void main() \n"
	"{ \n"
	"	vec3 fragment_tex_color = texture2D(fragment_tex, vec2(0.5), 0.0).xyz; \n"
	"	gl_FragColor = vec4(fragment_tex_color + vertex_tex_color, 1.0); \n"
	"} \n";

static const char *prog = "fragment-and-vertex-texturing";

/* debug aid */
static void
check_error(int line)
{
	GLenum err = glGetError();
	if (err) {
		printf("%s: GL error 0x%x at line %d\n", prog, err, line);
	}
}

/* debug aid */
static void
check_bad_location(GLint location, int line)
{
	if (location == -1) {
		printf("%s: bad GL location at line %d\n", prog, line);
	}
}

static GLuint
make_texture(int texture_unit, unsigned char r, unsigned char g, unsigned char b)
{
	#define texSize  8
	GLuint tex;
	unsigned char texImage[texSize * texSize * 4];
	GLuint i, j;

	for (i = 0; i < texSize; i++) {
		for (j = 0; j < texSize; j++) {
			int k = (i * texSize + j) * 3;
			texImage[k] = r;
			texImage[k + 1] = g;
			texImage[k + 2] = b;
		}
	}

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texSize, texSize, 0,
					 GL_RGB, GL_UNSIGNED_BYTE, texImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	check_error(__LINE__);

	return tex;
}

static void
display(void)
{
	const int vertex_tex_unit = 0;
	const int fragment_tex_unit = 1;
	GLuint fs, vs, prog;
        GLuint red, green;
        GLint vertex_tex_loc, fragment_tex_loc;

	/* Clear all to blue so we see if the shader rendering happens. */
	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Build the shader that spams green to all outputs. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);

	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	check_error(__LINE__);

	red = make_texture(vertex_tex_unit, 127, 0, 0);
	green = make_texture(fragment_tex_unit, 0, 127, 0);

	// -1 is bad
	vertex_tex_loc = glGetUniformLocation(prog, "vertex_tex");
	check_bad_location(vertex_tex_loc, __LINE__);

	fragment_tex_loc = glGetUniformLocation(prog, "fragment_tex");
	check_bad_location(fragment_tex_loc, __LINE__);

	glUniform1i(vertex_tex_loc, vertex_tex_unit);
	glUniform1i(fragment_tex_loc, fragment_tex_unit);

	piglit_draw_rect(-1, -1, 2, 2);

	check_error(__LINE__);

	glDeleteTextures(1, &red);
	glDeleteTextures(1, &green);

	check_error(__LINE__);
}

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {0.5, 0.5, 0.0};
	int pass;

	display();

	pass = piglit_probe_pixel_rgb(1, 1, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int m;

	printf("The result should be a solid block of half-bright yellow color\n");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(20);

	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &m);
	if (m < 1) {
		printf("No vertex shader texture units supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

