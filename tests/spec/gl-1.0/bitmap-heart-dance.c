/*
 * Copyright (C) 2018 Laura Ekstrand
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
 * Test glBitmap in a methodical way using a series of heart shapes.
 * Heart shape is diagram A.2 from Garnstudio free sock pattern Heart Dance
 * (https://www.garnstudio.com/pattern.php?id=7440&cid=17).
 * Knitting color work is basically glBitmap for knits!
 *
 *       _ * _ _ _ * _ _        where  _ = 0
 *       * * * _ * * * _               * = 1
 *       * * * * * * * _
 *       * * * * * * * _
 *       _ * * * * * _ _
 *       _ _ * * * _ _ _
 *       _ _ _ * _ _ _ _
 *       _ _ _ _ _ _ _ _
 *
 * Or:                          Little end    Big end
 *       0 1 0 0 0 1 0 0         68   0x44     0x22
 *       1 1 1 0 1 1 1 0        238   0xEE     0x77
 *       1 1 1 1 1 1 1 0        254   0xFE     0xF7
 *       1 1 1 1 1 1 1 0        254   0xFE     0xF7
 *       0 1 1 1 1 1 0 0        124   0x7C     0xE3
 *       0 0 1 1 1 0 0 0         56   0x38     0xC2
 *       0 0 0 1 0 0 0 0         16   0x10     0x80
 *       0 0 0 0 0 0 0 0          0   0x00     0x00
 *
 * Laura Ekstrand
 * March 2018
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE |
			       PIGLIT_GL_VISUAL_RGBA;
	config.window_width = 340;
	config.window_height = 200;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const float      red[4] = {0.502, 0.082, 0.082, 1.0};
static const float   salmon[4] = {1.000, 0.353, 0.353, 1.0};
static const float     pink[4] = {0.945, 0.471, 0.639, 1.0};
static const float   orange[4] = {1.000, 0.286, 0.000, 1.0};
static const float ltorange[4] = {1.000, 0.514, 0.322, 1.0};
static const float   yellow[4] = {1.000, 0.871, 0.133, 1.0};
static GLubyte bitmap[8] = { 0x00, 0x10, 0x38, 0x7C,
			     0xFE, 0xFE, 0xEE, 0x44 };

static const char *fragShaderText =
	"#version 130 \n"
	"uniform vec4      red; \n"
	"uniform vec4   salmon; \n"
	"uniform vec4     pink; \n"
	"uniform vec4   orange; \n"
	"uniform vec4 ltorange; \n"
	"uniform vec4   yellow; \n"
	"uniform int     xorig; \n"
	"uniform int     yorig; \n"
	"uniform int    length; \n"
	"uniform int       ysp; \n"
	"uniform int    height; \n"
	"uniform int  heart[8]; \n"
	"\n"
	"void main() \n"
	"{ \n"
	"    float zoom = 1.0; \n"
	"    vec4 black = vec4(0.0, 0.0, 0.0, 1.0); \n"
	"    int xsp = ysp + 8;  // Must be > 8. \n"
	"    vec2 fragCoord = gl_FragCoord.xy; \n"
	"    if ((fragCoord.x < xorig) || (fragCoord.y < height + yorig) ||\n"
	"        (fragCoord.x > xorig + ((length - 1) * xsp) + 8) || \n"
	"        (fragCoord.y > height + yorig + (5*ysp) + 8)) { \n"
	"        gl_FragColor = black; \n"
	"        return; \n"
	"    } \n"
	"    fragCoord = fragCoord/zoom; \n"
	"    int i = int(fragCoord.y - yorig - height) % ysp; \n"
	"    int pointmask = i < 8 ? heart[i] : 0; \n"
	"    int j = int(fragCoord.x - xorig) % xsp; \n"
	"    if (j > 8) { \n"
	"      j = 0; \n"
	"    } \n"
	"    for (int r = 0; r < j; r++) { \n"
	"      pointmask = pointmask/2; //left shift. \n"
	"    } \n"
	"    if (pointmask % 2 == 1) { \n"
	"        int c = (int(fragCoord.y - height - yorig) / ysp) % 6; \n"
	"        switch (c) { \n"
	"           case 0: \n"
	"             gl_FragColor = yellow; \n"
	"             break; \n"
	"           case 1: \n"
	"             gl_FragColor = ltorange; \n"
	"             break; \n"
	"           case 2: \n"
	"             gl_FragColor = orange; \n"
	"             break; \n"
	"           case 3: \n"
	"             gl_FragColor = pink; \n"
	"             break; \n"
	"           case 4: \n"
	"             gl_FragColor = salmon; \n"
	"             break; \n"
	"           case 5: \n"
	"             gl_FragColor = red; \n"
	"             break; \n"
	"        } \n"
	"    } else { \n"
	"        gl_FragColor = black; \n"
	"    } \n"
	"} \n";


static void
draw_row(const float* color, int length,
	 int x, int y, int spacex) {
	glColor4fv(color);
	glRasterPos2f(x, y);
	for (int i = 0; i < length; i++) {
		/* A line of hearts. */
		glBitmap(8, 8, 0, 0, 8 + spacex, 0, bitmap);
	}
}

static GLuint fragShader, program;

enum piglit_result
piglit_display(void)
{
	/* Draw with glBitmap */
	bool pass = true;
	int length = 17;
	int x = 20;
	int y = 30;
	int spacing = 10;

	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_TRUE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClear(GL_COLOR_BUFFER_BIT);
	draw_row(     red, length, x, y + 5*spacing, spacing);
	draw_row(  salmon, length, x, y + 4*spacing, spacing);
	draw_row(    pink, length, x, y + 3*spacing, spacing);
	draw_row(  orange, length, x, y + 2*spacing, spacing);
	draw_row(ltorange, length, x, y + 1*spacing, spacing);
	draw_row(  yellow, length, x, y + 0*spacing, spacing);


	/*
	 * Upload heart pattern. glBitmap is a bit mysterious in its bit
	 * interpretation, and GLSL doesn't have ubyte.
	 */
	int heart[8];
	for (int i = 0; i < 8; i++) {
		heart[i] = (int) bitmap[i];
	}
	glUniform1iv(glGetUniformLocation(program, "heart"), 8, heart);

	/* Load Colors. */
	glUniform4fv(glGetUniformLocation(program, "red"),      1, red);
	glUniform4fv(glGetUniformLocation(program, "salmon"),   1, salmon);
	glUniform4fv(glGetUniformLocation(program, "pink"),     1, pink);
	glUniform4fv(glGetUniformLocation(program, "orange"),   1, orange);
	glUniform4fv(glGetUniformLocation(program, "ltorange"), 1, ltorange);
	glUniform4fv(glGetUniformLocation(program, "yellow"),   1, yellow);

	/* Load spacing. */
	glUniform1i(glGetUniformLocation(program, "xorig"), x);
	glUniform1i(glGetUniformLocation(program, "yorig"), y);
	glUniform1i(glGetUniformLocation(program, "length"), length);
	glUniform1i(glGetUniformLocation(program, "ysp"), spacing);

	/* Draw shader in top half. */
	glUniform1i(glGetUniformLocation(program, "height"), piglit_height/2);
	piglit_draw_rect(0, piglit_height/2, piglit_width, piglit_height/2);

	piglit_present_results();

	pass = piglit_probe_rects_equal(0, 0, 0, piglit_height/2,
		piglit_width, piglit_height/2, GL_RGB);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	fragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						fragShaderText);
	program = piglit_link_simple_program(0, fragShader);
	glUseProgram(program);
}
