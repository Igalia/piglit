/*
 * Copyright © 2020 Google LLC
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
 * Tests rendering with GL_ATI_fragment_shader:
 * - various data sources for calculations in fragment shader
 *   - texture coordinates
 *   - texture sample
 *   - constant
 *   - primary color
 *   - secondary interpolator
 *   - one, zero
 * - switch between named fragment shaders
 * - use undefined default shader (rendering is undefined but shouldn't crash)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define C0R 0.1
#define C0G 0.2
#define C0B 0.3
#define C0A 0.4

#define C1R 0.5
#define C1G 0.6
#define C1B 0.7
#define C1A 0.8

#define C2R 1.0
#define C2G 0.5
#define C2B 1.5
#define C2A -0.5

struct atifs_op_test {
	GLenum op;
	uint32_t dstMask, dstMod;
	uint32_t arg0c, arg0rep, arg0mod;
	uint32_t arg1c, arg1rep, arg1mod;
	uint32_t arg2c, arg2rep, arg2mod;
	const char *name;
	const char *glsl;
};

#define SCALAR_RESULT(x) { (x), (x), (x), (x) }

static struct atifs_op_test rgba_tests[] = {
    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c0", "gl_FragColor = c[0];" },
    { GL_ADD_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "add c0, c1", "gl_FragColor = c[0] + c[1];" },
    { GL_MUL_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mul c0, c1", "gl_FragColor = c[0] * c[1];" },
    { GL_SUB_ATI, GL_NONE, GL_NONE,
     1, 0, 0,
     0, 0, 0,
     2, 0, 0,
     "sub c1, c0", "gl_FragColor = c[1] - c[0];" },
    { GL_DOT3_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "dot3 c0, c1", "gl_FragColor = vec4(dot(c[0].xyz, c[1].xyz));" },
    { GL_DOT4_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "dot4 c0, c1", "gl_FragColor = vec4(dot(c[0], c[1]));" },
    { GL_MAD_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mad c0, c1, c2", "gl_FragColor = c[0] * c[1] + c[2];" },
    { GL_LERP_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "lerp c0, c1, c2", "gl_FragColor = (c[0] * c[1]) + ((1.0 - c[0]) * c[2]);" },

    { GL_CND_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "cnd c0, c1, c2", "gl_FragColor = mix(c[0], c[1], step(c[2], vec4(0.5)));" },
    { GL_CND_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, GL_NEGATE_BIT_ATI,
     "cnd c0, c1, -c2", "gl_FragColor = mix(c[0], c[1], step(-c[2], vec4(0.5)));" },

    { GL_CND0_ATI, GL_NONE, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "cnd0 c0, c1, c2", "gl_FragColor = mix(c[1], c[0], step(0.0, c[2]));" },

    { GL_DOT2_ADD_ATI, GL_NONE, GL_NONE,
     1, 0, 0,
     2, 0, 0,
     0, 0, 0,
     "dot2_add c1, c2, c0", "gl_FragColor = vec4(c[1].r * c[2].r + c[1].g * c[2].g + c[0].b);" },

    { GL_MOV_ATI, GL_RED_BIT_ATI, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov r0.xw c0.xw", "gl_FragColor.xw = c[0].xw;" },
    { GL_MOV_ATI, GL_GREEN_BIT_ATI, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov r0.yw, c0.yw", "gl_FragColor.yw = c[0].yw;" },
    { GL_MOV_ATI, GL_BLUE_BIT_ATI, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov r0.zw c0.zw", "gl_FragColor.zw = c[0].zw;" },
    { GL_MOV_ATI, GL_GREEN_BIT_ATI | GL_BLUE_BIT_ATI, GL_NONE,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov r0.yzw c0.yzw", "gl_FragColor.yzw = c[0].yzw;" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, 0, GL_COMP_BIT_ATI,
     1, 0, 0,
     2, 0, 0,
     "mov 1-c0", "gl_FragColor = 1.0 - c[0];" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     1, 0, GL_BIAS_BIT_ATI,
     1, 0, 0,
     2, 0, 0,
     "mov c1-0.5", "gl_FragColor = c[1] - 0.5;" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, 0, GL_COMP_BIT_ATI | GL_BIAS_BIT_ATI,
     1, 0, 0,
     2, 0, 0,
     "mov (1-c0)-0.5", "gl_FragColor = (1.0 - c[0]) - 0.5;" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, 0, GL_COMP_BIT_ATI | GL_BIAS_BIT_ATI | GL_2X_BIT_ATI,
     1, 0, 0,
     2, 0, 0,
     "mov 2*((1-c0)-0.5)", "gl_FragColor = 2.0 * ((1.0 - c[0]) - 0.5);" },

    { GL_ADD_ATI, GL_NONE, GL_NONE,
     1, 0, 0,
     0, 0, GL_2X_BIT_ATI,
     2, 0, 0,
     "add c1, 2*c0", "gl_FragColor = c[1] + 2.0 * c[0];" },

    { GL_MOV_ATI, GL_NONE, GL_2X_BIT_ATI,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov 2*c0", "gl_FragColor = 2.0 * c[0];" },

    { GL_MOV_ATI, GL_NONE, GL_4X_BIT_ATI,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov 4*c0", "gl_FragColor = 4.0 * c[0];" },

    { GL_MOV_ATI, GL_NONE, GL_8X_BIT_ATI,
     0, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov 8*c0", "gl_FragColor = 8.0 * c[0];" },

    { GL_MOV_ATI, GL_NONE, GL_HALF_BIT_ATI,
     1, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c1/2", "gl_FragColor = c[1] / 2.0;" },

    { GL_MOV_ATI, GL_NONE, GL_QUARTER_BIT_ATI,
     1, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c1/4", "gl_FragColor = c[1] / 4.0;" },

    { GL_MOV_ATI, GL_NONE, GL_EIGHTH_BIT_ATI,
     1, 0, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c1/8", "gl_FragColor = c[1] / 8.0;" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, GL_RED, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c0.r", "gl_FragColor = vec4(c[0].r);" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, GL_GREEN, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c0.g", "gl_FragColor = vec4(c[0].g);" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, GL_BLUE, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c0.b", "gl_FragColor = vec4(c[0].b);" },

    { GL_MOV_ATI, GL_NONE, GL_NONE,
     0, GL_ALPHA, 0,
     1, 0, 0,
     2, 0, 0,
     "mov c0.a", "gl_FragColor = vec4(c[0].a);" },
};

static int arg_count(GLenum op)
{
	switch (op) {
	case GL_MOV_ATI:
		return 1;
	case GL_ADD_ATI:
	case GL_MUL_ATI:
	case GL_SUB_ATI:
	case GL_DOT3_ATI:
	case GL_DOT4_ATI:
		return 2;
	case GL_MAD_ATI:
	case GL_LERP_ATI:
	case GL_CND_ATI:
	case GL_CND0_ATI:
	case GL_DOT2_ADD_ATI:
		return 3;
	default:
		abort();
	}
}

static const int w = 8, h = 8;
static int
get_test_x(int i)
{
	return 5 + (5 + w * 2) * (i % 5);
}

static int
get_test_y(int i)
{
	return 5 + (5 + h) * (i / 5);
}

/* AIT_fs only allows 2 consts per instr, so put one of our args in
 * the color input instead.
 */
static GLenum
src(int src)
{
	if (src == 0)
		return GL_PRIMARY_COLOR_ARB;
	else
		return GL_CON_0_ATI + src - 1;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	glBindFragmentShaderATI(1);

	for (int i = 0; i < ARRAY_SIZE(rgba_tests); i++)
	{
		const struct atifs_op_test *test = &rgba_tests[i];
		int x = get_test_x(i);
		int y = get_test_y(i);

		glBeginFragmentShaderATI();

                /* Start with a default 0.75 value in reg0, to support checking dst masking */
		glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
				      GL_CON_2_ATI, 0, 0);

		switch (arg_count(test->op))
		{
		case 1:
			glColorFragmentOp1ATI(test->op, GL_REG_0_ATI, test->dstMask, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod);
			glAlphaFragmentOp1ATI(test->op, GL_REG_0_ATI, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod);
			break;
		case 2:
			glColorFragmentOp2ATI(test->op, GL_REG_0_ATI, test->dstMask, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod,
					      src(test->arg1c), test->arg1rep, test->arg1mod);
			glAlphaFragmentOp2ATI(test->op, GL_REG_0_ATI, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod,
					      src(test->arg1c), test->arg1rep, test->arg1mod);
			break;
		case 3:
			glColorFragmentOp3ATI(test->op, GL_REG_0_ATI, test->dstMask, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod,
					      src(test->arg1c), test->arg1rep, test->arg1mod,
					      src(test->arg2c), test->arg2rep, test->arg2mod);
			glAlphaFragmentOp3ATI(test->op, GL_REG_0_ATI, test->dstMod,
					      src(test->arg0c), test->arg0rep, test->arg0mod,
					      src(test->arg1c), test->arg1rep, test->arg1mod,
					      src(test->arg2c), test->arg2rep, test->arg2mod);
			break;
		default:
			abort();
		}

		static const float c[] = {
			C0R, C0G, C0B, C0A,
			C1R, C1G, C1B, C1A,
			C2R, C2G, C2B, C2A,
			0.75, 0.75, 0.75, 0.75,
		};
		glColor4fv(&c[0]);
		glSetFragmentShaderConstantATI(GL_CON_0_ATI, &c[4]);
		glSetFragmentShaderConstantATI(GL_CON_1_ATI, &c[8]);
		glSetFragmentShaderConstantATI(GL_CON_2_ATI, &c[12]);

		glEndFragmentShaderATI();

		piglit_draw_rect(x, y, w, h);

		const char *vs = "void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }";

		char *fs;
		if (asprintf(&fs,
			     "uniform vec4 c[3];\n"
			     "void main() {\n"
			     "gl_FragColor = vec4(0.75);\n"
			     "%s\n"
			     "}\n",
			     test->glsl) < 0)
		{
			abort();
		}

		GLuint prog = piglit_build_simple_program(vs, fs);
		glUseProgram(prog);
		glUniform4fv(glGetUniformLocation(prog, "c"), 3, c);
		piglit_draw_rect(x + w, y, w, h);
		glUseProgram(0);
		glDeleteProgram(prog);
	}

	enum piglit_result result = PIGLIT_PASS;
	for (int i = 0; i < ARRAY_SIZE(rgba_tests); i++) {
		const struct atifs_op_test *test = &rgba_tests[i];

		bool ok = piglit_probe_rect_halves_equal_rgba(get_test_x(i), get_test_y(i),
							      w * 2, h);
		enum piglit_result sub_result = ok ? PIGLIT_PASS : PIGLIT_FAIL;
		piglit_report_subtest_result(sub_result, "%s", test->name);
		piglit_merge_result(&result, sub_result);
	}

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ATI_fragment_shader");
}
