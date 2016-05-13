/*
 * Copyright © 2015 Tobias Klausmann <tobias.johannes.klausmann@mni.thm.de>
 * Copyright © 2016 Intel Corporation
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
 * \file exceed.c
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 10;
   config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_tmpl[] =
   "#version 130\n"
   "#extension GL_ARB_cull_distance: enable\n"
   "%sout float gl_CullDistance[%d];\n"
   "%sout float gl_ClipDistance[%d];\n"
   "void main()\n"
   "{\n"
   "  gl_Position = gl_Vertex;\n"
   "}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint prog, vs;
	char vert[2048];
	GLint max_clip_distances;
	GLint max_cull_distances;
	GLint max_combined_clip_and_cull_distances;
	GLint clip_distances;
	GLint cull_distances;
	char *use_cull = "", *use_clip = "";

	glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
	glGetIntegerv(GL_MAX_CULL_DISTANCES, &max_cull_distances);
	glGetIntegerv(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES,
		      &max_combined_clip_and_cull_distances);

	if (argc != 2) {
		printf("usage: %s cull/clip/total\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (strcmp(argv[1], "cull") == 0) {
		use_clip = "// ";
		cull_distances = max_cull_distances + 2;
	} else if (strcmp(argv[1], "clip") == 0) {
		clip_distances = max_clip_distances + 2;
		use_cull = "// ";
	} else if (strcmp(argv[1], "total") == 0) {
		clip_distances = max_combined_clip_and_cull_distances / 2 + 1;
		cull_distances = max_combined_clip_and_cull_distances / 2 + 1;
	} else {
		printf("unknown subtest: %s\n", argv[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	snprintf(vert, sizeof(vert), vert_tmpl,
		 use_clip, clip_distances,
		 use_cull, cull_distances);

	piglit_require_gl_version(30);
	piglit_require_GLSL();
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_cull_distance");

	vs = piglit_compile_shader_text_nothrow(GL_VERTEX_SHADER, vert);
	if (vs == 0)
		piglit_report_result(PIGLIT_PASS);

	prog = glCreateProgram();
	glAttachShader(prog, vs);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_PASS);

	piglit_report_result(PIGLIT_FAIL);
}
