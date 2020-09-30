/*
 * Copyright © 2020 Intel Corporation
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
 *
 * Author:
 *    Eleni Maria Stea <estea@igalia.com>
 *    Juan A. Suarez Romero <jasuarez@igalia.com>
 */

#include <piglit-util-gl.h>
#include "interop.h"
#include "params.h"
#include "helpers.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 30;
config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLenum gl_target = GL_TEXTURE_2D;

static struct vk_ctx vk_core;
static struct vk_image_props vk_img_props;
static struct vk_image_obj vk_img_obj;
static GLuint gl_mem_obj;
static GLuint gl_tex;
static GLuint gl_fbo;
static GLuint gl_rbo;

static int gl_prog_flt;
static int gl_prog_int;
static int gl_prog_uint;

static const char vs[] =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"in vec2 piglit_texcoord;\n"
	"out vec2 tex_coords;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    tex_coords = piglit_texcoord;\n" "}\n";

/* we want this shader to work with most color formats
 * so we subtract the expected and the sampled color
 * if the colors match, all the components will be 0
 * and their sum (res.x) will be 0. We pass the negative
 * of the subtraction result to step that is going to
 * return 0 or non 0 depending on the result (edge = 0)
 * and we use mix to select between red if there's no match
 * and green if there's match for the output color */

#define MAKE_FS(SAMPLER, VEC4)				                \
	"#version 130\n"						\
	"in vec2 tex_coords;\n"						\
	"uniform " #SAMPLER " tex; \n"					\
	"uniform " #VEC4 " expected_color;\n"                           \
	"out vec4 color;\n"						\
	"void main() \n"                                                \
	"{\n"                                                           \
	"    " #VEC4 " sampled_color = texture(tex, tex_coords);\n"     \
	"    " #VEC4 " res = " #VEC4 " (abs(expected_color - sampled_color));\n"    \
	"    res.x += res.y + res.z + res.w;\n"                         \
	"    color = mix(vec4(1.0, 0.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), step(0, -float(res.x)));\n" \
	"}\n"

static const char *fs[] = {
	MAKE_FS(sampler2D, vec4),
	MAKE_FS(isampler2D, ivec4),
	MAKE_FS(usampler2D, uvec4),
};

#undef MAKE_FS

static float exp_color_flt[4];
static int exp_color_int[4];
static unsigned int exp_color_uint[4];

static enum piglit_result
run_subtest(int case_num);

static bool
vk_init(void);

static bool
gl_init(void);

static bool
vk_set_image_props(uint32_t w, uint32_t h,
		   uint32_t depth,
		   uint32_t num_samples,
		   uint32_t num_levels, VkFormat format,
		   VkImageTiling tiling, VkImageUsageFlagBits usage);

static bool
gl_draw_texture(enum fragment_type fs_type, uint32_t w, uint32_t h);

static void
gl_cleanup(void);

static void
cleanup(void);

void
piglit_init(int argc, char **argv)
{
	/* From the EXT_external_objects spec:
	 *
	 *   "GL_EXT_memory_object requires ARB_texture_storage or a
	 *   version of OpenGL or OpenGL ES that incorporates it."
	 */
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_EXT_memory_object");
	piglit_require_extension("GL_EXT_memory_object_fd");

	atexit(cleanup);

	if (!vk_init())
		piglit_report_result(PIGLIT_SKIP);

	if (!gl_init())
		piglit_report_result(PIGLIT_SKIP);
}

enum piglit_result
piglit_display()
{
	enum piglit_result piglit_test_state = PIGLIT_SKIP;
	enum piglit_result piglit_subtest_state;
	int c;

	for (c = 0; c < ARRAY_SIZE(vk_gl_format); c++) {
		piglit_subtest_state = run_subtest(c);
		piglit_merge_result(&piglit_test_state, piglit_subtest_state);
	}

	return piglit_test_state;
}

/* static functions */

static enum piglit_result
run_subtest(int case_num)
{
	bool result = false;
	enum piglit_result subtest_result;
	const float color_prb[] = { 0.0, 1.0, 0.0, 1.0 };
	GLint loc = -1;

	/* We don't set the usage flags as the purpose of this test is to test different formats
	 * We will check different combinations of usage/tiling mode in another test */
	if (!vk_set_image_props(piglit_width, piglit_height, d,
				num_samples, num_levels,
			        vk_gl_format[case_num].vkformat,
				vk_gl_format[case_num].tiling, 0)) {
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "%s: Unsupported image format.",
					     vk_gl_format[case_num].name);
		return PIGLIT_SKIP;
	}

	if (!vk_create_ext_image(&vk_core, &vk_img_props, &vk_img_obj)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "%s: Failed to create external Vulkan image.",
					     vk_gl_format[case_num].name);
		return PIGLIT_FAIL;
	}

	/* call function that generates a texture in image props */
	if (!gl_create_mem_obj_from_vk_mem(&vk_core, &vk_img_obj.mobj,
					   &gl_mem_obj)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "%s: Failed to create GL memory object from Vulkan memory.",
					     vk_gl_format[case_num].name);
		vk_destroy_ext_image(&vk_core, &vk_img_obj);
		return PIGLIT_FAIL;
	}

	if (!gl_gen_tex_from_mem_obj(&vk_img_props, vk_gl_format[case_num].glformat,
				     gl_mem_obj, 0,
				     &gl_tex)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "%s: Failed to create texture from GL memory object.",
					     vk_gl_format[case_num].name);
		vk_destroy_ext_image(&vk_core, &vk_img_obj);
		gl_cleanup();
		return PIGLIT_FAIL;
	}

	if (!gl_draw_texture(vk_gl_format[case_num].fs_type, vk_img_props.w, vk_img_props.h)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
					     "%s: Failed to initialize OpenGL FBO/RBO",
					     vk_gl_format[case_num].name);
		vk_destroy_ext_image(&vk_core, &vk_img_obj);
		gl_cleanup();
		return PIGLIT_FAIL;
	}

	switch(vk_gl_format[case_num].fs_type) {
	case INT_FS:
	{
		glUseProgram(gl_prog_int);

		if ((loc = glGetUniformLocation(gl_prog_int, "expected_color")) == -1) {
			fprintf(stderr, "Failed to get int expected color location.\n");
			vk_destroy_ext_image(&vk_core, &vk_img_obj);
			return PIGLIT_FAIL;
		}

		exp_color_int[0] = pow(2, vk_gl_format[case_num].rbits) / 2 - 1;
		exp_color_int[1] = pow(2, vk_gl_format[case_num].gbits) / 2 - 1;
		exp_color_int[2] = 0;
		exp_color_int[3] = pow(2, vk_gl_format[case_num].abits) / 2 - 1;

		glUniform4iv(loc, 1, exp_color_int);
	} break;
	case UINT_FS:
	{
		glUseProgram(gl_prog_uint);

		if ((loc = glGetUniformLocation(gl_prog_uint, "expected_color")) == -1) {
			fprintf(stderr, "Failed to get uint expected color location.\n");
			vk_destroy_ext_image(&vk_core, &vk_img_obj);
			return PIGLIT_FAIL;
		}

		exp_color_uint[0] = pow(2, vk_gl_format[case_num].rbits) - 1;
		exp_color_uint[1] = pow(2, vk_gl_format[case_num].gbits) - 1;
		exp_color_uint[2] = 0;
		exp_color_uint[3] = pow(2, vk_gl_format[case_num].abits) - 1;

		glUniform4uiv(loc, 1, exp_color_uint);
	} break;
	case FLOAT_FS:
	{
		glUseProgram(gl_prog_flt);

		if ((loc = glGetUniformLocation(gl_prog_flt, "expected_color")) == -1) {
			fprintf(stderr, "Failed to get float expected color location.\n");
			vk_destroy_ext_image(&vk_core, &vk_img_obj);
			return PIGLIT_FAIL;
		}

		exp_color_flt[0] = 1.0;
		exp_color_flt[1] = 1.0;
		exp_color_flt[2] = 0.0;
		exp_color_flt[3] = 1.0;

		glUniform4fv(loc, 1, exp_color_flt);
	} break;
	default:
		fprintf(stderr, "Invalid format. Shouldn't reach.\n");
		vk_destroy_ext_image(&vk_core, &vk_img_obj);
		gl_cleanup();
		return PIGLIT_FAIL;
	};


	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(gl_target, gl_tex);

	piglit_draw_rect_tex(-1, -1,
			     2.0 * vk_img_props.w / piglit_width,
			     2.0 * vk_img_props.h / piglit_height,
			     0, 0, 1, 1);

	result = piglit_probe_pixel_rgba((float)piglit_width / 2.0,
					 (float)piglit_height / 2.0,
					 color_prb);

	subtest_result = result ? PIGLIT_PASS : PIGLIT_FAIL;

	piglit_report_subtest_result(subtest_result, "%s", vk_gl_format[case_num].name);

	piglit_present_results();

	vk_destroy_ext_image(&vk_core, &vk_img_obj);
	gl_cleanup();

	return subtest_result;
}

static bool
vk_init(void)
{
	if (!vk_init_ctx(&vk_core)) {
		fprintf(stderr, "Failed to initialize Vulkan\n");
		return false;
	}

	if (!vk_check_gl_compatibility(&vk_core)) {
		fprintf(stderr, "Mismatch in driver/device UUID\n");
		return false;
	}

	return true;
}

static bool
vk_set_image_props(uint32_t w, uint32_t h, uint32_t d,
		   uint32_t num_samples, uint32_t num_levels,
		   VkFormat format, VkImageTiling tiling,
		   VkImageUsageFlagBits usage)
{
	VkImageLayout in_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	VkImageLayout end_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	uint32_t num_layers = 1;

	return vk_fill_ext_image_props(&vk_core, w, h, d,
				       num_samples, num_levels,
				       num_layers,
				       format, tiling, usage,
				       in_layout, end_layout,
				       true, &vk_img_props);
}

static bool
gl_draw_texture(enum fragment_type fs_type, uint32_t w, uint32_t h)
{
	glBindTexture(gl_target, gl_tex);

	glBindFramebuffer(GL_FRAMEBUFFER, gl_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gl_rbo);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
			      w, h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
				  GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_RENDERBUFFER, gl_rbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       gl_target, gl_tex, 0);

	if (!check_bound_fbo_status())
		return false;

	glClearColor(1.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	return glGetError() == GL_NO_ERROR;
}

static void
gl_cleanup(void)
{
	glBindTexture(gl_get_target(&vk_img_props), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(0);

	glDeleteTextures(1, &gl_tex);
	glDeleteMemoryObjectsEXT(1, &gl_mem_obj);
}

static void
cleanup(void)
{
	vk_cleanup_ctx(&vk_core);
	gl_cleanup();

	glDeleteRenderbuffers(1, &gl_rbo);
	glDeleteFramebuffers(1, &gl_fbo);

	glDeleteProgram(gl_prog_flt);
	glDeleteProgram(gl_prog_int);
	glDeleteProgram(gl_prog_uint);
}

static bool
gl_init(void)
{
	gl_prog_flt = piglit_build_simple_program(vs, fs[0]);
	gl_prog_int = piglit_build_simple_program(vs, fs[1]);
	gl_prog_uint = piglit_build_simple_program(vs, fs[2]);

	glGenFramebuffers(1, &gl_fbo);
	glGenRenderbuffers(1, &gl_rbo);

	glUseProgram(0);

	return glGetError() == GL_NO_ERROR;
}
