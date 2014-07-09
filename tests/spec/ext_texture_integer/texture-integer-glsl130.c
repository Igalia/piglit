/*
 * Copyright (c) 2010 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * Tests GL_EXT_texture_integer texture formats.
 * simpler test modified for GLSL1.30 by airlied
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "texture-integer";

static GLint TexWidth = 16, TexHeight = 16;
static GLuint Texture;

static GLint BiasUniform = -1, TexUniform = -1;

struct format_info
{
	const char *Name;
	GLenum IntFormat, BaseFormat;
	GLuint BitsPerChannel;
	GLboolean Signed;
};

static const struct format_info Formats[] = {
	/*   { "GL_RGBA", GL_RGBA, GL_RGBA, 8, GL_FALSE },*/
	{ "GL_RGBA8I_EXT",   GL_RGBA8I_EXT,   GL_RGBA_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_RGBA8UI_EXT",  GL_RGBA8UI_EXT , GL_RGBA_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_RGBA16I_EXT",  GL_RGBA16I_EXT,  GL_RGBA_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_RGBA16UI_EXT", GL_RGBA16UI_EXT, GL_RGBA_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_RGBA32I_EXT",  GL_RGBA32I_EXT,  GL_RGBA_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_RGBA32UI_EXT", GL_RGBA32UI_EXT, GL_RGBA_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_RGBA8I_EXT (bgra)",   GL_RGBA8I_EXT,   GL_BGRA_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_RGBA8UI_EXT (bgra)",  GL_RGBA8UI_EXT , GL_BGRA_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_RGBA16I_EXT (bgra)",  GL_RGBA16I_EXT,  GL_BGRA_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_RGBA16UI_EXT (bgra)", GL_RGBA16UI_EXT, GL_BGRA_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_RGBA32I_EXT (bgra)",  GL_RGBA32I_EXT,  GL_BGRA_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_RGBA32UI_EXT (bgra)", GL_RGBA32UI_EXT, GL_BGRA_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_RGB8I_EXT",   GL_RGB8I_EXT,   GL_RGB_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_RGB8UI_EXT",  GL_RGB8UI_EXT , GL_RGB_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_RGB16I_EXT",  GL_RGB16I_EXT,  GL_RGB_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_RGB16UI_EXT", GL_RGB16UI_EXT, GL_RGB_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_RGB32I_EXT",  GL_RGB32I_EXT,  GL_RGB_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_RGB32UI_EXT", GL_RGB32UI_EXT, GL_RGB_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_ALPHA8I_EXT",   GL_ALPHA8I_EXT,   GL_ALPHA_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_ALPHA8UI_EXT",  GL_ALPHA8UI_EXT , GL_ALPHA_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_ALPHA16I_EXT",  GL_ALPHA16I_EXT,  GL_ALPHA_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_ALPHA16UI_EXT", GL_ALPHA16UI_EXT, GL_ALPHA_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_ALPHA32I_EXT",  GL_ALPHA32I_EXT,  GL_ALPHA_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_ALPHA32UI_EXT", GL_ALPHA32UI_EXT, GL_ALPHA_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_LUMINANCE8I_EXT",   GL_LUMINANCE8I_EXT,   GL_LUMINANCE_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_LUMINANCE8UI_EXT",  GL_LUMINANCE8UI_EXT , GL_LUMINANCE_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_LUMINANCE16I_EXT",  GL_LUMINANCE16I_EXT,  GL_LUMINANCE_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_LUMINANCE16UI_EXT", GL_LUMINANCE16UI_EXT, GL_LUMINANCE_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_LUMINANCE32I_EXT",  GL_LUMINANCE32I_EXT,  GL_LUMINANCE_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_LUMINANCE32UI_EXT", GL_LUMINANCE32UI_EXT, GL_LUMINANCE_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_LUMINANCE_ALPHA8I_EXT",   GL_LUMINANCE_ALPHA8I_EXT,   GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_LUMINANCE_ALPHA8UI_EXT",  GL_LUMINANCE_ALPHA8UI_EXT , GL_LUMINANCE_ALPHA_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_LUMINANCE_ALPHA16I_EXT",  GL_LUMINANCE_ALPHA16I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_LUMINANCE_ALPHA16UI_EXT", GL_LUMINANCE_ALPHA16UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_LUMINANCE_ALPHA32I_EXT",  GL_LUMINANCE_ALPHA32I_EXT,  GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_LUMINANCE_ALPHA32UI_EXT", GL_LUMINANCE_ALPHA32UI_EXT, GL_LUMINANCE_ALPHA_INTEGER_EXT, 32, GL_FALSE },

	{ "GL_INTENSITY8I_EXT",   GL_INTENSITY8I_EXT,   GL_RED_INTEGER_EXT, 8,  GL_TRUE  },
	{ "GL_INTENSITY8UI_EXT",  GL_INTENSITY8UI_EXT , GL_RED_INTEGER_EXT, 8,  GL_FALSE },
	{ "GL_INTENSITY16I_EXT",  GL_INTENSITY16I_EXT,  GL_RED_INTEGER_EXT, 16, GL_TRUE  },
	{ "GL_INTENSITY16UI_EXT", GL_INTENSITY16UI_EXT, GL_RED_INTEGER_EXT, 16, GL_FALSE },
	{ "GL_INTENSITY32I_EXT",  GL_INTENSITY32I_EXT,  GL_RED_INTEGER_EXT, 32, GL_TRUE  },
	{ "GL_INTENSITY32UI_EXT", GL_INTENSITY32UI_EXT, GL_RED_INTEGER_EXT, 32, GL_FALSE },

};

static const struct format_info rg_formats[] = {
	{ "GL_RG8I",   GL_RG8I,   GL_RG_INTEGER, 8,  GL_TRUE  },
	{ "GL_RG8UI",  GL_RG8UI , GL_RG_INTEGER, 8,  GL_FALSE },
	{ "GL_RG16I",  GL_RG16I,  GL_RG_INTEGER, 16, GL_TRUE  },
	{ "GL_RG16UI", GL_RG16UI, GL_RG_INTEGER, 16, GL_FALSE },
	{ "GL_RG32I",  GL_RG32I,  GL_RG_INTEGER, 32, GL_TRUE  },
	{ "GL_RG32UI", GL_RG32UI, GL_RG_INTEGER, 32, GL_FALSE },
	{ "GL_R8I",   GL_R8I,   GL_RED_INTEGER, 8,  GL_TRUE  },
	{ "GL_R8UI",  GL_R8UI , GL_RED_INTEGER, 8,  GL_FALSE },
	{ "GL_R16I",  GL_R16I,  GL_RED_INTEGER, 16, GL_TRUE  },
	{ "GL_R16UI", GL_R16UI, GL_RED_INTEGER, 16, GL_FALSE },
	{ "GL_R32I",  GL_R32I,  GL_RED_INTEGER, 32, GL_TRUE  },
	{ "GL_R32UI", GL_R32UI, GL_RED_INTEGER, 32, GL_FALSE },
};

/* the rgb10 formats overload the Signed TRUE/FALSE member to test
 * the _REV and non-REV component ordering.
 */
static const struct format_info rgb10_formats[] = {
	{ "GL_RGB10_A2UI", GL_RGB10_A2UI, GL_RGBA_INTEGER_EXT, 10, GL_FALSE },
	{ "GL_RGB10_A2UI (bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER_EXT, 10, GL_FALSE },
	{ "GL_RGB10_A2UI (rev)", GL_RGB10_A2UI, GL_RGBA_INTEGER_EXT, 10, GL_TRUE },
	{ "GL_RGB10_A2UI (rev bgra)", GL_RGB10_A2UI, GL_BGRA_INTEGER_EXT, 10, GL_TRUE },
};

static const char *FragShaderText =
	"#version 130\n"
	"uniform vec4 bias; \n"
	"uniform isampler2D tex; \n"
	"void main() \n"
	"{ \n"
	"   vec4 t = vec4(texture(tex, gl_TexCoord[0].xy)); \n"
	"   gl_FragColor = t + bias; \n"
	"} \n";

static GLuint Program;


static int
get_max_val(const struct format_info *info)
{
	int max;

	switch (info->BitsPerChannel) {
	case 8:
		if (info->Signed)
			max = 127;
		else
			max = 255;
		break;
	case 10:
		max = 1023;
		break;
	case 16:
		if (info->Signed)
			max = 32767;
		else
			max = 65535;
		break;
	case 32:
		if (info->Signed)
			max = 10*1000; /* don't use 0x8fffffff to avoid overflow issues */
		else
			max = 20*1000;
		break;
	default:
		assert(0);
		max = 0;
	}

	return max;
}


static int
num_components(GLenum format)
{
	switch (format) {
	case GL_RGBA:
	case GL_RGBA_INTEGER_EXT:
	case GL_BGRA_INTEGER_EXT:
		return 4;
	case GL_RGB_INTEGER_EXT:
		return 3;
	case GL_RG_INTEGER:
		return 2;
	case GL_ALPHA_INTEGER_EXT:
		return 1;
	case GL_LUMINANCE_INTEGER_EXT:
		return 1;
	case GL_LUMINANCE_ALPHA_INTEGER_EXT:
		return 2;
	case GL_RED_INTEGER_EXT:
		return 1;
	default:
		assert(0);
		return 0;
	}
}


static void
fill_array(int comps, int texels, void *buf, int bpp, const int val[4])
{
	int i, j;

	switch (bpp) {
	case 8:
		{
			GLubyte *b = (GLubyte *) buf;
			for (i = 0; i < texels; i++) {
				for (j = 0; j < comps; j++) {
					b[i * comps + j] = val[j];
				}
			}
		}
		break;
	case 16:
		{
			GLushort *b = (GLushort *) buf;
			for (i = 0; i < texels; i++) {
				for (j = 0; j < comps; j++) {
					b[i * comps + j] = val[j];
				}
			}
		}
		break;
	case 32:
		{
			GLuint *b = (GLuint *) buf;
			for (i = 0; i < texels; i++) {
				for (j = 0; j < comps; j++) {
					b[i * comps + j] = val[j];
				}
			}
		}
		break;
	default:
		assert(0);
	}
}

static void
fill_array_rgb10(int comps, int texels, void *buf, int type,
	   const int val[4])
{
	int i;
	GLuint *b = (GLuint *)buf;

	if (type == GL_UNSIGNED_INT_2_10_10_10_REV) {
		for (i = 0; i < texels; i++) {
			b[i] = (val[0] & 0x3ff) << 0 |
				(val[1] & 0x3ff) << 10 |
				(val[2] & 0x3ff) << 20 |
				(val[3] & 0x3) << 30;
		}
	} else if (type == GL_UNSIGNED_INT_10_10_10_2) {
		for (i = 0; i < texels; i++) {
			b[i] = (val[3] & 0x3) << 0 |
			       (val[2] & 0x3ff) << 2 |
			       (val[1] & 0x3ff) << 12 |
			       (val[0] & 0x3ff) << 22;
		}
	}
}

static GLenum
get_datatype(const struct format_info *info)
{
	switch (info->BitsPerChannel) {
	case 8:
		return info->Signed ? GL_BYTE : GL_UNSIGNED_BYTE;
	case 10:
		return info->Signed ? GL_UNSIGNED_INT_10_10_10_2 : GL_UNSIGNED_INT_2_10_10_10_REV;
	case 16:
		return info->Signed ? GL_SHORT : GL_UNSIGNED_SHORT;
	case 32:
		return info->Signed ? GL_INT : GL_UNSIGNED_INT;
	default:
		assert(0);
		return 0;
	}
}


static GLboolean
check_error(const char *file, int line)
{
	GLenum err = glGetError();
	if (err) {
		fprintf(stderr, "%s: error 0x%x at %s:%d\n",
			TestName, err, file, line);
		return GL_TRUE;
	}
	return GL_FALSE;
}


/** \return GL_TRUE for pass, GL_FALSE for fail */
static GLboolean
test_format(const struct format_info *info)
{
	const int max = get_max_val(info);
	const int comps = num_components(info->BaseFormat);
	const int texels = TexWidth * TexHeight;
	const GLenum type = get_datatype(info);
	const int w = piglit_width / 10;
	const int h = piglit_height / 10;
	const float error = 2.0 / 255.0; /* XXX fix */
	GLfloat expected[4];
	void *buf;
	int value[4];
	GLfloat result[4], bias[4];
	GLint f;
	GLfloat temp;

	/* pick random texture color */
	value[0] = rand() % max;
	value[1] = rand() % max;
	value[2] = rand() % max;
	value[3] = rand() % max;

	/* alloc, fill texture image */
	if (info->BitsPerChannel == 10) {
		value[3] = rand() % 3;
		buf = malloc(texels * 4);
		fill_array_rgb10(comps, texels, buf, type, value);
	} else {
		buf = malloc(comps * texels * info->BitsPerChannel / 8);
		fill_array(comps, texels, buf, info->BitsPerChannel, value);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, info->IntFormat, TexWidth, TexHeight, 0,
		     info->BaseFormat, type, buf);

	if (check_error(__FILE__, __LINE__))
		return GL_FALSE;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
				 &f);
	/*
	  assert(f == info->IntFormat);
	*/

	/* setup expected polygon color */
	expected[0] = 0.25;
	expected[1] = 0.50;
	expected[2] = 0.75;
	expected[3] = 1.00;

	/* Need to swizzle things depending on texture format.
	 *
	 * Also, for texture formats with no storage for a particular
	 * channel, instead of reading the randomly-chosen value
	 * above, we need to expect to read a 0, (for Green or Blue
	 * channels), or a 1 (for Alpha).
	 *
	 * Note: The alpha value read is an integer 1, not a
	 * maximum-valued integer representing 1.0.
	 */
	switch (info->BaseFormat) {
	case GL_RGBA_INTEGER_EXT:
		/* nothing */
		break;
	case GL_BGRA_INTEGER_EXT:
		/* swap 0 and 2 */
		temp = expected[2];
		expected[2] = expected[0];
		expected[0] = temp;
		temp = value[2];
		value[2] = value[0];
		value[0] = temp;
		break;
	case GL_RGB_INTEGER_EXT:
		value[3] = 1;
		break;
	case GL_RG_INTEGER:
		value[2] = 0;
		value[3] = 1;
		break;
	case GL_ALPHA_INTEGER_EXT:
		expected[0] = expected[1] = expected[2] = 0.0;
		expected[3] = 0.25;
		value[3] = value[0];
		break;
	case GL_LUMINANCE_INTEGER_EXT:
		expected[0] = expected[1] = expected[2] = 0.25;
		expected[3] = 1.0;
		value[1] = value[2] = value[0];
		value[3] = 1.0;
		break;
	case GL_LUMINANCE_ALPHA_INTEGER_EXT:
		expected[0] = expected[1] = expected[2] = 0.25;
		value[3] = value[1];
		value[1] = value[2] = value[0];
		break;
	case GL_RED_INTEGER_EXT:
		if (info->IntFormat == GL_INTENSITY8I_EXT ||
		    info->IntFormat == GL_INTENSITY8UI_EXT ||
		    info->IntFormat == GL_INTENSITY16I_EXT ||
		    info->IntFormat == GL_INTENSITY16UI_EXT ||
		    info->IntFormat == GL_INTENSITY32I_EXT ||
		    info->IntFormat == GL_INTENSITY32UI_EXT) {
			expected[0] = expected[1] = expected[2] = expected[3] = 0.25;
			value[1] = value[2] = value[3] = value[0];
		} else {
			value[1] = value[2] = 0;
			value[3] = 1;
		}			
		break;
	default:
		;
	}

	/* compute, set test bias */
	bias[0] = expected[0] - value[0];
	bias[1] = expected[1] - value[1];
	bias[2] = expected[2] - value[2];
	bias[3] = expected[3] - value[3];
	glUniform4fv(BiasUniform, 1, bias);

	/* draw */
	glClearColor(0, 1, 1, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);   glVertex2f(0, 0);
	glTexCoord2f(1, 0);   glVertex2f(w, 0);
	glTexCoord2f(1, 1);   glVertex2f(w, h);
	glTexCoord2f(0, 1);   glVertex2f(0, h);
	glEnd();

	if (check_error(__FILE__, __LINE__))
		return GL_FALSE;

	/* test */
	glReadPixels(w/2, h/2, 1, 1, GL_RGBA, GL_FLOAT, result);

	if (check_error(__FILE__, __LINE__))
		return GL_FALSE;

	if (fabsf(result[0] - expected[0]) > error ||
	    fabsf(result[1] - expected[1]) > error ||
	    fabsf(result[2] - expected[2]) > error ||
	    fabsf(result[3] - expected[3]) > error) {
		fprintf(stderr, "%s: failure with format %s:\n",
			TestName, info->Name);
		fprintf(stderr, "  texture color = %d, %d, %d, %d\n",
			value[0], value[1], value[2], value[3]);
		fprintf(stderr, "  expected color = %g, %g, %g, %g\n",
			expected[0], expected[1], expected[2], expected[3]);
		fprintf(stderr, "  result color = %g, %g, %g, %g\n",
			result[0], result[1], result[2], result[3]);
		return GL_FALSE;
	}

	piglit_present_results();

	free(buf);

	return GL_TRUE;
}


static GLboolean
test_general_formats(void)
{
	int f, i;

	for (f = 0; f < ARRAY_SIZE(Formats); f++) {
		for (i = 0; i < 5; i++) {
			if (!test_format(&Formats[f]))
				return GL_FALSE;
		}
	}

	if (piglit_is_extension_supported("GL_ARB_texture_rg")) {
		for (f = 0; f < ARRAY_SIZE(rg_formats); f++) {
			for (i = 0; i < 5; i++) {
				if (!test_format(&rg_formats[f]))
					return GL_FALSE;
			}
		}
	}

	if (piglit_is_extension_supported("GL_ARB_texture_rgb10_a2ui")) {
		for (f = 0; f < ARRAY_SIZE(rgb10_formats); f++) {
			for (i = 0; i < 5; i++) {
				if (!test_format(&rgb10_formats[f]))
					return GL_FALSE;
			}
		}
	}
	return GL_TRUE;
}


static GLboolean
test_specific_formats(void)
{
	/* These format combinations should all work */
	struct {
		GLenum intFormat, srcFormat, srcType;
	} formats[] = {
		{ GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE },
		{ GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_SHORT },
		{ GL_RGBA8UI_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
		{ GL_RGBA8UI_EXT, GL_BGRA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
		{ GL_LUMINANCE8I_EXT, GL_RGBA_INTEGER, GL_UNSIGNED_INT_8_8_8_8 },
		{ GL_RGB16I_EXT, GL_RGB_INTEGER, GL_UNSIGNED_SHORT_5_6_5 },
		{ GL_RGB32I_EXT, GL_RGB_INTEGER, GL_UNSIGNED_SHORT_5_6_5 }
	};
	int i;
	GLenum err;
	GLboolean pass = GL_TRUE;

	while (glGetError() != GL_NO_ERROR)
		;

	/* All of the packed formats require GL_ARB_texture_rgb10_a2ui.
	 */
	if (!piglit_is_extension_supported("GL_ARB_texture_rgb10_a2ui")) {
		return GL_TRUE;
	}

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		glTexImage2D(GL_TEXTURE_2D, 0, formats[i].intFormat,
			     16, 16, 0,
			     formats[i].srcFormat, formats[i].srcType, NULL);
		err = glGetError();
		if (err != GL_NO_ERROR) {
			fprintf(stderr, "%s failure: glTexImage2D(0x%x, 0x%x, 0x%x) generated"
				" error 0x%x (case %d)\n",
				TestName, formats[i].intFormat,
				formats[i].srcFormat, formats[i].srcType, err, i);
			pass = GL_FALSE;
		}
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	if (!test_general_formats())
		return PIGLIT_FAIL;

	if (!test_specific_formats())
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_integer");
	piglit_require_GLSL_version(130);

	Program = piglit_build_simple_program(NULL, FragShaderText);

	glUseProgram(Program);

	BiasUniform = glGetUniformLocation(Program, "bias");
	TexUniform = glGetUniformLocation(Program, "tex");

	glUniform1i(TexUniform, 0);  /* tex unit zero */

	(void) check_error(__FILE__, __LINE__);

	glGenTextures(1, &Texture);
	glBindTexture(GL_TEXTURE_2D, Texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	(void) check_error(__FILE__, __LINE__);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
