/*
 * Copyright 2014 Intel Corporation
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

/** @file dsa-utils.c
 *
 * Contains some common functionality for writing arb_direct_state_access
 * Piglit tests.
 */

#include "dsa-utils.h"
#include "piglit-shader.h"

/*
 * You must use shaders in order to use different texture units.
 * These duplicate fixed-function gl 1.0 pipeline shading.
 * Adapted from arb_clear_texture/3d.c.
 */
static const char vs_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"attribute vec4 piglit_vertex;\n"
	"attribute vec2 piglit_texcoord;\n"
	"varying vec2 tex_coord;\n"
	"#else\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 tex_coord;\n"
	"#endif\n"
	"uniform mat3 xform;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_Position = vec4((xform * piglit_vertex.xyw).xy, 0, 1);\n"
	"        tex_coord = piglit_texcoord;\n"
	"}\n"
	;

static const char fs_1d_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"#define piglit_color gl_FragColor\n"
	"#define texture(s,t) texture1D(s,t)\n"
	"varying vec2 tex_coord;\n"
	"#else\n"
	"out vec4 piglit_color;\n"
	"in vec2 tex_coord;\n"
	"#endif\n"
	"uniform sampler1D tex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        piglit_color = texture(tex, tex_coord.x);\n"
	"}\n"
	;

static const char fs_2d_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"#define piglit_color gl_FragColor\n"
	"#define texture(s,t) texture2D(s,t)\n"
	"varying vec2 tex_coord;\n"
	"#else\n"
	"out vec4 piglit_color;\n"
	"in vec2 tex_coord;\n"
	"#endif\n"
	"uniform sampler2D tex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_FragColor = texture(tex, tex_coord);\n"
	"}\n"
	;

static const char fs_3d_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"#define piglit_color gl_FragColor\n"
	"#define texture(s,t) texture3D(s,t)\n"
	"varying vec2 tex_coord;\n"
	"#else\n"
	"out vec4 piglit_color;\n"
	"in vec2 tex_coord;\n"
	"#endif\n"
	"uniform sampler3D tex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        piglit_color = texture(tex, vec3(tex_coord, 0));\n"
	"}\n"
	;

static const char fs_rect_template[] =
	"#version %s\n"
	"#if __VERSION__ < 130\n"
	"#define piglit_color gl_FragColor\n"
	"#define texture(s,t) texture2DRect(s,t)\n"
	"varying vec2 tex_coord;\n"
	"#else\n"
	"out vec4 piglit_color;\n"
	"in vec2 tex_coord;\n"
	"#endif\n"
	"uniform sampler2DRect tex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        piglit_color = texture(tex, tex_coord);\n"
	"}\n"
	;

GLuint
dsa_create_program(GLenum target)
{
	char *fs_source;
	char *vs_source;
	GLuint prog;
	bool es;
	int major;
	int minor;
	const char * ver;
	GLint loc;
	GLfloat xform[9];

	piglit_get_glsl_version(&es, &major, &minor);
	ver = ((major * 100 + minor) >= 140) ? "140" : "110";

	asprintf(&vs_source, vs_template, ver);
	switch (target) {
	case GL_TEXTURE_1D:
		asprintf(&fs_source, fs_1d_template, ver);
		break;
	case GL_TEXTURE_2D:
		asprintf(&fs_source, fs_2d_template, ver);
		break;
	case GL_TEXTURE_3D:
		asprintf(&fs_source, fs_3d_template, ver);
		break;
	case GL_TEXTURE_RECTANGLE_ARB:
		asprintf(&fs_source, fs_rect_template, ver);
		break;
	default:
		fprintf(stderr, "Invalid texture target in %s\n", __func__);
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = piglit_build_simple_program(vs_source, fs_source);
	free(vs_source);
	free(fs_source);

	/* Note: the default value for all uniforms after linking is zero, so
	 * there is no need to explicitly set it here.  However, the xform
	 * matrix needs to be set to the identity matrix.
	 */
	loc = glGetUniformLocation(prog, "xform");

	xform[0] = 1.0;
	xform[1] = 0.0;
	xform[2] = 0.0;

	xform[3] = 0.0;
	xform[4] = 1.0;
	xform[5] = 0.0;

	xform[6] = 0.0;
	xform[7] = 0.0;
	xform[8] = 1.0;

	glProgramUniformMatrix3fv(prog, loc, 1, GL_FALSE, xform);

	return prog;
}

void
dsa_texture_with_unit(GLuint prog, GLuint unit)
{
	const GLuint loc = glGetUniformLocation(prog, "tex");
	glProgramUniform1i(prog, loc, unit);
}

void
dsa_set_xform(GLuint prog, int width, int height)
{
	const GLint loc = glGetUniformLocation(prog, "xform");
	GLfloat xform[9];

	xform[0] = 2.0 / width;
	xform[1] = 0.0;
	xform[2] = 0.0;

	xform[3] = 0.0;
	xform[4] = 2.0 / height;
	xform[5] = 0.0;

	xform[6] = -1.0;
	xform[7] = -1.0;
	xform[8] = 1.0;

	glProgramUniformMatrix3fv(prog, loc, 1, GL_FALSE, xform);
}
