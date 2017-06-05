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

/** @file gl-layer-render-clipped.c
 * This tests correct passing of layer to post-clip stages (when clipping
 * is needed). This parameter must not be interpolated in clip (there's no
 * corresponding fs input from where the interpolation info could be taken).
 * And clipping needs to make sure the right value is copied to the right
 * vertex. We'll test both first and last provoking vertex convention (albeit
 * we use the same layer for all vertices as we don't use
 * GL_LAYER_PROVOKING_VERTEX query). (Could also test vp index)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


const char *vs_source = {
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 vert;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	vert = piglit_vertex;\n"
	"}\n"
};

/*
 * Use the same layer for all tris. A meaner test could use only the correct
 * layer for the "right" vertex, depending on GL_LAYER_PROVOKING_VERTEX query.
 */
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
create_bind_texture() {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, 6, 6, 6, 0, GL_RGB,
		     GL_FLOAT, NULL);

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


bool
probe_texture_layered_rgb(GLuint texture, int x, int y,
			  int z, int w, int h, int d, const float *expected)
{
	int k;
	GLuint fbo;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	for(k = 0; k < d; k++ ) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					  texture, 0, k+z);

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
	int j;
	bool pass = true;
	GLuint fbo, texture, program;

	static const float colors[6*3] = {
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
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	texture = create_bind_texture();
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			     texture, 0);

	if(!check_framebuffer_status(GL_FRAMEBUFFER,
				     GL_FRAMEBUFFER_COMPLETE) ||
	   !piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error with setup\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* draw quad on each layer with set color*/
	glProvokingVertex(GL_LAST_VERTEX_CONVENTION);
	for(j = 0; j < 6; j++) {
		if (j == 3) {
			glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
		}
		glUniform1i(layer_uniform, j);
		glUniform3fv(color_uniform, 1, &colors[j*3]);
		/* rect larger than vp */
		piglit_draw_rect(-2, -2, 4, 4);
	}

	pass = probe_texture_layered_rgb(texture,
					 0, 0, 0, 6, 6, 6, colors) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Clean up */
	glDeleteTextures(1, &texture);
	glDeleteFramebuffers(1, &fbo);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
