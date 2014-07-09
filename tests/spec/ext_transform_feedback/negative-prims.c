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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/** @file negative-prims.c
 *
 * Tests that glBeginTransformFeedback emits errors when attempting to
 * draw primitives other than those allowed by the current transform
 * feedback primitiveMode.
 *
 * From the EXT_transform_feedback spec:
 *
 *     "The error INVALID_OPERATION is generated if Begin, or any
 *      command that performs an explicit Begin, is called when:
 *
 *        * a geometry shader is not active and <mode> does not match
 *          the allowed begin modes for the current transform feedback
 *          state as given by table X.1."
 *
 * (the test also executes primitives that should pass, to ensure that
 * the test is correctly generating GL errors just due to the bad
 * primitives)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct {
	GLenum tf_prim;
	GLenum prim;
	const char *name;
} prims[] = {
	{ GL_POINTS, GL_POINTS, "GL_POINTS" },
	{ GL_LINES, GL_LINES, "GL_LINES" },
	{ GL_LINES, GL_LINE_STRIP, "GL_LINE_STRIP" },
	{ GL_LINES, GL_LINE_LOOP, "GL_LINE_LOOP" },
	{ GL_TRIANGLES, GL_TRIANGLES, "GL_TRIANGLES" },
	{ GL_TRIANGLES, GL_TRIANGLE_STRIP, "GL_TRIANGLE_STRIP" },
	{ GL_TRIANGLES, GL_TRIANGLE_FAN, "GL_TRIANGLE_FAN" },
	{ GL_TRIANGLES, GL_QUADS, "GL_QUADS" },
	{ GL_TRIANGLES, GL_QUAD_STRIP, "GL_QUAD_STRIP" },
	{ GL_TRIANGLES, GL_POLYGON, "GL_POLYGON" },
};

static bool
test_one_prim(GLenum tf_prim, const char *tf_name, int i)
{
	GLenum error;

	glDrawArrays(prims[i].prim, 0, 4);

	error = glGetError();
	if (prims[i].tf_prim != tf_prim) {
		if (error != GL_INVALID_OPERATION) {
			printf("Expected GL error 0x%x, got 0x%x, when "
			       "rendering %s during %s transform feedback\n",
			       GL_INVALID_OPERATION, error,
			       prims[i].name, tf_name);
			return false;
		}
	} else {
		if (error != 0) {
			printf("Unxpected GL error 0x%x when "
			       "rendering %s during %s transform feedback\n",
			       error,
			       prims[i].name, tf_name);
			return false;
		}
	}
	return true;
}

static bool
test_transform_feedback_prim(GLenum tf_prim, const char *tf_name)
{
	bool pass = true;
	int i;

	glBeginTransformFeedbackEXT(tf_prim);
	for (i = 0; i < ARRAY_SIZE(prims); i++) {
		pass = pass && test_one_prim(tf_prim, tf_name, i);
	}
	glEndTransformFeedbackEXT();

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = pass && test_transform_feedback_prim(GL_POINTS, "GL_POINTS");
	pass = pass && test_transform_feedback_prim(GL_LINES, "GL_LINES");
	pass = pass && test_transform_feedback_prim(GL_TRIANGLES, "GL_TRIANGLES");

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	/* UNREACHED */
	return PIGLIT_FAIL;
}

static const char *vs_source =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static const char *fs_source =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	float verts[] = {
		-1, -1,
		1, -1,
		1, 1,
		-1, 1
	};
	GLuint vbo, xfb, vs, fs, prog;
	const char *varying = "gl_Position";

	piglit_require_extension("GL_EXT_transform_feedback");

	piglit_require_gl_version(30);
	piglit_require_transform_feedback();

	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 8 * sizeof(float),
			verts, GL_DYNAMIC_DRAW);

	glGenBuffersARB(1, &xfb);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb);
	glBufferDataARB(GL_TRANSFORM_FEEDBACK_BUFFER, 4096, NULL,
			GL_DYNAMIC_DRAW);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glTransformFeedbackVaryings(prog, 1, &varying, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!fs || !vs || !prog)
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb, 0, 4096);
}
