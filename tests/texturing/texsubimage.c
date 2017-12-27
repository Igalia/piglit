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
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

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

static const GLenum srcFormat = GL_RGBA;

/* List of texture targets to test, terminated by GL_NONE */
static const GLenum *test_targets;

/* If set to GL_TRUE then the texture sub image upload will be read
 * from a PBO */
static GLboolean use_pbo = GL_FALSE;

struct sub_region {
	int tx, ty, tz, tw, th, td;
};

/* If set, format and sub-region are given from command line. */
struct single_test {
	bool enabled;
	GLenum targets[2];
	GLenum format;
	struct sub_region region;
};

static struct single_test manual_dispatch = {
	.enabled = false, .targets = { GL_NONE, GL_NONE }
};

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

static const char vertex_cube_map_array[] =
	"const float N_SIDES = 6.0;\n"
	"const float TEX_DEPTH = " STRINGIFY(DEFAULT_TEX_DEPTH) ".0 *\n"
	"                        N_SIDES;\n"
	"void\n"
	"main()\n"
	"{\n"
	"        vec2 face_coord;\n"
	"        vec3 res;\n"
	"        float slice = gl_MultiTexCoord0.p * TEX_DEPTH - 0.5;\n"
	"        float layer = floor(slice / N_SIDES);\n"
	"        int face = int(floor(mod(slice, N_SIDES)));\n"
	"\n"
	"        face_coord = gl_MultiTexCoord0.st * 2.0 - 1.0;\n"
	"        if (face == 0)\n"
	"                res = vec3(1.0, -face_coord.ts);\n"
	"        else if (face == 1)\n"
	"                res = vec3(-1.0, face_coord.ts * vec2(-1.0, 1.0));\n"
	"        else if (face == 2)\n"
	"                res = vec3(face_coord.s, 1.0, face_coord.t);\n"
	"        else if (face == 3)\n"
	"                res = vec3(face_coord.s, -1.0, -face_coord.t);\n"
	"        else if (face == 4)\n"
	"                res = vec3(face_coord.st * vec2(1.0, -1.0), 1.0);\n"
	"        else\n"
	"                res = vec3(-face_coord.st, -1.0);\n"
	"        gl_TexCoord[0] = vec4(res, layer);\n"
	"        gl_Position = ftransform();\n"
	"}\n";

static const char fragment_cube_map_array[] =
	"#extension GL_ARB_texture_cube_map_array : require\n"
	"uniform samplerCubeArray tex;\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_FragColor = texture(tex, gl_TexCoord[0]);\n"
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

	return piglit_equal_images_update_rgba8(original_ref, updated_ref, testImg,
						w, h, d,
						tx, ty, tz, tw, th, td,
						8);
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
		piglit_draw_rect_tex3d(i / 8 * w, i % 8 * h, /* x/y */
				       w, h,
				       0.0, 0.0, /* tx/ty */
				       1.0, 1.0, /* tw/th */
				       tz, tz /* tz0/tz1 */);
	}

	for (i = 0; i < d; i += 8) {
		glReadPixels(i / 8 * w, i % 8 * h,
			     w, h * MIN2(8, d - i),
			     GL_RGBA, GL_UNSIGNED_BYTE,
			     ref);
		ref += 8 * w * h * 4;
	}
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

static bool
test_region(GLuint pbo, GLenum target, GLenum internal_format,
	    const unsigned char *original_img,
	    const unsigned char *original_ref,
	    const unsigned char *updated_img,
	    const unsigned char *updated_ref,
	    unsigned w, unsigned h, unsigned d,
	    const struct sub_region *region)
{
	bool pass = true;
	GLuint tex;
	unsigned char *test_img = (unsigned char *)malloc(w * h * d * 4);

	/* Recreate the original texture */
	tex = create_texture(target, internal_format, w, h, d,
			     srcFormat, original_img);

	if (use_pbo)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);

	/* replace texture region with data from updated image */
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, region->tx);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, region->ty);
	glPixelStorei(GL_UNPACK_SKIP_IMAGES, region->tz);
	if (d > 1) {
		glTexSubImage3D(target, 0,
				region->tx, region->ty, region->tz,
				region->tw, region->th, region->td,
				srcFormat, GL_UNSIGNED_BYTE,
				use_pbo ? NULL : updated_img);
	} else if (h > 1) {
		glTexSubImage2D(target, 0,
				region->tx, region->ty,
				region->tw, region->th,
				srcFormat, GL_UNSIGNED_BYTE,
				use_pbo ? NULL : updated_img);
	} else if (w > 1) {
		glTexSubImage1D(target, 0, region->tx, region->tw,
				srcFormat, GL_UNSIGNED_BYTE,
				use_pbo ? NULL : updated_img);
	} else {
		assert(!"Unknown image dimensions");
	}

	if (use_pbo)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	/* draw test image */
	draw_and_read_texture(w, h, d, test_img);

	glDeleteTextures(1, &tex);

	piglit_present_results();

	if (!equal_images(target,
			  original_ref, updated_ref, test_img,
			  w, h, d,
			  region->tx, region->ty, region->tz,
			  region->tw, region->th, region->td)) {
		printf("texsubimage failed\n");
		printf("  target: %s\n", piglit_get_gl_enum_name(target));
		printf("  internal format: %s\n",
			get_format_name(internal_format));
		printf("  region: %d, %d  %d x %d\n",
		       region->tx, region->ty, region->tw, region->th);
		pass = false;
	}

	free(test_img);

	return pass;
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
test_format(GLenum target, GLenum intFormat,
	    unsigned w, unsigned h, unsigned d,
	    const struct sub_region *regions, unsigned num_regions)
{
	GLuint tex, i, j, k, n, t;
	GLubyte *original_img, *original_ref;
	GLubyte *updated_img, *updated_ref;
	GLboolean pass = GL_TRUE;
	GLuint pbo = 0;

	original_img = (GLubyte *) malloc(w * h * d * 4);
	original_ref = (GLubyte *) malloc(w * h * d * 4);
	updated_img = (GLubyte *) malloc(w * h * d * 4);
	updated_ref = (GLubyte *) malloc(w * h * d * 4);

	/* fill source tex images */
	n = 0;
	for (i = 0; i < d; i++) {
		for (j = 0; j < h; j++) {
			for (k = 0; k < w; k++) {
				original_img[n + 0] = j * 4;
				original_img[n + 1] = k * 2;
				original_img[n + 2] = i * 128 / d;
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

	if (use_pbo) {
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
		glBufferData(GL_PIXEL_UNPACK_BUFFER,
			     w * h * d * 4,
			     updated_img,
			     GL_STATIC_DRAW);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
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

	for (t = 0; t < num_regions; t++) {
		if (!test_region(pbo, target, intFormat,
				 original_img, original_ref,
				 updated_img, updated_ref,
				 w, h, d, &regions[t])) {
			pass = GL_FALSE;
			break;
		}
	}

	free(original_img);
	free(original_ref);
	free(updated_img);
	free(updated_ref);
	if (use_pbo)
		glDeleteBuffers(1, &pbo);

	return pass;
}

static void
select_regions(unsigned w, unsigned h, unsigned d, GLenum internal_format,
	       struct sub_region *regions, unsigned num_regions)
{
	int i;
	GLuint bw, bh, bb, wMask, hMask, dMask;

	piglit_get_compressed_block_size(internal_format, &bw, &bh, &bb);
	wMask = ~(bw-1);
	hMask = ~(bh-1);
	dMask = ~0;

	for (i = 0; i < num_regions; i++) {
		/* Choose random region of texture to update.
		 * Use sizes and positions that are multiples of
		 * the compressed block size.
		 */
		regions[i].tw = (rand() % w) & wMask;
		regions[i].th = (rand() % h) & hMask;
		regions[i].td = (rand() % d) & dMask;
		regions[i].tx = (rand() % (w - regions[i].tw)) & wMask;
		regions[i].ty = (rand() % (h - regions[i].th)) & hMask;
		regions[i].tz = (rand() % (d - regions[i].td)) & dMask;

		assert(regions[i].tx + regions[i].tw <= w);
		assert(regions[i].ty + regions[i].th <= h);
		assert(regions[i].tz + regions[i].td <= d);
	}
}

/**
 * Test all formats in texsubimage_test_sets[] for the given
 * texture target.
 */
static GLboolean
test_formats(GLenum target, unsigned w, unsigned h, unsigned d)
{
	GLboolean pass = GL_TRUE;
	int i, j;

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
			struct sub_region regions[10];

			select_regions(w, h, d, set->format[j].internalformat,
				       regions, ARRAY_SIZE(regions));

			if (!test_format(target,
					 set->format[j].internalformat,
		                         w, h, d,
					 regions, ARRAY_SIZE(regions))) {
				pass = GL_FALSE;
			}
		}
	}

	return pass;
}

static GLuint
prepare_tex_to_fbo_blit_program(GLenum target)
{
	GLuint program = 0;

	switch (target) {
	case GL_TEXTURE_1D_ARRAY:
		program = piglit_build_simple_program(NULL, fragment_1d_array);
		break;
	case GL_TEXTURE_2D_ARRAY:
		program = piglit_build_simple_program(NULL, fragment_2d_array);
		break;
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		program = piglit_build_simple_program(vertex_cube_map_array,
						      fragment_cube_map_array);
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

	return program;
}

static void
print_usage_and_exit(const char *s)
{
	printf("Invalid argument: %s\n", s);
	printf("Usage: texsubimage <pbo> manual <target> <format> "
	       "<tx> <ty> <tz> <tw> <th> <tz>");
}

static unsigned
read_integer(const char *s)
{
	char *endptr = NULL;
	unsigned res = strtol(s, &endptr, 0);

	if (endptr != s + strlen(s))
		print_usage_and_exit(s);

	return res;
}

static void
init_manual_dispatch(char **argv, unsigned argc)
{
	static const unsigned manual_arg_num = 8;
	if (argc < manual_arg_num)
		print_usage_and_exit(argv[0]);

	manual_dispatch.targets[0] = piglit_get_gl_enum_from_name(argv[0]);
	manual_dispatch.format = piglit_get_gl_enum_from_name(argv[1]);

	manual_dispatch.region.tx = read_integer(argv[2]);
	manual_dispatch.region.ty = read_integer(argv[3]);
	manual_dispatch.region.tz = read_integer(argv[4]);
	manual_dispatch.region.tw = read_integer(argv[5]);
	manual_dispatch.region.th = read_integer(argv[6]);
	manual_dispatch.region.td = read_integer(argv[7]);

	manual_dispatch.enabled = true;
}

static void
adjust_tex_dimensions(GLenum target, unsigned *w, unsigned *h, unsigned *d)
{
	if (target == GL_TEXTURE_CUBE_MAP_ARRAY_ARB) {
		*w = *h;
		*d *= 6;
	} else if (target != GL_TEXTURE_3D &&
		   target != GL_TEXTURE_2D_ARRAY) {
		*d = 1;
	}

	if (target == GL_TEXTURE_1D)
		*h = 1;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i;

	if (manual_dispatch.enabled)
		test_targets = manual_dispatch.targets;

	/* Loop over 1/2/3D texture targets */
	for (i = 0; test_targets[i] != GL_NONE; i++) {
		unsigned w = DEFAULT_TEX_WIDTH;
		unsigned h = DEFAULT_TEX_HEIGHT;
		unsigned d = DEFAULT_TEX_DEPTH;
		const GLuint program =
			prepare_tex_to_fbo_blit_program(test_targets[i]);

		adjust_tex_dimensions(test_targets[i], &w, &h, &d);

		if (manual_dispatch.enabled) {
			pass = test_format(test_targets[i],
					   manual_dispatch.format,
					   w, h, d,
					   &manual_dispatch.region, 1);
		} else {
			pass = test_formats(test_targets[i], w, h, d) && pass;
		}

		if (program == 0) {
			glDisable(test_targets[i]);
		} else {
			glUseProgram(0);
			glDeleteProgram(program);
		}
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
	static const GLenum cube_map_array_targets[] = {
		GL_TEXTURE_CUBE_MAP_ARRAY_ARB,
		GL_NONE
	};
	int remaining_argc = 1;
	int i;

	test_targets = core_targets;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "array")) {
			piglit_require_extension("GL_EXT_texture_array");
			piglit_require_GLSL();
			test_targets = array_targets;
		} else if (!strcmp(argv[i], "cube_map_array")) {
			piglit_require_extension
				("GL_ARB_texture_cube_map_array");
			piglit_require_GLSL();
			test_targets = cube_map_array_targets;
		} else if (!strcmp(argv[i], "pbo")) {
			piglit_require_extension("GL_ARB_pixel_buffer_object");
			use_pbo = GL_TRUE;
		} else if (!strcmp(argv[i], "manual")) {
			init_manual_dispatch(argv + i + 1, argc - (i + 1));
			break;
		} else {
			argv[remaining_argc++] = argv[i];
		}
	}

	if (!manual_dispatch.enabled)
		fbo_formats_init(remaining_argc, argv, 0);
	(void) fbo_formats_display;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
