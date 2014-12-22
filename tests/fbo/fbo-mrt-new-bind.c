/*
 * Copyright © 2013 Intel Corporation
 * Copyright © 2014 LunarG, Inc.
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
 *
 * Authors:
 *    Chris Forbes <chrisf@ijw.co.nz>
 *    Mike Stroyan <mike@lunarg.com>
 *
 */

/*
 * fbo-mrt-new-bind asserts correct behavior for changing framebuffer binding
 * without changing the shaders.  That can cause trouble if the new binding
 * selects a different shader kernel without updating all resources.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
GLuint fbos[3];
GLint prog0, prog1;
GLuint textures[4];

void
piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_GLSL_version(130);

	glGenTextures(5, textures);
	for (i=0; i<4; i++) {
	    glBindTexture(GL_TEXTURE_2D, textures[i]);
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 640, 360, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glGenFramebuffers(3, fbos);

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[1]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[4], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures[4], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fbos[2]);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[2], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[3], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textures[4], 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, textures[4], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glDrawBuffers(2, buffers);

	prog0 = piglit_build_simple_program(
		"#version 130\n"
		"in vec4 pos;\n"
		"in vec2 tex_coord;\n"
		"void main() {\n"
		"	gl_Position = pos;\n"
		"}\n",

		"#version 130\n"
		"void main() {\n"
		"	float blue = float(int(gl_FragCoord.x / 16 + gl_FragCoord.y / 16 + 1) % 2);\n"
		"	gl_FragData[0] = vec4(0.0, 0.0, blue, 1.0);\n"
		"}\n"
		);

	prog1 = piglit_build_simple_program(
		"#version 130\n"
		"attribute vec4 position;\n"
		"attribute vec2 texture_coord;\n"
		"varying vec2 tex_coord;\n"
		"void main() {\n"
		"	gl_Position = position;\n"
		"	tex_coord = texture_coord;\n"
		"}\n",

		"#version 130\n"
		"uniform sampler2D S0;\n"
		"varying vec2 tex_coord;\n"
		"void main() {\n"
		"	gl_FragData[0] = texture2D(S0, tex_coord );\n"
		"	gl_FragDepth = texture2D(S0, tex_coord ).b;\n"
		"	gl_FragData[1] = texture2D(S0, tex_coord );\n"
		"}\n"
		);

	glBindAttribLocation(prog1, 0, "position");
	glBindAttribLocation(prog1, 1, "texture_coord");

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Setup for test failed.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

static const float positions[] = {
	-1.0, -1.0,
	 1.0, -1.0,
	 1.0,  1.0,
	-1.0,  1.0,
};

static const float texture_coords[] = {
	 0.0,  0.0,
	 1.0,  0.0,
	 1.0,  1.0,
	 0.0,  1.0,
};

enum piglit_result
piglit_display(void)
{
	GLint S0_location;

	/* prepare a source texture */
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[0]);
	glDrawBuffers(1, buffers);
	glClearColor(0,0,1,0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog0);
	glViewport(0, 0, 64, 64);
	piglit_draw_rect(-1, -1, 2, 2);

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[0]);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glDepthMask(GL_TRUE);

	glUseProgram(prog1);
	S0_location = glGetUniformLocation(prog1, "S0");
	glUniform1i(S0_location, 1);

	/* render to 1 buffer */
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[1]);
	glDrawBuffers(1, buffers);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat), positions);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat), texture_coords);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* render to 2 buffers */
	glBindFramebuffer(GL_FRAMEBUFFER, fbos[2]);
	glDrawBuffers(2, buffers);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	/* visualize it */
	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, 128, 128);
	glClearColor(0,0,0.5,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	piglit_draw_rect_tex(-1, -1, 1, 1,
			0, 0, 1, 1);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	piglit_draw_rect_tex(-1, 0.0, 1, 1,
			0, 0, 1, 1);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	piglit_draw_rect_tex(0.0, 0.0, 1, 1,
			0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);

	{
		bool pass = true;
		float black[] = {0,0,0};
		float blue[] = {0,0,1};
		pass = piglit_probe_pixel_rgb(4, 4, blue) && pass;
		pass = piglit_probe_pixel_rgb(12, 4, black) && pass;
		pass = piglit_probe_pixel_rgb(4, 64+4, blue) && pass;
		pass = piglit_probe_pixel_rgb(12, 64+4, black) && pass;
		pass = piglit_probe_pixel_rgb(64+4, 64+4, blue) && pass;
		pass = piglit_probe_pixel_rgb(64+12, 64+4, black) && pass;

		piglit_present_results();

		return pass ? PIGLIT_PASS : PIGLIT_FAIL;
	}
}
