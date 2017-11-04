/*
 * Copyright © 2017 Ilia Mirkin <imirkin@alum.mit.edu>
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

/*
 * The purpose of this test is to find out what happens when the
 * shader produces a NaN value with a floating point RT. This is all
 * undefined as far as the GL spec goes, but it's useful to be able to
 * compare implementations.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 130 \n"
	"in vec4 piglit_vertex; \n"
	"void main() { \n"
	"  gl_Position = piglit_vertex; \n"
	"}\n";

static const char *fs_text =
	"#version 130 \n"
	"#extension GL_ARB_shader_bit_encoding: require \n"
	"out vec4 color; \n"
	"uniform uint c; \n"
	"uniform uint a; \n"
	"void main() { \n"
	"  color = vec4(vec3(uintBitsToFloat(c)), uintBitsToFloat(a)); \n"
	"}\n";

GLuint program, fb, rb;

static const unsigned uINF = 0x7f800000;
static const unsigned uNAN = 0x7fc00000;

bool is_nan(unsigned arg)
{
	return ((arg & 0x7f800000) == 0x7f800000) &&
		(arg & 0x007fffff);
}

void test_draw(unsigned u_c, unsigned u_a, unsigned r, unsigned g, unsigned b, unsigned a)
{
	unsigned pixel[4];

	glUniform1ui(glGetUniformLocation(program, "c"), u_c);
	glUniform1ui(glGetUniformLocation(program, "a"), u_a);
	piglit_draw_rect(-1, -1, 2, 2);
	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, pixel);

	bool rmatch = pixel[0] == r || (is_nan(pixel[0]) && is_nan(r));
	bool gmatch = pixel[1] == g || (is_nan(pixel[1]) && is_nan(g));
	bool bmatch = pixel[2] == b || (is_nan(pixel[2]) && is_nan(b));
	bool amatch = pixel[3] == a || (is_nan(pixel[3]) && is_nan(a));
	if (!rmatch || !gmatch || !bmatch || !amatch)
		printf("Unexpected result c=%x, a=%x: %x %x %x %x != %x %x %x %x\n",
		       u_c, u_a,
		       pixel[0], pixel[1], pixel[2], pixel[3],
		       r, g, b, a);
}

/* inf_x_zero: If a zero blend factor is multiplied with an infinity
 *             or nan [or vice versa], whether the resutl is nan or zero.
 *
 * blend_zero: If one uses a GL_ZERO factor, but the source is
 *             infinity or nan, what value does that become. i.e. does
 *             GL_ZERO always win, or should be multiplication be done
 *             per IEEE.
 */
void run_test(unsigned inf_x_zero, unsigned blend_zero)
{
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glClearColor(0, 0, 0, 0);

	printf("Testing without blending.\n");
	glDisable(GL_BLEND);
	test_draw(0, 0, 0, 0, 0, 0);
	test_draw(uINF, uINF, uINF, uINF, uINF, uINF);
	test_draw(uNAN, uNAN, uNAN, uNAN, uNAN, uNAN);

	printf("Testing with blending src * SRC_ALPHA + dst * ZERO.\n");
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(0, uNAN, inf_x_zero, inf_x_zero, inf_x_zero, uNAN);
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(uNAN, 0, inf_x_zero, inf_x_zero, inf_x_zero, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(0, uINF, inf_x_zero, inf_x_zero, inf_x_zero, uINF);
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(uINF, 0, inf_x_zero, inf_x_zero, inf_x_zero, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(uINF, uNAN, uNAN, uNAN, uNAN, uNAN); /* NaN * Inf = NaN */
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(uNAN, uINF, uNAN, uNAN, uNAN, uINF); /* NaN * Inf = NaN */
	/* No clear. Use the DST's nan/inf values to test against GL_ZERO */
	test_draw(0, 0, blend_zero, blend_zero, blend_zero, blend_zero);

	printf("Testing with blending src * DST_ALPHA + dst * ZERO.\n");
	glBlendFunc(GL_DST_ALPHA, GL_ZERO);
	/* Zero in DST_ALPHA */
	glClear(GL_COLOR_BUFFER_BIT);
	test_draw(uINF, uNAN, inf_x_zero, inf_x_zero, inf_x_zero, inf_x_zero);
	/* Get infinity into DST_ALPHA */
	glDisable(GL_BLEND);
	test_draw(0, uINF, 0, 0, 0, uINF);
	glEnable(GL_BLEND);
	test_draw(0, uINF, inf_x_zero, inf_x_zero, inf_x_zero, blend_zero ? blend_zero : uINF);

	/* Get NaN into DST_ALPHA */
	glDisable(GL_BLEND);
	test_draw(0, uNAN, 0, 0, 0, uNAN);
	glEnable(GL_BLEND);
	test_draw(0, uINF, inf_x_zero, inf_x_zero, inf_x_zero, uNAN);
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_shader_bit_encoding");
	program = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(program);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	unsigned zero_x_inf = 0, blend_zero = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-nan") == 0) {
			zero_x_inf = uNAN;
		} else if (strcmp(argv[i], "-blend_zero") == 0) {
			blend_zero = uNAN;
		}
	}

	printf("Testing GL_RGBA16F\n");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, 64, 64);
	run_test(zero_x_inf, blend_zero);

	printf("Testing GL_RGBA32F\n");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, 64, 64);
	run_test(zero_x_inf, blend_zero);

#if 0
	printf("Testing GL_RGBA8. Lots of failures expected.\n");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, 64, 64);
	run_test(0, 0);
#endif

	piglit_report_result(PIGLIT_PASS);
}
