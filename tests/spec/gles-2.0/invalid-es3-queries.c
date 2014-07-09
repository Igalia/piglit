/* Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

struct enums {
	const char *name;
	GLenum val;
};

static bool try(const struct enums *list, unsigned len) {
	GLint param;
	int i;
	for (i = 0; i < len; i++) {
		glGetIntegerv(list[i].val, &param);

		if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
			fprintf(stderr, "\t%s\n", list[i].name);
			return false;
		}
	}
	return true;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	/* ES 3 adds many new queries over ES 2. This test confirms that ES 2
	 * correctly rejects them with an INVALID_ENUM error.
	 */
	static const struct enums
	GL_ARB_ES3_compatibility_enums[] = {
		{ "GL_MAX_ELEMENT_INDEX", 0x8D6B },
	},
	GL_ARB_fragment_shader_enums[] = {
		{ "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS", 0x8B49 },
	},
	GL_ARB_framebuffer_object_enums[] = {
		{ "GL_MAX_SAMPLES", 0x8D57 },
	},
	GL_ARB_sync_enums[] = {
		{ "GL_MAX_SERVER_WAIT_TIMEOUT", 0x9111 },
	},
	GL_ARB_transform_feedback2_enums[] = {
		{ "GL_TRANSFORM_FEEDBACK_PAUSED", 0x8E23 },
		{ "GL_TRANSFORM_FEEDBACK_ACTIVE", 0x8E24 },
		{ "GL_TRANSFORM_FEEDBACK_BINDING", 0x8E25 },
	},
	GL_ARB_uniform_buffer_object_enums[] = {
		{ "GL_MAX_VERTEX_UNIFORM_BLOCKS", 0x8A2B },
		{ "GL_MAX_FRAGMENT_UNIFORM_BLOCKS", 0x8A2D },
		{ "GL_MAX_COMBINED_UNIFORM_BLOCKS", 0x8A2E },
		{ "GL_MAX_UNIFORM_BLOCK_SIZE", 0x8A30 },
		{ "GL_MAX_UNIFORM_BUFFER_BINDINGS", 0x8A2F },
		{ "GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS", 0x8A31 },
		{ "GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS", 0x8A33 },
		{ "GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT", 0x8A34 },
		{ "GL_UNIFORM_BUFFER_BINDING", 0x8A28 },
	},
	GL_ARB_vertex_shader_enums[] = {
		{ "GL_MAX_VERTEX_UNIFORM_COMPONENTS", 0x8B4A },
		{ "GL_MAX_VARYING_COMPONENTS", 0x8B4B },
	},
	GL_EXT_framebuffer_blit_enums[] = {
		{ "GL_READ_FRAMEBUFFER_BINDING", 0x8CAA },
	},
	GL_EXT_pixel_buffer_object_enums[] = {
		{ "GL_PIXEL_PACK_BUFFER_BINDING", 0x88ED },
		{ "GL_PIXEL_UNPACK_BUFFER_BINDING", 0x88EF },
	},
	GL_EXT_texture_lod_bias_enums[] = {
		{ "GL_MAX_TEXTURE_LOD_BIAS", 0x84FD },
	},
	GL_EXT_transform_feedback_enums[] = {
		{ "GL_TRANSFORM_FEEDBACK_BUFFER_BINDING", 0x8C8F },
		{ "GL_RASTERIZER_DISCARD", 0x8C89 },
		{ "GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS", 0x8C8A },
		{ "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS", 0x8C8B },
		{ "GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS", 0x8C80 },
	};

	if (piglit_get_gl_version() >= 30) {
		fprintf(stderr, "Test requires ES < 30\n");
		piglit_report_result(PIGLIT_SKIP);
	}

#define CHECK_QUERIES(extension)				\
	if (!piglit_is_extension_supported( #extension )) {	\
		pass = try(extension ## _enums,			\
			   ARRAY_SIZE(extension ## _enums));	\
	}

	CHECK_QUERIES(GL_ARB_ES3_compatibility);
	CHECK_QUERIES(GL_ARB_fragment_shader);
	CHECK_QUERIES(GL_ARB_framebuffer_object);
	CHECK_QUERIES(GL_ARB_sync);
	CHECK_QUERIES(GL_ARB_transform_feedback2);
	CHECK_QUERIES(GL_ARB_uniform_buffer_object);
	CHECK_QUERIES(GL_ARB_vertex_shader);
	CHECK_QUERIES(GL_EXT_framebuffer_blit);
	CHECK_QUERIES(GL_EXT_pixel_buffer_object);
	CHECK_QUERIES(GL_EXT_texture_lod_bias);
	CHECK_QUERIES(GL_EXT_transform_feedback);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
