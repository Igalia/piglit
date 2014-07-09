/* Copyright © 2014 Advanced Micro Devices, Inc.
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
 * Verify that glDrawBuffers with one or several of the drawbuffers set
 * to GL_NONE works as expected. Also test a random order of
 * GL_COLOR_ATTACHMENTi enums (not necessarily starting from i=0).
 *
 * For such drawbuffer configurations, test the following:
 * - glClear
 * - glClearBuffer
 * - drawing with gl_FragColor being written
 * - drawing with gl_FragData being written
 * - per-drawbuffer colormasks
 * - per-drawbuffer blend functions
 * - glDrawPixels
 * - glBlitFramebuffer (the read buffer is an FBO with one color attachment)
 *
 * The fragment shader writes gl_FragData[0..3] or gl_FragColor.
 *
 * \author Marek Olšák <marek.olsak@amd.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define FB_SIZE 32

static const GLenum drawbuf_config[][4] = {
	/* All 4 color attachments are used */
	{GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT0},
	/* NONE appears once */
	{GL_NONE, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT2},
	{GL_COLOR_ATTACHMENT1, GL_NONE, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT0},
	{GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT3, GL_NONE, GL_COLOR_ATTACHMENT2},
	{GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3, GL_NONE},
	/* NONE appears twice */
	{GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT2},
	{GL_NONE, GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT1},
	{GL_NONE, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT0, GL_NONE},
	{GL_COLOR_ATTACHMENT3, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT1},
	{GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT1, GL_NONE},
	{GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE},
	/* NONE appears three times */
	{GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT1},
	{GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT2, GL_NONE},
	{GL_NONE, GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE},
	{GL_COLOR_ATTACHMENT3, GL_NONE, GL_NONE, GL_NONE},
};

#define CLEAR_COLOR 0.2
static const float color_initial[4] = {CLEAR_COLOR, CLEAR_COLOR, CLEAR_COLOR, CLEAR_COLOR};
static const float color_red[4] = {1, 0, 0, 1};
static const float color_green[4] = {0, 1, 0, 1};
static const float color_blue[4] = {0, 0, 1, 1};
static const float color_yellow[4] = {1, 1, 0, 1};

static const float *colors_all_red[] = {
	color_red,
	color_red,
	color_red,
	color_red
};

static const float *colors_all_different[] = {
	color_red,
	color_green,
	color_blue,
	color_yellow,
};

static const char *vs =
	"void main() \n"
	"{ \n"
	"   gl_Position = gl_Vertex; \n"
	"}\n";

static const char *fs_write_red =
	"void main() \n"
	"{ \n"
	"   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
	"}\n";

static const char *fs_template_write_different =
	"%s \n"
	"#define OUTVAR %s \n"
	"void main() \n"
	"{ \n"
	"   OUTVAR[0] = vec4(1.0, 0.0, 0.0, 1.0); \n"
	"   OUTVAR[1] = vec4(0.0, 1.0, 0.0, 1.0); \n"
	"   OUTVAR[2] = vec4(0.0, 0.0, 1.0, 1.0); \n"
	"   OUTVAR[3] = vec4(1.0, 1.0, 0.0, 1.0); \n"
	"}\n";

static char *fs_write_different;
static char *test_name, *prog_name;
static GLuint fb, prog_write_all_red, prog_write_all_different;


static void
create_shaders(void)
{
	bool fs_uses_out_variables = streq(test_name, "use_frag_out");

	prog_write_all_red = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs,
				GL_FRAGMENT_SHADER, fs_write_red,
				0);

	asprintf(&fs_write_different, fs_template_write_different,
		 fs_uses_out_variables ?
		 "#version 130 \nout vec4[4] color;" : "",
		 fs_uses_out_variables ? "color" : "gl_FragData");

	prog_write_all_different = piglit_build_simple_program_multiple_shaders(
				GL_VERTEX_SHADER, vs,
				GL_FRAGMENT_SHADER, fs_write_different,
				0);
}

static void
create_and_bind_fbo(void)
{
	GLuint rb[4];
	GLuint i;
	GLenum status;

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(4, rb);

	/* Add 4 color attachments. */
	for (i = 0; i < 4; i++) {
		glBindRenderbuffer(GL_RENDERBUFFER, rb[i]);
		/* Buffer storage is allocated based on render buffer format */
		glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, FB_SIZE, FB_SIZE);
		/* Attach the render buffer to a color attachment */
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0 + i,
					  GL_RENDERBUFFER, rb[i]);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"Framebuffer with color"
			"attachment was not complete: 0x%04x\n",
			status);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glViewport(0, 0, FB_SIZE, FB_SIZE);
}

static void
clear_all_attachments_to_initial_value(void)
{
	static const GLenum drawbufs[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3
	};

	glDrawBuffers(4, drawbufs);
	glClearColor(color_initial[0], color_initial[1], color_initial[2], color_initial[3]);
	glClear(GL_COLOR_BUFFER_BIT);
}

static bool
probe_buffers(const GLenum drawbufs[4],
              const float *expected_color[4])
{
	int i, j;
	bool pass = true;

	for (i = 0; i < 4; i++) {
		const float *expected = color_initial;

		/* Set the expected color if the color attachment is bound
		 * for drawing.
		 */
		for (j = 0; j < 4; j++) {
			if (GL_COLOR_ATTACHMENT0 + i == drawbufs[j]) {
				expected = expected_color[j];
				break;
			}
		}

		glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
		if (!piglit_probe_rect_rgba(0, 0, FB_SIZE, FB_SIZE,
					    expected)) {
			printf("  from color attachment %i,\n"
			       "  config (", i);

			for (j = 0; j < 4; j++) {
				if (drawbufs[j] == GL_NONE) {
					printf("NONE");
				}
				else {
					printf("COLOR_ATTACHMENT%i",
					       drawbufs[j] - GL_COLOR_ATTACHMENT0);
				}
				printf(j < 3 ? ", " : ")\n");
			}
			pass = false;
			continue;
		}
	}
	return pass;
}

static bool
test_glClear(const GLenum drawbufs[4])
{
	glClearColor(color_red[0], color_red[1], color_red[2], color_red[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	return probe_buffers(drawbufs, colors_all_red);
}

static bool
test_glClearBuffer(const GLenum drawbufs[4])
{
	int i;

	for (i = 0; i < 4; i++)
		glClearBufferfv(GL_COLOR, i, colors_all_different[i]);

	return probe_buffers(drawbufs, colors_all_different);
}

static bool
test_fragcolor(const GLenum drawbufs[4])
{
	glUseProgram(prog_write_all_red);
	piglit_draw_rect(-1, -1, 2, 2);
	glUseProgram(0);

	return probe_buffers(drawbufs, colors_all_red);
}

static bool
test_fragout(const GLenum drawbufs[4])
{
	glUseProgram(prog_write_all_different);
	piglit_draw_rect(-1, -1, 2, 2);
	glUseProgram(0);

	return probe_buffers(drawbufs, colors_all_different);
}

static bool
test_fragdata(const GLenum drawbufs[4])
{
	glUseProgram(prog_write_all_different);
	piglit_draw_rect(-1, -1, 2, 2);
	glUseProgram(0);

	return probe_buffers(drawbufs, colors_all_different);
}

static bool
test_glColorMaskIndexed(const GLenum drawbufs[4])
{
	static const float color_masked_red[4] = {1, 0, CLEAR_COLOR, CLEAR_COLOR};
	static const float color_masked_green[4] = {CLEAR_COLOR, 1, 0, CLEAR_COLOR};
	static const float color_masked_blue[4] = {CLEAR_COLOR, CLEAR_COLOR, 1, 1};
	static const float color_masked_yellow[4] = {1, 1, CLEAR_COLOR, 1};
	static const float *colors_masked[] = {
		color_masked_red,
		color_masked_green,
		color_masked_blue,
		color_masked_yellow,
	};

	glColorMaskIndexedEXT(0, 1, 1, 0, 0);
	glColorMaskIndexedEXT(1, 0, 1, 1, 0);
	glColorMaskIndexedEXT(2, 0, 0, 1, 1);
	glColorMaskIndexedEXT(3, 1, 1, 0, 1);

	glUseProgram(prog_write_all_different);
	piglit_draw_rect(-1, -1, 2, 2);
	glUseProgram(0);

	glColorMask(1, 1, 1, 1);

	return probe_buffers(drawbufs, colors_masked);
}

static bool
test_glBlendFunci(const GLenum drawbufs[4])
{
	static const float color_blended_red[4] = {1, CLEAR_COLOR, CLEAR_COLOR, 1};
	static const float color_blended_green[4] = {0, CLEAR_COLOR, 0, CLEAR_COLOR};
	static const float color_blended_blue[4] = {0, 0, CLEAR_COLOR*2, CLEAR_COLOR*2};
	static const float color_blended_yellow[4] = {1-CLEAR_COLOR, 1-CLEAR_COLOR, 0, 1-CLEAR_COLOR};
	static const float *colors_blended[] = {
		color_blended_red,
		color_blended_green,
		color_blended_blue,
		color_blended_yellow,
	};

	glEnable(GL_BLEND);

	glBlendFunciARB(0, GL_ONE, GL_ONE);
	glBlendFunciARB(1, GL_DST_COLOR, GL_ZERO);
	glBlendFunciARB(2, GL_DST_COLOR, GL_SRC_COLOR);
	glBlendFunciARB(3, GL_ONE_MINUS_DST_COLOR, GL_ZERO);

	glUseProgram(prog_write_all_different);
	piglit_draw_rect(-1, -1, 2, 2);
	glUseProgram(0);

	glDisable(GL_BLEND);

	return probe_buffers(drawbufs, colors_blended);
}

static bool
test_glDrawPixels(const GLenum drawbufs[4])
{
	unsigned char pixels[FB_SIZE*FB_SIZE];

	memset(pixels, 0xff, sizeof(pixels));

	glUseProgram(prog_write_all_red);
	glDrawPixels(FB_SIZE, FB_SIZE, GL_RED, GL_UNSIGNED_BYTE, pixels);
	glUseProgram(0);

	return probe_buffers(drawbufs, colors_all_red);
}

static bool
test_glBlitFramebuffer(const GLenum drawbufs[4])
{
	GLuint rb, readfb;
	GLenum status;

	/* Create a new renderbuffer and attach it to a new FBO. */
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, FB_SIZE, FB_SIZE);

	glGenFramebuffers(1, &readfb);
	glBindFramebuffer(GL_FRAMEBUFFER, readfb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"Framebuffer with color"
			"attachment was not complete: 0x%04x\n",
			status);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Clear the renderbuffer to red. */
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Blit the renderbuffer to our FBO with MRT. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glBlitFramebuffer(0, 0, FB_SIZE, FB_SIZE, 0, 0, FB_SIZE, FB_SIZE,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	return probe_buffers(drawbufs, colors_all_red);
}

static void
print_usage_and_exit(void)
{
	printf("Usage: %s <test_name>\n"
	       "  where <test_name> is one of:\n"
	       "    glClear\n"
	       "    glClearBuffer\n"
	       "    gl_FragColor\n"
	       "    gl_FragData\n"
	       "    use_frag_out\n"
	       "    glColorMaskIndexed\n"
	       "    glBlendFunci\n"
	       "    glDrawPixels\n"
	       "    glBlitFramebuffer\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	int max_draw_bufs;

	prog_name = malloc((strlen(argv[0]) + 1));
	assert(prog_name);
	strcpy(prog_name, argv[0]);

	if (argc != 2)
		print_usage_and_exit();

	test_name = malloc((strlen(argv[1]) + 1));
	assert(test_name);
	strcpy(test_name, argv[1]);

	piglit_require_gl_version(21);
	piglit_require_extension("GL_ARB_framebuffer_object");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &max_draw_bufs);
	if (max_draw_bufs < 4) {
		puts("At least 4 draw buffers are required.");
		piglit_report_result(PIGLIT_SKIP);
	}

	create_shaders();
	create_and_bind_fbo();
}

enum piglit_result
piglit_display(void)
{
	int i;
	bool pass = true;
	printf("Testing %s.\n", test_name);

	for (i = 0; i < ARRAY_SIZE(drawbuf_config); i++) {
		clear_all_attachments_to_initial_value();

		glDrawBuffers(4, drawbuf_config[i]);

		if (streq(test_name, "glClear")) {
			pass = test_glClear(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "glClearBuffer")) {
			piglit_require_gl_version(30);
			pass = test_glClearBuffer(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "gl_FragColor")) {
			pass = test_fragcolor(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "gl_FragData")) {
			pass = test_fragdata(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "use_frag_out")) {
			piglit_require_GLSL_version(130);
			pass = test_fragout(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "glColorMaskIndexed")) {
			piglit_require_extension("GL_EXT_draw_buffers2");
			pass = test_glColorMaskIndexed(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "glBlendFunci")) {
			piglit_require_extension("GL_ARB_draw_buffers_blend");
			pass = test_glBlendFunci(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "glDrawPixels")) {
			pass = test_glDrawPixels(drawbuf_config[i]) && pass;
		}
		else if (streq(test_name, "glBlitFramebuffer")) {
			pass = test_glBlitFramebuffer(drawbuf_config[i]) && pass;
		}
		else {
			printf("Unknown subtest: %s\n", test_name);
			print_usage_and_exit();
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
