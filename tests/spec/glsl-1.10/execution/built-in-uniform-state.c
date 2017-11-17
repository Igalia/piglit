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
 * @file built-in-uniform-state.c:  Access uniform state in GLSL
 *
 * Set uniform state with the OpenGL API and access it in GLSL shaders.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
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

static const char *vs_text =
	"void main() {\n"
	"   gl_Position = gl_Vertex;\n"
	"}\n";
static const char *fs_float =
	"void main() {\n"
	"   float epsilon = 1.0 / 256.0;\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   float a = %s;\n"
	"   float b = %f;\n"
	"   gl_FragColor = abs(a - b) < epsilon ? green : red;\n"
	"}\n";
static const char *fs_vec3 =
	"void main() {\n"
	"   vec3 epsilon = vec3(1.0 / 256.0);\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   vec3 a = %s;\n"
	"   vec3 b = vec3(%f, %f, %f);\n"
	"   gl_FragColor = all(lessThan(abs(a - b), epsilon)) ? green : red;\n"
	"}\n";
static const char *fs_vec4 =
	"void main() {\n"
	"   vec4 epsilon = vec4(1.0 / 256.0);\n"
	"   vec4 green = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"   vec4 red = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"   vec4 a = %s;\n"
	"   vec4 b = vec4(%f, %f, %f, %f);\n"
	"   gl_FragColor = all(lessThan(abs(a - b), epsilon)) ? green : red;\n"
	"}\n";

/**
 * Check that the built-in shader uniform \name of type \type is equal to \val
 *
 * Since we also test for derived state involving floating point computation
 * don't test for strict equality but rather only check if the uniform's
 * components are within and espilon of their expected values.
 */
static bool
check_shader_builtin_(const GLenum type, const float *val, const char *name)
{
	char *fs_text;
	const float green[3] = {0.0, 1.0, 0.0};

	switch (type) {
	case GL_FLOAT:
		asprintf(&fs_text, fs_float, name, *val);
		break;
	case GL_FLOAT_VEC3:
		asprintf(&fs_text, fs_vec3, name, val[0], val[1], val[2]);
		break;
	case GL_FLOAT_VEC4:
		asprintf(&fs_text, fs_vec4, name, val[0], val[1], val[2],
			 val[3]);
		break;
	default:
		assert(0);
	}

	const GLuint program = piglit_build_simple_program(vs_text, fs_text);
	free(fs_text);
	glUseProgram(program);
	glDeleteProgram(program);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1, -1, 2, 2);

	if (piglit_probe_pixel_rgb_silent(piglit_width / 2, piglit_height / 2,
					  green, NULL))
		return true;
	printf("Failed uniform: '%s'.\n", name);
	return false;
}

/**
 * printf-like version of function above.
 */
static bool
check_shader_builtin(const GLenum type, const float *val, const char *format,
		     ...) PRINTFLIKE(3, 4);
static bool
check_shader_builtin(const GLenum type, const float *val, const char *format,
		     ...)
{
	char *name;
	va_list ap;

	va_start(ap, format);
	vasprintf(&name, format, ap);
	va_end(ap);

	const bool r = check_shader_builtin_(type, val, name);
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
 * Get name fragment used in GLSL for GLenum \pname.
 */
static const char *
enum2glsl(const GLenum pname)
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
	case GL_SPOT_EXPONENT:
		return "spotExponent";
	case GL_SPOT_CUTOFF:
		return "spotCutoff";
	case GL_CONSTANT_ATTENUATION:
		return "constantAttenuation";
	case GL_LINEAR_ATTENUATION:
		return "linearAttenuation";
	case GL_QUADRATIC_ATTENUATION:
		return "quadraticAttenuation";
	case GL_S:
		return "S";
	case GL_T:
		return "T";
	case GL_R:
		return "R";
	case GL_Q:
		return "Q";
	}
	assert(0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float val[4];

	/* Depth range */
	val[0] = DRAND();
	val[1] = DRAND();
	const float diff = val[1] - val[0];
	glDepthRangef(val[0], val[1]);
	pass = check_shader_builtin(GL_FLOAT, val, "gl_DepthRange.near") &&
	       pass;
	pass = check_shader_builtin(GL_FLOAT, &val[1], "gl_DepthRange.far") &&
	       pass;
	pass = check_shader_builtin(GL_FLOAT, &diff, "gl_DepthRange.diff") &&
	       pass;

	/* Clip planes */
	int max_clip_planes;
	glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
	for (int cp = 0; cp < max_clip_planes; ++cp) {
		double vald[4];
		for (int c = 0; c < 4; ++c)
			vald[c] = val[c] = DRAND();
		glClipPlane(GL_CLIP_PLANE0 + cp, vald);
		pass = check_shader_builtin(GL_FLOAT_VEC4, val,
					    "gl_ClipPlane[%d]", cp) &&
		       pass;
	}

	/* Point Size */
	val[0] = DRAND();
	glPointSize(val[0]);
	pass = check_shader_builtin(GL_FLOAT, val, "gl_Point.size") && pass;

	val[0] = DRAND();
	glPointParameterf(GL_POINT_SIZE_MIN, val[0]);
	pass = check_shader_builtin(GL_FLOAT, val, "gl_Point.sizeMin") &&
	       pass;

	val[0] = DRAND();
	glPointParameterf(GL_POINT_SIZE_MAX, val[0]);
	pass = check_shader_builtin(GL_FLOAT, val, "gl_Point.sizeMax") &&
	       pass;

	val[0] = DRAND();
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, val[0]);
	pass = check_shader_builtin(GL_FLOAT, val,
				    "gl_Point.fadeThresholdSize") &&
	       pass;

	val[0] = DRAND();
	val[1] = DRAND();
	val[2] = DRAND();
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, val);
	pass = check_shader_builtin(GL_FLOAT, &val[0],
				    "gl_Point.distanceConstantAttenuation") &&
	       pass;
	pass = check_shader_builtin(GL_FLOAT, &val[1],
				    "gl_Point.distanceLinearAttenuation") &&
	       pass;
	pass = check_shader_builtin(
		       GL_FLOAT, &val[2],
		       "gl_Point.distanceQuadraticAttenuation") &&
	       pass;

	/* Material State */
	for (int s = 0; s < 2; ++s) {
		for (int p = 0; p < 4; ++p) {
			const GLenum pname[] = {GL_EMISSION, GL_AMBIENT,
						GL_DIFFUSE, GL_SPECULAR};

			random_vec4(val);
			glMaterialfv(GL_FRONT + s, pname[p], val);
			pass = check_shader_builtin(GL_FLOAT_VEC4, val,
						    "gl_%sMaterial.%s",
						    s ? "Back" : "Front",
						    enum2glsl(pname[p])) &&
			       pass;
		}

		val[0] = DRAND();
		glMaterialf(GL_FRONT + s, GL_SHININESS, val[0]);
		pass = check_shader_builtin(GL_FLOAT, val,
					    "gl_%sMaterial.shininess",
					    s ? "Back" : "Front") &&
		       pass;
	}

	/* Light State */
	int max_lights;
	glGetIntegerv(GL_MAX_LIGHTS, &max_lights);
	for (int l = 0; l < max_lights; ++l) {
		for (int p = 0; p < 4; ++p) {
			const GLenum pname[] = {GL_AMBIENT, GL_DIFFUSE,
						GL_SPECULAR, GL_POSITION};
			random_vec4(val);
			glLightfv(GL_LIGHT0 + l, pname[p], val);
			pass = check_shader_builtin(GL_FLOAT_VEC4, val,
						    "gl_LightSource[%d].%s",
						    l, enum2glsl(pname[p])) &&
			       pass;
		}

		random_vec4(val);
		glLightfv(GL_LIGHT0 + l, GL_SPOT_DIRECTION, val);
		pass = check_shader_builtin(
			       GL_FLOAT_VEC3, val,
			       "gl_LightSource[%d].spotDirection", l) &&
		       pass;

		for (int p = 0; p < 5; ++p) {
			const GLenum pname[] = {GL_SPOT_EXPONENT,
						GL_SPOT_CUTOFF,
						GL_CONSTANT_ATTENUATION,
						GL_LINEAR_ATTENUATION,
						GL_QUADRATIC_ATTENUATION};
			val[0] = DRAND();
			glLightf(GL_LIGHT0 + l, pname[p], val[0]);
			pass = check_shader_builtin(GL_FLOAT, val,
						    "gl_LightSource[%d].%s",
						    l, enum2glsl(pname[p])) &&
			       pass;
		}

		val[0] = DRAND() * 90;
		glLightf(GL_LIGHT0 + l, GL_SPOT_CUTOFF, val[0]);
		const float cos_cutoff = cosf(val[0] / 180 * M_PI);
		pass = check_shader_builtin(
			       GL_FLOAT, &cos_cutoff,
			       "gl_LightSource[%d].spotCosCutoff", l) &&
		       pass;

		for (int c = 0; c < 3; ++c)
			val[c] = DRAND();
		val[3] = 1;
		glLightfv(GL_LIGHT0 + l, GL_POSITION, val);
		normalize(val);
		val[2] += 1;
		normalize(val);
		pass = check_shader_builtin(GL_FLOAT_VEC4, val,
					    "gl_LightSource[%d].halfVector",
					    l) &&
		       pass;
	}

	random_vec4(val);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, val);
	pass = check_shader_builtin(GL_FLOAT_VEC4, val,
				    "gl_LightModel.ambient") &&
	       pass;

	/* Derived state from products of light and material. */
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

		pass = check_shader_builtin(
			       GL_FLOAT_VEC4, scene_color,
			       s ? "gl_BackLightModelProduct.sceneColor"
				 : "gl_FrontLightModelProduct.sceneColor") &&
		       pass;
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

				pass = check_shader_builtin(
					       GL_FLOAT_VEC4, light_product,
					       "gl_%sLightProduct[%d].%s",
					       s ? "Back" : "Front", l,
					       enum2glsl(pname[p])) &&
				       pass;
			}
		}
	}

	/* Texture Environment and Generation */
	int max_tu;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_tu);
	int max_tiu;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tiu);
	const int max_textures = MIN2(max_tu, max_tiu);
	for (int t = 0; t < max_textures; ++t) {
		glActiveTexture(GL_TEXTURE0 + t);

		random_vec4(val);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, val);
		pass = check_shader_builtin(GL_FLOAT_VEC4, val,
					    "gl_TextureEnvColor[%d]", t) &&
		       pass;
	}

	int max_texture_coords;
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
	for (int t = 0; t < max_texture_coords; ++t) {
		const GLenum coord[] = {GL_S, GL_T, GL_R, GL_Q};
		glActiveTexture(GL_TEXTURE0 + t);

		for (int co = 0; co < 4; ++co) {
			const GLenum plane[] = {GL_EYE_PLANE,
						GL_OBJECT_PLANE};
			const char *plane_name[] = {"Eye", "Object"};
			for (int pl = 0; pl < 2; ++pl) {
				random_vec4(val);
				glTexGenfv(coord[co], plane[pl], val);
				pass = check_shader_builtin(
					       GL_FLOAT_VEC4, val,
					       "gl_%sPlane%s[%d]",
					       plane_name[pl],
					       enum2glsl(coord[co]), t) &&
				       pass;
			}
		}
	}

	/* Fog */
	random_vec4(val);
	glFogfv(GL_FOG_COLOR, val);
	pass = check_shader_builtin(GL_FLOAT_VEC4, val, "gl_Fog.color") &&
	       pass;

	val[0] = DRAND();
	glFogf(GL_FOG_DENSITY, val[0]);
	pass = check_shader_builtin(GL_FLOAT, val, "gl_Fog.density") && pass;

	val[1] = DRAND();
	glFogf(GL_FOG_START, val[1]);
	pass = check_shader_builtin(GL_FLOAT, &val[1], "gl_Fog.start") &&
	       pass;

	val[2] = DRAND();
	glFogf(GL_FOG_END, val[2]);
	pass = check_shader_builtin(GL_FLOAT, &val[2], "gl_Fog.end") && pass;

	const float fog_scale = 1 / (val[2] - val[1]);
	pass = check_shader_builtin(GL_FLOAT, &fog_scale, "gl_Fog.scale") &&
	       pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	SRAND(17);
}
