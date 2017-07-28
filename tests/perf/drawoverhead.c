/*
 * Copyright (C) 2009 VMware, Inc.
 * Copyright (C) 2017 Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "common.h"
#include <stdbool.h>
#include "piglit-util-gl.h"

static bool is_compat;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 0;
	config.supports_gl_core_version = 32;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-compat")) {
			config.supports_gl_compat_version = 10;
			config.supports_gl_core_version = 0;
			is_compat = true;
			break;
		}
	}
	puts(config.supports_gl_core_version ? "Using Core profile." :
					       "Using Compatibility profile.");
	puts("Draw calls per second:");

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE |
                               PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog[2], uniform_loc, tex[8], ubo[4], tbo[8];
static bool indexed;
static GLenum enable_enum;

void
piglit_init(int argc, char **argv)
{
	static const unsigned indices[4] = {0, 1, 2, 3};
	GLuint vao, ebo;

	piglit_require_gl_version(30);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(indices), indices, GL_STATIC_DRAW);
}

static void
get_vs_text(char *s, unsigned num_vbos, unsigned num_tbos, bool is_second)
{
	unsigned i;

	sprintf(s, "#version %u\n", num_tbos ? 140 : 130);
	strcat(s, "#extension GL_ARB_explicit_attrib_location : require\n");
	for (i = 0; i < num_vbos; i++) {
		sprintf(s + strlen(s),
			"layout (location = %u) in vec4 v%u;\n", i, i);
	}
	strcat(s, "void main() {\n"
		  "	gl_Position = vec4(0.0)");
	for (i = 0; i < num_vbos; i++)
		sprintf(s + strlen(s), " + v%u", i);
	if (is_second)
		strcat(s, " + vec4(0.5)");
	strcat(s, ";\n}\n");
}

static void
get_fs_text(char *s, unsigned num_ubos, unsigned num_textures,
	    unsigned num_tbos, unsigned num_images, unsigned num_imgbos,
	    bool is_second)
{
	unsigned i;

	sprintf(s, "#version %u\n", num_tbos ? 140 : 130);
	strcat(s, "#extension GL_ARB_uniform_buffer_object : require\n");
	if (num_images || num_imgbos)
		strcat(s, "#extension GL_ARB_shader_image_load_store : require\n");
	sprintf(s + strlen(s),
		"uniform int index = 0;\n"
		"uniform vec4 u[%u];\n", is_second ? 240 : 1);

	for (i = 0; i < num_textures; i++)
		sprintf(s + strlen(s), "uniform sampler2D s%u;\n", i);
	for (i = 0; i < num_tbos; i++)
		sprintf(s + strlen(s), "uniform samplerBuffer sb%u;\n", i);
	for (i = 0; i < num_images; i++)
		sprintf(s + strlen(s), "layout(rgba8) readonly uniform image2D i%u;\n", i);
	for (i = 0; i < num_imgbos; i++)
		sprintf(s + strlen(s), "layout(rgba8) readonly uniform imageBuffer ib%u;\n", i);
	for (i = 0; i < num_ubos; i++)
		sprintf(s + strlen(s), "uniform ub%u { vec4 ubu%u[10]; };\n", i, i);
	strcat(s, "void main() {\n");
	strcat(s, "	gl_FragData[0] = u[index]");
	for (i = 0; i < num_textures; i++)
		sprintf(s + strlen(s), " + texture(s%u, u[0].xy)", i);
	for (i = 0; i < num_tbos; i++)
		sprintf(s + strlen(s), " + texelFetch(sb%u, int(u[0].x))", i);
	for (i = 0; i < num_images; i++)
		sprintf(s + strlen(s), " + imageLoad(i%u, ivec2(u[0].xy))", i);
	for (i = 0; i < num_imgbos; i++)
		sprintf(s + strlen(s), " + imageLoad(ib%u, int(u[0].x))", i);
	for (i = 0; i < num_ubos; i++)
		sprintf(s + strlen(s), " + ubu%u[index]", i);
	if (is_second)
		strcat(s, " + vec4(0.5)");
	strcat(s, ";\n}\n");
}

static void
setup_shaders_and_resources(unsigned num_vbos,
			    unsigned num_ubos,
			    unsigned num_textures,
			    unsigned num_tbos,
			    unsigned num_images,
			    unsigned num_imgbos)
{
	const unsigned max = 16;
	char vs[4096], fs[4096];
	unsigned p, i;

	assert(num_vbos <= max);
	assert(num_ubos <= max);
	assert(num_textures <= max);
	assert(num_tbos <= max);

	for (i = 0; i < max; i++)
		glDisableVertexAttribArray(i);

	/* Create two programs in case we want to test program changes. */
	for (p = 0; p < 2; p++) {
		get_vs_text(vs, num_vbos, num_tbos, p);
		get_fs_text(fs, num_ubos, num_textures, num_tbos, num_images,
			    num_imgbos, p);
		prog[p] = piglit_build_simple_program(vs, fs);

		/* Assign texture units to samplers. */
		glUseProgram(prog[p]);
		for (i = 0; i < num_textures; i++) {
			char sampler[20];
			int loc;

			snprintf(sampler, sizeof(sampler), "s%u", i);
			loc = glGetUniformLocation(prog[p], sampler);
			assert(loc >= 0);
			glUniform1i(loc, i);
		}
		/* Assign texture units to samplers. */
		for (i = 0; i < num_tbos; i++) {
			char sampler[20];
			int loc;

			snprintf(sampler, sizeof(sampler), "sb%u", i);
			loc = glGetUniformLocation(prog[p], sampler);
			assert(loc >= 0);
			glUniform1i(loc, num_textures + i);
		}
		for (i = 0; i < num_images; i++) {
			char name[20];
			int loc;

			snprintf(name, sizeof(name), "i%u", i);
			loc = glGetUniformLocation(prog[p], name);
			assert(loc >= 0);
			glUniform1i(loc, i);
		}
		for (i = 0; i < num_imgbos; i++) {
			char name[20];
			int loc;

			snprintf(name, sizeof(name), "ib%u", i);
			loc = glGetUniformLocation(prog[p], name);
			assert(loc >= 0);
			glUniform1i(loc, num_images + i);
		}
		/* Assign UBO slots to uniform blocks. */
		for (i = 0; i < num_ubos; i++) {
			char block[20];
			int index;

			snprintf(block, sizeof(block), "ub%u", i);
			index = glGetUniformBlockIndex(prog[p], block);
			assert(index != GL_INVALID_INDEX);
			glUniformBlockBinding(prog[p], index, i);
		}
	}
	glUseProgram(prog[0]);

	for (i = 0; i < num_ubos; i++) {
		static const float data[10*4];
		GLuint ub;

		glGenBuffers(1, &ub);
		glBindBuffer(GL_UNIFORM_BUFFER, ub);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(data), data,
			     GL_STATIC_DRAW);

		glBindBufferBase(GL_UNIFORM_BUFFER, i, ub);
		/* Save the last UBOs for testing UBO changes. */
		ubo[i % 4] = ub;
	}
	/* setup VBO w/ vertex data, we need a different buffer in each attrib */
	for (i = 0; i < num_vbos; i++) {
		/* Vertex positions are all zeroed - we want all primitives
		 * to be culled.
		 */
		static const float vertices[4][3];
		GLuint vbo;

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
			     GL_STATIC_DRAW);

		glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE,
				      3 * sizeof(float), NULL);
		glEnableVertexAttribArray(i);
	}
	for (i = 0; i < MAX2(num_textures, num_images); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		/* Save the last texture IDs for testing texture changes. */
		tex[i % 8] = piglit_rgbw_texture(GL_RGBA8, 4, 4, false, true,
						 GL_UNSIGNED_BYTE);
	}
	for (i = 0; i < MAX2(num_tbos, num_imgbos); i++) {
		static const float data[10*4];
		GLuint buf, tb;

		glGenBuffers(1, &buf);
		glBindBuffer(GL_TEXTURE_BUFFER, buf);
		glBufferData(GL_TEXTURE_BUFFER, sizeof(data), data,
			     GL_STATIC_DRAW);

		glActiveTexture(GL_TEXTURE0 + num_textures + i);
		glGenTextures(1, &tb);
		glBindTexture(GL_TEXTURE_BUFFER, tb);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, buf);

		/* Save the last TBOs for testing UBO changes. */
		tbo[i % 8] = tb;
	}
	glActiveTexture(GL_TEXTURE0);
}

static void
draw(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++)
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
	} else {
		for (i = 0; i < count; i++)
			glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}

static void
draw_shader_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glUseProgram(prog[i & 1]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glUseProgram(prog[i & 1]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	glUseProgram(prog[0]);
}

static void
draw_uniform_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glUniform4f(uniform_loc, i & 1, 0, 0, 0);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glUniform4f(uniform_loc, i & 1, 0, 0, 0);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_one_texture_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glBindTexture(GL_TEXTURE_2D, tex[i & 1]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glBindTexture(GL_TEXTURE_2D, tex[i & 1]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_many_texture_change(unsigned count)
{
	unsigned i,j;
	if (indexed) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++) {
				glActiveTexture(GL_TEXTURE0 + j);
				glBindTexture(GL_TEXTURE_2D, tex[(i + j) % 8]);
			}
			glActiveTexture(GL_TEXTURE0);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++) {
				glActiveTexture(GL_TEXTURE0 + j);
				glBindTexture(GL_TEXTURE_2D, tex[(i + j) % 8]);
			}
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_one_tbo_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glBindTexture(GL_TEXTURE_BUFFER, tbo[i & 1]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glBindTexture(GL_TEXTURE_BUFFER, tbo[i & 1]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_many_tbo_change(unsigned count)
{
	unsigned i,j;
	if (indexed) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++) {
				glActiveTexture(GL_TEXTURE0 + j);
				glBindTexture(GL_TEXTURE_BUFFER, tbo[(i + j) % 8]);
			}
			glActiveTexture(GL_TEXTURE0);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++) {
				glActiveTexture(GL_TEXTURE0 + j);
				glBindTexture(GL_TEXTURE_BUFFER, tbo[(i + j) % 8]);
			}
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_one_img_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glBindImageTexture(0, tex[i & 1], 0, false, 0,
					   GL_READ_ONLY, GL_RGBA8);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glBindImageTexture(0, tex[i & 1], 0, false, 0,
					   GL_READ_ONLY, GL_RGBA8);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_many_img_change(unsigned count)
{
	unsigned i,j;
	if (indexed) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++)
				glBindImageTexture(0, tex[(i + j) % 8], 0, false,
						   0, GL_READ_ONLY, GL_RGBA8);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++)
				glBindImageTexture(0, tex[(i + j) % 8], 0, false,
						   0, GL_READ_ONLY, GL_RGBA8);
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_one_imgbo_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glBindImageTexture(0, tbo[i & 1], 0, false, 0,
					   GL_READ_ONLY, GL_RGBA8);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glBindImageTexture(0, tbo[i & 1], 0, false, 0,
					   GL_READ_ONLY, GL_RGBA8);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_many_imgbo_change(unsigned count)
{
	unsigned i,j;
	if (indexed) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++)
				glBindImageTexture(0, tbo[(i + j) % 8], 0, false,
						   0, GL_READ_ONLY, GL_RGBA8);
			glActiveTexture(GL_TEXTURE0);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 8; j++)
				glBindImageTexture(0, tbo[(i + j) % 8], 0, false,
						   0, GL_READ_ONLY, GL_RGBA8);
			glActiveTexture(GL_TEXTURE0);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_one_ubo_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[i & 1]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[i & 1]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_many_ubo_change(unsigned count)
{
	unsigned i,j;
	if (indexed) {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 4; j++)
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[(i + j) % 4]);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			for (j = 0; j < 4; j++)
				glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo[(i + j) % 4]);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

static void
draw_state_change(unsigned count)
{
	unsigned i;
	PFNGLENABLEPROC enable = glEnable;
	PFNGLDISABLEPROC disable = glDisable;
	GLenum glenum = enable_enum;

	if (is_compat && enable_enum == GL_PRIMITIVE_RESTART) {
		enable = glEnableClientState;
		disable = glDisableClientState;
		glenum = GL_PRIMITIVE_RESTART_NV;
	}

	if (indexed) {
		for (i = 0; i < count; i++) {
			if (i & 1)
				enable(glenum);
			else
				disable(glenum);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			if (i & 1)
				enable(glenum);
			else
				disable(glenum);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	disable(glenum);
}

static void
draw_scissor_change(unsigned count)
{
	unsigned i;
	glEnable(GL_SCISSOR_TEST);
	if (indexed) {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glScissor(0, 0, piglit_width / 2, piglit_height / 2);
			else
				glScissor(0, 0, piglit_width, piglit_height);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glScissor(0, 0, piglit_width / 2, piglit_height / 2);
			else
				glScissor(0, 0, piglit_width, piglit_height);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	glDisable(GL_SCISSOR_TEST);
}

static void
draw_viewport_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glViewport(0, 0, piglit_width / 2, piglit_height / 2);
			else
				glViewport(0, 0, piglit_width, piglit_height);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glViewport(0, 0, piglit_width / 2, piglit_height / 2);
			else
				glViewport(0, 0, piglit_width, piglit_height);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
	glViewport(0, 0, piglit_width, piglit_height);
}

static void
draw_vertex_attrib_change(unsigned count)
{
	unsigned i;
	if (indexed) {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
						      3 * sizeof(float), NULL);
			else
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
						      3 * sizeof(float), NULL);
			glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);
		}
	} else {
		for (i = 0; i < count; i++) {
			if (i & 1)
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
						      3 * sizeof(float), NULL);
			else
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
						      3 * sizeof(float), NULL);
			glDrawArrays(GL_TRIANGLES, 0, 3);
		}
	}
}

#define COLOR_RESET	"\033[0m"
#define COLOR_RED	"\033[31m"
#define COLOR_GREEN	"\033[1;32m"
#define COLOR_YELLOW	"\033[1;33m"
#define COLOR_CYAN	"\033[1;36m"

static double
perf_run(const char *call, unsigned num_vbos, unsigned num_ubos,
	 unsigned num_textures, unsigned num_tbos, unsigned num_images,
	 unsigned num_imgbos,
	 const char *change, perf_rate_func f, double base_rate)
{
	static unsigned test_index;
	test_index++;

	double rate = perf_measure_rate(f);
	double ratio = base_rate ? rate / base_rate : 1;

	printf(" %3u: %s (%2u VBO, %u UBO, %2u %s) w/ %s change:%*s"
	       COLOR_CYAN "%s" COLOR_RESET " %s(%.1f%%)" COLOR_RESET "\n",
	       test_index, call, num_vbos, num_ubos,
	       num_textures ? num_textures :
	         num_tbos ? num_tbos :
	         num_images ? num_images : num_imgbos,
	       num_textures ? "Tex" :
	         num_tbos ? "TBO" :
	         num_images ? "Img" :
	         num_imgbos ? "ImB" : "   ",
	       change,
	       MAX2(36 - (int)strlen(change) - (int)strlen(call), 0), "",
	       perf_human_float(rate),
	       base_rate == 0 ? COLOR_RESET :
				ratio > 0.7 ? COLOR_GREEN :
				ratio > 0.4 ? COLOR_YELLOW : COLOR_RED,
	       100 * ratio);
	return rate;
}

struct enable_state_t {
	GLenum enable;
	const char *name;
};

static struct enable_state_t enable_states[] = {
	{GL_PRIMITIVE_RESTART, "primitive restart enable"},
	{GL_BLEND,	"blend enable"},
	{GL_DEPTH_TEST, "depth enable"},
	{GL_DEPTH_CLAMP, "depth clamp enable"},
	{GL_STENCIL_TEST, "stencil enable"},
	{GL_SCISSOR_TEST, "scissor enable"},
	{GL_MULTISAMPLE, "MSAA enable"},
	{GL_SAMPLE_MASK, "sample mask enable"},
	{GL_SAMPLE_ALPHA_TO_COVERAGE, "alpha-to-coverage enable"},
	{GL_SAMPLE_SHADING, "sample shading enable"},
	{GL_CULL_FACE,	"cull face enable"},
	{GL_CLIP_DISTANCE0, "clip distance enable"},
};

static void
perf_draw_variant(const char *call, bool is_indexed)
{
	double base_rate = 0;

	indexed = is_indexed;

	/* Test different shader resource usage without state changes. */
	unsigned num_ubos = 0;
	unsigned num_textures = 0;
	unsigned num_tbos = 0;
	unsigned num_images = 0;
	unsigned num_imgbos = 0;
	unsigned num_vbos;

	for (num_vbos = 1; num_vbos <= 16; num_vbos *= 4) {
		setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
					    num_tbos, num_images, num_imgbos);

		double rate = perf_run(call, num_vbos, num_ubos, num_textures,
				       num_tbos, num_images, num_imgbos,
				       "no state", draw, base_rate);
		if (num_vbos == 1)
			base_rate = rate;
	}

	num_vbos = 1;
	num_ubos = 0;
	num_textures = 16;
	setup_shaders_and_resources(num_vbos, num_ubos, num_textures, num_tbos,
				    num_images, num_imgbos);
	perf_run(call, num_vbos, num_ubos, num_textures, num_tbos, num_images,
		 num_imgbos, "no state", draw, base_rate);

	/* Test state changes. */
	num_ubos = 8;
	num_textures = 8;
	for (num_vbos = 1; num_vbos <= 16; num_vbos *= 16) {
		setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
					    num_tbos, num_images, num_imgbos);

		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "no state", draw, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "shader program", draw_shader_change, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "vertex attrib", draw_vertex_attrib_change, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "1 texture", draw_one_texture_change, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "8 textures", draw_many_texture_change, base_rate);

		if (!is_compat) {
			num_textures = 0;

			num_tbos = 8;
			setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
						    num_tbos, num_images, num_imgbos);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "1 TBO", draw_one_tbo_change, base_rate);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "8 TBOs", draw_many_tbo_change, base_rate);
			num_tbos = 0;

			num_images = 8;
			setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
						    num_tbos, num_images, num_imgbos);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "1 image", draw_one_img_change, base_rate);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "8 images", draw_many_img_change, base_rate);
			num_images = 0;

			num_imgbos = 8;
			setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
						    num_tbos, num_images, num_imgbos);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "1 image buffer", draw_one_imgbo_change, base_rate);
			perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
				 num_images, num_imgbos,
				 "8 image buffers", draw_many_imgbo_change, base_rate);
			num_imgbos = 0;

			num_textures = 8;
			num_tbos = 0;
			setup_shaders_and_resources(num_vbos, num_ubos, num_textures,
						    num_tbos, num_images, num_imgbos);
		}

		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "1 UBO", draw_one_ubo_change, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "8 UBOs", draw_many_ubo_change, base_rate);

		glUseProgram(prog[0]);
		uniform_loc = glGetUniformLocation(prog[0], "u");
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "few uniforms / 1", draw_uniform_change, base_rate);

		glUseProgram(prog[1]);
		uniform_loc = glGetUniformLocation(prog[1], "u");
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "many uniforms / 1", draw_uniform_change, base_rate);
		glUseProgram(prog[0]);

		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "scissor", draw_scissor_change, base_rate);
		perf_run(call, num_vbos, num_ubos, num_textures, num_tbos,
			 num_images, num_imgbos,
			 "viewport", draw_viewport_change, base_rate);

		for (int state = 0; state < ARRAY_SIZE(enable_states); state++) {
			enable_enum = enable_states[state].enable;
			perf_run(call, num_vbos, num_ubos, num_textures,
				 num_tbos, num_images, num_imgbos,
				 enable_states[state].name,
				 draw_state_change, base_rate);
		}
	}
}

/** Called from test harness/main */
enum piglit_result
piglit_display(void)
{
	perf_draw_variant("DrawElements", true);
	perf_draw_variant("DrawArrays", false);

	exit(0);
	return PIGLIT_SKIP;
}
