/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 *
 */

/** @file tex-miplevel-selection.c
 *
 * This tests interactions between GL_TEXTURE_BASE/MAX_LEVEL GL_TEXTURE_MIN/
 * MAX_LOD, TEXTURE_LOD_BIAS, mipmap filtering on/off, and scaling of texture
 * coordinates as a means to "bias" the LOD.
 *
 * On top of that, test as many texture GLSL functions, sampler types, and
 * texture targets which allow mipmapping as possible, e.g. with an explicit
 * LOD, bias, and derivatives.
 *
 * Each mipmap level is set to a different color/depth value, so that we can
 * check that the correct level is read.
 *
 * When testing the shader-provided texture offset, only the texel which is
 * expected to be fetched is set to the correct color. All other texels are
 * black. This trivially verifies that the texture offset works.
 *
 * When testing GL_TEXTURE_RECTANGLE, only the texel which is expected to be
 * fetched is set to the correct color.
 *
 * Texture targets with multiple layers/slices/faces have only one layer/etc
 * set to the expected value. The other layers are black, so that we can check
 * that the correct layer is read.
 *
 * Shadow samplers are tricky because we can't use the GL_EQUAL compare mode
 * because of precision issues. Therefore, we bind the same texture twice:
 * the first unit uses GL_LESS and a small number (tolerance) is subtracted
 * from Z, and the second unit uses GL_GREATER and a small number (tolerance)
 * is added to Z. If both shadow samplers return 1, which means the texel
 * value lies in between, the test passes.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 900;
	config.window_height = 600;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_SIZE 32
#define TEST_LAYER 9 // the layer index used for testing
#define LAST_LEVEL 5

static const float clear_colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0},
	{0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0},
};

static const float shadow_colors[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 1.0},
};

static const float clear_depths[] = {
	0.1,
	0.2,
	0.3,
	0.4,
	0.5,
	0.6
};

enum target_type {
	TEX_1D,		  /* proj. coords = vec2(x,w) */
	TEX_1D_PROJ_VEC4, /* proj. coords = vec4(x,0,0,w) */
	TEX_2D,		  /* proj. coords = vec3(x,y,w) */
	TEX_2D_PROJ_VEC4, /* proj. coords = vec4(x,y,0,w) */
	TEX_RECT,         /* proj. coords = vec3(x,y,w) */
	TEX_RECT_PROJ_VEC4, /* proj. coords = vec4(x,y,0,w) */
	TEX_3D,
	TEX_CUBE,
	TEX_1D_ARRAY,
	TEX_2D_ARRAY,
	TEX_CUBE_ARRAY,
	TEX_1D_SHADOW,
	TEX_2D_SHADOW,
	TEX_RECT_SHADOW,
	TEX_CUBE_SHADOW,
	TEX_1D_ARRAY_SHADOW,
	TEX_2D_ARRAY_SHADOW,
	TEX_CUBE_ARRAY_SHADOW,
};

#define IS_SHADOW(t) ((t) >= TEX_1D_SHADOW)

enum shader_type {
	FIXED_FUNCTION,
	GL2_TEXTURE,
	GL2_TEXTURE_BIAS,
	GL2_TEXTURE_PROJ,
	GL2_TEXTURE_PROJ_BIAS,
	ARB_TEXTURE_LOD,
	ARB_TEXTURE_PROJ_LOD,
	ARB_TEXTURE_GRAD,
	ARB_TEXTURE_PROJ_GRAD,
	GL3_TEXTURE_LOD,
	GL3_TEXTURE_BIAS,
	GL3_TEXTURE,
	GL3_TEXTURE_OFFSET,
	GL3_TEXTURE_OFFSET_BIAS,
	GL3_TEXTURE_PROJ,
	GL3_TEXTURE_PROJ_BIAS,
	GL3_TEXTURE_PROJ_OFFSET,
	GL3_TEXTURE_PROJ_OFFSET_BIAS,
	GL3_TEXTURE_LOD_OFFSET,
	GL3_TEXTURE_PROJ_LOD,
	GL3_TEXTURE_PROJ_LOD_OFFSET,
	GL3_TEXTURE_GRAD,
	GL3_TEXTURE_GRAD_OFFSET,
	GL3_TEXTURE_PROJ_GRAD,
	GL3_TEXTURE_PROJ_GRAD_OFFSET,
};

#define NEED_ARB_LOD(t) ((t) >= ARB_TEXTURE_LOD && (t) < GL3_TEXTURE_LOD)
#define NEED_GL3(t) ((t) >= GL3_TEXTURE_LOD)

static enum shader_type test = FIXED_FUNCTION;
static enum target_type target = TEX_2D;
static GLenum gltarget;
static GLboolean has_offset;
static GLboolean in_place_probing, no_bias, no_lod_clamp;
static GLint loc_lod = -1, loc_bias = -1, loc_z = -1, loc_dx = -1, loc_dy = -1;
static GLuint samp[2];
static int last_level = LAST_LEVEL;
static int offset[] = {3, -1, 2};

#define GL3_FS_PREAMBLE \
	"#version %s \n" \
	"#extension GL_ARB_texture_cube_map_array : enable \n" \
	"#extension GL_ARB_shader_texture_lod : enable\n" \
	"#extension GL_ARB_texture_rectangle : enable\n" \
	"uniform sampler%s tex, tex2; \n" \
	"uniform float z, lod, bias; \n" \
	"uniform vec3 dx, dy; \n" \
	"#define TYPE %s \n" \
	"#define DERIV_TYPE %s \n" \
	"#define MASK %s \n" \
	"#define OFFSET %s(ivec3(3, -1, 2)) \n" \
	"%s" \
	"#define textureInst %s \n" \
	"#define PARAMS %s \n" \
	"void main() {\n"

#define GL3_FS_CODE \
	GL3_FS_PREAMBLE \
	"  gl_FragColor = textureInst(tex, TYPE(gl_TexCoord[0]) PARAMS); \n" \
	"} \n"

#define GL3_FS_CODE_SHADOW \
	GL3_FS_PREAMBLE \
	"  gl_FragColor = vec4(textureInst(tex, TYPE(gl_TexCoord[0]) - 0.05 * MASK PARAMS) * \n" \
	"                      textureInst(tex2, TYPE(gl_TexCoord[0]) + 0.05 * MASK PARAMS)); \n" \
	"} \n"

#define GL3_FS_CODE_SHADOW_CUBEARRAY \
	GL3_FS_PREAMBLE \
	"  gl_FragColor = vec4(textureInst(tex, gl_TexCoord[0], z - 0.05) * \n" \
	"                      textureInst(tex2, gl_TexCoord[0], z + 0.05)); \n" \
	"} \n"

static void set_sampler_parameter(GLenum pname, GLint value)
{
	glSamplerParameteri(samp[0], pname, value);
	glSamplerParameteri(samp[1], pname, value);
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex, fb, prog;
	GLenum status;
	int i, level, layer, dim, num_layers;
	const char *target_str, *type_str, *compare_value_mask = "";
	const char *offset_type_str = "", *declaration = "", *instruction;
	const char *version = "130", *other_params = "", *deriv_type = "";
	GLenum format, attachment, clearbits;
	char fscode[2048];

        for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-inplace") == 0)
			in_place_probing = GL_TRUE;
		else if (strcmp(argv[i], "-nobias") == 0)
			no_bias = GL_TRUE;
		else if (strcmp(argv[i], "-nolod") == 0)
			no_lod_clamp = GL_TRUE;
		else if (strcmp(argv[i], "GL2:texture()") == 0)
			test = GL2_TEXTURE;
		else if (strcmp(argv[i], "GL2:texture(bias)") == 0)
			test = GL2_TEXTURE_BIAS;
		else if (strcmp(argv[i], "GL2:textureProj") == 0)
			test = GL2_TEXTURE_PROJ;
		else if (strcmp(argv[i], "GL2:textureProj(bias)") == 0)
			test = GL2_TEXTURE_PROJ_BIAS;
		else if (strcmp(argv[i], "*Lod") == 0)
			test = ARB_TEXTURE_LOD;
		else if (strcmp(argv[i], "*ProjLod") == 0)
			test = ARB_TEXTURE_PROJ_LOD;
		else if (strcmp(argv[i], "*GradARB") == 0)
			test = ARB_TEXTURE_GRAD;
		else if (strcmp(argv[i], "*ProjGradARB") == 0)
			test = ARB_TEXTURE_PROJ_GRAD;
		else if (strcmp(argv[i], "textureLod") == 0)
			test = GL3_TEXTURE_LOD;
		else if (strcmp(argv[i], "texture(bias)") == 0)
			test = GL3_TEXTURE_BIAS;
		else if (strcmp(argv[i], "texture()") == 0)
			test = GL3_TEXTURE;
		else if (strcmp(argv[i], "textureOffset") == 0)
			test = GL3_TEXTURE_OFFSET;
		else if (strcmp(argv[i], "textureOffset(bias)") == 0)
			test = GL3_TEXTURE_OFFSET_BIAS;
		else if (strcmp(argv[i], "textureProj") == 0)
			test = GL3_TEXTURE_PROJ;
		else if (strcmp(argv[i], "textureProj(bias)") == 0)
			test = GL3_TEXTURE_PROJ_BIAS;
		else if (strcmp(argv[i], "textureProjOffset") == 0)
			test = GL3_TEXTURE_PROJ_OFFSET;
		else if (strcmp(argv[i], "textureProjOffset(bias)") == 0)
			test = GL3_TEXTURE_PROJ_OFFSET_BIAS;
		else if (strcmp(argv[i], "textureLodOffset") == 0)
			test = GL3_TEXTURE_LOD_OFFSET;
		else if (strcmp(argv[i], "textureProjLod") == 0)
			test = GL3_TEXTURE_PROJ_LOD;
		else if (strcmp(argv[i], "textureProjLodOffset") == 0)
			test = GL3_TEXTURE_PROJ_LOD_OFFSET;
		else if (strcmp(argv[i], "textureGrad") == 0)
			test = GL3_TEXTURE_GRAD;
		else if (strcmp(argv[i], "textureGradOffset") == 0)
			test = GL3_TEXTURE_GRAD_OFFSET;
		else if (strcmp(argv[i], "textureProjGrad") == 0)
			test = GL3_TEXTURE_PROJ_GRAD;
		else if (strcmp(argv[i], "textureProjGradOffset") == 0)
			test = GL3_TEXTURE_PROJ_GRAD_OFFSET;
		else if (strcmp(argv[i], "1D") == 0)
			target = TEX_1D;
		else if (strcmp(argv[i], "1D_ProjVec4") == 0)
			target = TEX_1D_PROJ_VEC4;
		else if (strcmp(argv[i], "2D") == 0)
			target = TEX_2D;
		else if (strcmp(argv[i], "2D_ProjVec4") == 0)
			target = TEX_2D_PROJ_VEC4;
		else if (strcmp(argv[i], "2DRect") == 0)
			target = TEX_RECT;
		else if (strcmp(argv[i], "2DRect_ProjVec4") == 0)
			target = TEX_RECT_PROJ_VEC4;
		else if (strcmp(argv[i], "3D") == 0)
			target = TEX_3D;
		else if (strcmp(argv[i], "Cube") == 0)
			target = TEX_CUBE;
		else if (strcmp(argv[i], "1DArray") == 0)
			target = TEX_1D_ARRAY;
		else if (strcmp(argv[i], "2DArray") == 0)
			target = TEX_2D_ARRAY;
		else if (strcmp(argv[i], "CubeArray") == 0)
			target = TEX_CUBE_ARRAY;
		else if (strcmp(argv[i], "1DShadow") == 0)
			target = TEX_1D_SHADOW;
		else if (strcmp(argv[i], "2DShadow") == 0)
			target = TEX_2D_SHADOW;
		else if (strcmp(argv[i], "2DRectShadow") == 0)
			target = TEX_RECT_SHADOW;
		else if (strcmp(argv[i], "CubeShadow") == 0)
			target = TEX_CUBE_SHADOW;
		else if (strcmp(argv[i], "1DArrayShadow") == 0)
			target = TEX_1D_ARRAY_SHADOW;
		else if (strcmp(argv[i], "2DArrayShadow") == 0)
			target = TEX_2D_ARRAY_SHADOW;
		else if (strcmp(argv[i], "CubeArrayShadow") == 0)
			target = TEX_CUBE_ARRAY_SHADOW;
		else {
			printf("Unknown parameter: %s\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
        }

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_sampler_objects");
	piglit_require_extension("GL_ARB_texture_storage");
	if (test != FIXED_FUNCTION) {
		piglit_require_gl_version(20);
		piglit_require_GLSL_version(120);
	}
	if (NEED_ARB_LOD(test)) {
		piglit_require_extension("GL_ARB_shader_texture_lod");
	}
	piglit_require_gl_version(NEED_GL3(test) ? 30 : 14);

	if (target == TEX_2D_ARRAY_SHADOW &&
	    test == GL3_TEXTURE_OFFSET) {
		piglit_require_GLSL_version(430);
		version = "430";
	}

	switch (target) {
	case TEX_1D:
		gltarget = GL_TEXTURE_1D;
		target_str = "1D";
		type_str = "float";
		deriv_type = "float";
		offset_type_str = "int";
		break;
	case TEX_1D_PROJ_VEC4:
		gltarget = GL_TEXTURE_1D;
		target_str = "1D";
		type_str = "vec4";
		deriv_type = "float";
		offset_type_str = "int";
		break;
	case TEX_2D:
		gltarget = GL_TEXTURE_2D;
		target_str = "2D";
		type_str = "vec2";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		break;
	case TEX_2D_PROJ_VEC4:
		gltarget = GL_TEXTURE_2D;
		target_str = "2D";
		type_str = "vec4";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		break;
	case TEX_RECT:
		piglit_require_extension("GL_ARB_texture_rectangle");
		gltarget = GL_TEXTURE_RECTANGLE;
		target_str = "2DRect";
		type_str = "vec2";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		break;
	case TEX_RECT_PROJ_VEC4:
		piglit_require_extension("GL_ARB_texture_rectangle");
		gltarget = GL_TEXTURE_RECTANGLE;
		target_str = "2DRect";
		type_str = "vec4";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		break;
	case TEX_3D:
		gltarget = GL_TEXTURE_3D;
		target_str = "3D";
		type_str = "vec3";
		deriv_type = "vec3";
		offset_type_str = "ivec3";
		break;
	case TEX_CUBE:
		gltarget = GL_TEXTURE_CUBE_MAP;
		target_str = "Cube";
		type_str = "vec3";
		deriv_type = "vec3";
		break;
	case TEX_1D_ARRAY:
		piglit_require_gl_version(30);
		gltarget = GL_TEXTURE_1D_ARRAY;
		target_str = "1DArray";
		type_str = "vec2";
		deriv_type = "float";
		offset_type_str = "int";
		break;
	case TEX_2D_ARRAY:
		piglit_require_gl_version(30);
		gltarget = GL_TEXTURE_2D_ARRAY;
		target_str = "2DArray";
		type_str = "vec3";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		break;
	case TEX_CUBE_ARRAY:
		piglit_require_gl_version(30);
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		gltarget = GL_TEXTURE_CUBE_MAP_ARRAY;
		target_str = "CubeArray";
		type_str = "vec4";
		deriv_type = "vec3";
		break;
	case TEX_1D_SHADOW:
		gltarget = GL_TEXTURE_1D;
		target_str = "1DShadow";
		type_str = "vec3";
		deriv_type = "float";
		offset_type_str = "int";
		compare_value_mask = "vec3(0.0, 0.0, 1.0)";
		break;
	case TEX_2D_SHADOW:
		gltarget = GL_TEXTURE_2D;
		target_str = "2DShadow";
		type_str = "vec3";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		compare_value_mask = "vec3(0.0, 0.0, 1.0)";
		break;
	case TEX_RECT_SHADOW:
		piglit_require_extension("GL_ARB_texture_rectangle");
		gltarget = GL_TEXTURE_RECTANGLE;
		target_str = "2DRectShadow";
		type_str = "vec3";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		compare_value_mask = "vec3(0.0, 0.0, 1.0)";
		break;
	case TEX_CUBE_SHADOW:
		piglit_require_gl_version(30);
		gltarget = GL_TEXTURE_CUBE_MAP;
		target_str = "CubeShadow";
		type_str = "vec4";
		deriv_type = "vec3";
		compare_value_mask = "vec4(0.0, 0.0, 0.0, 1.0)";
		break;
	case TEX_1D_ARRAY_SHADOW:
		piglit_require_gl_version(30);
		gltarget = GL_TEXTURE_1D_ARRAY;
		target_str = "1DArrayShadow";
		type_str = "vec3";
		deriv_type = "float";
		offset_type_str = "int";
		compare_value_mask = "vec3(0.0, 0.0, 1.0)";
		break;
	case TEX_2D_ARRAY_SHADOW:
		piglit_require_gl_version(30);
		gltarget = GL_TEXTURE_2D_ARRAY;
		target_str = "2DArrayShadow";
		type_str = "vec4";
		deriv_type = "vec2";
		offset_type_str = "ivec2";
		compare_value_mask = "vec4(0.0, 0.0, 0.0, 1.0)";
		break;
	case TEX_CUBE_ARRAY_SHADOW:
		piglit_require_gl_version(30);
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		gltarget = GL_TEXTURE_CUBE_MAP_ARRAY;
		target_str = "CubeArrayShadow";
		type_str = "vec4";
		deriv_type = "vec3";
		break;
	}

	if (test == GL2_TEXTURE_PROJ ||
	    test == GL2_TEXTURE_PROJ_BIAS ||
	    test == ARB_TEXTURE_PROJ_LOD ||
	    test == ARB_TEXTURE_PROJ_GRAD ||
	    test == GL3_TEXTURE_PROJ ||
	    test == GL3_TEXTURE_PROJ_BIAS ||
	    test == GL3_TEXTURE_PROJ_OFFSET ||
	    test == GL3_TEXTURE_PROJ_OFFSET_BIAS ||
	    test == GL3_TEXTURE_PROJ_LOD ||
	    test == GL3_TEXTURE_PROJ_LOD_OFFSET ||
	    test == GL3_TEXTURE_PROJ_GRAD ||
	    test == GL3_TEXTURE_PROJ_GRAD_OFFSET) {
		if (!strcmp(type_str, "float"))
			type_str = "vec2";
		else if (!strcmp(type_str, "vec2"))
			type_str = "vec3";
		else if (!strcmp(type_str, "vec3"))
			type_str = "vec4";

		if (!strcmp(compare_value_mask, "vec3(0.0, 0.0, 1.0)"))
			compare_value_mask = "vec4(0.0, 0.0, 1.0, 0.0)";
	}

	switch (test) {
	case FIXED_FUNCTION:
		break;
	case GL2_TEXTURE_BIAS:
		other_params = ", bias";
		/* fall through */
	case GL2_TEXTURE:
		version = "120";
		switch (target) {
		case TEX_1D:
			instruction = "texture1D";
			break;
		case TEX_2D:
			instruction = "texture2D";
			break;
		case TEX_3D:
			instruction = "texture3D";
			break;
		case TEX_CUBE:
			instruction = "textureCube";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1D";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2D";
			break;
		case TEX_RECT:
			instruction = "texture2DRect";
			break;
		case TEX_RECT_SHADOW:
			instruction = "shadow2DRect";
			break;
		default:
			assert(0);
		}
		break;
	case GL2_TEXTURE_PROJ_BIAS:
		other_params = ", bias";
		/* fall through */
	case GL2_TEXTURE_PROJ:
		version = "120";
		switch (target) {
		case TEX_1D:
		case TEX_1D_PROJ_VEC4:
			instruction = "texture1DProj";
			break;
		case TEX_2D:
		case TEX_2D_PROJ_VEC4:
			instruction = "texture2DProj";
			break;
		case TEX_3D:
			instruction = "texture3DProj";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1DProj";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2DProj";
			break;
		case TEX_RECT:
		case TEX_RECT_PROJ_VEC4:
			instruction = "texture2DRectProj";
			break;
		case TEX_RECT_SHADOW:
			instruction = "shadow2DRectProj";
			break;
		default:
			assert(0);
		}
		break;
	case ARB_TEXTURE_LOD:
		version = "120";
		switch (target) {
		case TEX_1D:
			instruction = "texture1DLod";
			break;
		case TEX_2D:
			instruction = "texture2DLod";
			break;
		case TEX_3D:
			instruction = "texture3DLod";
			break;
		case TEX_CUBE:
			instruction = "textureCubeLod";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1DLod";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2DLod";
			break;
		default:
			assert(0);
		}
		other_params = ", lod";
		break;
	case ARB_TEXTURE_PROJ_LOD:
		version = "120";
		switch (target) {
		case TEX_1D:
		case TEX_1D_PROJ_VEC4:
			instruction = "texture1DProjLod";
			break;
		case TEX_2D:
		case TEX_2D_PROJ_VEC4:
			instruction = "texture2DProjLod";
			break;
		case TEX_3D:
			instruction = "texture3DProjLod";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1DProjLod";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2DProjLod";
			break;
		default:
			assert(0);
		}
		other_params = ", lod";
		break;
	case ARB_TEXTURE_GRAD:
		version = "120";
		switch (target) {
		case TEX_1D:
			instruction = "texture1DGradARB";
			break;
		case TEX_2D:
			instruction = "texture2DGradARB";
			break;
		case TEX_3D:
			instruction = "texture3DGradARB";
			break;
		case TEX_CUBE:
			instruction = "textureCubeGradARB";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1DGradARB";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2DGradARB";
			break;
		case TEX_RECT:
			instruction = "texture2DRectGradARB";
			break;
		case TEX_RECT_SHADOW:
			instruction = "shadow2DRectGradARB";
			break;
		default:
			assert(0);
		}
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy)";
		break;
	case ARB_TEXTURE_PROJ_GRAD:
		version = "120";
		switch (target) {
		case TEX_1D:
		case TEX_1D_PROJ_VEC4:
			instruction = "texture1DProjGradARB";
			break;
		case TEX_2D:
		case TEX_2D_PROJ_VEC4:
			instruction = "texture2DProjGradARB";
			break;
		case TEX_3D:
			instruction = "texture3DProjGradARB";
			break;
		case TEX_1D_SHADOW:
			instruction = "shadow1DProjGradARB";
			break;
		case TEX_2D_SHADOW:
			instruction = "shadow2DProjGradARB";
			break;
		case TEX_RECT:
		case TEX_RECT_PROJ_VEC4:
			instruction = "texture2DRectProjGradARB";
			break;
		case TEX_RECT_SHADOW:
			instruction = "shadow2DRectProjGradARB";
			break;
		default:
			assert(0);
		}
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy)";
		break;
	case GL3_TEXTURE_LOD:
		instruction = "textureLod";
		other_params = ", lod";
		break;
	case GL3_TEXTURE_BIAS:
		instruction = "texture";
		other_params = ", bias";
		break;
	case GL3_TEXTURE:
		instruction = "texture";
		break;
	case GL3_TEXTURE_OFFSET:
		instruction = "textureOffset";
		other_params = ", OFFSET";
		break;
	case GL3_TEXTURE_OFFSET_BIAS:
		instruction = "textureOffset";
		other_params = ", OFFSET, bias";
		break;
	case GL3_TEXTURE_PROJ:
		instruction = "textureProj";
		break;
	case GL3_TEXTURE_PROJ_BIAS:
		instruction = "textureProj";
		other_params = ", bias";
		break;
	case GL3_TEXTURE_PROJ_OFFSET:
		instruction = "textureProjOffset";
		other_params = ", OFFSET";
		break;
	case GL3_TEXTURE_PROJ_OFFSET_BIAS:
		instruction = "textureProjOffset";
		other_params = ", OFFSET, bias";
		break;
	case GL3_TEXTURE_LOD_OFFSET:
		instruction = "textureLodOffset";
		other_params = ", lod, OFFSET";
		break;
	case GL3_TEXTURE_PROJ_LOD:
		instruction = "textureProjLod";
		other_params = ", lod";
		break;
	case GL3_TEXTURE_PROJ_LOD_OFFSET:
		instruction = "textureProjLodOffset";
		other_params = ", lod, OFFSET";
		break;
	case GL3_TEXTURE_GRAD:
		instruction = "textureGrad";
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy)";
		break;
	case GL3_TEXTURE_GRAD_OFFSET:
		instruction = "textureGradOffset";
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy), OFFSET";
		break;
	case GL3_TEXTURE_PROJ_GRAD:
		instruction = "textureProjGrad";
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy)";
		break;
	case GL3_TEXTURE_PROJ_GRAD_OFFSET:
		instruction = "textureProjGradOffset";
		other_params = ", DERIV_TYPE(dx), DERIV_TYPE(dy), OFFSET";
		break;
	default:
		assert(0);
	}

	if (test != FIXED_FUNCTION) {
		GLuint loc_tex, loc_tex2;

		if (test == GL3_TEXTURE &&
		    target == TEX_CUBE_ARRAY_SHADOW)
			sprintf(fscode, GL3_FS_CODE_SHADOW_CUBEARRAY,
				version, target_str, type_str, deriv_type,
				compare_value_mask, offset_type_str,
				declaration, instruction, other_params);
		else if (IS_SHADOW(target))
			sprintf(fscode, GL3_FS_CODE_SHADOW, version, target_str,
				type_str, deriv_type, compare_value_mask,
				offset_type_str, declaration, instruction,
				other_params);
		else
			sprintf(fscode, GL3_FS_CODE, version, target_str,
				type_str, deriv_type, compare_value_mask,
				offset_type_str, declaration, instruction,
				other_params);

		prog = piglit_build_simple_program(NULL, fscode);

		glUseProgram(prog);
		loc_tex = glGetUniformLocation(prog, "tex");
		glUniform1i(loc_tex, 0);

		if (IS_SHADOW(target)) {
			loc_tex2 = glGetUniformLocation(prog, "tex2");
			glUniform1i(loc_tex2, 1);
		}

		if (test == GL3_TEXTURE &&
		    target == TEX_CUBE_ARRAY_SHADOW)
			loc_z = glGetUniformLocation(prog, "z");

		if (test == ARB_TEXTURE_LOD ||
		    test == ARB_TEXTURE_PROJ_LOD ||
		    test == GL3_TEXTURE_LOD ||
		    test == GL3_TEXTURE_LOD_OFFSET ||
		    test == GL3_TEXTURE_PROJ_LOD ||
		    test == GL3_TEXTURE_PROJ_LOD_OFFSET)
			loc_lod = glGetUniformLocation(prog, "lod");

		if (test == GL2_TEXTURE_BIAS ||
		    test == GL2_TEXTURE_PROJ_BIAS ||
		    test == GL3_TEXTURE_BIAS ||
		    test == GL3_TEXTURE_OFFSET_BIAS ||
		    test == GL3_TEXTURE_PROJ_BIAS ||
		    test == GL3_TEXTURE_PROJ_OFFSET_BIAS)
			loc_bias = glGetUniformLocation(prog, "bias");

		if (test == ARB_TEXTURE_GRAD ||
		    test == ARB_TEXTURE_PROJ_GRAD ||
		    test == GL3_TEXTURE_GRAD ||
		    test == GL3_TEXTURE_GRAD_OFFSET ||
		    test == GL3_TEXTURE_PROJ_GRAD ||
		    test == GL3_TEXTURE_PROJ_GRAD_OFFSET) {
			loc_dx = glGetUniformLocation(prog, "dx");
			loc_dy = glGetUniformLocation(prog, "dy");
		}

		if (test == GL3_TEXTURE_OFFSET ||
		    test == GL3_TEXTURE_OFFSET_BIAS ||
		    test == GL3_TEXTURE_PROJ_OFFSET ||
		    test == GL3_TEXTURE_PROJ_OFFSET_BIAS ||
		    test == GL3_TEXTURE_LOD_OFFSET ||
		    test == GL3_TEXTURE_PROJ_LOD_OFFSET ||
		    test == GL3_TEXTURE_GRAD_OFFSET ||
		    test == GL3_TEXTURE_PROJ_GRAD_OFFSET) {
			has_offset = GL_TRUE;
			no_lod_clamp = GL_TRUE; /* not implemented for now */
		}
	}

	glGenTextures(1, &tex);
	glBindTexture(gltarget, tex);

	if (IS_SHADOW(target)) {
		format = GL_DEPTH_COMPONENT24;
		attachment = GL_DEPTH_ATTACHMENT;
		clearbits = GL_DEPTH_BUFFER_BIT;
	}
	else {
		format = GL_RGBA8;
		attachment = GL_COLOR_ATTACHMENT0;
		clearbits = GL_COLOR_BUFFER_BIT;
	}

	switch (gltarget) {
	case GL_TEXTURE_1D:
		num_layers = 1;
		glTexStorage1D(gltarget, 6, format, TEX_SIZE);
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_CUBE_MAP:
	case GL_TEXTURE_1D_ARRAY:
		num_layers = gltarget == GL_TEXTURE_CUBE_MAP ? 6 :
			     gltarget == GL_TEXTURE_1D_ARRAY ? TEX_SIZE : 1;
		glTexStorage2D(gltarget, 6, format, TEX_SIZE, TEX_SIZE);
		break;
	case GL_TEXTURE_RECTANGLE:
		num_layers = 1;
		last_level = 0;
		glTexStorage2D(gltarget, 1, format, TEX_SIZE, TEX_SIZE);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		num_layers = gltarget == GL_TEXTURE_CUBE_MAP_ARRAY ? 36 : TEX_SIZE;
		glTexStorage3D(gltarget, 6, format, TEX_SIZE, TEX_SIZE, num_layers);
		break;
	default:
		assert(0);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (test == FIXED_FUNCTION)
		glDisable(gltarget);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	/* set one layer/face to the expected color and the other layers/faces to black */
	for (level = 0, dim = TEX_SIZE; dim > 0; level++, dim /= 2) {
		if (gltarget == GL_TEXTURE_3D)
			num_layers = dim;

		for (layer = 0; layer < num_layers; layer++) {
			switch (gltarget) {
			case GL_TEXTURE_1D:
				glFramebufferTexture1D(GL_FRAMEBUFFER, attachment,
						       gltarget, tex, level);
				break;
			case GL_TEXTURE_2D:
			case GL_TEXTURE_RECTANGLE:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
						       gltarget, tex, level);
				break;
			case GL_TEXTURE_CUBE_MAP:
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
						       GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
						       tex, level);
				break;
			case GL_TEXTURE_3D:
				glFramebufferTexture3D(GL_FRAMEBUFFER, attachment,
						       gltarget, tex, level, layer);
				break;
			case GL_TEXTURE_1D_ARRAY:
			case GL_TEXTURE_2D_ARRAY:
			case GL_TEXTURE_CUBE_MAP_ARRAY:
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
							  tex, level, layer);
				break;
			}
			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);

			status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				fprintf(stderr, "FBO incomplete status 0x%X for level %i, layer %i\n",
					status, level, layer);
				piglit_report_result(PIGLIT_SKIP);
			}

			/* For array and cube textures, only TEST_LAYER is
			 * cleared to the expected value.
			 * For 3D textures, the middle slice is cleared. */
			if (num_layers == 1 ||
			    (gltarget == GL_TEXTURE_3D && layer == num_layers/2 + (has_offset ? offset[2] : 0)) ||
			    (gltarget != GL_TEXTURE_3D && layer == TEST_LAYER % num_layers)) {
				if (has_offset || gltarget == GL_TEXTURE_RECTANGLE) {
					/* For testing the shader-provided texture offset,
					 * only clear the texel which is expected
					 * to be fetched. The other texels are black. */
					glClearColor(0, 0, 0, 0);
					glClearDepth(0);
					glClear(clearbits);

					glClearColor(clear_colors[level][0],
							clear_colors[level][1],
							clear_colors[level][2],
							0.0);
					glClearDepth(clear_depths[level]);
					glEnable(GL_SCISSOR_TEST);
					/* Add +1, because the probed texel is at (1,1). */
					if (has_offset) {
						glScissor(offset[0]+1,
							  gltarget == GL_TEXTURE_1D ||
							  gltarget == GL_TEXTURE_1D_ARRAY ? 0 : offset[1]+1,
							  1, 1);
					}
					else {
						glScissor(1,
							  gltarget == GL_TEXTURE_1D ||
							  gltarget == GL_TEXTURE_1D_ARRAY ? 0 : 1,
							  1, 1);
					}
					glClear(clearbits);
					glDisable(GL_SCISSOR_TEST);
				}
				else {
					glClearColor(clear_colors[level][0],
							clear_colors[level][1],
							clear_colors[level][2],
							0.0);
					glClearDepth(clear_depths[level]);
					glClear(clearbits);
				}
			}
			else {
				glClearColor(0, 0, 0, 0);
				glClearDepth(0);
				glClear(clearbits);
			}

			if (!piglit_check_gl_error(GL_NO_ERROR))
				piglit_report_result(PIGLIT_FAIL);
		}

		if (gltarget == GL_TEXTURE_RECTANGLE)
			break;
	}

	glDeleteFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	if (test == FIXED_FUNCTION)
		glEnable(gltarget);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glGenSamplers(2, samp);
	glBindSampler(0, samp[0]);

	set_sampler_parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	set_sampler_parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	/* need to set this for rect targets, otherwise default GL_REPEAT
	 * in sampler obj should trigger incomplete tex behavior */
	set_sampler_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	set_sampler_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (IS_SHADOW(target)) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(gltarget, tex);
		glActiveTexture(GL_TEXTURE0);
		glBindSampler(1, samp[1]);

		set_sampler_parameter(GL_TEXTURE_COMPARE_MODE,
				      GL_COMPARE_REF_TO_TEXTURE);
		glSamplerParameteri(samp[0], GL_TEXTURE_COMPARE_FUNC, GL_LESS);
		glSamplerParameteri(samp[1], GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

#define SET_VEC(c, x, y, z, w) do { c[0] = x; c[1] = y; c[2] = z; c[3] = w; } while (0)

static void
fix_normalized_coordinates(int expected_level,
			   float *s0, float *t0, float *s1, float *t1)
{
	/* When testing the texture offset, there is only one pixel which
	 * is not black and it has the same integer coordinates in every
	 * mipmap level, but not the same normalized coordinates. Therefore,
	 * we have to fix the normalized ones, so that GLSL always reads
	 * from the same integer coordinates.
	 */
	if (expected_level > 0) {
		float pixsize_base = 1.0 / TEX_SIZE;
		float offset = pixsize_base * ((1 << (expected_level-1))*3 - 1.5);

		*s0 += offset;
		*t0 += offset;
		*s1 += offset;
		*t1 += offset;
	}
}

static void
draw_quad(int x, int y, int w, int h, int expected_level, int fetch_level,
	  int baselevel, int maxlevel, int bias, int mipfilter)
{
	/* 2D coordinates */
	float s0 = 0;
	float t0 = 0;
	float s1 = (float)w / TEX_SIZE;
	float t1 = (float)h / TEX_SIZE;
	float x0, z0, x1, z1;
	/* Final coordinates */
	float c0[4], c1[4], c2[4], c3[4];
	/* shadow compare value */
	float z = clear_depths[expected_level];
	/* multiplier for textureProj */
	float p = 1;
	/* explicit derivative */
	float deriv = 1.0 / (TEX_SIZE >> fetch_level);

	/* Explicit derivatives for cubemaps.
	 *
	 * Each of these vectors should be a difference between (x,y,z)
	 * coordinates from one pixel and its neighbor. The Y coordinate is
	 * always -1 (negative Y face).
	 *
	 * The range for X and Z is [-1,1] for cubemaps as opposed
	 * to [0,1] for 2D textures, therefore multiply the 2D derivative
	 * by 2. The result is 2 vectors on the -Y surface of the cube.
	 */
	float cube_deriv_x[3] = {deriv*2, 0,       0};
	float cube_deriv_y[3] = {      0, 0, deriv*2};

	switch (test) {
	case ARB_TEXTURE_LOD:
	case ARB_TEXTURE_PROJ_LOD:
	case GL3_TEXTURE_LOD:
	case GL3_TEXTURE_PROJ_LOD:
		/* set an explicit LOD */
		glUniform1f(loc_lod, fetch_level - baselevel);
		break;

	case GL2_TEXTURE_BIAS:
	case GL2_TEXTURE_PROJ_BIAS:
	case GL3_TEXTURE_BIAS:
	case GL3_TEXTURE_PROJ_BIAS:
		/* set a bias */
		glUniform1f(loc_bias, bias);
		/* fall through to scale the coordinates */
	case GL2_TEXTURE:
	case GL2_TEXTURE_PROJ:
	case GL3_TEXTURE:
	case GL3_TEXTURE_PROJ:
	case FIXED_FUNCTION:
		/* scale the coordinates (decrease the texel size),
		 * so that the texture fetch selects this level
		 */
		s1 *= 1 << fetch_level;
		t1 *= 1 << fetch_level;
		break;

	case GL3_TEXTURE_GRAD_OFFSET:
	case GL3_TEXTURE_PROJ_GRAD_OFFSET:
		fix_normalized_coordinates(expected_level, &s0, &t0, &s1, &t1);
		/* fall through */
	case ARB_TEXTURE_GRAD:
	case ARB_TEXTURE_PROJ_GRAD:
	case GL3_TEXTURE_GRAD:
	case GL3_TEXTURE_PROJ_GRAD:
		if (gltarget == GL_TEXTURE_CUBE_MAP ||
		    gltarget == GL_TEXTURE_CUBE_MAP_ARRAY) {
			glUniform3fv(loc_dx, 1, cube_deriv_x);
			glUniform3fv(loc_dy, 1, cube_deriv_y);
		}
		else if (gltarget == GL_TEXTURE_3D) {
			glUniform3f(loc_dx, 0, 0, deriv);
			glUniform3f(loc_dy, 0, 0, deriv);
		}
		else if (gltarget == GL_TEXTURE_1D ||
			 gltarget == GL_TEXTURE_1D_ARRAY) {
			glUniform3f(loc_dx, deriv, 0, 0);
			glUniform3f(loc_dy, deriv, 0, 0);
		}
		else {
			glUniform3f(loc_dx, 0, deriv, 0);
			glUniform3f(loc_dy, 0, deriv, 0);
		}
		break;

	case GL3_TEXTURE_OFFSET_BIAS:
	case GL3_TEXTURE_PROJ_OFFSET_BIAS:
		glUniform1f(loc_bias, bias);
		/* fall through */
	case GL3_TEXTURE_OFFSET:
	case GL3_TEXTURE_PROJ_OFFSET:
	{
		/* When testing the texture offset, there is only one pixel which
		 * is not black and it has the same integer coordinates in every
		 * mipmap level, but not the same normalized coordinates. Therefore,
		 * we have to fix the normalized ones, so that GLSL always reads
		 * from the same integer coordinates.
		 */
		int maxlevel_clamped = mipfilter ? maxlevel : baselevel;
		int bias_clamped =
			CLAMP(fetch_level + bias, baselevel, maxlevel_clamped) - fetch_level;

		/* scale the coordinates */
		s1 *= 1 << fetch_level;
		t1 *= 1 << fetch_level;

		if (bias_clamped > 0) {
			float pixsize_before_bias = 1.0 / (TEX_SIZE >> fetch_level);
			float offset = pixsize_before_bias * ((1 << (bias_clamped-1))*3 - 1.5);

			s0 += offset;
			t0 += offset;
			s1 += offset;
			t1 += offset;
		}
		else if (bias_clamped < 0) {
			float pixsize_after_bias = 1.0 / (TEX_SIZE >> (fetch_level + bias_clamped));
			float offset = -pixsize_after_bias * ((1 << (-bias_clamped-1))*3 - 1.5);

			s0 += offset;
			t0 += offset;
			s1 += offset;
			t1 += offset;
		}
		break;
	}

	case GL3_TEXTURE_LOD_OFFSET:
	case GL3_TEXTURE_PROJ_LOD_OFFSET:
		glUniform1f(loc_lod, fetch_level - baselevel);
		fix_normalized_coordinates(expected_level, &s0, &t0, &s1, &t1);
		break;
	default:
		assert(0);
	}

	if (test == GL2_TEXTURE_PROJ ||
	    test == GL2_TEXTURE_PROJ_BIAS ||
	    test == ARB_TEXTURE_PROJ_LOD ||
	    test == ARB_TEXTURE_PROJ_GRAD ||
	    test == GL3_TEXTURE_PROJ ||
	    test == GL3_TEXTURE_PROJ_BIAS ||
	    test == GL3_TEXTURE_PROJ_OFFSET ||
	    test == GL3_TEXTURE_PROJ_OFFSET_BIAS ||
	    test == GL3_TEXTURE_PROJ_LOD ||
	    test == GL3_TEXTURE_PROJ_LOD_OFFSET ||
	    test == GL3_TEXTURE_PROJ_GRAD ||
	    test == GL3_TEXTURE_PROJ_GRAD_OFFSET)
		p = 7;

	/* Cube coordinates */
	x0 = 2*s0 - 1;
	z0 = 2*t0 - 1;
	x1 = 2*s1 - 1;
	z1 = 2*t1 - 1;

	switch (target) {
	case TEX_1D:
		SET_VEC(c0, s0*p, p, 0, 1);
		SET_VEC(c1, s1*p, p, 0, 1);
		SET_VEC(c2, s1*p, p, 0, 1);
		SET_VEC(c3, s0*p, p, 0, 1);
		break;
	case TEX_2D:
		SET_VEC(c0, s0*p, t0*p, p, 1);
		SET_VEC(c1, s1*p, t0*p, p, 1);
		SET_VEC(c2, s1*p, t1*p, p, 1);
		SET_VEC(c3, s0*p, t1*p, p, 1);
		break;
	case TEX_1D_PROJ_VEC4:
	case TEX_2D_PROJ_VEC4:
		SET_VEC(c0, s0*p, t0*p, 0, p);
		SET_VEC(c1, s1*p, t0*p, 0, p);
		SET_VEC(c2, s1*p, t1*p, 0, p);
		SET_VEC(c3, s0*p, t1*p, 0, p);
		break;
	case TEX_RECT:
		SET_VEC(c0, 0*p, 0*p, p, 1);
		SET_VEC(c1, w*p, 0*p, p, 1);
		SET_VEC(c2, w*p, h*p, p, 1);
		SET_VEC(c3, 0*p, h*p, p, 1);
		break;
	case TEX_RECT_PROJ_VEC4:
		SET_VEC(c0, 0*p, 0*p, 0, p);
		SET_VEC(c1, w*p, 0*p, 0, p);
		SET_VEC(c2, w*p, h*p, 0, p);
		SET_VEC(c3, 0*p, h*p, 0, p);
		break;
	case TEX_2D_ARRAY:
		SET_VEC(c0, s0, t0, TEST_LAYER, 1);
		SET_VEC(c1, s1, t0, TEST_LAYER, 1);
		SET_VEC(c2, s1, t1, TEST_LAYER, 1);
		SET_VEC(c3, s0, t1, TEST_LAYER, 1);
		break;
	case TEX_1D_SHADOW:
	case TEX_2D_SHADOW:
		SET_VEC(c0, s0*p, t0*p, z*p, p);
		SET_VEC(c1, s1*p, t0*p, z*p, p);
		SET_VEC(c2, s1*p, t1*p, z*p, p);
		SET_VEC(c3, s0*p, t1*p, z*p, p);
		break;
	case TEX_RECT_SHADOW:
		SET_VEC(c0, 0*p, 0*p, z*p, p);
		SET_VEC(c1, w*p, 0*p, z*p, p);
		SET_VEC(c2, w*p, h*p, z*p, p);
		SET_VEC(c3, 0*p, h*p, z*p, p);
		break;
	case TEX_1D_ARRAY_SHADOW:
		SET_VEC(c0, s0, TEST_LAYER, z, 1);
		SET_VEC(c1, s1, TEST_LAYER, z, 1);
		SET_VEC(c2, s1, TEST_LAYER, z, 1);
		SET_VEC(c3, s0, TEST_LAYER, z, 1);
		break;
	case TEX_2D_ARRAY_SHADOW:
		SET_VEC(c0, s0, t0, TEST_LAYER, z);
		SET_VEC(c1, s1, t0, TEST_LAYER, z);
		SET_VEC(c2, s1, t1, TEST_LAYER, z);
		SET_VEC(c3, s0, t1, TEST_LAYER, z);
		break;
	case TEX_3D:
		SET_VEC(c0, s0*p, t0*p, 0.5*p, p);
		SET_VEC(c1, s1*p, t0*p, 0.5*p, p);
		SET_VEC(c2, s1*p, t1*p, 0.5*p, p);
		SET_VEC(c3, s0*p, t1*p, 0.5*p, p);
		break;
	case TEX_1D_ARRAY:
		SET_VEC(c0, s0, TEST_LAYER, 0, 1);
		SET_VEC(c1, s1, TEST_LAYER, 0, 1);
		SET_VEC(c2, s1, TEST_LAYER, 0, 1);
		SET_VEC(c3, s0, TEST_LAYER, 0, 1);
		break;
	case TEX_CUBE_ARRAY_SHADOW:
		/* Set the compare value through a uniform, because all
		 * components of TexCoord0 are taken. */
		glUniform1f(loc_z, z);
		/* fall through */
	case TEX_CUBE:
	case TEX_CUBE_ARRAY:
		assert(TEST_LAYER % 6 == 3); /* negative Y */
		SET_VEC(c0, x0, -1, z0, TEST_LAYER / 6);
		SET_VEC(c1, x1, -1, z0, TEST_LAYER / 6);
		SET_VEC(c2, x1, -1, z1, TEST_LAYER / 6);
		SET_VEC(c3, x0, -1, z1, TEST_LAYER / 6);
		break;
	case TEX_CUBE_SHADOW:
		assert(TEST_LAYER % 6 == 3); /* negative Y */
		SET_VEC(c0, x0, -1, z0, z);
		SET_VEC(c1, x1, -1, z0, z);
		SET_VEC(c2, x1, -1, z1, z);
		SET_VEC(c3, x0, -1, z1, z);
		break;
	default:
		assert(0);
	}

	glBegin(GL_QUADS);

	glTexCoord4fv(c0);
	glVertex2f(x, y);
	glTexCoord4fv(c1);
	glVertex2f(x + w, y);
	glTexCoord4fv(c2);
	glVertex2f(x + w, y + h);
	glTexCoord4fv(c3);
	glVertex2f(x, y + h);

	glEnd();
}

static bool
colors_equal(const float *c1, const unsigned char *c2)
{
	int i;

	for (i = 0; i < 3; i++)
		if (fabs(c1[i] - (c2[i]/255.0)) > 0.01)
			return false;
	return true;
}

static bool
check_result(const unsigned char *probed, int expected_level,
	     int fetch_level, int baselevel, int maxlevel, int minlod, int maxlod,
	     int bias, int mipfilter)
{
	const float (*colors)[3] = IS_SHADOW(target) ? shadow_colors : clear_colors;

	if (!colors_equal(colors[expected_level], probed)) {
		int i;
		static const float black[] = {0, 0, 0};

		printf("Failure:\n");
#if 0 /* disabled, not needed unless you are debugging the test */
		printf("  Expected: %f %f %f\n", colors[expected_level][0],
				colors[expected_level][1], colors[expected_level][2]);
		printf("  Observed: %f %f %f\n", probed[0]/255.0,
				probed[1]/255.0, probed[2]/255.0);
#endif
		printf("  Expected level: %i\n", expected_level);

		if (IS_SHADOW(target)) {
			if (colors_equal(black, probed))
				puts("  Observed: shadow comparison failed");
			else
				puts("  Observed: unknown value (broken driver?)");
		}
		else {
			for (i = 0; i <= last_level; i++) {
				if (colors_equal(colors[i], probed)) {
					printf("  Observed level: %i\n", i);
					break;
				}
			}
			if (i == last_level+1) {
				if (colors_equal(black, probed))
					puts("  Observed: wrong layer/face/slice or wrong level or wrong offset");
				else
					puts("  Observed: unknown value (broken driver?)");
			}
		}

		printf("  Fetch level: %i, baselevel: %i, maxlevel: %i, "
		       "minlod: %i, maxlod: %i, bias: %i, mipfilter: %s\n",
		       fetch_level, baselevel, maxlevel, minlod,
		       no_lod_clamp ? last_level : maxlod, bias, mipfilter ? "yes" : "no");
		return false;
	}
	return true;
}

static int
calc_expected_level(int fetch_level, int baselevel, int maxlevel, int minlod,
		    int maxlod, int bias, int mipfilter)
{
	int expected_level;

	if (mipfilter) {
		if (no_lod_clamp) {
			expected_level = CLAMP(fetch_level + bias,
					       baselevel,
					       maxlevel);
		} else {
			expected_level = CLAMP(fetch_level + bias,
					       MIN2(baselevel + minlod, maxlevel),
					       MIN2(baselevel + maxlod, maxlevel));
		}
	} else {
		expected_level = baselevel;
	}
	assert(expected_level >= 0 && expected_level <= last_level);
	return expected_level;
}

enum piglit_result
piglit_display(void)
{
	int fetch_level, baselevel, maxlevel, minlod, maxlod, bias, mipfilter;
	int expected_level, x, y, total, failed;
	int start_bias, end_bias;
	int end_min_lod, end_max_lod, end_mipfilter, end_fetch_level;

	if (no_bias) {
		start_bias = 0;
		end_bias = 0;
	} else {
		start_bias = -last_level;
		end_bias = last_level;
	}

	if (no_lod_clamp) {
		end_min_lod = 0;
		end_max_lod = 0;
	} else {
		end_min_lod = last_level;
		end_max_lod = last_level;
	}

	end_mipfilter = gltarget == GL_TEXTURE_RECTANGLE ? 0 : 1;

	/* It's impossible to scale texture coordinates to fetch the last
	 * level of a cubemap on a 3x3 quad. */
	if ((gltarget == GL_TEXTURE_CUBE_MAP ||
	     gltarget == GL_TEXTURE_CUBE_MAP_ARRAY) &&
	    (test == GL2_TEXTURE ||
	     test == GL2_TEXTURE_BIAS ||
	     test == GL3_TEXTURE ||
	     test == GL3_TEXTURE_BIAS)) {
		end_fetch_level = last_level - 1;
	}
	else {
		end_fetch_level = last_level;
	}

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	total = 0;
	failed = 0;
	for (fetch_level = 0; fetch_level <= end_fetch_level; fetch_level++)
		for (baselevel = 0; baselevel <= last_level; baselevel++)
			for (maxlevel = baselevel; maxlevel <= last_level; maxlevel++)
				for (minlod = 0; minlod <= end_min_lod; minlod++)
					for (maxlod = minlod; maxlod <= end_max_lod; maxlod++)
						for (bias = start_bias; bias <= end_bias; bias++)
							for (mipfilter = 0; mipfilter <= end_mipfilter; mipfilter++) {
								expected_level = calc_expected_level(fetch_level, baselevel,
											maxlevel, minlod, maxlod, bias,
											mipfilter);

								/* Skip this if the offset pixel lies outside of the texture. */
								if (has_offset &&
								    (TEX_SIZE >> expected_level) <= 1+MAX2(offset[0], offset[1]))
									continue;

								if (gltarget != GL_TEXTURE_RECTANGLE) {
									glTexParameteri(gltarget, GL_TEXTURE_BASE_LEVEL, baselevel);
									glTexParameteri(gltarget, GL_TEXTURE_MAX_LEVEL, maxlevel);
									if (!no_lod_clamp) {
										set_sampler_parameter(GL_TEXTURE_MIN_LOD, minlod);
										set_sampler_parameter(GL_TEXTURE_MAX_LOD, maxlod);
									}
									if (!no_bias &&
									    test != GL2_TEXTURE_BIAS &&
									    test != GL2_TEXTURE_PROJ_BIAS &&
									    test != GL3_TEXTURE_BIAS &&
									    test != GL3_TEXTURE_PROJ_BIAS &&
									    test != GL3_TEXTURE_OFFSET_BIAS &&
									    test != GL3_TEXTURE_PROJ_OFFSET_BIAS)
										set_sampler_parameter(GL_TEXTURE_LOD_BIAS, bias);
									set_sampler_parameter(GL_TEXTURE_MIN_FILTER,
											mipfilter ? GL_NEAREST_MIPMAP_NEAREST
												  : GL_NEAREST);
								}

								x = (total % (piglit_width/3)) * 3;
								y = (total / (piglit_width/3)) * 3;

								draw_quad(x, y, 3, 3, expected_level, fetch_level,
									  baselevel, maxlevel, bias, mipfilter);

								if (in_place_probing) {
									unsigned char probe[3];

									glReadPixels(x+1, y+1, 1, 1, GL_RGB,
										     GL_UNSIGNED_BYTE, probe);

									if (!check_result(probe, expected_level,
											  fetch_level, baselevel,
											  maxlevel, minlod, maxlod,
											  bias, mipfilter)) {
										failed++;
										if (failed > 100) {
											printf("Stopping after 100 failures\n");
											goto end;
										}
									}
								}
								total++;
							}

	if (!in_place_probing) {
		unsigned char *pix, *p;

		pix = malloc(piglit_width * piglit_height * 4);
		glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, GL_UNSIGNED_BYTE, pix);

		total = 0;
		for (fetch_level = 0; fetch_level <= end_fetch_level; fetch_level++)
			for (baselevel = 0; baselevel <= last_level; baselevel++)
				for (maxlevel = baselevel; maxlevel <= last_level; maxlevel++)
					for (minlod = 0; minlod <= end_min_lod; minlod++)
						for (maxlod = minlod; maxlod <= end_max_lod; maxlod++)
							for (bias = start_bias; bias <= end_bias; bias++)
								for (mipfilter = 0; mipfilter <= end_mipfilter; mipfilter++) {
									expected_level = calc_expected_level(fetch_level,
												baselevel, maxlevel, minlod,
												maxlod, bias, mipfilter);

									/* Skip this if the offset pixel lies outside of the texture. */
									if (has_offset &&
									    (TEX_SIZE >> expected_level) <= 1+MAX2(offset[0], offset[1]))
										continue;

									x = (total % (piglit_width/3)) * 3 + 1;
									y = (total / (piglit_width/3)) * 3 + 1;
									p = pix + (y*piglit_width + x)*4;

									if (!check_result(p, expected_level, fetch_level,
											  baselevel, maxlevel, minlod,
											  maxlod, bias, mipfilter)) {
										failed++;
										if (failed > 100) {
											printf("Stopping after 100 failures\n");
											goto end;
										}
									}
									total++;
								}
		free(pix);
	}

end:
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	printf("Summary: %i/%i passed\n", total-failed, total);

	piglit_present_results();

	return !failed ? PIGLIT_PASS : PIGLIT_FAIL;
}
