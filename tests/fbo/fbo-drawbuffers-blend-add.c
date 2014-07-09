/*
 * Copyright © 2010 Intel Corporation
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
 *	  Eric Anholt <eric@anholt.net>
 *	  Marek Olšák <maraeo@gmail.com>
 *
 */

/** @file fbo-drawbuffers-blend-add.c
 *
 * Tests that additive blending is enabled for all render targets with ARB_draw_buffers.
 */

#include <string.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint max_targets;

static char *vs_source =
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static char *fs_source_start =
	"void main()\n"
	"{\n";

static char *fs_source_output =
	"	gl_FragData[%d].xyzw = vec4(%f, %f, %f, %f);\n";

static char *fs_source_end =
	"}\n"; 

float output_values[] = {
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.0, 0.5, 0.5, 0.0, 
	0.5, 0.5, 0.0, 0.0,
	
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 0.5, 0.5,
	0.0, 0.5, 0.5, 0.5, 
	0.5, 0.5, 0.0, 0.5,

	0.0, 0.25, 0.0, 0.0,
	0.0, 0.0, 0.25, 0.0,
	0.0, 0.25, 0.25, 0.0, 
	0.25, 0.25, 0.0, 0.0,
	
	0.0, 0.25, 0.0, 0.25,
	0.0, 0.0, 0.25, 0.25,
	0.0, 0.25, 0.25, 0.25, 
	0.25, 0.25, 0.0, 0.25};

float clear_value = 0.25;

static GLuint
attach_texture(int i)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
			 32, 32, 0,
			 GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT + i,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	return tex;
}

static void
generate_and_display_drawbuffers(int count)
{
	GLuint tex[16], fb, fs, vs, prog;
	GLenum attachments[16], status;
	int i;
	char fs_output_line[256];
	char fs_full_source[1024];

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (i = 0; i < count; i++) {
		tex[i] = attach_texture(i);
		attachments[i] = GL_COLOR_ATTACHMENT0 + i;
	}

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	glDrawBuffersARB(count, attachments);

	/* Clear all to 0.25 so we see if the shader rendering happens. */
	glClearColor(clear_value, clear_value, clear_value, clear_value);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Build the shader that spams green to all outputs. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);

	strcpy(fs_full_source, fs_source_start);
		
	for (i = 0; i < count; i++) {
		sprintf(fs_output_line, fs_source_output, i, output_values[i * 4], 
				output_values[(i * 4) + 1], output_values[(i * 4) + 2], 
				output_values[(i * 4) + 3]);
		
		strcat(fs_full_source, fs_output_line);
	}
	
	strcat(fs_full_source, fs_source_end);

	assert(strlen(fs_full_source) + 1 < sizeof(fs_full_source) / sizeof(fs_full_source[0]));
	
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_full_source);

	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);

	/* Now render to all the color buffers. */
	piglit_draw_rect(-1, -1, 2, 2);

	glDisable(GL_BLEND);
	
	/* OK, now draw each of these textures to the winsys framebuffer. */
	glUseProgram(0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);
	for (i = 0; i < count; i++) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		piglit_draw_rect_tex(16 * i, 16 * (count - 1),
					 16, 16,
					 0, 0,
					 1, 1);
	}
	glDisable(GL_TEXTURE_2D);

	for (i = 0; i < count; i++) {
		glDeleteTextures(1, &tex[i]);
	}
	glDeleteFramebuffersEXT(1, &fb);
}

enum piglit_result
piglit_display(void)
{
	#define N_VECTOR_ITEMS 4
	GLboolean pass = GL_TRUE;
	int count, i, j;
	float expected[N_VECTOR_ITEMS] = {0};
	

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (count = 1; count <= max_targets; count++) {
		generate_and_display_drawbuffers(count);
	}

	for (count = 1; count <= max_targets; count++) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < N_VECTOR_ITEMS; j++) {
				expected[j] = output_values[(i * N_VECTOR_ITEMS) + j] + 
					clear_value;
			}
	
			pass = pass &&
				piglit_probe_pixel_rgba(16 * i + 8,
							   16 * (count - 1) + 8,
							   expected);
		}
	}
	
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_attachments;

	printf("The result should be a rows of boxes of differing colors, \n"
		   "one for each drawbuffer target used (none black).\n");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(20);

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_draw_buffers");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &max_targets);
	if (max_targets < 2)
		piglit_report_result(PIGLIT_SKIP);

	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS_EXT, &max_attachments);
	if (max_targets > max_attachments)
		max_targets = max_attachments;

	if (max_targets > 16)
		max_targets = 16;
}
