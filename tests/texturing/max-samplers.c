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
 *  Marek Olšák <maraeo@gmail.com>
 */

/**
 * Tests that binding the maximum number of textures in both the vertex and
 * fragment shader works.
 */

#include <string.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_source =
	"#define NUM %i \n"
	"uniform sampler2D vertex_tex[NUM]; \n"
	"uniform int vertex_index;"
	"varying vec3 vertex_tex_color; \n"
	"void main() \n"
	"{ \n"
	"	int i; \n"
	"	gl_Position = gl_Vertex; \n"
	"	vertex_tex_color = vec3(0.0); \n"
	"	for (i = 0; i < NUM; i++) \n"
	"		if (i == vertex_index) \n"
	"			vertex_tex_color = texture2DLod(vertex_tex[i], vec2(%f), 0.0).xyz; \n"
	"} \n";

static const char *vs_source_no_textures =
	"varying vec3 vertex_tex_color; \n"
	"void main() \n"
	"{ \n"
	"	gl_Position = gl_Vertex; \n"
	"	vertex_tex_color = vec3(0.0); \n"
	"} \n";

static const char *fs_source =
	"#define NUM %i \n"
	"uniform sampler2D fragment_tex[NUM]; \n"
	"uniform int fragment_index;"
	"varying vec3 vertex_tex_color; \n"
	"void main() \n"
	"{ \n"
	"	int i; \n"
	"	vec3 fragment_tex_color = vec3(0.0); \n"
	"	for (i = 0; i < NUM; i++) \n"
	"		if (i == fragment_index) \n"
	"			fragment_tex_color = texture2D(fragment_tex[i], vec2(%f), 0.0).xyz; \n"
	"	gl_FragColor = vec4(fragment_tex_color + vertex_tex_color, 1.0); \n"
	"} \n";

GLuint prog;
static int max_vs_textures, max_fs_textures;

static void
get_texture_color(int unit, float out[4])
{
	out[0] = (unit % 16) / 15.0;
	out[1] = (unit / 16) / 15.0;
	out[2] = 0;
	out[3] = 1;
}

static void
set_uniform(GLuint prog, const char *name, int value)
{
	GLuint loc = glGetUniformLocation(prog, name);
	if (loc != -1)
		glUniform1i(loc, value);
}

static GLvoid
draw_rect_core(int ix, int iy, int iw, int ih)
{
	float x = -1 + 2.0*ix/piglit_width;
	float y = -1 + 2.0*iy/piglit_height;
	float w = 2.0*iw/piglit_width;
	float h = 2.0*ih/piglit_height;
	float verts[4][4];
	GLuint vbo;

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

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &vbo);
}

static GLboolean
probe_pixel(int unit, int x, int y)
{
	float expected[4];

	get_texture_color(unit, expected);

	if (piglit_probe_pixel_rgb(x, y, expected))
		return GL_TRUE;

	printf("  When testing texture unit %i\n", unit);
	return GL_FALSE;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i, unit, x, y;

	glClear(GL_COLOR_BUFFER_BIT);

	x = 0;
	y = 0;
	unit = 0;

	set_uniform(prog, "fragment_index", max_fs_textures);
	for (i = 0; i < max_vs_textures; i++) {
		set_uniform(prog, "vertex_index", i);
		draw_rect_core(x, y, 20, 20);
		pass = probe_pixel(unit, x+10, y+10) && pass;

		unit++;
		x += 20;
		if (x+20 > piglit_width) {
			x = 0;
			y += 20;
		}
	}

	set_uniform(prog, "vertex_index", max_vs_textures);
	for (i = 0; i < max_fs_textures; i++) {
		set_uniform(prog, "fragment_index", i);
		draw_rect_core(x, y, 20, 20);
		pass = probe_pixel(unit, x+10, y+10) && pass;

		unit++;
		x += 20;
		if (x+20 > piglit_width) {
			x = 0;
			y += 20;
		}
	}

	piglit_check_gl_error(GL_NO_ERROR);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void
set_texture(int unit)
{
	GLuint tex;
	float color[4];

	get_texture_color(unit, color);

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0,
		     GL_RGB, GL_FLOAT, color);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

	piglit_check_gl_error(GL_NO_ERROR);
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs, vao;
	int max_combined_textures, i, unit;
	char str[2048];
	float texcoord = 0.5;

	if (argc == 2 && strcmp(argv[1], "border") == 0) {
		/* Sample outside of the texture, testing border color. */
		texcoord = 5.0;
	} else if (argc != 1) {
		printf("Usage: max-samples [border]\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* get limits */
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_fs_textures);
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &max_vs_textures);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_combined_textures);
	printf("GL_MAX_TEXTURE_IMAGE_UNITS = %d\n", max_fs_textures);
	printf("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = %d\n", max_vs_textures);
	printf("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS = %d\n", max_combined_textures);

	assert(max_fs_textures <= max_combined_textures);

	max_vs_textures = MIN2(max_vs_textures, max_combined_textures - max_fs_textures);

	/* compile shaders */
	if (max_vs_textures) {
		sprintf(str, vs_source, max_vs_textures, texcoord);
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, str);
	}
	else {
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source_no_textures);
	}

	sprintf(str, fs_source, max_fs_textures, texcoord);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, str);

	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	piglit_check_gl_error(GL_NO_ERROR);

	/* initialize textures */
	unit = 0;
	for (i = 0; i < max_vs_textures; i++) {
		char name[64];
		sprintf(name, "vertex_tex[%i]", i);
		set_uniform(prog, name, unit);
		set_texture(unit);
		unit++;
	}

	for (i = 0; i < max_fs_textures; i++) {
		char name[64];
		sprintf(name, "fragment_tex[%i]", i);
		set_uniform(prog, name, unit);
		set_texture(unit);
		unit++;
	}

	glClearColor(0.0, 0.0, 1.0, 1.0);

	if (piglit_get_gl_version() >= 30) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}

	piglit_check_gl_error(GL_NO_ERROR);
}
