/*
 * Copyright © 2014 Intel Corporation
 * Copyright © 2016 Advanced Micro Devices, Inc.
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

/**
 * \file rendering-formats.c
 * This tests that texturing from a view and rendering to a view works when
 * the view has a different internalformat to the original texture.
 */

#include "piglit-util-gl.h"
#include "common.h"

#define TEX_SIZE 512 /* I need to test large textures for radeonsi */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.window_width = TEX_SIZE,
	config.window_height = TEX_SIZE,
	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
#define GLSL_VERSION "130"
#else
#define GLSL_VERSION "310 es"
#endif

static const char *vs =
	"#version " GLSL_VERSION "\n"
	"in vec4 piglit_vertex;\n"
	"void main() { \n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs_render_float =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"#endif\n"
	"uniform vec4 v;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	color = v;\n"
	"}\n";

static const char *fs_render_uint =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp int;\n"
	"#endif\n"
	"uniform uvec4 v;\n"
	"out uvec4 color;\n"
	"void main() { \n"
	"	color = v;\n"
	"}\n";

static const char *fs_render_sint =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp int;\n"
	"#endif\n"
	"uniform ivec4 v;\n"
	"out ivec4 color;\n"
	"void main() { \n"
	"	color = v;\n"
	"}\n";

static const char *fs128_uint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0x3f800000u,\n"
	"		0x3e800000u,\n"
	"		0xbf800000u,\n"
	"		0x00000000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs128_sint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0x3f800000,\n"
	"		0x3e800000,\n"
	"		0xbf800000,\n"
	"		0x00000000)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs128_float32 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (floatBitsToUint(texture(s, vec2(0.0))) == uvec4(\n"
	"		0x3f800000u,\n"
	"		0x3e800000u,\n"
	"		0xbf800000u,\n"
	"		0x00000000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs96_uint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0x3f800000u,\n"
	"		0x3e800000u,\n"
	"		0xbf800000u,\n"
	"		0x00000001u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs96_sint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0x3f800000,\n"
	"		0x3e800000,\n"
	"		0xbf800000,\n"
	"		0x00000001)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs96_float32 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (floatBitsToUint(texture(s, vec2(0.0))) == uvec4(\n"
	"		0x3f800000u,\n"
	"		0x3e800000u,\n"
	"		0xbf800000u,\n"
	"		0x3f800000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_uint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0x3fe0a4b5u,\n"
	"		0x439ac3f7u,\n"
	"		0u,\n"
	"		1u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_sint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0x3fe0a4b5,\n"
	"		0x439ac3f7,\n"
	"		0,\n"
	"		1)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_float32 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (floatBitsToUint(texture(s, vec2(0.0))) == uvec4(\n"
	"		0x3fe0a4b5u,\n"
	"		0x439ac3f7u,\n"
	"		0x00000000u,\n"
	"		0x3f800000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_float16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shading_language_packing : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
	"	uvec2 h = uvec2(packHalf2x16(t.xy), packHalf2x16(t.zw));\n"
	"	if (h == uvec2(\n"
	"		0x3fe0a4b5u,\n"
	"		0x439ac3f7u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_uint16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0xc3f7u,\n"
	"		0x439au)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_sint16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0xffffc3f7,\n"
	"		0x439a)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_unorm16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 65535.0 + 0.5) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0xc3f7u,\n"
	"		0x439au)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs64_snorm16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 32767.0 + off) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0xffffc3f7,\n"
	"		0x439a)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs48_float16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shading_language_packing : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
	"	uvec2 h = uvec2(packHalf2x16(t.xy), packHalf2x16(t.zw));\n"
	"	if (h == uvec2(\n"
	"		0x3fe0a4b5u,\n"
	"		0x3c00c3f7u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs48_uint16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0xc3f7u,\n"
	"		0x0001u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs48_sint16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0xffffc3f7,\n"
	"		0x0001)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs48_unorm16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 65535.0 + 0.5) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0xc3f7u,\n"
	"		0xffffu)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs48_snorm16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 32767.0 + off) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0xffffc3f7,\n"
	"		0x7fff)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_uint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0x3fe0a4b5u,\n"
	"		0u,\n"
	"		0u,\n"
	"		1u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_sint32 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0x3fe0a4b5,\n"
	"		0,\n"
	"		0,\n"
	"		1)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_float32 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (floatBitsToUint(texture(s, vec2(0.0))) == uvec4(\n"
	"		0x3fe0a4b5u,\n"
	"		0x00000000u,\n"
	"		0x00000000u,\n"
	"		0x3f800000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_float16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shading_language_packing : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
	"	uvec2 h = uvec2(packHalf2x16(t.xy), packHalf2x16(t.zw));\n"
	"	if (h == uvec2(\n"
	"		0x3fe0a4b5u,\n"
	"		0x3c000000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_uint16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0x0000u,\n"
	"		0x0001u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_sint16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0x0000,\n"
	"		0x0001)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_unorm16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 65535.0 + 0.5) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x3fe0u,\n"
	"		0x0000u,\n"
	"		0xffffu)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_snorm16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 32767.0 + off) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x3fe0,\n"
	"		0x0000,\n"
	"		0x7fff)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_uint8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0xe0u,\n"
	"		0x3fu)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_sint8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0xffffffe0,\n"
	"		0x3f)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_unorm8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 255.0 + 0.5) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0xe0u,\n"
	"		0x3fu)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_snorm8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 127.0 + off) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0xffffffe0,\n"
	"		0x3f)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_uint10 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0x0b5u,\n"
	"		0x029u,\n"
	"		0x3feu,\n"
	"		0x000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs32_unorm10 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * vec4(1023.0, 1023.0, 1023.0, 3.0) + 0.5) == uvec4(\n"
	"		0x0b5u,\n"
	"		0x029u,\n"
	"		0x3feu,\n"
	"		0x000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs24_uint8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0xe0u,\n"
	"		0x01u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs24_sint8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0xffffffe0,\n"
	"		0x01)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs24_unorm8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 255.0 + 0.5) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0xe0u,\n"
	"		0xffu)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs24_snorm8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 127.0 + off) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0xffffffe0,\n"
	"		0x7f)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_float16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shading_language_packing : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
	"	uvec2 h = uvec2(packHalf2x16(t.xy), packHalf2x16(t.zw));\n"
	"	if (h == uvec2(\n"
	"		0x0000a4b5u,\n"
	"		0x3c000000u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_uint16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x0000u,\n"
	"		0x0000u,\n"
	"		0x0001u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_sint16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x0000,\n"
	"		0x0000,\n"
	"		0x0001)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_unorm16 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 65535.0 + 0.5) == uvec4(\n"
	"		0xa4b5u,\n"
	"		0x0000u,\n"
	"		0x0000u,\n"
	"		0xffffu)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_snorm16 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 32767.0 + off) == ivec4(\n"
	"		0xffffa4b5,\n" /* sign-extended */
	"		0x0000,\n"
	"		0x0000,\n"
	"		0x7fff)) {\n" /* 1.0 */
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_uint8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0x00u,\n"
	"		0x01u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_sint8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0x00,\n"
	"		0x01)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_unorm8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 255.0 + 0.5) == uvec4(\n"
	"		0xb5u,\n"
	"		0xa4u,\n"
	"		0x00u,\n"
	"		0xffu)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs16_snorm8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 127.0 + off) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0xffffffa4,\n"
	"		0x00,\n"
	"		0x7f)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs8_uint8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp usampler2D;\n"
	"#endif\n"
	"uniform usampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == uvec4(\n"
	"		0xb5u,\n"
	"		0x00u,\n"
	"		0x00u,\n"
	"		0x01u)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs8_sint8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp isampler2D;\n"
	"#endif\n"
	"uniform isampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	if (texture(s, vec2(0.0)) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0x00,\n"
	"		0x00,\n"
	"		0x01)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs8_unorm8 =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
		/* correct float -> unorm conversion */
	"	if (uvec4(texture(s, vec2(0.0)) * 255.0 + 0.5) == uvec4(\n"
	"		0xb5u,\n"
	"		0x00u,\n"
	"		0x00u,\n"
	"		0xffu)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

static const char *fs8_snorm8 =
	"#version " GLSL_VERSION "\n"
	"#extension GL_ARB_shader_bit_encoding : enable\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D s;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	vec4 t = texture(s, vec2(0.0));\n"
		/* correct float -> snorm conversion */
	"	vec4 off = vec4(t.x >= 0.0 ? 0.5 : -0.5,\n"
	"			t.y >= 0.0 ? 0.5 : -0.5,\n"
	"			t.z >= 0.0 ? 0.5 : -0.5,\n"
	"			t.w >= 0.0 ? 0.5 : -0.5);\n"
	"	if (ivec4(t * 127.0 + off) == ivec4(\n"
	"		0xffffffb5,\n" /* sign-extended */
	"		0x00,\n"
	"		0x00,\n"
	"		0x7f)) {\n"
	"		color = vec4(0,1,0,0);\n"
	"	} else {\n"
	"		color = vec4(1,0,0,0);\n"
	"	}\n"
	"}\n";

struct format_info {
	const char **fs;
	GLenum internalformat, format, type;
	uint32_t render_value[4];
};

struct view_class {
	struct format_info formats[18];
	unsigned bpp;
	uint32_t data[4];
};

static const struct view_class classes[] = {
	{
		{
			{&fs128_float32, GL_RGBA32F, GL_RGBA, GL_FLOAT,
			 {0x3f800000, 0x3e800000, 0xbf800000, 0x00000000}},
			{&fs128_uint32, GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT,
			 {0x3f800000, 0x3e800000, 0xbf800000, 0x00000000}},
			{&fs128_sint32, GL_RGBA32I, GL_RGBA_INTEGER, GL_INT,
			 {0x3f800000, 0x3e800000, 0xbf800000, 0x00000000}},
		},
		16,
		{0x3f800000, 0x3e800000, 0xbf800000, 0x00000000}, /* raw texture contents */
	},
	{
		{
			{&fs96_float32, GL_RGB32F, GL_RGB, GL_FLOAT,
			 {0x3f800000, 0x3e800000, 0xbf800000}},
			{&fs96_uint32, GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT,
			 {0x3f800000, 0x3e800000, 0xbf800000}},
			{&fs96_sint32, GL_RGB32I, GL_RGB_INTEGER, GL_INT,
			 {0x3f800000, 0x3e800000, 0xbf800000}},
		},
		12,
		{0x3f800000, 0x3e800000, 0xbf800000}, /* raw texture contents */
	},
	{
		{
			{&fs64_float32, GL_RG32F, GL_RG, GL_FLOAT,
			 {0x3fe0a4b5, 0x439ac3f7}},
			{&fs64_uint32, GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT,
			 {0x3fe0a4b5, 0x439ac3f7}},
			{&fs64_sint32, GL_RG32I, GL_RG_INTEGER, GL_INT,
			 {0x3fe0a4b5, 0x439ac3f7}},

			{&fs64_float16, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT,
			 {0xbc96a000, 0x3ffc0000, 0xc07ee000, 0x40734000}}, /* half converted to float */
			{&fs64_uint16, GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT,
			 {0xa4b5, 0x3fe0, 0xc3f7, 0x439a}},
			{&fs64_sint16, GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT,
			 {0xffffa4b5, 0x3fe0, 0xffffc3f7, 0x439a}}, /* sign-extended */
			{&fs64_unorm16, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT,
			 {0x3f24b5a5, 0x3e7f8100, 0x3f43f7c4, 0x3e873487}}, /* unorm converted to float */
			{&fs64_snorm16, GL_RGBA16_SNORM, GL_RGBA, GL_SHORT,
			 {0xbf36976d, 0x3eff81ff, 0xbef025e0, 0x3f07350e}}, /* snorm converted to float */
		},
		8,
		{0x3fe0a4b5, 0x439ac3f7}, /* raw texture contents */
	},
	{
		{
			{&fs48_float16, GL_RGB16F, GL_RGB, GL_HALF_FLOAT,
			 {0xbc96a000, 0x3ffc0000, 0xc07ee000}}, /* half converted to float */
			{&fs48_uint16, GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT,
			 {0xa4b5, 0x3fe0, 0xc3f7}},
			{&fs48_sint16, GL_RGB16I, GL_RGB_INTEGER, GL_SHORT,
			 {0xffffa4b5, 0x3fe0, 0xffffc3f7}}, /* sign-extended */
			{&fs48_unorm16, GL_RGB16, GL_RGB, GL_UNSIGNED_SHORT,
			 {0x3f24b5a5, 0x3e7f8100, 0x3f43f7c4}}, /* unorm converted to float */
			{&fs48_snorm16, GL_RGB16_SNORM, GL_RGB, GL_SHORT,
			 {0xbf36976d, 0x3eff81ff, 0xbef025e0}}, /* snorm converted to float */
		},
		6,
		{0x3fe0a4b5, 0xc3f7}, /* raw texture contents */
	},
	{
		{
			{&fs32_float32, GL_R32F, GL_RED, GL_FLOAT, {0x3fe0a4b5}},
			{&fs32_uint32, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, {0x3fe0a4b5}},
			{&fs32_sint32, GL_R32I, GL_RED_INTEGER, GL_INT, {0x3fe0a4b5}},

			{&fs32_float16, GL_RG16F, GL_RG, GL_HALF_FLOAT, {0xbc96a000, 0x3ffc0000}}, /* half converted to float */
			{&fs32_uint16, GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, {0xa4b5, 0x3fe0}},
			{&fs32_sint16, GL_RG16I, GL_RG_INTEGER, GL_SHORT, {0xffffa4b5, 0x3fe0}}, /* sign-extended */
			{&fs32_unorm16, GL_RG16, GL_RG, GL_UNSIGNED_SHORT, {0x3f24b5a5, 0x3e7f8100}}, /* unorm converted to float */
			{&fs32_snorm16, GL_RG16_SNORM, GL_RG, GL_SHORT, {0xbf36976d, 0x3eff81ff}}, /* snorm converted to float */

			{&fs32_uint8, GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
			 {0xb5, 0xa4, 0xe0, 0x3f}},
			{&fs32_sint8, GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE,
			 {0xffffffb5, 0xffffffa4, 0xffffffe0, 0x3f}}, /* sign-extended */
			{&fs32_unorm8, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
			 {0x3f35b5b6, 0x3f24a4a5, 0x3f60e0e1, 0x3e7cfcfd}}, /* unorm converted to float */
			{&fs32_snorm8, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE,
			 {0xbf172e5d, 0xbf3972e6, 0xbe810204, 0x3efdfbf8}}, /* snorm converted to float */
			/*{&fs32_srgb8, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE},*/

			{&fs32_uint10, GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV,
			 {0x0b5, 0x029, 0x3fe, 0x000}},
			{&fs32_unorm10, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV,
			 {0x3e352d4b, 0x3d24290a, 0x3f7fbff0, 0x0}}, /* unorm converted to float */

			/*{&fs32_float11, GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10_11_11_REV},
			{&fs32_float9, GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV},*/
		},
		4,
		{0x3fe0a4b5}, /* raw texture contents */
	},
	{
		{
			{&fs24_uint8, GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE,
			 {0xb5, 0xa4, 0xe0}},
			{&fs24_sint8, GL_RGB8I, GL_RGB_INTEGER, GL_BYTE,
			 {0xffffffb5, 0xffffffa4, 0xffffffe0}}, /* sign-extended */
			{&fs24_unorm8, GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE,
			 {0x3f35b5b6, 0x3f24a4a5, 0x3f60e0e1}}, /* unorm converted to float */
			{&fs24_snorm8, GL_RGB8_SNORM, GL_RGB, GL_BYTE,
			 {0xbf172e5d, 0xbf3972e6, 0xbe810204}}, /* snorm converted to float */
			/*{&fs24_srgb8, GL_SRGB8, GL_RGBA, GL_UNSIGNED_BYTE},*/
		},
		3,
		{0xe0a4b5}, /* raw texture contents */
	},
	{
		{
			{&fs16_float16, GL_R16F, GL_RED, GL_HALF_FLOAT, {0xbc96a000}}, /* half converted to float */
			{&fs16_uint16, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, {0xa4b5}},
			{&fs16_sint16, GL_R16I, GL_RED_INTEGER, GL_SHORT, {0xffffa4b5}}, /* sign-extended */
			{&fs16_unorm16, GL_R16, GL_RED, GL_UNSIGNED_SHORT, {0x3f24b5a5}}, /* unorm converted to float */
			{&fs16_snorm16, GL_R16_SNORM, GL_RED, GL_SHORT, {0xbf36976d}}, /* snorm converted to float */

			{&fs16_uint8, GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, {0xb5, 0xa4}},
			{&fs16_sint8, GL_RG8I, GL_RG_INTEGER, GL_BYTE, {0xffffffb5, 0xffffffa4}}, /* sign-extended */
			{&fs16_unorm8, GL_RG8, GL_RG, GL_UNSIGNED_BYTE, {0x3f35b5b6, 0x3f24a4a5}}, /* unorm converted to float */
			{&fs16_snorm8, GL_RG8_SNORM, GL_RG, GL_BYTE, {0xbf172e5d, 0xbf3972e6}}, /* snorm converted to float */
		},
		2,
		{0xa4b5}, /* raw texture contents */
	},
	{
		{
			{&fs8_uint8, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, {0xb5}},
			{&fs8_sint8, GL_R8I, GL_RED_INTEGER, GL_BYTE, {0xffffffb5}},
			{&fs8_unorm8, GL_R8, GL_RED, GL_UNSIGNED_BYTE, {0x3f35b5b6}}, /* unorm converted to float */
			{&fs8_snorm8, GL_R8_SNORM, GL_RED, GL_BYTE, {0xbf172e5d}}, /* snorm converted to float */
		},
		1,
		{0xb5}, /* raw texture contents */
	},
};

static const float green[] = {0.0f, 1.0f, 0.0f, 0.0f};
static GLuint prog_float, prog_uint, prog_sint;
static GLuint loc_float, loc_uint, loc_sint;

static GLuint
create_texture(const struct view_class *vclass,
	       const struct format_info *base_format,
	       bool clear_to_zero)
{
	GLuint tex;
	char *p, *data;
	unsigned size, i;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D, 1, base_format->internalformat,
		       TEX_SIZE, TEX_SIZE);

	size = vclass->bpp * TEX_SIZE * TEX_SIZE;

	p = data = malloc(size);

	/* always fill the whole texture - needed by radeonsi */
	if (clear_to_zero)
		memset(data, 0, size);
	else
		for (i = 0; i < size; i++)
			*p++ = ((char*)vclass->data)[i % vclass->bpp];

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEX_SIZE, TEX_SIZE,
			base_format->format, base_format->type, data);
	free(data);
	return tex;
}

static GLuint
create_view(const struct format_info *view_format, GLuint tex)
{
	GLuint view;

	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, tex, view_format->internalformat,
		      0, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, view);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return view;
}

static GLuint
create_program(const struct format_info *view_format)
{
	GLuint prog = piglit_build_simple_program(vs, *view_format->fs);

	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "s"), 0);
	return prog;
}

static void
test_by_sampling(const char *test_name,
		 const struct format_info *vformat,
		 enum piglit_result *all)
{
	GLuint prog;
	bool pass;
	enum piglit_result one_result;

	prog = create_program(vformat);

	/* Draw only one pixel. We don't need more. */
	piglit_draw_rect(-1, -1, 2.0/TEX_SIZE, 2.0/TEX_SIZE);
	pass = piglit_probe_pixel_rgba_silent(0, 0, green, NULL);

	one_result = pass ? PIGLIT_PASS : PIGLIT_FAIL;
	piglit_report_subtest_result(one_result, "%s", test_name);
	piglit_merge_result(all, one_result);

	glDeleteProgram(prog);
}

static bool
is_any_integer(const struct format_info *vformat)
{
	return vformat->format == GL_RGBA_INTEGER ||
		vformat->format == GL_RGB_INTEGER ||
		vformat->format == GL_RG_INTEGER ||
		vformat->format == GL_RED_INTEGER;
}

static bool
is_signed(const struct format_info *vformat)
{
	return vformat->type == GL_BYTE ||
		vformat->type == GL_SHORT ||
		vformat->type == GL_INT;
}

static unsigned
get_num_components(const struct format_info *vformat)
{
	switch (vformat->format) {
	case GL_RGBA:
	case GL_RGBA_INTEGER:
		return 4;
	case GL_RGB:
	case GL_RGB_INTEGER:
		return 3;
	case GL_RG:
	case GL_RG_INTEGER:
		return 2;
	case GL_RED:
	case GL_RED_INTEGER:
		return 1;
	default:
		assert(0);
		return 0;
	}
}

static GLuint
create_test_clear_program(const struct format_info *base,
			  const struct format_info *clear)
{
	char fs[4096];
	GLuint prog;
	const char *sampler = "";
	const char *conv = "";
	unsigned expected[4] = { 0 };
	unsigned i;
	unsigned base_num_components = get_num_components(base);
	bool test_nan = false;

	/* Find out what the clear value is when fetched using the base format. */
	for (i = 0; i < base_num_components; i++) {
		switch (clear->internalformat) {
		case GL_RGBA32F:
		case GL_RGB32F:
		case GL_RG32F:
		case GL_R32F:
			if (base->internalformat == GL_RGBA16UI ||
			    base->internalformat == GL_RGBA16I ||
			    base->internalformat == GL_RG16UI ||
			    base->internalformat == GL_RG16I) {
				expected[i++] = 0x0000;
				expected[i] = 0x3f80;
			} else if (base->internalformat == GL_RGBA16 ||
				   base->internalformat == GL_RG16) {
				expected[i++] = 0;
				expected[i] = 0x3e7e00fe;
			} else if (base->internalformat == GL_RGBA16_SNORM ||
				   base->internalformat == GL_RG16_SNORM) {
				expected[i++] = 0;
				expected[i] = 0x3efe01fc;
			} else if (base->internalformat == GL_RGBA16F ||
				   base->internalformat == GL_RG16F) {
				expected[i++] = 0;
				expected[i] = 0x3ff00000;
			} else if (base->internalformat == GL_RGBA8UI) {
				expected[i++] = 0x00;
				expected[i++] = 0x00;
				expected[i++] = 0x80;
				expected[i] = 0x3f;
			} else if (base->internalformat == GL_RGBA8I) {
				expected[i++] = 0x00;
				expected[i++] = 0x00;
				expected[i++] = 0xffffff80;
				expected[i] = 0x3f;
			} else if (base->internalformat == GL_RGBA8) {
				expected[i++] = 0;
				expected[i++] = 0;
				expected[i++] = 0x3f008081;
				expected[i] = 0x3e7cfcfd;
			} else if (base->internalformat == GL_RGBA8_SNORM) {
				expected[i++] = 0;
				expected[i++] = 0;
				expected[i++] = 0xbf800000;
				expected[i] = 0x3efdfbf8;
			} else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0;
				expected[i++] = 0;
				expected[i++] = 0x3f8;
				expected[i] = 0;
			} else if (base->internalformat == GL_RGB10_A2) {
				expected[i++] = 0;
				expected[i++] = 0;
				expected[i++] = 0x3f7e3f90;
				expected[i] = 0;
			} else {
				expected[i] = 0x3f800000;
			}
			break;
		case GL_RGBA32I:
		case GL_RGB32I:
		case GL_RG32I:
		case GL_R32I:
			if (base->internalformat == GL_RGBA16UI ||
			    base->internalformat == GL_RG16UI) {
				expected[i++] = 0xffff;
				expected[i] = 0x7fff;
			} else if (base->internalformat == GL_RGBA16I ||
				   base->internalformat == GL_RG16I) {
				expected[i++] = 0xffffffff;
				expected[i] = 0x7fff;
			} else if (base->internalformat == GL_RGBA16 ||
				   base->internalformat == GL_RG16) {
				expected[i++] = 0x3f800000;
				expected[i] = 0x3effff00;
			} else if (base->internalformat == GL_RGBA16_SNORM ||
				   base->internalformat == GL_RG16_SNORM) {
				expected[i++] = 0xb8000100;
				expected[i] = 0x3f800000;
			} else if (base->internalformat == GL_RGBA16F ||
				   base->internalformat == GL_RG16F) {
				test_nan = true;
				expected[i] = true;
			} else if (base->internalformat == GL_RGBA8UI) {
				expected[i++] = 0xff;
				expected[i++] = 0xff;
				expected[i++] = 0xff;
				expected[i] = 0x7f;
			} else if (base->internalformat == GL_RGBA8I) {
				expected[i++] = 0xffffffff;
				expected[i++] = 0xffffffff;
				expected[i++] = 0xffffffff;
				expected[i] = 0x7f;
			} else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0x3ff;
				expected[i++] = 0x3ff;
				expected[i++] = 0x3ff;
				expected[i] = 0x1;
			} else if (base->internalformat == GL_RGBA8) {
				expected[i++] = 0x3f800000;
				expected[i++] = 0x3f800000;
				expected[i++] = 0x3f800000;
				expected[i] = 0x3efefeff;
			} else if (base->internalformat == GL_RGBA8_SNORM) {
				expected[i++] = 0xbc010204;
				expected[i++] = 0xbc010204;
				expected[i++] = 0xbc010204;
				expected[i] = 0x3f800000;
			} else if (base->internalformat == GL_RGB10_A2) {
				expected[i++] = 0x3f800000;
				expected[i++] = 0x3f800000;
				expected[i++] = 0x3f800000;
				expected[i] = 0x3eaaaaab;
			} else {
				expected[i] = 0x7fffffff;
			}
			break;
		case GL_RGBA16F:
		case GL_RGB16F:
		case GL_RG16F:
		case GL_R16F:
			if (base->internalformat == GL_RG32F ||
			    base->internalformat == GL_RG32I ||
			    base->internalformat == GL_RG32UI ||
			    base->internalformat == GL_R32F ||
			    base->internalformat == GL_R32I ||
			    base->internalformat == GL_R32UI)
				expected[i] = 0x3c003c00;
			else if (base->internalformat == GL_RGBA16UI ||
				 base->internalformat == GL_RGBA16I ||
				 base->internalformat == GL_RGB16UI ||
				 base->internalformat == GL_RGB16I ||
				 base->internalformat == GL_RG16UI ||
				 base->internalformat == GL_RG16I ||
				 base->internalformat == GL_R16UI ||
				 base->internalformat == GL_R16I)
				expected[i] = 0x3c00;
			else if (base->internalformat == GL_RGBA16 ||
				 base->internalformat == GL_RGB16 ||
				 base->internalformat == GL_RG16 ||
				 base->internalformat == GL_R16)
				expected[i] = 0x3e7000f0;
			else if (base->internalformat == GL_RGBA16_SNORM ||
				 base->internalformat == GL_RGB16_SNORM ||
				 base->internalformat == GL_RG16_SNORM ||
				 base->internalformat == GL_R16_SNORM)
				expected[i] = 0x3ef001e0;
			else if (base->internalformat == GL_RGBA8UI ||
				 base->internalformat == GL_RGBA8I ||
				 base->internalformat == GL_RG8UI ||
				 base->internalformat == GL_RG8I) {
				expected[i++] = 0x00;
				expected[i] = 0x3c;
			} else if (base->internalformat == GL_RGBA8 ||
				   base->internalformat == GL_RG8) {
				expected[i++] = 0x00;
				expected[i] = 0x3e70f0f1;
			} else if (base->internalformat == GL_RGBA8_SNORM ||
				   base->internalformat == GL_RG8_SNORM) {
				expected[i++] = 0x00;
				expected[i] = 0x3ef1e3c8;
			} else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0;
				expected[i++] = 0xf;
				expected[i++] = 0x3c0;
				expected[i] = 0;
			} else if (base->internalformat == GL_RGB10_A2) {
				expected[i++] = 0;
				expected[i++] = 0x3c703c0f;
				expected[i++] = 0x3f703c0f;
				expected[i] = 0;
			} else
				expected[i] = 0x3f800000;
			break;
		case GL_RGBA32UI:
		case GL_RGB32UI:
		case GL_RG32UI:
		case GL_R32UI:
		case GL_RGBA16UI:
		case GL_RGB16UI:
		case GL_RG16UI:
		case GL_R16UI:
		case GL_RGBA8UI:
		case GL_RGB8UI:
		case GL_RG8UI:
		case GL_R8UI:
		case GL_RGB10_A2UI:
		case GL_RGBA16:
		case GL_RGB16:
		case GL_RG16:
		case GL_R16:
		case GL_RGBA8:
		case GL_RGB8:
		case GL_RG8:
		case GL_R8:
		case GL_RGB10_A2:
			if (base->internalformat == GL_RGBA32F ||
			    base->internalformat == GL_RGB32F ||
			    base->internalformat == GL_RGBA32UI ||
			    base->internalformat == GL_RGB32UI ||
			    base->internalformat == GL_RGBA32I ||
			    base->internalformat == GL_RGB32I ||
			    base->internalformat == GL_RG32F ||
			    base->internalformat == GL_RG32I ||
			    base->internalformat == GL_RG32UI ||
			    base->internalformat == GL_R32F ||
			    base->internalformat == GL_R32I ||
			    base->internalformat == GL_R32UI ||
			    base->internalformat == GL_RGBA16I ||
			    base->internalformat == GL_RGB16I ||
			    base->internalformat == GL_RG16I ||
			    base->internalformat == GL_R16I ||
			    base->internalformat == GL_RGBA8I ||
			    base->internalformat == GL_RGB8I ||
			    base->internalformat == GL_RG8I ||
			    base->internalformat == GL_R8I)
				expected[i] = 0xffffffff;
			else if (base->internalformat == GL_RGBA16 ||
				 base->internalformat == GL_RGB16 ||
				 base->internalformat == GL_RG16 ||
				 base->internalformat == GL_R16 ||
				 base->internalformat == GL_RGBA8 ||
				 base->internalformat == GL_RGB8 ||
				 base->internalformat == GL_RG8 ||
				 base->internalformat == GL_R8 ||
				 base->internalformat == GL_RGB10_A2)
				expected[i] = 0x3f800000;
			else if (base->internalformat == GL_RGBA16_SNORM ||
				 base->internalformat == GL_RGB16_SNORM ||
				 base->internalformat == GL_RG16_SNORM ||
				 base->internalformat == GL_R16_SNORM)
				expected[i] = 0xb8000100;
			else if (base->internalformat == GL_RGBA16F ||
				 base->internalformat == GL_RGB16F ||
				 base->internalformat == GL_RG16F ||
				 base->internalformat == GL_R16F) {
				test_nan = true;
				expected[i] = true;
			} else if (base->internalformat == GL_RGBA8UI ||
				   base->internalformat == GL_RGB8UI ||
				   base->internalformat == GL_RG8UI ||
				   base->internalformat == GL_R8UI)
				expected[i] = 0xff;
			else if (base->internalformat == GL_RGBA8_SNORM ||
				 base->internalformat == GL_RGB8_SNORM ||
				 base->internalformat == GL_RG8_SNORM ||
				 base->internalformat == GL_R8_SNORM)
				expected[i] = 0xbc010204;
			else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0x3ff;
				expected[i++] = 0x3ff;
				expected[i++] = 0x3ff;
				expected[i] = 0x3;
			} else
				expected[i] = 0xffff;
			break;
		case GL_RGBA16I:
		case GL_RGB16I:
		case GL_RG16I:
		case GL_R16I:
		case GL_RGBA16_SNORM:
		case GL_RGB16_SNORM:
		case GL_RG16_SNORM:
		case GL_R16_SNORM:
			if (base->internalformat == GL_RG32F ||
			    base->internalformat == GL_RG32I ||
			    base->internalformat == GL_RG32UI ||
			    base->internalformat == GL_R32F ||
			    base->internalformat == GL_R32I ||
			    base->internalformat == GL_R32UI)
				expected[i] = 0x7fff7fff;
			else if (base->internalformat == GL_RGBA16 ||
				 base->internalformat == GL_RGB16 ||
				 base->internalformat == GL_RG16 ||
				 base->internalformat == GL_R16)
				expected[i] = 0x3effff00;
			else if (base->internalformat == GL_RGBA16_SNORM ||
				 base->internalformat == GL_RGB16_SNORM ||
				 base->internalformat == GL_RG16_SNORM ||
				 base->internalformat == GL_R16_SNORM)
				expected[i] = 0x3f800000;
			else if (base->internalformat == GL_RGBA16F ||
				 base->internalformat == GL_RGB16F ||
				 base->internalformat == GL_RG16F ||
				 base->internalformat == GL_R16F) {
				test_nan = true;
				expected[i] = true;
			} else if (base->internalformat == GL_RGBA8UI ||
				   base->internalformat == GL_RG8UI) {
				expected[i++] = 0xff;
				expected[i] = 0x7f;
			} else if (base->internalformat == GL_RGBA8I ||
				   base->internalformat == GL_RG8I) {
				expected[i++] = 0xffffffff;
				expected[i] = 0x7f;
			} else if (base->internalformat == GL_RGBA8 ||
				   base->internalformat == GL_RG8) {
				expected[i++] = 0x3f800000;
				expected[i] = 0x3efefeff;
			} else if (base->internalformat == GL_RGBA8_SNORM ||
				   base->internalformat == GL_RG8_SNORM) {
				expected[i++] = 0xbc010204;
				expected[i] = 0x3f800000;
			} else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0x3ff;
				expected[i++] = 0x3df;
				expected[i++] = 0x3ff;
				expected[i] = 0x1;
			} else if (base->internalformat == GL_RGB10_A2) {
				expected[i++] = 0x3f800000;
				expected[i++] = 0x3f77fdff;
				expected[i++] = 0x3f800000;
				expected[i] = 0x3eaaaaab;
			} else
				expected[i] = 0x7fff;
			break;
		case GL_RGBA8I:
		case GL_RGB8I:
		case GL_RG8I:
		case GL_R8I:
		case GL_RGBA8_SNORM:
		case GL_RGB8_SNORM:
		case GL_RG8_SNORM:
		case GL_R8_SNORM:
			if (base->internalformat == GL_R32F ||
			    base->internalformat == GL_R32I ||
			    base->internalformat == GL_R32UI)
				expected[i] = 0x7f7f7f7f;
			else if (base->internalformat == GL_RG16UI ||
				 base->internalformat == GL_RG16I ||
				 base->internalformat == GL_R16UI ||
				 base->internalformat == GL_R16I)
				expected[i] = 0x7f7f;
			else if (base->internalformat == GL_RG16 ||
				 base->internalformat == GL_R16 ||
				 base->internalformat == GL_RGBA8 ||
				 base->internalformat == GL_RGB8 ||
				 base->internalformat == GL_RG8 ||
				 base->internalformat == GL_R8)
				expected[i] = 0x3efefeff;
			else if (base->internalformat == GL_RG16F ||
				 base->internalformat == GL_R16F) {
				test_nan = true;
				expected[i] = true;
			} else if (base->internalformat == GL_RG16_SNORM ||
				 base->internalformat == GL_R16_SNORM)
				expected[i] = 0x3f7efffe;
			else if (base->internalformat == GL_RGBA8UI ||
				 base->internalformat == GL_RGB8UI ||
				 base->internalformat == GL_RG8UI ||
				 base->internalformat == GL_R8UI ||
				 base->internalformat == GL_RGBA8I ||
				 base->internalformat == GL_RGB8I ||
				 base->internalformat == GL_RG8I ||
				 base->internalformat == GL_R8I)
				expected[i] = 0x7f;
			else if (base->internalformat == GL_RGB10_A2UI) {
				expected[i++] = 0x37f;
				expected[i++] = 0x3df;
				expected[i++] = 0x3f7;
				expected[i] = 0x1;
			} else if (base->internalformat == GL_RGB10_A2) {
				expected[i++] = 0x3f5ff7fe;
				expected[i++] = 0x3f77fdff;
				expected[i++] = 0x3f7dff80;
				expected[i] = 0x3eaaaaab;
			} else
				expected[i] = 0x3f800000;
			break;
		}
	}

	for (; i < 4; i++)
		expected[i] = test_nan ? false :
			      is_any_integer(base) ? (i == 3 ? 1 : 0) :
						     (i == 3 ? 0x3f800000 : 0);

	if (is_any_integer(base)) {
		if (is_signed(base)) {
			sampler = "isampler2D";
			conv = "uvec4";
		} else {
			sampler = "usampler2D";
		}
	} else {
		sampler = "sampler2D";
		if (test_nan)
			conv = "test_nan";
		else
			conv = "floatBitsToUint";
	}

	/* Use texelFetch, because normal texture instructions flush denormals
	 * to 0 on radeonsi.
	 */
	snprintf(fs, sizeof(fs),
		 "#version " GLSL_VERSION "\n"
		 "#extension GL_ARB_shader_bit_encoding : enable\n"
		 "#ifdef GL_ES\n"
		 "precision highp float;\n"
		 "precision highp int;\n"
		 "precision highp %s;\n"
		 "#endif\n"
		 "out vec4 color;\n"
		 "uniform %s s;\n"
		 "uniform uvec4 expected;\n"
		 "uvec4 test_nan(vec4 v) { return uvec4(isnan(v)); }\n"
		 "void main() { \n"
		 "	if (%s(texelFetch(s, ivec2(0), 0)) == expected) {\n"
		 "		color = vec4(0,1,0,0);\n"
		 "	} else {\n"
		 "		color = vec4(1,0,0,0);\n"
		 "	}\n"
		 "}\n",
		 sampler, sampler, conv);

	prog = piglit_build_simple_program(vs, fs);
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "s"), 0);
	glUniform4uiv(glGetUniformLocation(prog, "expected"), 1, expected);
	return prog;
}

static void
test_clear_by_sampling(const char *test_name,
		       const struct format_info *base,
		       const struct format_info *vformat,
		       enum piglit_result *all)
{
	GLuint prog;
	bool pass;
	enum piglit_result one_result;

	prog = create_test_clear_program(base, vformat);

	/* Draw only one pixel. We don't need more. */
	piglit_draw_rect(-1, -1, 2.0/TEX_SIZE, 2.0/TEX_SIZE);
	pass = piglit_probe_pixel_rgba_silent(0, 0, green, NULL);

	one_result = pass ? PIGLIT_PASS : PIGLIT_FAIL;
	piglit_report_subtest_result(one_result, "%s", test_name);
	piglit_merge_result(all, one_result);

	glDeleteProgram(prog);
}

static bool
render_to_view(const struct format_info *vformat, GLuint tex)
{
	GLuint fbo, status, view;

	view = create_view(vformat, tex);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, view, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &view);
		glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
		return false;
	}

	if (is_any_integer(vformat)) {
		if (is_signed(vformat)) {
			glUseProgram(prog_sint);
			glUniform4iv(loc_sint, 1, (int*)vformat->render_value);
		} else {
			glUseProgram(prog_uint);
			glUniform4uiv(loc_uint, 1, vformat->render_value);
		}
	} else {
		glUseProgram(prog_float);
		glUniform4fv(loc_float, 1, (float*)vformat->render_value);
	}

	/* Fill the whole texture - needed by radeonsi. */
	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &view);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	return true;
}

static bool
clear_view(const struct format_info *vformat, GLuint tex)
{
	/* Use these clear values - needed to test radeonsi codepaths. */
	static unsigned ucolor[] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
	static int icolor[] = {0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};
	static float fcolor[] = {1, 1, 1, 1};
	GLuint fbo, status, view;

	view = create_view(vformat, tex);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D, view, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		glDeleteFramebuffers(1, &fbo);
		glDeleteTextures(1, &view);
		glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
		return false;
	}

	if (is_any_integer(vformat)) {
		if (is_signed(vformat)) {
			glClearBufferiv(GL_COLOR, 0, icolor);
		} else {
			glClearBufferuiv(GL_COLOR, 0, ucolor);
		}
	} else {
		glClearBufferfv(GL_COLOR, 0, fcolor);
	}

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &view);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	return true;
}

static bool norm16;

static bool
format_supported(const struct format_info *vformat)
{
	if (norm16)
		return true;
	if ((vformat->format == GL_RED ||
	     vformat->format == GL_RG ||
	     vformat->format == GL_RGB ||
	     vformat->format == GL_RGBA) &&
	    (vformat->type == GL_SHORT ||
	     vformat->type == GL_UNSIGNED_SHORT))
		return false;

	return true;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	int classi;

	glClear(GL_COLOR_BUFFER_BIT);

	/* Reinterpret sampler formats. */
	for (classi = 0; classi < ARRAY_SIZE(classes); classi++) {
		const struct view_class *vclass = &classes[classi];
		int i;

		for (i = 0; vclass->formats[i].fs; i++) {
			const struct format_info *base = &vclass->formats[i];
			GLuint tex;
			int j;

			if (!format_supported(base))
				continue;

			tex = create_texture(vclass, base, false);

			for (j = 0; vclass->formats[j].fs; j++) {
				const struct format_info *vformat = &vclass->formats[j];
				GLuint view;
				char test_name[128];

				if (!format_supported(vformat))
					continue;

				view = create_view(vformat, tex);

				snprintf(test_name, sizeof(test_name),
					 "sample %s as %s",
				         piglit_get_gl_enum_name(base->internalformat),
				         piglit_get_gl_enum_name(vformat->internalformat));

				test_by_sampling(test_name, vformat, &result);
				glDeleteTextures(1, &view);

				piglit_check_gl_error(GL_NO_ERROR);
			}

			glDeleteTextures(1, &tex);
		}
	}

	/* Reinterpret color buffer formats. */
	for (classi = 0; classi < ARRAY_SIZE(classes); classi++) {
		const struct view_class *vclass = &classes[classi];
		int i;

		for (i = 0; vclass->formats[i].fs; i++) {
			const struct format_info *base = &vclass->formats[i];
			int j;

			if (!format_supported(base))
				continue;

			for (j = 0; vclass->formats[j].fs; j++) {
				const struct format_info *vformat = &vclass->formats[j];
				char test_name[128];
				GLuint tex;

				if (!format_supported(vformat))
					continue;

				tex = create_texture(vclass, base, true);
				glBindTexture(GL_TEXTURE_2D, 0);

				/* Test rendering. */
				snprintf(test_name, sizeof(test_name),
					 "render to %s as %s",
				         piglit_get_gl_enum_name(base->internalformat),
				         piglit_get_gl_enum_name(vformat->internalformat));

				if (!render_to_view(vformat, tex)) {
					piglit_report_subtest_result(PIGLIT_SKIP, "%s", test_name);
					piglit_merge_result(&result, PIGLIT_SKIP);
					glDeleteTextures(1, &tex);
					continue;
				}

				glBindTexture(GL_TEXTURE_2D, tex);
				test_by_sampling(test_name, base, &result);
				glBindTexture(GL_TEXTURE_2D, 0);

				/* Test clearing. */
				snprintf(test_name, sizeof(test_name),
					 "clear %s as %s",
				         piglit_get_gl_enum_name(base->internalformat),
				         piglit_get_gl_enum_name(vformat->internalformat));

				if (!clear_view(vformat, tex)) {
					piglit_report_subtest_result(PIGLIT_SKIP, "%s", test_name);
					piglit_merge_result(&result, PIGLIT_SKIP);
					glDeleteTextures(1, &tex);
					continue;
				}

				glBindTexture(GL_TEXTURE_2D, tex);
				test_clear_by_sampling(test_name, base, vformat, &result);
				glBindTexture(GL_TEXTURE_2D, 0);

				glDeleteTextures(1, &tex);

				piglit_check_gl_error(GL_NO_ERROR);
			}
		}
	}

	piglit_report_result(result);
	return PIGLIT_FAIL; /* unreachable */
}

void
piglit_init(int argc, char **argv)
{
	piglit_automatic = true;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(30);
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_shader_bit_encoding");
	piglit_require_extension("GL_ARB_shading_language_packing");
	piglit_require_extension("GL_ARB_texture_rgb10_a2ui");
	norm16 = true;
#else
	piglit_require_extension("GL_OES_texture_view");
	norm16 = piglit_is_extension_supported("GL_EXT_texture_norm16");
#endif

	glClearColor(0.2, 0.2, 0.2, 0.2);

#ifdef PIGLIT_USE_OPENGL
	/* Don't clamp SNORM rendering to [0,1]. */
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
#endif

	prog_float = piglit_build_simple_program(vs, fs_render_float);
	loc_float = glGetUniformLocation(prog_float, "v");
	assert(loc_float != -1);

	prog_uint = piglit_build_simple_program(vs, fs_render_uint);
	loc_uint = glGetUniformLocation(prog_uint, "v");
	assert(loc_uint != -1);

	prog_sint = piglit_build_simple_program(vs, fs_render_sint);
	loc_sint = glGetUniformLocation(prog_sint, "v");
	assert(loc_sint != -1);

	glUseProgram(prog_float);
}
