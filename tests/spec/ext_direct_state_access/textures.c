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

	config.supports_gl_compat_version = 12;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");

}

static GLuint
init_texture(GLenum target, int* image_size, float** expected_pixels)
{
	GLuint tex;

	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	const int depth = (target == GL_TEXTURE_3D) ? 2 : 1;
	float* image = piglit_rgbw_image(GL_RGBA, piglit_width, height * depth,
					 false, GL_UNSIGNED_NORMALIZED);
	*image_size = piglit_width * height * depth * 4 * sizeof(float);

	glGenTextures(1, &tex);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteriEXT(tex, target, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	if (target == GL_TEXTURE_1D) {
		glTextureImage1DEXT(tex, target, 0, GL_RGBA,
				    piglit_width, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else if (target == GL_TEXTURE_2D) {
		glTextureImage2DEXT(tex, target, 0, GL_RGBA,
				    piglit_width, height, 0,
				    GL_RGBA, GL_FLOAT, image);
	} else {
		/* 2 layers 3D image */
		glTextureImage3DEXT(tex, target, 0, GL_RGBA,
		    piglit_width, height, depth, 0,
		    GL_RGBA, GL_FLOAT, image);
	}

	if (expected_pixels != NULL) {
		*expected_pixels = image;
	} else {
		free(image);
	}

	return tex;
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
test_TextureImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* expected_pixels, *got_pixels;
	GLuint tex;

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	tex = init_texture(target, &image_size, &expected_pixels);

	if (use_display_list != GL_NONE)
		glEndList(list);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been initialized yet */
		pass = !glIsTexture(tex);
		glCallList(list);
	}

	got_pixels = (float*) malloc(image_size);
	glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	pass = memcmp(expected_pixels, got_pixels, image_size) == 0 && pass;

	free(expected_pixels);
	free(got_pixels);

	/* The GL_EXT_direct_state_access spec says:
	 *
	 *    INVALID_OPERATION is generated [...] if the target parameter does
	 *    not match the target type of the texture object named by the texture
	 *    parameter.
	 */
	if (n == 2) {
		glTextureImage2DEXT(tex, GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA,
				    piglit_width, piglit_height, 0,
				    GL_RGBA, GL_FLOAT, got_pixels);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	}

	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_TextureSubImageNDEXT(void* data)
{
	bool pass = true;
	int i;
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* original_pixels, *modified_pixels, *got_pixels;
	GLuint tex = init_texture(target, &image_size, &original_pixels);
	int len = image_size / sizeof(float);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	/* Replace the whole texture using glTextureSubImageNDEXT, line by line */
	modified_pixels = (float*) malloc(image_size);
	for (i = 0; i < len; i++) {
		modified_pixels[i] = original_pixels[(i + 1) % len];
	}

	if (n == 1) {
		glTextureSubImage1DEXT(tex, target, 0,
				       0, piglit_width,
				       GL_RGBA, GL_FLOAT,
				       modified_pixels);
	} else {
		for (i = 0; i < piglit_height; i++) {
			if (n == 2) {
				glTextureSubImage2DEXT(tex,
					target, 0,
					0, i, piglit_width, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * i]);
			} else {
				/* Update the 1st layer of the 3D image */
				glTextureSubImage3DEXT(tex,
					target, 0,
					0, i, 0, piglit_width, 1, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * i]);
				/* And the 2nd layer */
				glTextureSubImage3DEXT(tex,
					target, 0,
					0, i, 1, piglit_width, 1, 1,
					GL_RGBA, GL_FLOAT,
					&modified_pixels[piglit_width * 4 * (i + piglit_height)]);
			}
		}
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	got_pixels = (float*) malloc(image_size);
	glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been modified yet */
		pass = memcmp(original_pixels, got_pixels, image_size) == 0 && pass;
		glCallList(list);
		/* Re-read */
		glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = memcmp(modified_pixels, got_pixels, image_size) == 0 && pass;

	/* Verify error added by the extension:
	     "INVALID_OPERATION is generated [...] if the target parameter does
	      not match the target type of the texture object named by the texture
	      parameter."
	 */
	if (n == 2) {
		glTextureSubImage2DEXT(tex,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0,
			0, 0, 4, 1,
			GL_RGBA, GL_FLOAT,
			modified_pixels);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	}

	glDeleteTextures(1, &tex);
	free(modified_pixels);
	free(original_pixels);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_CopyTextureImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t) data;
	assert(n == 1 || n == 2);
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* original_pixels;
	GLuint tex = init_texture(target, &image_size, &original_pixels);
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	float* got_pixels;

	glClearColor(0.25, 0.5, 0.75, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	switch (target) {
		case GL_TEXTURE_1D:
			glCopyTextureImage1DEXT(tex, target, 0, GL_RGBA, 0, 0, piglit_width, 0);
			break;
		case GL_TEXTURE_2D:
			glCopyTextureImage2DEXT(tex, target, 0, GL_RGBA, 0, 0, piglit_width, piglit_height, 0);
			break;
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	got_pixels = (float*) malloc(piglit_width * height * 4 * sizeof(float));

	/* Compare glGetTextureImageEXT and on screen pixels */
	glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been modified yet */
		pass = memcmp(got_pixels, original_pixels, image_size) == 0;
		glCallList(list);
		glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, height, got_pixels);

	free(got_pixels);
	free(original_pixels);

	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_CopyTextureSubImageNDEXT(void* data)
{
	bool pass = true;
	const int n = (int)(intptr_t) data;
	const GLenum target = dimension_to_target(n);
	int image_size;
	float* original_pixels;
	GLuint tex = init_texture(target, &image_size, &original_pixels);
	const int height = (target == GL_TEXTURE_1D) ? 1 : piglit_height;
	float* got_pixels;

	glClearColor(0.25, 0.5, 0.75, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (use_display_list != GL_NONE)
		glNewList(list, use_display_list);

	switch (target) {
		case GL_TEXTURE_1D:
			glCopyTextureSubImage1DEXT(tex, target, 0, 0, 0, 0, piglit_width);
			break;
		case GL_TEXTURE_2D:
			glCopyTextureSubImage2DEXT(tex, target, 0, 0, 0, 0, 0, piglit_width, piglit_height);
			break;
		case GL_TEXTURE_3D:
			glCopyTextureSubImage3DEXT(tex, target, 0, 0, 0, 0, 0, 0, piglit_width, piglit_height);
			break;
	}

	if (use_display_list != GL_NONE)
		glEndList(list);

	got_pixels = (float*) malloc(image_size);

	/* Compare glGetTextureImageEXT and on screen pixels */
	glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);

	if (use_display_list == GL_COMPILE) {
		/* Texture shouldn't have been modified yet */
		pass = memcmp(got_pixels, original_pixels, image_size) == 0;
		glCallList(list);
		glGetTextureImageEXT(tex, target, 0, GL_RGBA, GL_FLOAT, got_pixels);
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, height, got_pixels);

	free(got_pixels);
	free(original_pixels);

	glDeleteTextures(1, &tex);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_TextureParameteriEXT(void* data)
{
	GLuint tex[2];
	static const GLenum targets[] = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
		GL_TEXTURE_CUBE_MAP,
	};
	int i, j, k, l, value;

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
		GLenum target = targets[i];

		glGenTextures(ARRAY_SIZE(tex), tex);

		for (j = 0; j < ARRAY_SIZE(tested); j++) {
			for (k = 0; k < tested[j].value_count; k++) {
				if (use_display_list != GL_NONE)
					glNewList(list, use_display_list);

				glTextureParameteriEXT(
					tex[0], target,
					tested[j].pname,
					tested[j].values[k]);

				glTextureParameterivEXT(
					tex[1], target,
					tested[j].pname,
					&tested[j].values[k]);

				if (use_display_list != GL_NONE)
					glEndList(list);

				if (use_display_list == GL_COMPILE)
					glCallList(list);

				for (l = 0; l < 2; l++) {
					glGetTextureParameterivEXT(tex[l], target, tested[j].pname, &value);

					if (value != tested[j].values[k]) {
						piglit_loge("%s(%s, %s, ...) failed. Expected %d but got %d\n",
							l == 0 ? "glTextureParameteriEXT" : "glTextureParameterivEXT",
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname),
							tested[j].values[k],
							value);
						return PIGLIT_FAIL;
					}
					 if (!piglit_check_gl_error(GL_NO_ERROR)) {
						piglit_loge("%s(%s, %s, ...) failed.\n",
							l == 0 ? "glTextureParameteriEXT" : "glTextureParameterivEXT",
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname));
						return PIGLIT_FAIL;
					}
				}
			}
		}
		glDeleteTextures(ARRAY_SIZE(tex), tex);
	}
	return PIGLIT_PASS;
}

static enum piglit_result
test_TextureParameterfEXT(void* data)
{
	GLuint tex[2];
	static const GLenum targets[] = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D,
		GL_TEXTURE_1D_ARRAY, GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_CUBE_MAP,
	};
	int i, j, k, l;
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
		GLenum target = targets[i];

		glGenTextures(ARRAY_SIZE(tex), tex);

		for (j = 0; j < ARRAY_SIZE(tested); j++) {
			for (k = 0; k < tested[j].value_count; k++) {
				if (use_display_list != GL_NONE)
					glNewList(list, use_display_list);

				glTextureParameterfEXT(
					tex[0], target,
					tested[j].pname,
					tested[j].values[k]);

				glTextureParameterfvEXT(
					tex[1], target,
					tested[j].pname,
					&tested[j].values[k]);

				if (use_display_list != GL_NONE)
					glEndList(list);

				if (use_display_list == GL_COMPILE)
					glCallList(list);

				for (l = 0; l < 2; l++) {
					glGetTextureParameterfvEXT(tex[l], target, tested[j].pname, &value);

					if (value != tested[j].values[k]) {
						piglit_loge("%s(%s, %s, ...) failed. Expected %f but got %f\n",
							l == 0 ? "glTextureParameterfEXT" : "glTextureParameterfvEXT",
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname),
							tested[j].values[k],
							value);
						return PIGLIT_FAIL;
					}
					 if (!piglit_check_gl_error(GL_NO_ERROR)) {
						piglit_loge("%s(%s, %s, ...) failed.\n",
							l == 0 ? "glTextureParameterfEXT" : "glTextureParameterfvEXT",
							piglit_get_gl_enum_name(target),
							piglit_get_gl_enum_name(tested[j].pname));
						return PIGLIT_FAIL;
					}
				}
			}
		}
		glDeleteTextures(ARRAY_SIZE(tex), tex);
	}
	return PIGLIT_PASS;
}

enum piglit_result
test_EnableDisableEXT(void* data)
{
	/* The GL_EXT_direct_state_access spec says:
	 *
	 * The following commands (introduced by EXT_draw_buffers2):
	 *
	 * 	void EnableIndexedEXT(enum cap, uint index);
	 * 	void DisableIndexedEXT(enum cap, uint index);
	 *
	 * are equivalent (assuming no errors) to the following:
	 *
	 * 	ActiveTexture(TEXTURE0+index);
	 * 	XXX(cap);
	 *
	 * [...] when the cap parameter is one of the texture-related
	 * enable token depending on the active texture state, namely
	 * TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, TEXTURE_CUBE_MAP,
	 * TEXTURE_RECTANGLE_ARB, TEXTURE_GEN_S, TEXTURE_GEN_T,
	 * TEXTURE_GEN_R, or TEXTURE_GEN_Q.
	 */
	static const GLenum caps[] = {
		GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_GEN_S, GL_TEXTURE_GEN_T, GL_TEXTURE_GEN_R,
    		GL_TEXTURE_GEN_Q
	};

	int i, max_texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);

	for (i = 0; i < ARRAY_SIZE(caps); i++) {
		const GLenum cap = caps[i];

		const int index = rand() % max_texture_units;
		const bool value = rand() % 2;
		GLboolean got;
		int got_i;

		glActiveTexture(GL_TEXTURE0 + (index + 1) % max_texture_units);

		if (use_display_list != GL_NONE)
			glNewList(list, use_display_list);

		if (value) {
			glEnableIndexedEXT(cap, index);
		} else {
			glDisableIndexedEXT(cap, index);
		}

		if (use_display_list != GL_NONE)
			glEndList(list);

		if (use_display_list == GL_COMPILE)
			glCallList(list);

		/* Read back with glIsEnabledIndexedEXT */
		if (value != glIsEnabledIndexedEXT(cap, index)) {
			piglit_loge("gl%sIndexedEXT(%s, %d) / glIsEnabledIndexedEXT failed.\n",
				    value ? "Enable" : "Disable",
				    piglit_get_gl_enum_name(cap),
				    index);
			return PIGLIT_FAIL;
		}

		/* Read back with glGetBooleanIndexedvEXT */
		glGetBooleanIndexedvEXT(cap, index, &got);
		if (value != got) {
			piglit_loge("gl%sIndexedEXT(%s, %d) / glGetBooleanIndexedvEXT failed.\n",
				    value ? "Enable" : "Disable",
				    piglit_get_gl_enum_name(cap),
				    index);
			return PIGLIT_FAIL;
		}

		/* Read back with glGetBooleanIndexedvEXT */
		glGetIntegerIndexedvEXT(cap, index, &got_i);
		if (value != got_i) {
			piglit_loge("gl%sIndexedEXT(%s, %d) / glGetIntegerIndexedvEXT failed.\n",
				    value ? "Enable" : "Disable",
				    piglit_get_gl_enum_name(cap),
				    index);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}


enum piglit_result
piglit_display(void)
{
	int i;
	struct piglit_subtest tests[] = {
		{
			"TextureParameteriEXT",
			NULL,
			test_TextureParameteriEXT,
			NULL
		},
		{
			"TextureParameterfEXT",
			NULL,
			test_TextureParameterfEXT,
			NULL
		},
		{
			"TextureImage1DEXT",
			NULL,
			test_TextureImageNDEXT,
			(void*) 1
		},
		{
			"TextureImage2DEXT",
			NULL,
			test_TextureImageNDEXT,
			(void*) 2
		},
		{
			"TextureImage3DEXT",
			NULL,
			test_TextureImageNDEXT,
			(void*) 3
		},
		{
			"TextureSubImage1DEXT",
			NULL,
			test_TextureSubImageNDEXT,
			(void*) 1
		},
		{
			"TextureSubImage2DEXT",
			NULL,
			test_TextureSubImageNDEXT,
			(void*) 2
		},
		{
			"TextureSubImage3DEXT",
			NULL,
			test_TextureSubImageNDEXT,
			(void*) 3
		},
		{
			"CopyTextureImage1DEXT",
			NULL,
			test_CopyTextureImageNDEXT,
			(void*) 1
		},
		{
			"CopyTextureImage2DEXT",
			NULL,
			test_CopyTextureImageNDEXT,
			(void*) 2
		},
		{
			"CopyTextureSubImage1DEXT",
			NULL,
			test_CopyTextureSubImageNDEXT,
			(void*) 1
		},
		{
			"CopyTextureSubImage2DEXT",
			NULL,
			test_CopyTextureSubImageNDEXT,
			(void*) 2
		},
		{
			"CopyTextureSubImage3DEXT",
			NULL,
			test_CopyTextureSubImageNDEXT,
			(void*) 3
		},
		{
			"EnableDisableEXT",
			NULL,
			test_EnableDisableEXT
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
