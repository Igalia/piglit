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
 * \file texelFetch.c
 *
 * Tests the GLSL 1.30+ texelFetch() built-in function.
 *
 * The "texelFetch" binary takes two arguments: shader stage and sampler type.
 *
 * For example:
 * ./bin/texelFetch fs sampler1DArray
 * ./bin/texelFetch usampler3D vs
 *
 * The test covers:
 * - All pipeline stages (VS, GS, FS)
 * - Integer and floating point texture formats
 * - Sampler dimensionality (1D, 2D, 3D, 1DArray, 2DArray)
 * - Mipmapping
 * - Non-power-of-two textures
 *
 * Draws a series of "rectangles" which display each miplevel and array slice,
 * at full size.  They are layed out as follows:
 *
 * miplevel 3 +          +          +          +          +
 *
 * miplevel 2 +-+        +-+        +-+        +-+        +-+
 *            +-+        +-+        +-+        +-+        +-+
 *
 * miplevel 1 +---+      +---+      +---+      +---+      +---+
 *            |   |      |   |      |   |      |   |      |   |
 *            +---+      +---+      +---+      +---+      +---+
 *
 *            +------+   +------+   +------+   +------+   +------+
 * miplevel 0 |      |   |      |   |      |   |      |   |      |
 *            |      |   |      |   |      |   |      |   |      |
 *            +------+   +------+   +------+   +------+   +------+
 *            slice #0   slice #1   slice #2   slice #3   slice #4
 *
 * Normally, we could draw each rectangle as a single quad (or two triangles),
 * interpolate the texture coordinates across the primitive, and let the
 * fragment shader look up the color values from the texture.
 *
 * However, this fails miserably for vertex shader texturing: a quad only has
 * four vertices, which means we could only fetch/display at most 4 texels.
 * If we used a simple RGBW checkerboard, as in other Piglit tests, this would
 * only tell us that we sampled somewhere in the right 1/4 of the texture.
 *
 * Instead, we take a clever approach: draw each "rectangle" via a series of
 * 1-pixel wide GL_POINT primitives.  This gives us one vertex per pixel; by
 * drawing the texture at full size, each pixel and vertex also correspond to
 * exactly one texel.  So every texel is sampled and verified for correctness.
 *
 * In other words: "One pixel, one texel, one vertex."
 *
 * For convenience, we take the same approach for fragment shader testing
 * as well, since it allows us to reuse the same VBO setup and drawing code.
 */
#include "common.h"

void
parse_args(int argc, char **argv);
static enum shader_target test_stage = UNKNOWN;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = 900;
	config.window_height = 600;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

	piglit_gl_process_args(&argc, argv, &config);

	parse_args(argc, argv);
	if (test_stage == GS) {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	} else {
		config.supports_gl_compat_version = 10;
		config.supports_gl_core_version = 31;
	}

PIGLIT_GL_TEST_CONFIG_END

/** Vertex shader attribute locations */
const int pos_loc = 0;
const int texcoord_loc = 1;

/** Whether we're testing texelFetchOffset() instead of texelFetch(). */
static bool test_offset = false;
const int fetch_x_offset = 7;
const int fetch_y_offset = -8;
const int fetch_z_offset = 4;
const char *fetch_1d_arg = ", int(7)";
const char *fetch_2d_arg = ", ivec2(7, -8)";
const char *fetch_3d_arg = ", ivec3(7, -8, 4)";

/** Uniform locations */
int divisor_loc;

/**
 * Expected color data for each rectangle drawn, indexed by miplevel and slice.
 * For example, expected_colors[1][3] contains the data for miplevel 1 slice 3.
 */
float ***expected_colors;

int prog;
GLuint tex;
GLuint pos_vbo, tc_vbo;

/**
 * Return the divisors necessary to scale the unnormalized texture data to
 * a floating point color value in the range [0, 1].
 */
static void
compute_divisors(int lod, float *divisors)
{
	divisors[0] = max2(level_size[lod][0] - 1, 1);
	divisors[1] = max2(level_size[lod][1] - 1, 1);
	divisors[2] = max2(level_size[lod][2] - 1, 1);
	divisors[3] = miplevels;

	if (sampler.data_type != GL_UNSIGNED_INT)
		divisors[0] = -divisors[0];
}

static bool test_once()
{
	int i, l, z, level_y = 0;
	bool pass = true;

	i = 0;
	for (l = 0; l < miplevels; ++l) {
		if (l && !has_samples())
			level_y += 1 + MAX2(base_size[1] >> (l-1), 1);

		for (z = 0; z < level_size[l][2]; z++) {
			/* Draw the "rectangle" for this miplevel/slice. */
			int points = level_size[l][0] * level_size[l][1];
			float divisors[4];

			compute_divisors(l, divisors);
			swizzle(divisors);
			glUniform4fv(divisor_loc, 1, divisors);

			glDrawArrays(GL_POINTS, i, points);

			i += points;

			/* Compare results against reference image. */
			pass &= piglit_probe_image_rgba(5+(1+base_size[0]) * z,
							has_samples() ? 5+(1+base_size[1]) * l
								      : 5 + level_y,
							level_size[l][0],
							level_size[l][1],
							expected_colors[l][z]);
			free(expected_colors[l][z]);
		}
		free(expected_colors[l]);
	}
	free(expected_colors);
	return pass;
}

/**
 * Generate two VBOs for our vertex attributes:
 * 1. Pixel position (in window coordinates).
 * 2. Texture coordinates.
 *
 * The VBOs contain the data for every rectangle being drawn (as opposed to
 * creating and binding a separate VBO per miplevel/slice.)
 */
void
generate_VBOs()
{
	int x, y, z, l;
	float *pos, *pos_data;
	int *tc, *tc_data;
	bool array_1D = sampler.target == GL_TEXTURE_1D_ARRAY;
	int num_texels = 0;
	int level_y = 0;

	/* Calculate the # of texels a.k.a. size of the VBOs */
	for (l = 0; l < miplevels; l++) {
		num_texels += level_size[l][0] * level_size[l][1] * level_size[l][2];
	}

	pos = pos_data = malloc(num_texels * 4 * sizeof(float));
	tc = tc_data = malloc(num_texels * 4 * sizeof(int));

	for (l = 0; l < miplevels; l++) {
		if (l && !has_samples())
			level_y += 1 + MAX2(base_size[1] >> (l-1), 1);

		for (z = 0; z < level_size[l][2]; z++) {
			for (y = 0; y < level_size[l][1]; y++) {
				for (x = 0; x < level_size[l][0]; x++) {
					/* Assign pixel positions: */
					pos[0] = 5.5 + (1 + base_size[0])*z + x;
					if (has_samples())
						pos[1] = 5.5 + (1 + base_size[1])*l + y;
					else
						pos[1] = 5.5 + level_y + y;
					pos[2] = 0.0;
					pos[3] = 1.0;

					pos[0] = -1.0 + 2.0 * (pos[0] /
							       piglit_width);
					pos[1] = -1.0 + 2.0 * (pos[1] /
							       piglit_height);

					pos += 4;

					/* Assign texture coordinates:
					 * 1D:      x _ _ l
					 * 2D:      x y _ l
					 * 3D:      x y z l
					 * 1DArray: x z _ l
					 * 2DArray: x y z l
					 */
					tc[0] = x;
					tc[1] = array_1D ? z : y;
					tc[2] = z;
					tc[3] = l;

					if (test_offset) {
						tc[0] -= fetch_x_offset;
						if (sampler.target != GL_TEXTURE_1D_ARRAY)
							tc[1] -= fetch_y_offset;
						if (sampler.target != GL_TEXTURE_2D_ARRAY)
							tc[2] -= fetch_z_offset;
					}

					tc += 4;
				}
			}
		}
	}

	/* Create VBO for pixel positions in NDC: */
	glGenBuffers(1, &pos_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     num_texels * 4 * sizeof(float),
		     pos_data, GL_STATIC_DRAW);
	glVertexAttribPointer(pos_loc, 4, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(pos_loc);

	/* Create VBO for texture coordinates: */
	glGenBuffers(1, &tc_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tc_vbo);
	glBufferData(GL_ARRAY_BUFFER,
		     num_texels * 4 * sizeof(int),
		     tc_data, GL_STATIC_DRAW);
	glVertexAttribIPointer(texcoord_loc, 4, GL_INT, 0, 0);
	glEnableVertexAttribArray(texcoord_loc);

	free(pos_data);
	free(tc_data);
}

/* like piglit_draw_rect(), but works in a core context too.
 * pretty silly and wasteful (binds a throwaway VAO and BO) */
static void
draw_rect_core(int x, int y, int w, int h)
{
    float verts[4][4];
    GLuint bo;

    verts[0][0] = x;
    verts[0][1] = y;
    verts[0][2] = 0.0;
    verts[0][3] = 1.0;
    verts[1][0] = x + w;
    verts[1][1] = y;
    verts[1][2] = 0.0;
    verts[1][3] = 1.0;
    verts[2][0] = x + w;
    verts[2][1] = y + h;
    verts[2][2] = 0.0;
    verts[2][3] = 1.0;
    verts[3][0] = x;
    verts[3][1] = y + h;
    verts[3][2] = 0.0;
    verts[3][3] = 1.0;

    glGenBuffers(1, &bo);
    glBindBuffer(GL_ARRAY_BUFFER, bo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float),
            verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDeleteBuffers(1, &bo);
}

/**
 * Generate the same test pattern as generate_texture() does, but do it in the shader.
 * This is a silly workaround for not being able to just upload directly.
 */
static void
upload_multisample_data(GLuint tex, int width, int height,
        int layers, int samples)
{
    GLuint oldFBO, FBO;
    int si;
    char *fs_code;
    int vs, fs;
    int staging_program = 0;
    int layer_loc = 0;
    int si_loc = 0;

    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint *) &oldFBO);
    if (is_array_sampler())
        glTexImage3DMultisample(sampler.target,
                samples, sampler.internal_format,
                width, height, layers, GL_TRUE);
    else
        glTexImage2DMultisample(sampler.target,
                samples, sampler.internal_format,
                width, height, GL_TRUE);
    if (!piglit_check_gl_error(GL_NO_ERROR)) {
        printf("Error creating multisample texture\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
            "#version 130\n"
            "#extension GL_ARB_explicit_attrib_location: require\n"
            "layout(location=0) in vec4 pos;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "}\n");
    (void)!asprintf(&fs_code,
            "#version 130\n"
            "#extension GL_ARB_explicit_attrib_location: require\n"
            "#extension GL_ARB_fragment_coord_conventions: require\n"
            "uniform int layer;\n"
            "uniform int si;\n"
            "layout(pixel_center_integer) in vec4 gl_FragCoord;\n"
            "layout(location=0) out %s o;\n"
            "void main() {\n"
            "  o = %s(%sgl_FragCoord.x, gl_FragCoord.y, layer, si);\n"
            "}\n",
            sampler.return_type,
            sampler.return_type,
            sampler.data_type == GL_UNSIGNED_INT ? "" : "-");
    fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
    if (!fs) {
        printf("Failed to compile staging program.\n");
        piglit_report_result(PIGLIT_FAIL);
    }
    staging_program = piglit_link_simple_program(vs,fs);
    if (!piglit_link_check_status(staging_program))
        piglit_report_result(PIGLIT_FAIL);

    layer_loc = glGetUniformLocation(staging_program, "layer");
    si_loc = glGetUniformLocation(staging_program, "si");

    glUseProgram(staging_program);
    if (!piglit_check_gl_error(GL_NO_ERROR)) {
        printf("glUseProgram failed\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glViewport(0, 0, width, height);

    glEnable(GL_SAMPLE_MASK);

    for (si=0; si < samples; si++) {
        glUniform1i(si_loc, si);
        glSampleMaski(0, 1<<si);

        if (is_array_sampler()) {
            int layer;
            for(layer = 0; layer < layers; layer++) {
                glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                    tex, 0, layer);
                glUniform1i(layer_loc, layer);
                draw_rect_core(-1, -1, 2, 2);
            }
        }
        else {
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                   sampler.target, tex, 0);

            glUniform1i(layer_loc, 0);
            draw_rect_core(-1, -1, 2, 2);
        }
    }

    glDisable(GL_SAMPLE_MASK);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldFBO);
    glDeleteFramebuffers(1, &FBO);
}

/**
 * Create texel data.
 */
void
generate_texture()
{
	int l, x, y, z;
	unsigned *u_level, *u_ptr;
	int      *i_level, *i_ptr;
	float    *f_level, *f_ptr;
	float    *expected_ptr;
	void *level_image;
	float divisors[4];
	GLenum target = sampler.target;

	glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	if (!has_samples()) {
		if (sampler.target == GL_TEXTURE_RECTANGLE) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if (swizzling)
			glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA,
					 (GLint *) sampler.swizzle);
	}

	expected_colors = calloc(miplevels, sizeof(float **));

	for (l = 0; l < miplevels; l++) {
		expected_colors[l] = calloc(level_size[l][2], sizeof(float *));

		i_ptr = i_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(int));

		u_ptr = u_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(unsigned));

		f_ptr = f_level =
			malloc(level_size[l][0] * level_size[l][1] *
			       level_size[l][2] * 4 * sizeof(float));

		for (z = 0; z < level_size[l][2]; z++) {
			expected_ptr = expected_colors[l][z] =
				malloc(level_size[l][0] * level_size[l][1] *
				       level_size[l][2] * 4 * sizeof(float));
			for (y = 0; y < level_size[l][1]; y++) {
				for (x = 0; x < level_size[l][0]; x++) {
					int nx = sampler.data_type == GL_UNSIGNED_INT ?  x : -x;
					f_ptr[0] = nx;
					f_ptr[1] = y;
					f_ptr[2] = z;
					f_ptr[3] = l;

					i_ptr[0] = nx;
					i_ptr[1] = y;
					i_ptr[2] = z;
					i_ptr[3] = l;

					u_ptr[0] = nx;
					u_ptr[1] = y;
					u_ptr[2] = z;
					u_ptr[3] = l;

					compute_divisors(l, divisors);

					expected_ptr[0] = f_ptr[0]/divisors[0];
					expected_ptr[1] = f_ptr[1]/divisors[1];
					expected_ptr[2] = f_ptr[2]/divisors[2];
					expected_ptr[3] = f_ptr[3]/divisors[3];
					swizzle(expected_ptr);

					f_ptr += 4;
					i_ptr += 4;
					u_ptr += 4;
					expected_ptr += 4;
				}
			}
		}

		switch (sampler.data_type) {
		case GL_FLOAT:
			level_image = f_level;
			break;
		case GL_UNSIGNED_INT:
			level_image = u_level;
			break;
		case GL_INT:
			level_image = i_level;
			break;
		default:
			assert(!"Unexpected data type");
			level_image = NULL;
			break;
		}

		if (!has_samples())
			upload_miplevel_data(target, l, level_image);

		free(f_level);
		free(u_level);
		free(i_level);
	}

	if (has_samples())
		upload_multisample_data(tex, level_size[0][0],
				level_size[0][1], level_size[0][2],
				sample_count);
}

/**
 * How many components are in the coordinate?
 */
static int
coordinate_size()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
		return 1;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_2D_MULTISAMPLE:
		return 2;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		return 3;
	default:
		assert(!"Should not get here.");
		return 0;
	}
}

/**
 * Generate, compile, and link the GLSL shaders.
 */
int
generate_GLSL(enum shader_target test_stage)
{
	int vs, gs, fs, prog;

	static char *vs_code;
	static char *gs_code = NULL;
	static char *fs_code;
	const char *offset_func, *offset_arg;

	if (test_offset) {
		offset_func = "Offset";
		switch (sampler.target) {
		case GL_TEXTURE_1D:
		case GL_TEXTURE_1D_ARRAY:
			offset_arg = fetch_1d_arg;
			break;
		case GL_TEXTURE_2D:
		case GL_TEXTURE_2D_ARRAY:
		case GL_TEXTURE_RECTANGLE:
			offset_arg = fetch_2d_arg;
			break;
		case GL_TEXTURE_3D:
			offset_arg = fetch_3d_arg;
			break;
		default:
			assert(!"Unexpected target texture");
			offset_arg = "";
			break;
		}
	} else {
		offset_func = "";
		offset_arg = "";
	}

	switch (test_stage) {
	case VS:
		(void)!asprintf(&vs_code,
			 "#version %d\n"
			 "%s\n"
			 "#define ivec1 int\n"
			 "flat out %s color;\n"
			 "in vec4 pos;\n"
			 "in ivec4 texcoord;\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    color = texelFetch%s(tex, ivec%d(texcoord)%s%s);\n"
			 "    gl_Position = pos;\n"
			 "}\n",
			 shader_version,
			 has_samples() ? "#extension GL_ARB_texture_multisample: require" : "",
			 sampler.return_type, sampler.name,
			 offset_func,
			 coordinate_size(),
			 ((sampler.target == GL_TEXTURE_RECTANGLE) ?
			  "" : ", texcoord.w"),
			 offset_arg);
		(void)!asprintf(&fs_code,
			 "#version %d\n"
			 "flat in %s color;\n"
			 "uniform vec4 divisor;\n"
			 "out vec4 fragColor;\n"
			 "void main()\n"
			 "{\n"
			 "    fragColor = vec4(color)/divisor;\n"
			 "}\n",
			 shader_version,
			 sampler.return_type);
		break;
	case GS:
		(void)!asprintf(&vs_code,
			 "#version %d\n"
			 "in vec4 pos;\n"
			 "in ivec4 texcoord;\n"
			 "flat out ivec4 texcoord_to_gs;\n"
			 "out vec4 pos_to_gs;\n"
			 "void main()\n"
			 "{\n"
			 "    texcoord_to_gs = texcoord;\n"
			 "    pos_to_gs = pos;\n"
			 "}\n",
			 shader_version);
		(void)!asprintf(&gs_code,
			 "#version %d\n"
			 "%s\n"
			 "#define ivec1 int\n"
			 "layout(points) in;\n"
			 "layout(points, max_vertices = 1) out;\n"
			 "flat out %s color;\n"
			 "flat in ivec4 texcoord_to_gs[1];\n"
			 "in vec4 pos_to_gs[1];\n"
			 "uniform %s tex;\n"
			 "void main()\n"
			 "{\n"
			 "    ivec4 texcoord = texcoord_to_gs[0];\n"
			 "    color = texelFetch%s(tex, ivec%d(texcoord)%s%s);\n"
			 "    gl_Position = pos_to_gs[0];\n"
			 "    EmitVertex();\n"
			 "}\n",
			 shader_version,
			 has_samples() ? "#extension GL_ARB_texture_multisample: require" : "",
			 sampler.return_type, sampler.name,
			 offset_func,
			 coordinate_size(),
			 ((sampler.target == GL_TEXTURE_RECTANGLE) ?
			  "" : ", texcoord.w"),
			 offset_arg);
		(void)!asprintf(&fs_code,
			 "#version %d\n"
			 "flat in %s color;\n"
			 "uniform vec4 divisor;\n"
			 "out vec4 fragColor;\n"
			 "void main()\n"
			 "{\n"
			 "    fragColor = vec4(color)/divisor;\n"
			 "}\n",
			 shader_version,
			 sampler.return_type);
		break;
	case FS:
		(void)!asprintf(&vs_code,
			 "#version %d\n"
			 "#define ivec1 int\n"
			 "in vec4 pos;\n"
			 "in ivec4 texcoord;\n"
			 "flat out ivec4 tc;\n"
			 "void main()\n"
			 "{\n"
			 "    tc = texcoord;\n"
			 "    gl_Position = pos;\n"
			 "}\n",
			 shader_version);
		(void)!asprintf(&fs_code,
			 "#version %d\n"
			 "%s\n"
			 "#define ivec1 int\n"
			 "flat in ivec4 tc;\n"
			 "uniform vec4 divisor;\n"
			 "uniform %s tex;\n"
			 "out vec4 fragColor;\n"
			 "void main()\n"
			 "{\n"
			 "    vec4 color = texelFetch%s(tex, ivec%d(tc)%s%s);\n"
			 "    fragColor = color/divisor;\n"
			 "}\n",
			 shader_version,
			 has_samples() ? "#extension GL_ARB_texture_multisample: require" : "",
			 sampler.name,
			 offset_func,
			 coordinate_size(),
			 (sampler.target == GL_TEXTURE_RECTANGLE ?
			  "" : ", tc.w"),
			 offset_arg);
		break;
	default:
		assert(!"Should not get here.");
		break;
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	if (!vs) {
		printf("VS code:\n%s", vs_code);
		piglit_report_result(PIGLIT_FAIL);
	}
	if (gs_code) {
		gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_code);
		if (!gs) {
			printf("GS code:\n%s", gs_code);
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
	if (!fs) {
		printf("FS code:\n%s", fs_code);
		piglit_report_result(PIGLIT_FAIL);
	}
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	if (gs_code)
		glAttachShader(prog, gs);
	glAttachShader(prog, fs);

	glBindAttribLocation(prog, pos_loc, "pos");
	glBindAttribLocation(prog, texcoord_loc, "texcoord");

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	return prog;
}

/**
 * Is this sampler supported by texelFetch?
 */
static bool
supported_sampler()
{
	switch (sampler.target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
	case GL_TEXTURE_3D:
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_2D_MULTISAMPLE:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		return true;
	}
	return false;
}

void
fail_and_show_usage()
{
	printf("Usage: texelFetch [140] [offset] <vs|gs|fs> <sampler type> "
	       "[sample_count] [swizzle] [piglit args...]\n");
	piglit_report_result(PIGLIT_FAIL);
}


void
parse_args(int argc, char **argv)
{
	int i;
	bool sampler_found = false;
	bool dim_range_found = false;

	sample_count = 0;
	minx = maxx = 65;
	miny = maxy = 32;
	minz = maxz = 5;

	for (i = 1; i < argc; i++) {
		if (test_stage == UNKNOWN) {
			/* Maybe it's the shader stage? */
			if (strcmp(argv[i], "vs") == 0) {
				test_stage = VS;
				continue;
			} else if (strcmp(argv[i], "gs") == 0) {
				test_stage = GS;
				continue;
			} else if (strcmp(argv[i], "fs") == 0) {
				test_stage = FS;
				continue;
			}
		}

		if (strcmp(argv[i], "140") == 0) {
			shader_version = 140;
			continue;
		}

		if (strcmp(argv[i], "offset") == 0) {
			test_offset = true;
			continue;
		}

		/* Maybe it's the sampler type? */
		if (!sampler_found && (sampler_found = select_sampler(argv[i])))
			continue;

		/* Maybe it's the sample count? */
		if (sampler_found && has_samples() && !sample_count) {
			sample_count = atoi(argv[i]);
			continue;
		}

		if (!swizzling && (swizzling = parse_swizzle(argv[i])))
			continue;

		if (!dim_range_found) {
			if (sscanf(argv[i], "%ux%ux%u-%ux%ux%u",
				   &minx, &miny, &minz, &maxx, &maxy, &maxz) == 6) {
				dim_range_found = true;
				continue;
			}
			if (sscanf(argv[i], "%ux%u-%ux%u",
				   &minx, &miny, &maxx, &maxy) == 4) {
				minz = maxz = 1;
				dim_range_found = true;
				continue;
			}
			if (sscanf(argv[i], "%u-%u", &minx, &maxx) == 2) {
				miny = maxy = minz = maxz = 1;
				dim_range_found = true;
				continue;
			}
			if (sscanf(argv[i], "%ux%ux%u", &minx, &miny, &minz) == 3) {
				maxx = minx;
				maxy = miny;
				maxz = minz;
				dim_range_found = true;
				continue;
			}
			if (sscanf(argv[i], "%ux%u", &minx, &miny) == 2) {
				maxx = minx;
				maxy = miny;
				minz = maxz = 1;
				dim_range_found = true;
				continue;
			}
		}

		fail_and_show_usage();
	}

	if (test_stage == UNKNOWN || !sampler_found)
		fail_and_show_usage();

	if (test_stage == GS && shader_version < 150)
		shader_version = 150;

	if (!has_height()) {
		miny = maxy = 1;
	}

	if (!has_slices()) {
		minz = maxz = 1;
	}

	if (minx < maxx || miny < maxy || minz < maxz)
		piglit_automatic = true;
}

enum piglit_result
piglit_display()
{
	int x, y, z;
	bool pass = true;

	glClearColor(0.1, 0.1, 0.1, 1.0);
	glPointSize(1.0);

	for (x = minx; x <= maxx; x++) {
		for (y = miny; y <= maxy; y++) {
			for (z = minz; z <= maxz; z++) {
				printf("%ix%ix%i\n", x, y, z);

				if (5 + (x+1) * z >= piglit_width) {
					printf("Width or depth is too big.\n");
					piglit_report_result(PIGLIT_FAIL);
				}

				if ((has_samples() && 5 + (y+1) * miplevels >= piglit_height) ||
				    (!has_samples() && 5 + y*2 + miplevels-2 >= piglit_height)) {
					printf("Height is too big or too many samples.\n");
					piglit_report_result(PIGLIT_FAIL);
				}

				base_size[0] = x;
				base_size[1] = y;
				base_size[2] = z;

				/* Create textures and set miplevel info */
				compute_miplevel_info();
				generate_texture();
				generate_VBOs();

				glViewport(0, 0, piglit_width, piglit_height);
				glClear(GL_COLOR_BUFFER_BIT);
				glUseProgram(prog);

				pass &= test_once();

				glUseProgram(0);
				glDeleteTextures(1, &tex);
				glDeleteBuffers(1, &pos_vbo);
				glDeleteBuffers(1, &tc_vbo);
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int tex_location;

	if (!supported_sampler()) {
		printf("%s unsupported\n", sampler.name);
		piglit_report_result(PIGLIT_FAIL);
	}

	require_GL_features(test_stage);

	if (sample_count) {
		/* check it */
		GLint max_samples;

		if (sampler.data_type == GL_INT ||
		    sampler.data_type == GL_UNSIGNED_INT) {
			glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &max_samples);
			if (sample_count > max_samples) {
				printf("Sample count of %d not supported,"
				       " >MAX_INTEGER_SAMPLES\n",
				       sample_count);
				piglit_report_result(PIGLIT_SKIP);
			}
		} else {
			glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
			if (sample_count > max_samples) {
				printf("Sample count of %d not supported,"
				       " >MAX_SAMPLES\n",
				       sample_count);
				piglit_report_result(PIGLIT_SKIP);
			}
		}
	}

	prog = generate_GLSL(test_stage);

	tex_location = glGetUniformLocation(prog, "tex");
	divisor_loc = glGetUniformLocation(prog, "divisor");

	glUseProgram(prog);
	glUniform1i(tex_location, 0);

	if (piglit_get_gl_version() >= 31) {
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
}
