/*
 * Copyright © 2017 Fabian Bieler
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
 * @file property-bindings.c:  Access GL state in ARB_vertex_program.
 *
 * Set constant parameter bindings with the OpenGL API and access it in
 * ARB vertex programs.
 *
 * Matrix state is not tested.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef _WIN32
#define SRAND(x) srand(x)
#define DRAND() ((float)rand() / RAND_MAX)
#else
#define SRAND(x) srand48(x)
#define DRAND() drand48()
#endif

/*
 * This vertex program compares test_param against expected using epsilon
 * as tolerance.  On match result.color is set to green, red otherwise.
 */
static const char *vp_template =
	"!!ARBvp1.0\n"
	"PARAM epsilon = 0.00390625;\n"
	"PARAM expected = {%f, %f, %f, %f};\n"
	"PARAM test_param = %s;\n"
	"TEMP temp;\n"
	"SUB temp, expected, test_param;\n"
	"ABS temp, temp;\n"
	"SLT temp, temp, epsilon;\n"
	"DP4 temp, temp, temp;\n"
	"SLT temp.x, temp.x, 4;\n"
	"SGE temp.y, temp.y, 4;\n"
	"SWZ result.color, temp, x, y, 0, 1;\n"
	"MOV result.position, vertex.position;\n"
	"END";

/**
 * Check that the constant parameter \name is equal to \val.
 *
 * Since we also test for derived state involving floating point computation
 * don't test for strict equality but rather only check if the parameter's
 * components are within and epsilon of their expected values.
 */
static bool
check_prg_param_(const float *val, const char *name)
{
	char *vp_text;
	const float green[3] = {0.0, 1.0, 0.0};

	asprintf(&vp_text, vp_template, val[0], val[1], val[2], val[3], name);
	GLuint prog = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, vp_text);
	free(vp_text);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, prog);

	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteProgramsARB(1, &prog);

	if (piglit_probe_pixel_rgb_silent(piglit_width / 2, piglit_height / 2,
					  green, NULL))
		return true;
	printf("Failed parameter: '%s'.\n", name);
	return false;
}

/**
 * printf-like version of function above.
 */
static bool
check_prg_param(const float *val, const char *format, ...) PRINTFLIKE(2, 3);
static bool
check_prg_param(const float *val, const char *format, ...)
{
	char *name;
	va_list ap;

	va_start(ap, format);
	vasprintf(&name, format, ap);
	va_end(ap);

	const bool r = check_prg_param_(val, name);
	free(name);
	return r;
}

static void
normalize(float *v)
{
	const float norm = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	for (int i = 0; i < 3; ++i)
		v[i] /= norm;
}

static void
random_vec4(float *v)
{
	for (int i = 0; i < 4; ++i)
		v[i] = DRAND();
}

/**
 * Get name fragment used in ARB program for GLenum \pname.
 */
static const char *
enum2program(const GLenum pname)
{
	switch (pname) {
	case GL_EMISSION:
		return "emission";
	case GL_AMBIENT:
		return "ambient";
	case GL_DIFFUSE:
		return "diffuse";
	case GL_SPECULAR:
		return "specular";
	case GL_POSITION:
		return "position";
	case GL_S:
		return "s";
	case GL_T:
		return "t";
	case GL_R:
		return "r";
	case GL_Q:
		return "q";
	}
	assert(!"unexpected state enum");
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float val[4];

	/* Material Property Bindings */
	for (int s = 0; s < 2; ++s) {
		for (int p = 0; p < 4; ++p) {
			const GLenum pname[] = {GL_EMISSION, GL_AMBIENT,
						GL_DIFFUSE, GL_SPECULAR};

			random_vec4(val);
			glMaterialfv(GL_FRONT + s, pname[p], val);
			pass = check_prg_param(val, "state.material.%s.%s",
					       s ? "back" : "front",
					       enum2program(pname[p])) &&
			       pass;

			/* The front material bindings are also accessible
			 * without ".front.".
			 */
			if (s == 0)
				pass = check_prg_param(
					       val, "state.material.%s",
					       enum2program(pname[p])) &&
				       pass;
		}

		val[0] = DRAND();
		val[1] = 0;
		val[2] = 0;
		val[3] = 1;
		glMaterialf(GL_FRONT + s, GL_SHININESS, val[0]);
		pass = check_prg_param(val, "state.material.%s.shininess",
				       s ? "back" : "front") && pass;

		if (s == 0)
			pass = check_prg_param(val,
					       "state.material.shininess") &&
			       pass;
	}

	/* Light Property Bindings */
	int max_lights;
	glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
	for (int l = 0; l < max_lights; ++l) {
		for (int p = 0; p < 4; ++p) {
			const GLenum pname[] = {GL_AMBIENT, GL_DIFFUSE,
						GL_SPECULAR, GL_POSITION};
			random_vec4(val);
			glLightfv(GL_LIGHT0 + l, pname[p], val);
			pass = check_prg_param(val, "state.light[%d].%s", l,
					       enum2program(pname[p])) &&
			       pass;
		}

		random_vec4(val);
		glLightf(GL_LIGHT0 + l, GL_CONSTANT_ATTENUATION, val[0]);
		glLightf(GL_LIGHT0 + l, GL_LINEAR_ATTENUATION, val[1]);
		glLightf(GL_LIGHT0 + l, GL_QUADRATIC_ATTENUATION, val[2]);
		glLightf(GL_LIGHT0 + l, GL_SPOT_EXPONENT, val[3]);
		pass = check_prg_param(val, "state.light[%d].attenuation",
				       l) && pass;

		random_vec4(val);
		glLightfv(GL_LIGHT0 + l, GL_SPOT_DIRECTION, val);
		glLightf(GL_LIGHT0 + l, GL_SPOT_CUTOFF, val[3]);
		val[3] = cosf(val[3] / 180 * M_PI);
		pass = check_prg_param(val, "state.light[%d].spot.direction",
				       l) && pass;

		for (int c = 0; c < 3; ++c)
			val[c] = DRAND();
		val[3] = 1;
		glLightfv(GL_LIGHT0 + l, GL_POSITION, val);
		normalize(val);
		val[2] += 1;
		normalize(val);
		pass = check_prg_param(val, "state.light[%d].half", l) &&
		       pass;
	}

	random_vec4(val);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, val);
	pass = check_prg_param(val, "state.lightmodel.ambient") && pass;

	for (int s = 0; s < 2; ++s) {
		float scene_color[4];

		for (int c = 0; c < 4; ++c)
			scene_color[c] = val[c] = DRAND();
		glMaterialfv(GL_FRONT + s, GL_AMBIENT, val);
		for (int c = 0; c < 4; ++c)
			scene_color[c] *= val[c] = DRAND();
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, val);
		for (int c = 0; c < 4; ++c)
			scene_color[c] += val[c] = DRAND();
		glMaterialfv(GL_FRONT + s, GL_EMISSION, val);

		/* Page 63 (77 of the PDF) of the OpenGL 2.0 spec says:
		 *
		 *      "The value of A produced by lighting is the alpha
		 *      value associated with d_{cm}."
		 *
		 * I'm not sure if this applies to the scene color, but both
		 * Mesa and the NVIDIA driver do this.
		 */
		random_vec4(val);
		glMaterialfv(GL_FRONT + s, GL_DIFFUSE, val);
		scene_color[3] = val[3];

		pass = check_prg_param(scene_color,
				       "state.lightmodel.%s.scenecolor",
				       s ? "back" : "front") && pass;

		if (s == 0)
			pass = check_prg_param(
				       scene_color,
				       "state.lightmodel.scenecolor") && pass;
	}

	for (int s = 0; s < 2; ++s) {
		for (int l = 0; l < max_lights; ++l) {
			const GLenum pname[] = {GL_AMBIENT, GL_DIFFUSE,
						GL_SPECULAR};
			for (int p = 0; p < 3; ++p) {
				float light_product[4];
				for (int c = 0; c < 4; ++c)
					light_product[c] = val[c] = DRAND();
				glLightfv(GL_LIGHT0 + l, pname[p], val);
				for (int c = 0; c < 4; ++c)
					light_product[c] *= val[c] = DRAND();
				glMaterialfv(GL_FRONT + s, pname[p], val);
				/* XXX: I have no Idea where the spec says the
				 * alpha value of the light product is the
				 * material's alpha value, but both Mesa and
				 * the NVIDIA driver do this.
				 */
				light_product[3] = val[3];

				pass = check_prg_param(
					       light_product,
					       "state.lightprod[%d].%s.%s", l,
					       s ? "back" : "front",
					       enum2program(pname[p])) &&
				       pass;

				if (s == 0)
					pass = check_prg_param(
						       light_product,
						       "state.lightprod[%d]."
						       "%s",
						       l,
						       enum2program(
							       pname[p])) &&
					       pass;
			}
		}
	}

	/* Texture Coordinate Generation Property Bindings */
	int max_texture_coords;
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
	for (int t = 0; t < max_texture_coords; ++t) {
		const GLenum coord[] = {GL_S, GL_T, GL_R, GL_Q};
		glActiveTexture(GL_TEXTURE0 + t);

		for (int co = 0; co < 4; ++co) {
			const GLenum plane[] = {GL_EYE_PLANE,
						GL_OBJECT_PLANE};
			const char *plane_name[] = {"eye", "object"};
			for (int pl = 0; pl < 2; ++pl) {
				random_vec4(val);
				glTexGenfv(coord[co], plane[pl], val);
				pass = check_prg_param(
					       val, "state.texgen[%d].%s.%s",
					       t, plane_name[pl],
					       enum2program(coord[co])) &&
				       pass;
				if (t == 0)
					pass = check_prg_param(
						       val,
						       "state.texgen.%s.%s",
						       plane_name[pl],
						       enum2program(
							       coord[co])) &&
					       pass;
			}
		}
	}

	/* Fog Property Bindings */
	random_vec4(val);
	glFogfv(GL_FOG_COLOR, val);
	pass = check_prg_param(val, "state.fog.color") && pass;

	random_vec4(val);
	glFogf(GL_FOG_DENSITY, val[0]);
	glFogf(GL_FOG_START, val[1]);
	glFogf(GL_FOG_END, val[2]);
	val[3] = 1 / (val[2] - val[1]);
	pass = check_prg_param(val, "state.fog.params") && pass;

	/* Clip Plane Property Bindings */
	int max_clip_planes;
	glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
	for (int cp = 0; cp < max_clip_planes; ++cp) {
		double vald[4];
		for (int c = 0; c < 4; ++c)
			vald[c] = val[c] = DRAND();
		glClipPlane(GL_CLIP_PLANE0 + cp, vald);
		pass = check_prg_param(val, "state.clip[%d].plane", cp) &&
		       pass;
	}

	/* Point Property Bindings */
	random_vec4(val);
	glPointSize(val[0]);
	glPointParameterf(GL_POINT_SIZE_MIN, val[1]);
	glPointParameterf(GL_POINT_SIZE_MAX, val[2]);
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, val[3]);
	pass = check_prg_param(val, "state.point.size") && pass;

	random_vec4(val);
	val[3] = 1;
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
	pass = check_prg_param(&val[0], "state.point.attenuation") && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_program");

	glEnable(GL_VERTEX_PROGRAM_ARB);

	SRAND(17);
}
