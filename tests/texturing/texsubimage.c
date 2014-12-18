/*
 * Copyright © 2011 VMware, Inc.
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
 *
 */


/**
 * This should expose any errors in texel addressing within a texture image
 * when calling glTexSubImage1D/2D/3D().
 *
 * Brian Paul
 * October 2011
 */


#include "piglit-util-gl.h"
#include "../fbo/fbo-formats.h"

#define STR(x) #x
#define STRINGIFY(x) STR(x)

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

	config.window_width = 512;
	config.window_height = 512;

PIGLIT_GL_TEST_CONFIG_END

/**
 * This is a subset of the formats in fbo-formats.h
 * We don't test non-color, float, or int/uint textures at this time.
 */
static const struct test_desc texsubimage_test_sets[] = {
	{
		core,
		ARRAY_SIZE(core),
		"Core formats",
		GL_UNSIGNED_NORMALIZED,
	},
	{
		tdfx_texture_compression_fxt1,
		ARRAY_SIZE(tdfx_texture_compression_fxt1),
		"GL_3DFX_texture_compression_FXT1",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression",
		 "GL_3DFX_texture_compression_FXT1"},
	},
	{
		ext_texture_compression_s3tc,
		ARRAY_SIZE(ext_texture_compression_s3tc),
		"GL_EXT_texture_compression_s3tc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_ARB_texture_compression",
		 "GL_EXT_texture_compression_s3tc"},
	},
	{
		ext_texture_compression_rgtc,
		ARRAY_SIZE(ext_texture_compression_rgtc),
		"GL_EXT_texture_compression_rgtc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_rgtc"}
	},
	{
		ext_texture_compression_latc,
		ARRAY_SIZE(ext_texture_compression_latc),
		"GL_EXT_texture_compression_latc",
		GL_UNSIGNED_NORMALIZED,
		{"GL_EXT_texture_compression_latc"}
	}
};

/* Default texture size. Other values might be used if the texture has
 * less dimensions or other restrictions */
#define DEFAULT_TEX_WIDTH 128
#define DEFAULT_TEX_HEIGHT 64
#define DEFAULT_TEX_DEPTH 8

/* List of texture targets to test, terminated by GL_NONE */
static const GLenum *test_targets;

static const char fragment_1d_array[] =
	"#extension GL_EXT_texture_array : require\n"
	"uniform sampler1DArray tex;\n"
	"const float TEX_HEIGHT = " STRINGIFY(DEFAULT_TEX_HEIGHT) ".0;\n"
	"void\n"
	"main()\n"
	"{\n"
	"        float layer = gl_TexCoord[0].t * TEX_HEIGHT - 0.5;\n"
	"        gl_FragColor = texture1DArray(tex, vec2(gl_TexCoord[0].s,\n"
	"                                                layer));\n"
	"}\n";

static const char fragment_2d_array[] =
	"#extension GL_EXT_texture_array : require\n"
	"uniform sampler2DArray tex;\n"
	"const float TEX_DEPTH = " STRINGIFY(DEFAULT_TEX_DEPTH) ".0;\n"
	"void\n"
	"main()\n"
	"{\n"
	"        float layer = gl_TexCoord[0].p * TEX_DEPTH - 0.5;\n"
	"        gl_FragColor = texture2DArray(tex, vec3(gl_TexCoord[0].st,\n"
	"                                                layer));\n"
	"}\n";

/**
 * XXX add this to piglit-util if useful elsewhere.
 */
static GLvoid
piglit_draw_rect_tex3d(float x, float y, float w, float h,
		       float tx, float ty, float tw, float th,
		       float tz0, float tz1)
{
	float verts[4][4];
	float tex[4][3];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	tex[0][2] = tz0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	tex[1][2] = tz1;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	tex[2][2] = tz1;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;
	tex[3][2] = tz0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static GLboolean
equal_images(GLenum target,
	     const GLubyte *original_ref,
	     const GLubyte *updated_ref,
	     const GLubyte *testImg,
	     GLuint w, GLuint h, GLuint d,
	     GLuint tx, GLuint ty, GLuint tz,
	     GLuint tw, GLuint th, GLuint td)
{
	const GLubyte *ref;
	GLuint z, y, x;

	switch (target) {
	case GL_TEXTURE_1D:
		ty = 0;
		th = 1;
		/* flow through */
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
		tz = 0;
		td = 1;
		break;
	}

	for (z = 0; z < d; z++) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				if (x >= tx && x < tx + tw &&
				    y >= ty && y < ty + th &&
				    z >= tz && z < tz + td)
					ref = updated_ref;
				else
					ref = original_ref;

				if (memcmp(ref, testImg, 4))
					return GL_FALSE;

				testImg += 4;
				original_ref += 4;
				updated_ref += 4;
			}
		}
	}

	return GL_TRUE;
}

/**
 * Get block size for compressed format.
 * \return GL_TRUE if format is compressed, GL_FALSE otherwise
 * XXX this could be a piglit util function if useful elsewhere.
 */
static GLboolean
get_format_block_size(GLenum format, GLuint *bw, GLuint *bh)
{
	switch (format) {
	case GL_COMPRESSED_RGB_FXT1_3DFX:
	case GL_COMPRESSED_RGBA_FXT1_3DFX:
		*bw = 8;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_RED:
	case GL_COMPRESSED_RED_RGTC1_EXT:
	case GL_COMPRESSED_RG:
	case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
	case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
		*bw = 4;
		*bh = 4;
		return GL_TRUE;
	default:
		*bw = *bh = 1;
		return GL_FALSE;
	}
}

/**
 * Draw each image of the texture to the framebuffer and then save the
 * entire thing to a buffer with glReadPixels().
 */
static void
draw_and_read_texture(GLuint w, GLuint h, GLuint d, GLubyte *ref)
{
	int i;

	for (i = 0; i < d; i++) {
		float tz = (i + 0.5f) / d;
		piglit_draw_rect_tex3d(0, i * h, /* x/y */
				       w, h,
				       0.0, 0.0, /* tx/ty */
				       1.0, 1.0, /* tw/th */
				       tz, tz /* tz0/tz1 */);
	}

	glReadPixels(0, 0, w, h * d, GL_RGBA, GL_UNSIGNED_BYTE, ref);
}

static GLuint
create_texture(GLenum target,
	       GLenum intFormat,
	       GLsizei w, GLsizei h, GLsizei d,
	       GLenum srcFormat,
	       const GLubyte *img)
{
	GLuint tex;

	glPixelStorei(GL_UNPACK_ROW_LENGTH, w);
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, h);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, 0);
	if (d > 1) {
		glTexImage3D(target, 0, intFormat, w, h, d, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	}
	else if (h > 1) {
		glTexImage2D(target, 0, intFormat, w, h, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	}
	else if (w > 1) {
		glTexImage1D(target, 0, intFormat, w, 0,
			     srcFormat, GL_UNSIGNED_BYTE, img);
	} else {
		assert(!"Unknown texture dimensions");
	}

	return tex;
}

/**
 * Create two textures with different reference values. Draw both of
 * the textures to the framebuffer and save the reference images with
 * glReadPixels.
 *
 * Loop:
 *  - Create another texture with the same initial values as the first
 *    texture
 *  - replace a random sub-region of the texture image with values from
 *    the 2nd texture
 *  - draw the texture to the framebuffer and read back with glReadPixels
 *  - compare reference images to test image choosing either the first
 *    or second reference image for each pixel depending on whether it
 *    is within the updated region
 * \param target  GL_TEXTURE_1D/2D/3D
 * \param intFormat  the internal texture format
 */
static GLboolean
test_format(GLenum target, GLenum intFormat)
{
	const GLenum srcFormat = GL_RGBA;
	GLuint w = DEFAULT_TEX_WIDTH;
	GLuint h = DEFAULT_TEX_HEIGHT;
	GLuint d = DEFAULT_TEX_DEPTH;
	GLuint tex, i, j, k, n, t;
	GLubyte *original_img, *original_ref;
	GLubyte *updated_img, *updated_ref;
	GLubyte *testImg;
	GLboolean pass = GL_TRUE;
	GLuint bw, bh, wMask, hMask, dMask;
	get_format_block_size(intFormat, &bw, &bh);
	wMask = ~(bw-1);
	hMask = ~(bh-1);
	dMask = ~0;

	if (target != GL_TEXTURE_3D && target != GL_TEXTURE_2D_ARRAY)
		d = 1;
	if (target == GL_TEXTURE_1D)
		h = 1;

	original_img = (GLubyte *) malloc(w * h * d * 4);
	original_ref = (GLubyte *) malloc(w * h * d * 4);
	updated_img = (GLubyte *) malloc(w * h * d * 4);
	updated_ref = (GLubyte *) malloc(w * h * d * 4);
	testImg = (GLubyte *) malloc(w * h * d * 4);

	/* fill source tex images */
	n = 0;
	for (i = 0; i < d; i++) {
		for (j = 0; j < h; j++) {
			for (k = 0; k < w; k++) {
				original_img[n + 0] = j * 4;
				original_img[n + 1] = k * 2;
				original_img[n + 2] = i * 16;
				original_img[n + 3] = 255;

				/* Swizzle the components in the
				 * updated image
				 */
				updated_img[n + 0] = original_img[n + 1];
				updated_img[n + 1] = original_img[n + 2];
				updated_img[n + 2] = original_img[n + 0];
				updated_img[n + 3] = original_img[n + 3];

				n += 4;
			}
		}
	}

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);


	/* draw original reference image */
	tex = create_texture(target, intFormat, w, h, d,
			     srcFormat, original_img);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_and_read_texture(w, h, d, original_ref);
	glDeleteTextures(1, &tex);

	/* draw updated reference image */
	tex = create_texture(target, intFormat, w, h, d,
			     srcFormat, updated_img);
	glClear(GL_COLOR_BUFFER_BIT);
	draw_and_read_texture(w, h, d, updated_ref);
	glDeleteTextures(1, &tex);

	for (t = 0; t < 10; t++) {
		/* Choose random region of texture to update.
		 * Use sizes and positions that are multiples of
		 * the compressed block size.
		 */
		GLint tw = (rand() % w) & wMask;
		GLint th = (rand() % h) & hMask;
		GLint td = (rand() % d) & dMask;
		GLint tx = (rand() % (w - tw)) & wMask;
		GLint ty = (rand() % (h - th)) & hMask;
		GLint tz = (rand() % (d - td)) & dMask;

		/* Recreate the original texture */
		tex = create_texture(target, intFormat, w, h, d,
				     srcFormat, original_img);

		assert(tx + tw <= w);
		assert(ty + th <= h);
		assert(tz + td <= d);

		/* replace texture region with data from updated image */
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, tx);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, ty);
		glPixelStorei(GL_UNPACK_SKIP_IMAGES, tz);
		if (d > 1) {
			glTexSubImage3D(target, 0, tx, ty, tz, tw, th, td,
					srcFormat, GL_UNSIGNED_BYTE,
					updated_img);
		} else if (h > 1) {
			glTexSubImage2D(target, 0, tx, ty, tw, th,
					srcFormat, GL_UNSIGNED_BYTE,
					updated_img);
		} else if (w > 1) {
			glTexSubImage1D(target, 0, tx, tw,
					srcFormat, GL_UNSIGNED_BYTE,
					updated_img);
		} else {
			assert(!"Unknown image dimensions");
		}

		/* draw test image */
		glClear(GL_COLOR_BUFFER_BIT);
		draw_and_read_texture(w, h, d, testImg);

		glDeleteTextures(1, &tex);

		piglit_present_results();

		if (!equal_images(target,
				  original_ref, updated_ref, testImg,
				  w, h, d,
				  tx, ty, tz, tw, th, td)) {
			printf("texsubimage failed\n");
			printf("  target: %s\n", piglit_get_gl_enum_name(target));
			printf("  internal format: %s\n", piglit_get_gl_enum_name(intFormat));
			printf("  region: %d, %d  %d x %d\n", tx, ty, tw, th);
			pass = GL_FALSE;
			break;
		}
	}

	free(original_img);
	free(original_ref);
	free(updated_img);
	free(updated_ref);
	free(testImg);

	return pass;
}


/**
 * Test all formats in texsubimage_test_sets[] for the given
 * texture target.
 */
static GLboolean
test_formats(GLenum target)
{
	GLboolean pass = GL_TRUE;
	GLuint program = 0;
	int i, j;

	switch (target) {
	case GL_TEXTURE_1D_ARRAY:
		program = piglit_build_simple_program(NULL, fragment_1d_array);
		break;
	case GL_TEXTURE_2D_ARRAY:
		program = piglit_build_simple_program(NULL, fragment_2d_array);
		break;
	default:
		glEnable(target);
		break;
	}

	if (program != 0) {
		GLuint tex_location;

		glUseProgram(program);
		tex_location = glGetUniformLocation(program, "tex");
		glUniform1i(tex_location, 0);
	}

	/* loop over the format groups */
	for (i = 0; i < ARRAY_SIZE(texsubimage_test_sets); i++) {
		const struct test_desc *set = &texsubimage_test_sets[i];
		GLboolean skip = GL_FALSE;

		/* only test compressed formats with 2D textures */
		if (i > 0 && target != GL_TEXTURE_2D)
			continue;

		/* skip formats for unsupported extensions */
		for (j = 0; j < ARRAY_SIZE(set->ext); j++) {
			if (set->ext[j] &&
			    !piglit_is_extension_supported(set->ext[j])) {
				/* req'd extension not supported */
				skip = GL_TRUE;
				break;
			}
		}
		if (skip)
			continue;

		/* loop over formats in the set */
		for (j = 0; j < set->num_formats; j++) {
			if (!test_format(target,
					 set->format[j].internalformat)) {
				pass = GL_FALSE;
			}
		}
	}

	if (program == 0) {
		glDisable(target);
	} else {
		glUseProgram(0);
		glDeleteProgram(program);
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	/* Loop over 1/2/3D texture targets */
	for (i = 0; test_targets[i] != GL_NONE; i++) {
		pass = test_formats(test_targets[i]) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	static const GLenum core_targets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D,
		GL_NONE
	};
	static const GLenum array_targets[] = {
		GL_TEXTURE_1D_ARRAY_EXT,
		GL_TEXTURE_2D_ARRAY_EXT,
		GL_NONE
	};

	test_targets = core_targets;

	if (argc > 1) {
		if (!strcmp(argv[1], "array")) {
			piglit_require_extension("GL_EXT_texture_array");
			piglit_require_GLSL();
			test_targets = array_targets;
		} else {
			goto handled_targets;
		}

		argc--;
		argv++;
	}
 handled_targets:

	fbo_formats_init(argc, argv, 0);
	(void) fbo_formats_display;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
