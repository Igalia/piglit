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

/** @file gl-layer-render.c
 * Section 4.12.4(Geometry Shaders) From GL spec 3.2 core:
 * "Geometry shaders can be used to render to one of several different layers
 * of cube map textures, three-dimensional textures, or one-or two-dimensional
 * texture arrays.
 *
 * The layer to render to is specified by writing to the built-in output
 * variable gl_Layer."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

GLenum textureType[] = {
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY
};

const char *vs_source = {
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 vert;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	vert = piglit_vertex;\n"
	"}\n"
};

const char *gs_source = {
	"#version 150\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"in vec4 vert[3];\n"
	"uniform int layer;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	for(int i = 0; i < 3; i++) {\n"
	"		gl_Position = vert[i];\n"
	"		gl_Layer = layer;\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n"
};

const char *fs_source = {
	"#version 150\n"
	"uniform vec3 color;\n"
	"void main() {\n"
	"	gl_FragColor = vec4(color.xyz, 1.);\n"
	"}\n"
};

static GLuint layer_uniform;
static GLuint color_uniform;

GLuint
create_bind_texture(GLenum textureType) {
	int i;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(textureType, texture);

	switch(textureType) {
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(textureType, 0, GL_RGB, 6, 6,
			     0, GL_RGB, GL_FLOAT, NULL);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
		glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glTexImage3D(textureType, 0, GL_RGB, 6, 6, 6, 0, GL_RGB,
			     GL_FLOAT, NULL);
		break;
	case GL_TEXTURE_CUBE_MAP:
		for(i = 0; i < 6; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
				     GL_RGB, 6, 6, 0, GL_RGB, GL_FLOAT, NULL);
		}
		break;
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(textureType, 4, GL_RGB,
					6, 6, 6, GL_FALSE);
		break;
	}

	return texture;
}

bool check_framebuffer_status(GLenum target, GLenum expected) {
	GLenum observed = glCheckFramebufferStatus(target);
	if(expected != observed) {
		printf("Unexpected framebuffer status!\n"
		       "  Observed: %s\n  Expected: %s\n",
		       piglit_get_gl_enum_name(observed),
		       piglit_get_gl_enum_name(expected));
		return false;
	}
	return true;
}

/* Take a framebuffer object, that has a GL_TEXTURE_2D_MULTISAMPLE
 * or a layer of a GL_TEXTURE_2D_MULTISAMPLE_ARRAY attached to
 * color attachment 0. Then blit that framebuffer object to
 * a new fbo that has a GL_TEXTURE_2D attached. Finally
 * attach the new GL_TEXTURE_2D to the original fbo.
 */
void
ConvertMultiSample2DToTexture2D(GLuint fboRead) {
	GLuint fboDraw, texture;

	glGenFramebuffers(1, &fboDraw);
	glGenTextures(1, &texture);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboRead);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboDraw);

	texture = create_bind_texture(GL_TEXTURE_2D);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texture, 0);

	if(!check_framebuffer_status(GL_DRAW_FRAMEBUFFER,
				     GL_FRAMEBUFFER_COMPLETE) ||
	   !check_framebuffer_status(GL_READ_FRAMEBUFFER,
				     GL_FRAMEBUFFER_COMPLETE)) {

		piglit_report_result(PIGLIT_FAIL);
	}

	glBlitFramebuffer(0, 0, 6, 6, 0, 0, 6, 6,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &texture);
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, fboRead);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, texture, 0);

	glDeleteFramebuffers(1, &fboDraw);

	if(!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteTextures(1, &texture);
		piglit_report_result(PIGLIT_FAIL);
	}
}


bool
probe_texture_layered_rbg(GLenum textureType, GLuint texture, int x, int y,
			  int z, int w, int h, int d, float *expected)
{
	int k;
	GLuint fbo;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	for(k = 0; k < d; k++ ) {
		if(textureType == GL_TEXTURE_CUBE_MAP) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X+k+z, texture, 0);
		} else {
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  texture, 0, k+z);
		}

		if(textureType == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
			ConvertMultiSample2DToTexture2D(fbo);
		}

		if(!piglit_probe_rect_rgb(0, 0, w, h, &expected[k*3])) {
			printf("Layer: %i\n", k);
			return false;
		}
	}
	return true;
}



void
piglit_init(int argc, char **argv)
{
	int i, j;
	bool pass = true;
	GLuint fbo, texture, program;

	float colors[6*3] = {
		0, 0, 1,
		0, 1, 0,
		0, 1, 1,
		1, 0, 0,
		1, 0, 1,
		1, 1, 0
	};

	program = piglit_build_simple_program_multiple_shaders(
					GL_VERTEX_SHADER, vs_source,
					GL_GEOMETRY_SHADER, gs_source,
					GL_FRAGMENT_SHADER, fs_source,
					0);
	glUseProgram(program);

	/* Retrieve index from vs */
	color_uniform = glGetUniformLocation(program, "color");
	layer_uniform = glGetUniformLocation(program, "layer");

	/* Gen textures */
	for(i = 0; i < ARRAY_SIZE(textureType); i++) {
		printf("Texture Type: %s\n",
			piglit_get_gl_enum_name(textureType[i]));
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		texture = create_bind_texture(textureType[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     texture, 0);

		if(!check_framebuffer_status(GL_FRAMEBUFFER,
					     GL_FRAMEBUFFER_COMPLETE) ||
		   !piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Texture Type: %s. Error with setup\n",
			      piglit_get_gl_enum_name(textureType[i]));
			piglit_report_result(PIGLIT_FAIL);
		}

		/* draw quad on each layer with set color*/
		for(j = 0; j < 6; j++) {
			glUniform1i(layer_uniform, j);
			glUniform3fv(color_uniform, 1, &colors[j*3]);

			piglit_draw_rect(-1, -1, 2, 2);
		}

		if(textureType[i] == GL_TEXTURE_1D_ARRAY) {
			pass = probe_texture_layered_rbg(textureType[i], texture,
						 0, 0, 0, 6, 1, 6, colors)
			       && pass;
		} else {
			pass = probe_texture_layered_rbg(textureType[i], texture,
					 0, 0, 0, 6, 6, 6, colors)
			       && pass;
		}

		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Clean up */
		glDeleteTextures(1, &texture);
		glDeleteFramebuffers(1, &fbo);
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
