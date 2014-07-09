/*
 * Copyright © 2011 Intel Corporation
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
 */

/**
 * \file isinf-and-isnan.c
 *
 * Test that isinf() and isnan() built-in functions behave properly.
 *
 * The GLSL 1.30 spec does not define when an implementation is required to
 * generate infinite or NaN values; in fact, it explicitly allows for
 * implementations that do not even have a representation of infinity or Nan.
 * Therefore, we cannot check that infinities and NaNs are created when we
 * expect them.  However, we can test: (a) that isnan() and isinf() return
 * false for finite values, (b) that isinf() and isnan() behave consistently
 * with each other, and (c) that when a floating-point value is read out the
 * shader (using transform feedback or a floating point framebuffer) the
 * behavior of isnan() and isinf() behave consistently with the value that is
 * read out.
 *
 * This test operates by generating several expressions, some of which are
 * likely to produce infinities, some of which are likely to produce NaN, and
 * some of which are expected to produce finite values.  For each expression,
 * it does the following:
 * - evaluates isinf(value) in the shader
 * - evaluates isnan(value) in the shader
 * - evaluates sign(value) in the shader
 * - evaluates (value > 0) in the shader
 * - reads the value out of the shader (using transform feedback or a floating
 *   point framebuffer)
 * - feeds that value back into the shader (using a uniform); the shader
 *   subtracts this uniform from the originally computed value to produce a
 *   delta.
 *
 * And then it performs the following checks:
 * - If the value was expected to be finite, verifies that isinf() and isnan()
 *   returned false.
 * - If the value was expected to be +Inf or -Inf, verifies that the sign is
 *   correct, using both sign(value) and (value > 0).  This check is skipped
 *   if isnan(value) is true, since it's possible that a conformant
 *   implementation might generate NaN instead of infinity, and NaN does not
 *   have a well-defined sign.
 * - Checks that isinf() and isnan() didn't both return true.
 * - Checks that the C isinf() and isnan() functions give the same result as
 *   the shader's isinf() and isnan() functions.
 * - If the value is finite, checks that the delta is zero (to within
 *   tolerance).
 *
 * The last two checks are only performed when using a floating point
 * framebuffer or transform feedback, because those are the only ways to get
 * infinities and NaNs out of the shader and into C code.
 *
 * Note: the reason for the final check is to verify that a value claimed by
 * the shader to be finite is truly behaving like a finite number.  Without
 * it, an implementation could pass all these tests by simply having isinf()
 * and isnan() return false, and converting infinities and NaNs to finite
 * values when they exit the shader.
 *
 * The output of the test is a table whose columns are:
 * - The expression being tested (this expression may refer to the uniforms
 *   z=0.0, u_inf=+Inf, u_minus_inf=-Inf, and u_nan=NaN).
 * - The expected behavior of the expression ("finite", "+Inf", "-Inf", or
 *   "NaN", indicating how the expression would be expected to evaluate on a
 *   fully IEEE 754 compliant architecture)
 * - isinf(value), as computed by the shader
 * - isnan(value), as computed by the shader
 * - sign(value), as computed by the shader
 * - (value > 0), as computed by the shader
 * - value, as read out of the shader using transform feedback or a
 *   floating-point framebuffer
 * - delta, the difference between the computed value and the value that was
 *   fed back into the shader.
 * - A pass/fail indication.
 *
 * Note: the uniform z=0.0 is present to defeat constant folding and ensure
 * that the expression is evaluated during shader execution rather than during
 * compilation.  For example, "exp(1000.0)" might be evaluated at compile-time,
 * preventing us from exercising this test case in the GPU.  But
 * "exp(1000.0+z)" will definitely be evaluated on the GPU.
 *
 * The test must be invoked with one of the following command-line arguments:
 * - vs_basic: test the VS without reading values out of the shader.
 * - fs_basic: test the FS without reading values out of the shader.
 * - vs_fbo: test the VS, using a floating-point framebuffer to read values
 *   out of the shader.
 * - vs_xfb: test the VS, using transform feedback to read values out of the
 *   shader.
 * - fs_fbo: test the FS, using a floating-point framebuffer to read values
 *   out of the shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static float gl_version;

static GLint stock_vs;
static GLint stock_fs;
static GLint main_vs;
static GLint main_fs;
static GLint do_test_vs;
static GLint do_test_fs;
static GLuint xfb_buffer;

/**
 * True if we are using a floating-point framebuffer to read data out of the
 * shader.
 */
static bool use_fbo = false;

/**
 * True if we are using transform feedback to read data out of the shader.
 */
static bool use_xfb = false;

/**
 * True if we are testing the fragment shader, false if we are testing the
 * vertex shader.
 */
static bool use_fs;

/**
 * True if we are reading data out of the shader using a mechanism that
 * preserves the full 32-bit floating point value, so we can do additional
 * checks.
 */
static bool precise;

enum modes
{
	/**
	 * Output = vec4(value, isinf(value), isnan(value),
	 *               (sign(value) + 1.0) / 2.0)
	 */
	MODE_VALUE_ISINF_ISNAN_SIGN = 0,

	/**
	 * Output = vec4(value > 0, value - ref, 0.0, 0.0)
	 */
	MODE_GTZERO_DELTA_ZERO_ZERO = 1,
};

static const char stock_vs_text[] =
	"#version 130\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"}\n";

static const char stock_fs_text[] =
	"#version 130\n"
	"flat in vec4 data;\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = data;\n"
	"}\n";

static const char main_vs_text[] =
	"#version 130\n"
	"flat out vec4 data;\n"
	"vec4 do_test();\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"  data = do_test();\n"
	"}\n";

static const char main_fs_text[] =
	"#version 130\n"
	"flat in vec4 data;\n"
	"vec4 do_test();\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = do_test();\n"
	"}\n";

static const char do_test_text[] =
	"#version 130\n"
	"uniform float ref;\n" /* Value fed back from C */
	"uniform int mode;\n" /* See enum modes */
	"float compute_value();\n"
	"vec4 do_test()\n"
	"{\n"
	"  float value = compute_value();\n"
	"  if (mode == 0) { /* MODE_VALUE_ISINF_ISNAN_SIGN */\n"
	"    return vec4(value,\n"
	"                isinf(value) ? 1 : 0,\n"
	"                isnan(value) ? 1 : 0,\n"
	"                (sign(value) + 1.0) / 2.0);\n"
	"  } else if (mode == 1) { /* MODE_GTZERO_DELTA_ZERO_ZERO */\n"
	"    return vec4(value > 0 ? 1 : 0,\n"
	"                value - ref,\n"
	"                0.0,\n"
	"                0.0);\n"
	"  } else { /* Unrecognized mode */\n"
	"    return vec4(0.0);\n"
	"  }\n"
	"}\n";

static void
setup_fbo()
{
	GLuint fb = 0;
	GLuint color_rb = 0;
	GLenum fb_status;

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	/* Bind color attachment. */
	glGenRenderbuffers(1, &color_rb);
	glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F,
			      piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, color_rb);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
		printf("error: FBO incomplete (status = 0x%04x)\n", fb_status);
		piglit_report_result(PIGLIT_SKIP);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
}

static void
setup_xfb()
{
	glGenBuffers(1, &xfb_buffer);
}

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <mode>\n"
	       "  where <mode> is one of:\n"
	       "    vs_basic\n"
	       "    fs_basic\n"
	       "    vs_fbo\n"
	       "    vs_xfb\n"
	       "    fs_fbo\n", prog_name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "vs_basic") == 0) {
		use_fs = false;
	} else if (strcmp(argv[1], "fs_basic") == 0) {
		use_fs = true;
	} else if (strcmp(argv[1], "vs_fbo") == 0) {
		use_fs = false;
		use_fbo = true;
	} else if (strcmp(argv[1], "vs_xfb") == 0) {
		use_fs = false;
		use_xfb = true;
	} else if (strcmp(argv[1], "fs_fbo") == 0) {
		use_fs = true;
		use_fbo = true;
	} else {
		print_usage_and_exit(argv[0]);
	}
	precise = use_fbo || use_xfb;

	gl_version = strtod((char *) glGetString(GL_VERSION), NULL);

	piglit_require_GLSL();
	piglit_require_GLSL_version(130);

	if (piglit_is_extension_supported("GL_EXT_gpu_shader4")) {
		piglit_require_gl_version(21);
	} else {
		piglit_require_gl_version(30);
	}

	if (use_fbo) {
		setup_fbo();
	}
	if (use_xfb) {
		setup_xfb();
	}

	stock_vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
					      stock_vs_text);
	stock_fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					      stock_fs_text);
	main_vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
					     main_vs_text);
	main_fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					     main_fs_text);
	do_test_vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
						do_test_text);
	do_test_fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						do_test_text);
}

/**
 * enum indicating how the expression would be expected to behave on a fully
 * IEEE 754 compliant architecture.  Note: since OpenGL implementations are
 * not required to respect all of IEEE 754's rules for infinities and NaN's,
 * we don't necessarily check all of these behaviors.
 */
enum behavior
{
	B_NAN    = 0, /* Expected to evaluate to NaN */
	B_FINITE = 1, /* Expected to evaluate to a finite value */
	B_POSINF = 2, /* Expected to evaluate to +Infinity */
	B_NEGINF = 3, /* Expected to evaluate to -Infinity */
};

struct expression_table_element
{
	char *expression;
	int expected_behavior;
};

static struct expression_table_element expressions[] = {
	{ "1000.0", B_FINITE },
	{ "1000.0+z", B_FINITE },
	{ "-1000.0", B_FINITE },
	{ "-1000.0+z", B_FINITE },
	{ "u_inf", B_POSINF },
	{ "exp(1000.0)", B_POSINF },
	{ "exp(1000.0+z)", B_POSINF },
	{ "u_minus_inf", B_NEGINF },
	{ "-exp(1000.0)", B_NEGINF },
	{ "-exp(1000.0+z)", B_NEGINF },
	{ "u_nan", B_NAN },
	{ "0.0/0.0", B_NAN },
	{ "z/z", B_NAN },
	{ "u_inf/u_minus_inf", B_NAN },
	{ "z*u_inf", B_NAN },
	{ "u_inf+u_minus_inf", B_NAN },
	{ "log(-1.0)", B_NAN },
	{ "log(-1.0+z)", B_NAN },
	{ "sqrt(-1.0)", B_NAN },
	{ "sqrt(-1.0+z)", B_NAN },
};

/**
 * Draw using the shader, and then read back values using either (a) the
 * floating-point framebuffer, (b) transform feedback, or (c) pixel reads from
 * the window.  Note that pixel reads from the window are only accurate to one
 * part in 255, so the caller must be careful not to rely on high precision in
 * case (c).
 */
static void
draw_and_readback(float *readback)
{
	if (use_xfb) {
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 4096, NULL,
			     GL_DYNAMIC_COPY);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buffer);
		glEnable(GL_RASTERIZER_DISCARD);
		glBeginTransformFeedback(GL_TRIANGLES);
	}

	piglit_draw_rect(-1, -1, 2, 2);

	if (use_xfb) {
		glEndTransformFeedback();
		memcpy(readback,
		       glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY),
		       4*sizeof(float));
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	} else {
		glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, readback);
	}
}

static void
set_uniform_float_if_present(GLint program, char *name, float value)
{
	GLint loc = glGetUniformLocation(program, name);
	if (loc != -1)
		glUniform1f(loc, value);
}

static void
set_uniform_int_if_present(GLint program, char *name, int value)
{
	GLint loc = glGetUniformLocation(program, name);
	if (loc != -1)
		glUniform1i(loc, value);
}

/**
 * Test the given expression, to make sure its behavior is self-consistent and
 * consistent with the expected behavior.
 */
static bool
test_expr(char *expression, int expected_behavior)
{
	char compute_value_text[4096];
	GLint shader;
	GLint prog;
	float readback[4];
	float value;
	bool isinf_in_shader;
	bool isnan_in_shader;
	int sign_in_shader;
	float delta;
	bool greater_than_zero;
	bool pass = true;
	char *expected_behavior_string;

	/* Create and link a program specifically to test this expression */
	prog = glCreateProgram();
	sprintf(compute_value_text,
		"#version 130\n"
		"uniform float z = 0.0;\n" /* To defeat constant folding */
		"uniform float u_inf;\n" /* Always == +infinity */
		"uniform float u_minus_inf;\n" /* Always == -infinity */
		"uniform float u_nan;\n" /* Always == NaN */
		"float compute_value() {\n"
		"  return %s;\n"
		"}\n",
		expression);
	if (use_fs) {
		glAttachShader(prog, stock_vs);
		glAttachShader(prog, main_fs);
		glAttachShader(prog, do_test_fs);
		shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						    compute_value_text);
		glAttachShader(prog, shader);
	} else {
		glAttachShader(prog, stock_fs);
		glAttachShader(prog, main_vs);
		glAttachShader(prog, do_test_vs);
		shader = piglit_compile_shader_text(GL_VERTEX_SHADER,
						    compute_value_text);
		glAttachShader(prog, shader);
	}
	if (use_xfb) {
		static const char *var_name = "data";
		glTransformFeedbackVaryings(prog, 1, &var_name,
					    GL_SEPARATE_ATTRIBS);
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buffer);
	}
	glLinkProgram(prog);
	glDeleteShader(shader);
	glUseProgram(prog);

	/* Set up uniforms */
	set_uniform_float_if_present(prog, "u_inf", INFINITY);
	set_uniform_float_if_present(prog, "u_minus_inf", -INFINITY);
	set_uniform_float_if_present(prog, "u_nan", NAN);

	/* Use one draw call to read out value, isinf(value), isnan(value),
	 * and sign(value).
	 */
	set_uniform_float_if_present(prog, "ref", 0.0);
	set_uniform_int_if_present(prog, "mode", MODE_VALUE_ISINF_ISNAN_SIGN);
	draw_and_readback(readback);
	value = readback[0];
	isinf_in_shader = readback[1] > 0.5;
	isnan_in_shader = readback[2] > 0.5;
	sign_in_shader = (int) (2.0*readback[3] + 0.5) - 1;

	/* Use a second draw call to feed value back into the shader, and read
	 * out (value > 0) and delta.
	 */
	set_uniform_float_if_present(prog, "ref", value);
	set_uniform_int_if_present(prog, "mode", MODE_GTZERO_DELTA_ZERO_ZERO);
	draw_and_readback(readback);
	greater_than_zero = readback[0] > 0.5;
	delta = readback[1];

	/* Check that the behavior was as expected */
	switch (expected_behavior) {
	case B_FINITE:
		expected_behavior_string = "finite";
		if (isinf_in_shader || isnan_in_shader) {
			/* Expected finite, got Inf or NaN */
			pass = false;
		}
		break;
	case B_POSINF:
		expected_behavior_string = "+Inf";
		if (!isnan_in_shader && sign_in_shader != 1.0) {
			/* Expected positive or NaN, got <= 0 */
			pass = false;
		}
		break;
	case B_NEGINF:
		expected_behavior_string = "-Inf";
		if (!isnan_in_shader && sign_in_shader != -1.0) {
			/* Expected negative or NaN, got >= 0 */
			pass = false;
		}
		break;
	default:
		expected_behavior_string = "NaN";
		break;
	}

	/* Do other sanity checks */
	if (isnan_in_shader && isinf_in_shader) {
		/* No value can be simultaneously Inf and NaN */
		pass = false;
	}
	if (!isnan_in_shader) {
		if (sign_in_shader == -1 || sign_in_shader == 0) {
			if (greater_than_zero) {
				/* sign(value) inconsistent with (value>0) */
				pass = false;
			}
		} else if (sign_in_shader == 1) {
			if (!greater_than_zero) {
				/* sign(value) inconsistent with (value>0) */
				pass = false;
			}
		} else {
			/* Illegal return value for sign() */
			pass = false;
		}
	}

	/* If we are using a high-precision technique to read data out of the
	 * shader (fbo or xfb), check the behavior of isinf and isnan against
	 * their C counterparts, and verify that delta ~= 0 for finite values.
	 */
	if (precise) {
		bool isinf_in_c = !!isinf(value);
		bool isnan_in_c = !!isnan(value);
		if (isinf_in_shader != isinf_in_c ||
		    isnan_in_shader != isnan_in_c) {
			/* Result of isinf() and isnan() in the shader did not
			 * match the result in C code.
			 */
			pass = false;
		}
		if (!isinf_in_shader && !isnan_in_shader) {
			float threshold = fabs(value * 1e-6);
			if (isinf(delta) || isnan(delta) ||
			    fabs(delta) > threshold) {
				/* The shader and C code agree that the value
				 * was finite, but it isn't behaving as a nice
				 * finite value should.
				 */
				pass = false;
			}
		}
	}

	/* Output a line for the results table */
	printf("%17s %6s %5s %5s %4d %5s ",
	       expression,
	       expected_behavior_string,
	       isinf_in_shader ? "true" : "false",
	       isnan_in_shader ? "true" : "false",
	       sign_in_shader,
	       greater_than_zero ? "true" : "false");
	if (precise) {
		printf("%12g %12g ", value, delta);
	}
	printf("%s\n", pass ? "OK" : "FAIL");

	glUseProgram(0);
	glDeleteProgram(prog);

	return pass;
}

enum piglit_result
piglit_display()
{
	int i;
	bool pass = true;

	printf("    expression    expect isinf isnan sign  >0?");
	if (precise)
		printf("      value        delta");
	printf("\n");

	for (i = 0; i < sizeof(expressions)/sizeof(*expressions); ++i) {
		pass = test_expr(expressions[i].expression,
				 expressions[i].expected_behavior) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
