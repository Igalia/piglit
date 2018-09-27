/*
 * Copyright (C) 2018 Intel Corporation
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
 */

/** @file fbo-blit-check-limits.c
 *
 * Test FBO blits with different possible buffer sizes
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=108088
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=110239
 *
 * \author Vadym Shovkoplias <vadym.shovkoplias@globallogic.com>
 * \author Sergii Romantsov <sergii.romantsov@globallogic.com>
 */

#include "piglit-util-gl.h"

#define FB_SIZE 160

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.requires_displayed_window = true;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_width = FB_SIZE;
	config.window_height = FB_SIZE;

PIGLIT_GL_TEST_CONFIG_END


typedef struct _rgba {
	float r;
	float g;
	float b;
	float a;
} rgba;

// Yellow - back buffer left side color
#define BACK1_R 1.0f
#define BACK1_G 1.0f
#define BACK1_B 0.0f
static const rgba BACK1 = { BACK1_R, BACK1_G, BACK1_B, 1.0f };

// Green - back buffer right side color
#define BACK2_R 0.0f
#define BACK2_G 1.0f
#define BACK2_B 0.0f
static const rgba BACK2 = { BACK2_R, BACK2_G, BACK2_B, 1.0f };

// Red - front buffer color
#define FRONT_R 1.0f
#define FRONT_G 0.0f
#define FRONT_B 0.0f
static const rgba FRONT = { FRONT_R, FRONT_G, FRONT_B, 1.0f };

typedef struct _fb_data {
	int read;
	int write;
	rgba color_lb;
	rgba color_lt;
	rgba color_right;
} fb_data;

static const fb_data fb_params[] = {
	{ .read = 0, .write = 0,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = 0, .write = FB_SIZE,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = 0, .write = FB_SIZE << 1,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = 0, .write = INT_MAX,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},

	{ .read = 1, .write = FB_SIZE,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = 1, .write = INT_MAX,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},

	{ .read = FB_SIZE >> 2, .write = 0,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE >> 2, .write = FB_SIZE << 1,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE >> 2, .write = FB_SIZE,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE >> 2, .write = 0x7ffffff,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE >> 2, .write = INT_MAX,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},

	{ .read = FB_SIZE >> 1, .write = 1,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE >> 1, .write = 2,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE >> 1, .write = FB_SIZE >> 1,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE >> 1, .write = FB_SIZE,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE >> 1, .write = 0x7ffffff,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE >> 1, .write = INT_MAX,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},

	{ .read = FB_SIZE, .write = 0,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = 1,
		.color_lb  = { BACK2_R, BACK2_G, BACK2_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = 2,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = FB_SIZE >> 1,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = FB_SIZE,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK2_R, BACK2_G, BACK2_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = 0x7ffffff,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},
	{ .read = FB_SIZE, .write = INT_MAX,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
	},

	{ .read = INT_MIN, .write = 1,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = INT_MIN, .write = 2,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = INT_MIN, .write = INT_MIN,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = INT_MIN, .write = 0x7ffffff,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},
	{ .read = INT_MIN, .write = INT_MAX,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},

	{ .read = -FB_SIZE, .write = -FB_SIZE,
		.color_lb  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_lt  = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
		.color_right = { FRONT_R, FRONT_G, FRONT_B, 1.0f },
	},

	{ .read = INT_MAX, .write = INT_MAX,
		.color_lb  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_lt  = { BACK1_R, BACK1_G, BACK1_B, 1.0f },
		.color_right = { BACK2_R, BACK2_G, BACK2_B, 1.0f },
	},
};

static bool draw(const fb_data *const sizes)
{
	bool success = 1;

	glDrawBuffer(GL_BACK);
	/* back buffer right side green */
	glClearColor(BACK2.r, BACK2.g, BACK2.b, BACK2.a);
	glClear(GL_COLOR_BUFFER_BIT);

	/* back buffer left side yellow */
	glColor4f(BACK1.r, BACK1.g, BACK1.b, BACK1.a);
	piglit_draw_rect(-1.0, -1.0, 1.0, 2.0);

        glDrawBuffer(GL_FRONT);
	/* front buffer red */
	glClearColor(FRONT.r, FRONT.g, FRONT.b, FRONT.a);
	glClear(GL_COLOR_BUFFER_BIT);

	glReadBuffer(GL_BACK);

	glBlitFramebufferEXT(0, 0, sizes->read, sizes->read,
			     0, 0, sizes->write, sizes->write,
			     GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glReadBuffer(GL_FRONT);

	success &= piglit_probe_pixel_rgb(0, 0, (float*)(&sizes->color_lb));
	success &= piglit_probe_pixel_rgb(FB_SIZE - 1, 0, (float*)(&sizes->color_right));
	success &= piglit_probe_pixel_rgb(0, FB_SIZE - 1, (float*)(&sizes->color_lt));
	success &= piglit_probe_pixel_rgb(FB_SIZE - 1, FB_SIZE - 1, (float*)(&sizes->color_right));

	if (!success)
		fprintf(stderr, "Failed blit src(0,0;%d,%d) - dst(0,0;%d,%d)\n",
			sizes->read, sizes->read, sizes->write, sizes->write);

	glFlush();
	return success;
}

enum piglit_result piglit_display(void)
{
	bool success = 1;
	for (int i = 0; i < sizeof(fb_params) / sizeof(fb_params[0]); ++i)
	{
		success &= draw(&fb_params[i]);
	}
	return success ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");
}
