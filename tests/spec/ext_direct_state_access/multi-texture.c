/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

	config.supports_gl_compat_version = 21;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static int max_texture_coords;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");
	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_texture_coords);
}

/* Returns n (<= 4) different texunits */
static GLenum*
n_texunits(int n)
{
	static GLenum out[4];
	int i, j;
	assert (n <= 4);
	for (i = 0; i < n; i++) {
		out[i] = rand() % max_texture_coords;
		if (n <= max_texture_coords) {
			for (j = 0; j < i; j++) {
				if (out[i] == out[j]) {
					/* Reset i */
					i--;
					break;
				}
			}
		}
	}
	for (i = 0; i < n; i++) {
		out[i] += GL_TEXTURE0;
	}
	return out;
}

static GLenum
dimension_to_target(int n)
{
	assert(n == 1 || n == 2 || n == 3);
	switch (n) {
		case 1: return GL_TEXTURE_1D;
		case 2: return GL_TEXTURE_2D;
		case 3:
		default:
			return GL_TEXTURE_3D;
	}
}

static GLenum use_display_list = GL_NONE;
static GLuint list;

static enum piglit_result
test_MultiTexEnviEXT(void* data)
{
	int i, j, k, l;
	static const GLenum targets[] = {
		GL_TEXTURE_ENV,
		GL_TEXTURE_FILTER_CONTROL,
		GL_POINT_SPRITE
	};

	struct pname_value {
		GLenum pname;
		int value_count;
		int values[8];
	};

	static const struct pname_value texture_env_test [] =
	{
		{
			GL_TEXTURE_ENV_MODE,
			6,
			{
				GL_ADD, GL_MODULATE, GL_DECAL,
				GL_BLEND, GL_REPLACE, GL_COMBINE
			}
		},
		{
			GL_COMBINE_RGB,
			8,
			{
				GL_REPLACE, GL_MODULATE, GL_ADD,
				GL_ADD_SIGNED, GL_INTERPOLATE, GL_SUBTRACT,
				GL_DOT3_RGB, GL_DOT3_RGBA
			}
		},
		{
			GL_COMBINE_ALPHA,
			6,
			{
				GL_REPLACE, GL_MODULATE, GL_ADD,
				GL_ADD_SIGNED, GL_INTERPOLATE, GL_SUBTRACT
			}
		},
		{
			GL_SRC0_RGB,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_SRC1_RGB,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_SRC2_RGB,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_SRC0_ALPHA,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_SRC1_ALPHA,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_SRC2_ALPHA,
			5, { GL_TEXTURE, GL_TEXTURE0, GL_CONSTANT, GL_PRIMARY_COLOR, GL_PREVIOUS }
		},
		{
			GL_OPERAND0_RGB,
			4, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			GL_OPERAND1_RGB,
			4, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			GL_OPERAND2_RGB,
			4, { GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			GL_OPERAND0_ALPHA,
			2, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			GL_OPERAND1_ALPHA,
			2, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			GL_OPERAND2_ALPHA,
			2, { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
		},
		{
			0
		}
	};
	static const struct pname_value texture_filter_control_test[] = {
		{
			GL_TEXTURE_LOD_BIAS,
			1, { 1 }
		},
		{
			0
		}
	};
	static const struct pname_value point_sprite_test[] = {
		{
			GL_COORD_REPLACE,
			2, { GL_TRUE, GL_FALSE }
		},
		{
			0
		}
	};

	static const struct pname_value* tested[] = {
		texture_env_test,
		texture_filter_control_test,
		point_sprite_test
	};
	bool pass = true;

	/* This test applies different values to the same pname to 4 texunit:
	 *   - texunit#0 will use glTexEnvi(...)
	 *   - texunit#1 will use glMultiTexEnvi(...)
	 *   - texunit#2 will use glMultiTexEnviv(...)
	 *   - texunit#3 will use glMultiTexEnvf(...)
	 * pname value is then read back and the 4 values are verified.
	 */

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		const GLenum target = targets[i];
		const GLenum* texunits = n_texunits(4);

		glActiveTexture(texunits[0]);

		/* Iterate on possible targets */
		for (j = 0; tested[i][j].pname != 0; j++) {
			const GLenum pname = tested[i][j].pname;
			const int value_count = tested[i][j].value_count;
			/* Iterate on possible values */
			for (k = 0; k < tested[i][j].value_count; k++) {
				int original_values[4];
				const int values[] = {
					tested[i][j].values[k],
					tested[i][j].values[(k + 1) % value_count],
					tested[i][j].values[(k + 2) % value_count],
					tested[i][j].values[(k + 3) % value_count]
				};
				for (l = 0; l < 4; l++) {
					glGetMultiTexEnvivEXT(texunits[l], target,
							      pname, &original_values[l]);
				}

				if (use_display_list != GL_NONE)
					glNewList(list, use_display_list);

				/* Set texunit#0 to values[0] */
				glTexEnvi(target,
					  pname,
					  values[0]);
				/* Set texunit#1 to values[1] using ext_dsa function */
				glMultiTexEnviEXT(texunits[1],
						  target,
						  pname,
						  values[1]);
				/* Set texunit#2 to values[2] using ext_dsa function */
				glMultiTexEnvivEXT(texunits[2],
						  target,
						  pname,
						  &values[2]);
				/* Set texunit#3 to values[3] using ext_dsa function */
				glMultiTexEnvfEXT(texunits[3],
						  target,
						  pname,
						  (float) values[3]);

				if (use_display_list != GL_NONE)
					glEndList(list);

				if (use_display_list == GL_COMPILE) {
					for (l = 0; l < 4; l++) {
						int v;
						glGetMultiTexEnvivEXT(texunits[l], target,
								      pname, &v);
						pass = v == original_values[l] && pass;
					}
					glCallList(list);
				}

				if (!piglit_check_gl_error(GL_NO_ERROR)) {
					return PIGLIT_FAIL;
				}

				for (l = 0; l < 4; l++) {
					int got;
					glGetMultiTexEnvivEXT(texunits[l], target, pname, &got);
					if (got != values[l]) {
						piglit_loge("glMultiTexEnv(%s, %s, %s) value error with variant %d\n."
							    "Expected %s but got %s\n",
							    piglit_get_gl_enum_name(texunits[l]),
							    piglit_get_gl_enum_name(target),
							    piglit_get_gl_enum_name(pname),
							    l,
							    piglit_get_gl_enum_name(values[l]),
							    piglit_get_gl_enum_name(got));
						return PIGLIT_FAIL;
					}
				}
			}

		}
	}

	return piglit_check_gl_error(GL_NO_ERROR) && pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_MultiTexEnvfvEXT(void* data)
{
	static const float color[] = { 0.3, 0.7, 0.1, 0.0 };
	float original_color[4];
	float got[4];
	const GLenum* texunits = n_texunits(2);

	glActiveTexture(texunits[0]);

	glGetMultiTexEnvfvEXT(texunits[1],
			GL_TEXTURE_ENV,
			GL_TEXTURE_ENV_COLOR,
			original_color);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	glMultiTexEnvfvEXT(texunits[1],
			GL_TEXTURE_ENV,
			GL_TEXTURE_ENV_COLOR,
			color);

	if (use_display_list != GL_NONE)
		glEndList(list);

	glGetMultiTexEnvfvEXT(texunits[1],
			GL_TEXTURE_ENV,
			GL_TEXTURE_ENV_COLOR,
			got);

	if (use_display_list == GL_COMPILE) {
		if (memcmp(original_color, got, sizeof(got)) != 0) {
			return PIGLIT_FAIL;
		}
		glCallList(list);
		glGetMultiTexEnvfvEXT(texunits[1],
			GL_TEXTURE_ENV,
			GL_TEXTURE_ENV_COLOR,
			got);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR) || memcmp(color, got, sizeof(got)) != 0) {
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static GLenum
init_texunit(GLenum target, int* image_size, float** expected_pixels)
{
	GLuint tex;
	const GLenum* texunits = n_texunits(2);
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	const int depth = (target == GL_TEXTURE_3D) ? 2 : 1;
	float* image = piglit_rgbw_image(GL_RGBA, piglit_width, height * depth,
					       false, GL_UNSIGNED_NORMALIZED);
	*image_size = piglit_width * height * depth * 4 * sizeof(float);
	int max_unit;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_unit);

	/* Make sure ActiveTexture is set to something different */
	glActiveTexture(texunits[1]);

	glGenTextures(1, &tex);
	glBindMultiTextureEXT(texunits[0], target, tex);

	glMultiTexParameteriEXT(texunits[0], target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glMultiTexParameteriEXT(texunits[0], target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glMultiTexParameteriEXT(texunits[0], target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glMultiTexParameteriEXT(texunits[0], target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	if (target == GL_TEXTURE_1D) {
		glMultiTexImage1DEXT(texunits[0], target, 0, GL_RGBA,
				    piglit_width, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else if (target == GL_TEXTURE_2D) {
		glMultiTexImage2DEXT(texunits[0], target, 0, GL_RGBA,
				    piglit_width, height, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else {
		glMultiTexImage3DEXT(texunits[0], target, 0, GL_RGBA,
		    piglit_width, height, depth, 0,
		    GL_RGBA, GL_FLOAT, image);
	}

	if (expected_pixels != NULL) {
		*expected_pixels = image;
	} else {
		free(image);
	}

	return texunits[0];
}

static GLint
get_bound_texture(GLenum target, GLenum texunit)
{
	GLint active, bound = 0;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &active);
	glActiveTexture(texunit);
	if (target == GL_TEXTURE_1D) {
		glGetIntegerv(GL_TEXTURE_BINDING_1D, &bound);
	} else if (target == GL_TEXTURE_2D) {
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &bound);
	} else {
		glGetIntegerv(GL_TEXTURE_BINDING_3D, &bound);
	}
	glActiveTexture(active);
	return bound;
}


static bool
check_no_texture_bound_on_texunit(GLenum target, GLenum texunit)
{
	if (get_bound_texture(target, texunit) != 0) {
		piglit_loge(
			"No texture should be bound on %s.",
			piglit_get_gl_enum_name(texunit));
		return false;
	}
	return true;
}

static enum piglit_result
test_MultiTexImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t)data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	GLuint tex;
	float* expected_pixels, *got_pixels;
	GLenum texunit;

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	texunit = init_texunit(target, &image_size, &expected_pixels);

	if (use_display_list != GL_NONE)
		glEndList(list);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been bound yet */
		pass = check_no_texture_bound_on_texunit(target, texunit) && pass;

		glCallList(list);
	}

	/* Compare glGetTextureImageEXT and glGetTexImage */
	got_pixels = (float*) malloc(image_size);
	glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	pass = pass && memcmp(expected_pixels, got_pixels, image_size) == 0;

	free(expected_pixels);
	free(got_pixels);

	tex = get_bound_texture(target, texunit);
	glDeleteTextures(1, &tex);

	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}


static enum piglit_result
test_MultiTexSubImageNDEXT(void* data)
{
	bool pass = true;
	int i;
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	GLuint tex;
	float* original_pixels, *modified_pixels, *got_pixels;
	GLenum texunit = init_texunit(target, &image_size, &original_pixels);
	int len = image_size / sizeof(float);

	/* Replace the whole texture using glTextureSubImageNDEXT, line by line */
	modified_pixels = (float*) malloc(image_size);
	for (i = 0; i < len; i++) {
		modified_pixels[i] = original_pixels[(i + 1) % len];
	}

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	if (n == 1) {
		glMultiTexSubImage1DEXT(texunit, target, 0,
				       0, piglit_width,
				       GL_RGBA, GL_FLOAT,
				       modified_pixels);
	} else {
		for (i = 0; i < piglit_height; i++) {
			if (n == 2) {
				glMultiTexSubImage2DEXT(texunit,
					target, 0,
					0, i, piglit_width, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * i]);
			} else {
				/* Update the 1st layer of the 3D image */
				glMultiTexSubImage3DEXT(texunit,
					target, 0,
					0, i, 0, piglit_width, 1, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * i]);
				/* And the 2nd layer */
				glMultiTexSubImage3DEXT(texunit,
					target, 0,
					0, i, 1, piglit_width, 1, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * (i + piglit_height)]);
			}
		}
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	/* Compare glGetMultiTexImageEXT output and modified_pixels */
	got_pixels = (float*) malloc(image_size);
	glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been updated yet */
		pass = memcmp(original_pixels, got_pixels, image_size) == 0 && pass;
		glCallList(list);
		/* Re-read */
		glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = pass && memcmp(modified_pixels, got_pixels, image_size) == 0;

	tex = get_bound_texture(target, texunit);
	glDeleteTextures(1, &tex);
	free(modified_pixels);
	free(original_pixels);

	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_CopyMultiTexImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t) data;
	assert(n == 1 || n == 2);
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* original_pixels;
	GLuint tex;
	GLenum texunit = init_texunit(target, &image_size, &original_pixels);
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	float* got_pixels;

	glClearColor(0.25, 0.5, 0.75, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	switch (target) {
		case GL_TEXTURE_1D:
			glCopyMultiTexImage1DEXT(texunit, target, 0, GL_RGBA, 0, 0, piglit_width, 0);
			break;
		case GL_TEXTURE_2D:
			glCopyMultiTexImage2DEXT(texunit, target, 0, GL_RGBA, 0, 0, piglit_width, piglit_height, 0);
			break;
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	got_pixels = (float*) malloc(piglit_width * height * 4 * sizeof(float));
	/* Compare glGetTextureImageEXT and read pixels */
	glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been modified yet */
		pass = memcmp(got_pixels, original_pixels, image_size) == 0;
		glCallList(list);
		glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, height, got_pixels) && pass;

	free(got_pixels);
	free(original_pixels);

	tex = get_bound_texture(target, texunit);
	glDeleteTextures(1, &tex);

	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_CopyMultiTexSubImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* original_pixels;
	GLuint tex;
	GLenum texunit = init_texunit(target, &image_size, &original_pixels);
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	float* got_pixels;

	glClearColor(0.25, 0.5, 0.75, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	switch (target) {
		case GL_TEXTURE_1D:
			glCopyMultiTexSubImage1DEXT(texunit, target, 0, 0, 0, 0, piglit_width);
			break;
		case GL_TEXTURE_2D:
			glCopyMultiTexSubImage2DEXT(texunit, target, 0, 0, 0, 0, 0, piglit_width, piglit_height);
			break;
		case GL_TEXTURE_3D:
			glCopyMultiTexSubImage3DEXT(texunit, target, 0, 0, 0, 0, 0, 0, piglit_width, piglit_height);
			break;
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	got_pixels = (float*) malloc(image_size);
	/* Compare glGetTextureImageEXT and read pixels */
	glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been modified yet */
		pass = memcmp(got_pixels, original_pixels, image_size) == 0;
		glCallList(list);
		glGetMultiTexImageEXT(texunit, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, height, got_pixels) && pass;

	free(got_pixels);

	tex = get_bound_texture(target, texunit);
	glDeleteTextures(1, &tex);
	free(original_pixels);

	return pass && piglit_check_gl_error(GL_NO_ERROR) ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum tex_gen_version {
	tex_gen_version_i = 0,
	tex_gen_version_iv,
	tex_gen_version_f,
	tex_gen_version_fv,
	tex_gen_version_d,
	tex_gen_version_dv
};

static int
MultiTexGenSingleValueRoundTrip(int texunit, GLenum coords, GLint param, enum tex_gen_version version) {
	switch (version) {
		case tex_gen_version_i:
		case tex_gen_version_iv: {
			int i_value;
			if (version == tex_gen_version_i) {
				glMultiTexGeniEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  param);
			} else {
				glMultiTexGenivEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  &param);
			}
			glGetMultiTexGenivEXT(texunit, coords,
					      GL_TEXTURE_GEN_MODE,
					      &i_value);
			return i_value;
		}
		case tex_gen_version_f:
		case tex_gen_version_fv: {
			float f_param = (float) param;
			float f_value;
			if (version == tex_gen_version_f) {
				glMultiTexGenfEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  f_param);
			} else {
				glMultiTexGenfvEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  &f_param);
			}
			glGetMultiTexGenfvEXT(texunit, coords,
					      GL_TEXTURE_GEN_MODE,
					      &f_value);
			return (int) f_value;
		}
		case tex_gen_version_d:
		case tex_gen_version_dv:
		default: {
			double d_param = (double) param;
			double d_value;
			if (version == tex_gen_version_d) {
				glMultiTexGendEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  d_param);
			} else {
				glMultiTexGendvEXT(texunit, coords,
						  GL_TEXTURE_GEN_MODE,
						  &d_param);
			}
			glGetMultiTexGendvEXT(texunit, coords,
					      GL_TEXTURE_GEN_MODE,
					      &d_value);
			return (int) d_value;
		}
	}
}

enum piglit_result
test_MultiTexGenSingleValueEXT(void* data)
{
	static const GLenum coords[] = {
		GL_S, GL_T, GL_R, GL_Q
	};
	static const GLint params[] = {
		GL_OBJECT_LINEAR, GL_EYE_LINEAR,
		GL_NORMAL_MAP, GL_REFLECTION_MAP,
		GL_SPHERE_MAP
	};
	static const int valid_params_count[] = {
		ARRAY_SIZE(params), ARRAY_SIZE(params),
		/* SPHERE_MAP is invalid for GL_R */
		ARRAY_SIZE(params) - 1,
		/* NORMAL_MAP, SPHERE_MAP, REFLECTION_MAP are invalid for GL_R */
		ARRAY_SIZE(params) - 3
	};

	int i, j, k, value;

	for (i = 0; i < ARRAY_SIZE(coords); i++) {
		for (j = 0; j < valid_params_count[i]; j++) {
			const GLenum* texunits = n_texunits(2);
			for (k = 0; k < 6; k++) {
				enum tex_gen_version v = (enum tex_gen_version) k;

				glActiveTexture(texunits[0]);

				value = MultiTexGenSingleValueRoundTrip(
						texunits[1],
						coords[i],
						params[j],
						v);

				if (!piglit_check_gl_error(GL_NO_ERROR) || value != params[j]) {
					piglit_loge("glMultiTexGenEXT(%d, %s, GL_TEXTURE_GEN_MODE, %s) failed.\n"
						    "Expected: %s but got %s [%d]\n",
						    texunits[1],
						    piglit_get_gl_enum_name(coords[i]),
						    piglit_get_gl_enum_name(params[j]),
						    piglit_get_gl_enum_name(params[j]),
						    piglit_get_gl_enum_name(value),
						    k);
					return PIGLIT_FAIL;
				}
			}
		}
	}
	return PIGLIT_PASS;
}

enum piglit_result
test_MultiTexCoordPointerEXT(void* data)
{
	int texunit;
	static const int array[] = {
		1, 2, 3, 4
	};
	bool pass = true;
	int value;
	void* ptr;

	glClientActiveTexture(GL_TEXTURE0);

	texunit = GL_TEXTURE0 + rand() % max_texture_coords;

	glMultiTexCoordPointerEXT(texunit, 2, GL_INT, 4, array);

	pass = piglit_check_gl_error(GL_NO_ERROR);

	glClientActiveTexture(texunit);

	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_SIZE, &value);
	pass = value == 2 && pass;

	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_TYPE, &value);
	pass = value == GL_INT && pass;

	glGetIntegerv(GL_TEXTURE_COORD_ARRAY_STRIDE, &value);
	pass = value == 4 && pass;

	glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &ptr);
	pass = ptr == array && pass;

	return piglit_check_gl_error(GL_NO_ERROR) && pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_MultiTexParameteriEXT(void* data)
{
	static const GLenum targets[] = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
		GL_TEXTURE_CUBE_MAP,
	};
	int i, j, k, l, value;
	bool pass = true;

	const struct pname_value {
		GLenum pname;
		int value_count;
		int values[8];
	} tested[] = {
		{
			GL_TEXTURE_WRAP_S,
			5,
			{
				GL_CLAMP, GL_CLAMP_TO_EDGE, GL_REPEAT,
				GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT
			}
		},
		{
			GL_TEXTURE_WRAP_R,
			5,
			{
				GL_CLAMP, GL_CLAMP_TO_EDGE, GL_REPEAT,
				GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT
			}
		},
		{
			GL_TEXTURE_WRAP_T,
			5,
			{
				GL_CLAMP, GL_CLAMP_TO_EDGE, GL_REPEAT,
				GL_CLAMP_TO_BORDER, GL_MIRRORED_REPEAT
			}
		},
		{
			GL_TEXTURE_MIN_FILTER,
			6,
			{
				GL_NEAREST, GL_LINEAR,
				GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_NEAREST,
				GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_NEAREST
			}
		},
		{
			GL_TEXTURE_MAG_FILTER,
			2,
			{
				GL_NEAREST, GL_LINEAR
			}
		},
		{
			GL_TEXTURE_BASE_LEVEL,
			1, { rand() }
		},
		{
			GL_TEXTURE_MAX_LEVEL,
			1, { rand() }
		},
		{
			GL_DEPTH_TEXTURE_MODE,
			4,
			{
				GL_RED, GL_LUMINANCE, GL_INTENSITY, GL_ALPHA
			}
		},
		{
			GL_TEXTURE_COMPARE_MODE,
			2,
			{
				GL_NONE, GL_COMPARE_REF_TO_TEXTURE
			}
		},
		{
			GL_TEXTURE_COMPARE_FUNC,
			8,
			{
				GL_LEQUAL, GL_GEQUAL, GL_LESS, GL_GREATER,
				GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS, GL_NEVER
			}
		},
		{
			GL_GENERATE_MIPMAP,
			2,
			{
				GL_TRUE, GL_FALSE
			}
		},
	};


	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		const GLenum target = targets[i];
		for (j = 0; j < ARRAY_SIZE(tested); j++) {
			const GLenum* texunits = n_texunits(3);
			glActiveTexture(texunits[0]);
			for (k = 0; k < tested[j].value_count; k++) {
				int original_values[2];

				if (use_display_list != GL_NONE)
					glNewList(list, use_display_list);

				for (l = 0; l < 2; l++) {
					glGetMultiTexParameterivEXT(texunits[1 + l], target,
						tested[j].pname, &original_values[l]);
				}

				glMultiTexParameteriEXT(
					texunits[1], target,
					tested[j].pname,
					tested[j].values[k]);

				glMultiTexParameterivEXT(
					texunits[2], target,
					tested[j].pname,
					&tested[j].values[k]);

				if (use_display_list != GL_NONE)
					glEndList(list);

				if (use_display_list == GL_COMPILE) {
					for (l = 0; l < 2; l++) {
						int v;
						glGetMultiTexParameterivEXT(texunits[1 + l], target,
							tested[j].pname, &v);
						pass = v == original_values[l] && pass;
					}
					glCallList(list);
				}

				for (l = 0; l < 2; l++) {
					glGetMultiTexParameterivEXT(texunits[1 + l], target, tested[j].pname, &value);

					if (value != tested[j].values[k]) {
						piglit_loge("%s(%s, %s, %s, ...) failed. Expected %d but got %d\n",
							l == 0 ? "glMultiTexParameteriEXT" : "glMultiTexParameterivEXT",
							piglit_get_gl_enum_name(texunits[1 + l]),
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname),
							tested[j].values[k],
							value);
						return PIGLIT_FAIL;
					}
					 if (!piglit_check_gl_error(GL_NO_ERROR)) {
						piglit_loge("%s(%s, %s, %s, ...) failed.\n",
							l == 0 ? "glMultiTexParameteriEXT" : "glMultiTexParameterivEXT",
							piglit_get_gl_enum_name(texunits[1 + l]),
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname));
						return PIGLIT_FAIL;
					}
				}
			}
		}
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_MultiTexParameterfEXT(void* data)
{
	static const GLenum targets[] = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
		GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_CUBE_MAP,
	};
	int i, j, k, l;
	bool pass = true;
	float value;

	const struct pname_value {
		GLenum pname;
		int value_count;
		float values[8];
	} tested[] = {
		{
			GL_TEXTURE_PRIORITY,
			1, { (float) rand() / RAND_MAX }
		},
		{
			GL_TEXTURE_MIN_LOD,
			1, { (float)rand() }
		},
		{
			GL_TEXTURE_MAX_LOD,
			1, { (float)rand() }
		},
		{
			GL_TEXTURE_LOD_BIAS,
			1, { (float)rand() }
		}
	};

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		const GLenum target = targets[i];

		for (j = 0; j < ARRAY_SIZE(tested); j++) {
			const GLenum* texunits = n_texunits(3);
			glActiveTexture(texunits[0]);
			for (k = 0; k < tested[j].value_count; k++) {
				float original_values[2];

				if (use_display_list != GL_NONE)
					glNewList(list, use_display_list);

				for (l = 0; l < 2; l++) {
					glGetMultiTexParameterfvEXT(texunits[1 + l], target,
						tested[j].pname, &original_values[l]);
				}

				glMultiTexParameterfEXT(
					texunits[1], target,
					tested[j].pname,
					tested[j].values[k]);

				glMultiTexParameterfvEXT(
					texunits[2], target,
					tested[j].pname,
					&tested[j].values[k]);

				if (use_display_list != GL_NONE)
					glEndList(list);

				if (use_display_list == GL_COMPILE) {
					for (l = 0; l < 2; l++) {
						float v;
						glGetMultiTexParameterfvEXT(texunits[1 + l], target,
							tested[j].pname, &v);
						pass = v == original_values[l] && pass;
					}
					glCallList(list);
				}

				for (l = 0; l < 2; l++) {
					glGetMultiTexParameterfvEXT(texunits[1 + l], target, tested[j].pname, &value);

					if (value != tested[j].values[k]) {
						piglit_loge("%s(%s, %s, %s, ...) failed. Expected %f but got %f\n",
							l == 0 ? "glMultiTexParameterfEXT" : "glMultiTexParameterfvEXT",
							piglit_get_gl_enum_name(texunits[1 + l]),
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname),
							tested[j].values[k],
							value);
						return PIGLIT_FAIL;
					}
					 if (!piglit_check_gl_error(GL_NO_ERROR)) {
						piglit_loge("%s(%s, %s, %s, ...) failed.\n",
							l == 0 ? "glTextureParameteriEXT" : "glTextureParameterivEXT",
							piglit_get_gl_enum_name(texunits[1 + l]),
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname));
						return PIGLIT_FAIL;
					}
				}
			}
		}
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	int i;
	struct piglit_subtest tests[] = {
		{
			"MultiTexEnviEXT",
			NULL,
			test_MultiTexEnviEXT,
			NULL
		},
		{
			"MultiTexEnvfvEXT",
			NULL,
			test_MultiTexEnvfvEXT,
			NULL
		},
		{
			"MultiTexImage1DEXT",
			NULL,
			test_MultiTexImageNDEXT,
			(void*) 1
		},
		{
			"MultiTexImage2DEXT",
			NULL,
			test_MultiTexImageNDEXT,
			(void*) 2
		},
		{
			"MultiTexImage3DEXT",
			NULL,
			test_MultiTexImageNDEXT,
			(void*) 3
		},
		{
			"MultiTexSubImage1DEXT",
			NULL,
			test_MultiTexSubImageNDEXT,
			(void*) 1
		},
		{
			"MultiTexSubImage2DEXT",
			NULL,
			test_MultiTexSubImageNDEXT,
			(void*) 2
		},
		{
			"MultiTexSubImage3DEXT",
			NULL,
			test_MultiTexSubImageNDEXT,
			(void*) 3
		},
		{
			"CopyMultiTexImage1DEXT",
			NULL,
			test_CopyMultiTexImageNDEXT,
			(void*) 1
		},
		{
			"CopyMultiTexImage2DEXT",
			NULL,
			test_CopyMultiTexImageNDEXT,
			(void*) 2
		},
		{
			"CopyMultiTexSubImage1DEXT",
			NULL,
			test_CopyMultiTexSubImageNDEXT,
			(void*) 1
		},
		{
			"CopyMultiTexSubImage2DEXT",
			NULL,
			test_CopyMultiTexSubImageNDEXT,
			(void*) 2
		},
		{
			"CopyMultiTexSubImage3DEXT",
			NULL,
			test_CopyMultiTexSubImageNDEXT,
			(void*) 3
		},
		{
			"MultiTexGen*EXT",
			NULL,
			test_MultiTexGenSingleValueEXT
		},
		{
			"MultiTexCoordPointerEXT",
			NULL,
			test_MultiTexCoordPointerEXT
		},
		{
			"MultiTexParameterfEXT",
			NULL,
			test_MultiTexParameterfEXT
		},
		{
			"MultiTexParameteriEXT",
			NULL,
			test_MultiTexParameteriEXT
		},
		{
			NULL
		}
	};

	enum piglit_result result = piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);
	list = glGenLists(1);

	/* Re-run the same test but using display list GL_COMPILE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s + display list GL_COMPILE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	/* Re-run the same test but using display list GL_COMPILE_AND_EXECUTE */
	for (i = 0; tests[i].name; i++) {
		char* test_name_display_list;
		asprintf(&test_name_display_list, "%s_AND_EXECUTE", tests[i].name);
		tests[i].name = test_name_display_list;
	}
	use_display_list = GL_COMPILE_AND_EXECUTE;
	result = piglit_run_selected_subtests(tests, NULL, 0, result);

	glDeleteLists(list, 1);

	return result;
}

