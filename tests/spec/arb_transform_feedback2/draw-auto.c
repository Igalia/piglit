/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 * Basic ARB_transform_feedback2 test.
 *
 * Test Pause, Resume, and DrawTF.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 64;
	config.window_height = 128;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = {
	"void main() {"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"  gl_FrontColor = gl_Color;"
	"}"
};
static const char *vstext_notransform = {
	"void main() {"
	"  gl_Position = gl_Vertex + vec4(0.0, 0.32, 0.0, 0.0);"
	"  gl_FrontColor = gl_Color;"
	"}"
};
static const char *vstext_notransform_instanced = {
	"#extension GL_ARB_draw_instanced : enable\n"
	"void main() {"
	"  gl_Position = gl_Vertex + vec4(0.0, 0.32 * float(gl_InstanceID+1), 0.0, 0.0);"
	"  gl_FrontColor = gl_Color;"
	"}"
};

static const char *varyings[] = {"gl_FrontColor", "gl_Position"};
GLuint buf;
GLuint prog, prog_notransform, prog_notransform_instanced;
GLuint tfb;
GLboolean instanced;

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLint maxcomps;

	if (argc == 2 && strcmp(argv[1], "instanced") == 0)
		instanced = GL_TRUE;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_extension("GL_EXT_transform_feedback");
	piglit_require_extension("GL_ARB_transform_feedback2");
	if (instanced)
		piglit_require_extension("GL_ARB_transform_feedback_instanced");

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT, &maxcomps);
	if (maxcomps < 8) {
		fprintf(stderr, "Not enough interleaved components supported by transform feedback.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryingsEXT(prog, sizeof(varyings)/sizeof(varyings[0]),
				       varyings, GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext_notransform);
	prog_notransform = piglit_link_simple_program(vs, 0);
	if (instanced) {
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext_notransform_instanced);
		prog_notransform_instanced = piglit_link_simple_program(vs, 0);
	}

	/* Set up transform feedback. */
	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
		     18*8*sizeof(float), NULL, GL_STREAM_READ);

	glGenTransformFeedbacks(1, &tfb);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
	glBindBufferBaseEXT(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	assert(glGetError() == 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float verts[] = {
		10, 10,
		10, 20,
		20, 20,
		20, 10
	};
	static const float red[] = {1, 0, 0};
	static const float green[] = {0, 1, 0};
	static const float blue[] = {0, 0, 1};
	static const float clearcolor[] = {0.2, 0.2, 0.2};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into the TFBO. */
	glUseProgram(prog);
	glLoadIdentity();

	glVertexPointer(2, GL_FLOAT, 0, verts);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
	glBeginTransformFeedbackEXT(GL_TRIANGLES);
	glColor3f(1, 0, 0);
	glDrawArrays(GL_QUADS, 0, 4);
	glPauseTransformFeedback();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	glTranslatef(20, 0, 0);
	glColor3f(0, 1, 0);
	glDrawArrays(GL_QUADS, 0, 4);

	glTranslatef(20, 0, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, tfb);
	glResumeTransformFeedback();
	glColor3f(0, 0, 1);
	glDrawArrays(GL_QUADS, 0, 4);
	glEndTransformFeedbackEXT();
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

	assert(glGetError() == 0);

	glUseProgram(instanced ? prog_notransform_instanced : prog_notransform);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glEnableClientState(GL_COLOR_ARRAY);
	glColorPointer(4, GL_FLOAT, sizeof(float)*8, NULL);
	glVertexPointer(4, GL_FLOAT, sizeof(float)*8, (void*)(intptr_t)(4*sizeof(float)));
	if (instanced) {
		glDrawTransformFeedbackInstanced(GL_TRIANGLES, tfb, 4);
	} else {
		glDrawTransformFeedback(GL_TRIANGLES, tfb);
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	assert(glGetError() == 0);

	pass = piglit_probe_pixel_rgb(15, 15, red) && pass;
	pass = piglit_probe_pixel_rgb(35, 15, green) && pass;
	pass = piglit_probe_pixel_rgb(55, 15, blue) && pass;

	pass = piglit_probe_pixel_rgb(15, 35, red) && pass;
	pass = piglit_probe_pixel_rgb(35, 35, clearcolor) && pass;
	pass = piglit_probe_pixel_rgb(55, 35, blue) && pass;

	if (instanced) {
		unsigned i;
		for (i = 1; i < 4; i++) {
			pass = piglit_probe_pixel_rgb(15, 35 + 20*i, red) && pass;
			pass = piglit_probe_pixel_rgb(35, 35 + 20*i, clearcolor) && pass;
			pass = piglit_probe_pixel_rgb(55, 35 + 20*i, blue) && pass;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
