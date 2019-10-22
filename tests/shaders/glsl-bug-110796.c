/*
 * Copyright © 2019 Intel Corporation
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
 * @file glsl-bug-110796.c
 * @author Lionel Landwerlin
 *
 * Reproduction for a compiler bug.
 */

#include <time.h>

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

static const char *vert_shader_text =
	"#version 320 es\n"
	"void main() \n"
	"{ \n"
	"   gl_Position = vec4(0.0); \n"
	"} \n";

static void
check_error(int line)
{
	GLenum e = glGetError();
	if (e)
		printf("GL Error 0x%x at line %d\n", e, line);
}

int main(int argc, char **argv)
{
	EGLint major, minor;
	EGLDisplay dpy;
	EGLContext ctx1, ctx2;
	EGLint attr[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 2,
		EGL_NONE
	};
	GLuint program;
	bool ok;
	char frag_shader_text1[256];
	char frag_shader_text2[386];

	srand(time(NULL));

	snprintf(frag_shader_text1, sizeof(frag_shader_text1),
		 "#version 320 es\n"
		 "uniform sampler2D s2D;\n"
		 "const ivec2 offset = ivec2(-%i, 7);\n"
		 "out mediump vec4 color;\n"
		 "\n"
		 "void main() \n"
		 "{ \n"
		 "   color = vec4(1.0) - textureGatherOffset(s2D, vec2(0), offset); \n"
		 "} \n", rand() % 10000);

	snprintf(frag_shader_text2, sizeof(frag_shader_text2),
		 "#version 320 es\n"
		 "uniform sampler2D s2D;\n"
		 "const ivec2[] offsets = ivec2[](\n"
		 "    ivec2(-%i, 7),\n"
		 "    ivec2(-%i, 2),\n"
		 "    ivec2(-%i, 3),\n"
		 "    ivec2(-%i, 4)\n"
		 ");\n"
		 "out mediump vec4 color;\n"
		 "\n"
		 "void main() \n"
		 "{ \n"
		 "   color = vec4(1.0) - textureGatherOffsets(s2D, vec2(0), offsets);\n"
		 "} \n", rand() % 10000, rand() % 10000,
		 rand() % 10000, rand() % 10000);

	dpy = piglit_egl_get_default_display(EGL_NONE);

	ok = eglInitialize(dpy, &major, &minor);
	if (!ok) {
		piglit_report_result(PIGLIT_FAIL);
	}

	ctx1 = eglCreateContext(dpy, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attr);

	if (!ctx1) {
		fprintf(stderr, "glsl-bug-110796: create contexts failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	dpy = piglit_egl_get_default_display(EGL_NONE);

	/*
	 * Bind first context, make some shaders, draw something.
	 */
	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx1);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	program = piglit_build_simple_program(vert_shader_text, frag_shader_text1);
	check_error(__LINE__);
	if (!program) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glLinkProgram(program);
	glUseProgram(program);
	check_error(__LINE__);

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroyContext(dpy, ctx1);

	ctx2 = eglCreateContext(dpy, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, attr);

	if (!ctx2) {
		fprintf(stderr, "glsl-bug-110796: create contexts failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx2);

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	program = piglit_build_simple_program(vert_shader_text, frag_shader_text2);
	check_error(__LINE__);
	if (!program) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glLinkProgram(program);
	glUseProgram(program);
	check_error(__LINE__);

	eglDestroyContext(dpy, ctx2);

	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
