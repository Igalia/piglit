/*
 * Copyright 2012 Intel Corporation
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

/* TODO: Automatically generate this file. */

#include "piglit-util-gl-common.h"

const char *
piglit_get_gl_enum_name(GLenum param)
{
#define CASE(x) case x: return #x;

	switch (param) {

	/* <GLES3/gl3.h> */

	/* BlendingFactorDest */
	CASE(GL_SRC_COLOR)                                      // 0x0300
	CASE(GL_ONE_MINUS_SRC_COLOR)                            // 0x0301
	CASE(GL_SRC_ALPHA)                                      // 0x0302
	CASE(GL_ONE_MINUS_SRC_ALPHA)                            // 0x0303
	CASE(GL_DST_ALPHA)                                      // 0x0304
	CASE(GL_ONE_MINUS_DST_ALPHA)                            // 0x0305

	/* BlendingFactorSrc */
	CASE(GL_DST_COLOR)                                      // 0x0306
	CASE(GL_ONE_MINUS_DST_COLOR)                            // 0x0307
	CASE(GL_SRC_ALPHA_SATURATE)                             // 0x0308

	/* BlendEquationSeparate */
	CASE(GL_FUNC_ADD)                                       // 0x8006
	CASE(GL_BLEND_EQUATION)                                 // 0x8009
	CASE(GL_BLEND_EQUATION_ALPHA)                           // 0x883D

	/* BlendSubtract */
	CASE(GL_FUNC_SUBTRACT)                                  // 0x800A
	CASE(GL_FUNC_REVERSE_SUBTRACT)                          // 0x800B

	/* Separate Blend Functions */
	CASE(GL_BLEND_DST_RGB)                                  // 0x80C8
	CASE(GL_BLEND_SRC_RGB)                                  // 0x80C9
	CASE(GL_BLEND_DST_ALPHA)                                // 0x80CA
	CASE(GL_BLEND_SRC_ALPHA)                                // 0x80CB
	CASE(GL_CONSTANT_COLOR)                                 // 0x8001
	CASE(GL_ONE_MINUS_CONSTANT_COLOR)                       // 0x8002
	CASE(GL_CONSTANT_ALPHA)                                 // 0x8003
	CASE(GL_ONE_MINUS_CONSTANT_ALPHA)                       // 0x8004
	CASE(GL_BLEND_COLOR)                                    // 0x8005

	/* Buffer Objects */
	CASE(GL_ARRAY_BUFFER)                                   // 0x8892
	CASE(GL_ELEMENT_ARRAY_BUFFER)                           // 0x8893
	CASE(GL_ARRAY_BUFFER_BINDING)                           // 0x8894
	CASE(GL_ELEMENT_ARRAY_BUFFER_BINDING)                   // 0x8895

	CASE(GL_STREAM_DRAW)                                    // 0x88E0
	CASE(GL_STATIC_DRAW)                                    // 0x88E4
	CASE(GL_DYNAMIC_DRAW)                                   // 0x88E8

	CASE(GL_BUFFER_SIZE)                                    // 0x8764
	CASE(GL_BUFFER_USAGE)                                   // 0x8765

	CASE(GL_CURRENT_VERTEX_ATTRIB)                          // 0x8626

	/* CullFaceMode */
	CASE(GL_FRONT)                                          // 0x0404
	CASE(GL_BACK)                                           // 0x0405
	CASE(GL_FRONT_AND_BACK)                                 // 0x0408

	/* EnableCap */
	CASE(GL_TEXTURE_2D)                                     // 0x0DE1
	CASE(GL_CULL_FACE)                                      // 0x0B44
	CASE(GL_BLEND)                                          // 0x0BE2
	CASE(GL_DITHER)                                         // 0x0BD0
	CASE(GL_STENCIL_TEST)                                   // 0x0B90
	CASE(GL_DEPTH_TEST)                                     // 0x0B71
	CASE(GL_SCISSOR_TEST)                                   // 0x0C11
	CASE(GL_POLYGON_OFFSET_FILL)                            // 0x8037
	CASE(GL_SAMPLE_ALPHA_TO_COVERAGE)                       // 0x809E
	CASE(GL_SAMPLE_COVERAGE)                                // 0x80A0

	/* ErrorCode */
	CASE(GL_INVALID_ENUM)                                   // 0x0500
	CASE(GL_INVALID_VALUE)                                  // 0x0501
	CASE(GL_INVALID_OPERATION)                              // 0x0502
	CASE(GL_OUT_OF_MEMORY)                                  // 0x0505

	/* FrontFaceDirection */
	CASE(GL_CW)                                             // 0x0900
	CASE(GL_CCW)                                            // 0x0901

	/* GetPName */
	CASE(GL_LINE_WIDTH)                                     // 0x0B21
	CASE(GL_ALIASED_POINT_SIZE_RANGE)                       // 0x846D
	CASE(GL_ALIASED_LINE_WIDTH_RANGE)                       // 0x846E
	CASE(GL_CULL_FACE_MODE)                                 // 0x0B45
	CASE(GL_FRONT_FACE)                                     // 0x0B46
	CASE(GL_DEPTH_RANGE)                                    // 0x0B70
	CASE(GL_DEPTH_WRITEMASK)                                // 0x0B72
	CASE(GL_DEPTH_CLEAR_VALUE)                              // 0x0B73
	CASE(GL_DEPTH_FUNC)                                     // 0x0B74
	CASE(GL_STENCIL_CLEAR_VALUE)                            // 0x0B91
	CASE(GL_STENCIL_FUNC)                                   // 0x0B92
	CASE(GL_STENCIL_FAIL)                                   // 0x0B94
	CASE(GL_STENCIL_PASS_DEPTH_FAIL)                        // 0x0B95
	CASE(GL_STENCIL_PASS_DEPTH_PASS)                        // 0x0B96
	CASE(GL_STENCIL_REF)                                    // 0x0B97
	CASE(GL_STENCIL_VALUE_MASK)                             // 0x0B93
	CASE(GL_STENCIL_WRITEMASK)                              // 0x0B98
	CASE(GL_STENCIL_BACK_FUNC)                              // 0x8800
	CASE(GL_STENCIL_BACK_FAIL)                              // 0x8801
	CASE(GL_STENCIL_BACK_PASS_DEPTH_FAIL)                   // 0x8802
	CASE(GL_STENCIL_BACK_PASS_DEPTH_PASS)                   // 0x8803
	CASE(GL_STENCIL_BACK_REF)                               // 0x8CA3
	CASE(GL_STENCIL_BACK_VALUE_MASK)                        // 0x8CA4
	CASE(GL_STENCIL_BACK_WRITEMASK)                         // 0x8CA5
	CASE(GL_VIEWPORT)                                       // 0x0BA2
	CASE(GL_SCISSOR_BOX)                                    // 0x0C10
	CASE(GL_COLOR_CLEAR_VALUE)                              // 0x0C22
	CASE(GL_COLOR_WRITEMASK)                                // 0x0C23
	CASE(GL_UNPACK_ALIGNMENT)                               // 0x0CF5
	CASE(GL_PACK_ALIGNMENT)                                 // 0x0D05
	CASE(GL_MAX_TEXTURE_SIZE)                               // 0x0D33
	CASE(GL_MAX_VIEWPORT_DIMS)                              // 0x0D3A
	CASE(GL_SUBPIXEL_BITS)                                  // 0x0D50
	CASE(GL_RED_BITS)                                       // 0x0D52
	CASE(GL_GREEN_BITS)                                     // 0x0D53
	CASE(GL_BLUE_BITS)                                      // 0x0D54
	CASE(GL_ALPHA_BITS)                                     // 0x0D55
	CASE(GL_DEPTH_BITS)                                     // 0x0D56
	CASE(GL_STENCIL_BITS)                                   // 0x0D57
	CASE(GL_POLYGON_OFFSET_UNITS)                           // 0x2A00
	CASE(GL_POLYGON_OFFSET_FACTOR)                          // 0x8038
	CASE(GL_TEXTURE_BINDING_2D)                             // 0x8069
	CASE(GL_SAMPLE_BUFFERS)                                 // 0x80A8
	CASE(GL_SAMPLES)                                        // 0x80A9
	CASE(GL_SAMPLE_COVERAGE_VALUE)                          // 0x80AA
	CASE(GL_SAMPLE_COVERAGE_INVERT)                         // 0x80AB

	/* GetTextureParameter */
	CASE(GL_NUM_COMPRESSED_TEXTURE_FORMATS)                 // 0x86A2
	CASE(GL_COMPRESSED_TEXTURE_FORMATS)                     // 0x86A3

	/* HintMode */
	CASE(GL_DONT_CARE)                                      // 0x1100
	CASE(GL_FASTEST)                                        // 0x1101
	CASE(GL_NICEST)                                         // 0x1102

	/* HintTarget */
	CASE(GL_GENERATE_MIPMAP_HINT)                           // 0x8192

	/* DataType */
	CASE(GL_BYTE)                                           // 0x1400
	CASE(GL_UNSIGNED_BYTE)                                  // 0x1401
	CASE(GL_SHORT)                                          // 0x1402
	CASE(GL_UNSIGNED_SHORT)                                 // 0x1403
	CASE(GL_INT)                                            // 0x1404
	CASE(GL_UNSIGNED_INT)                                   // 0x1405
	CASE(GL_FLOAT)                                          // 0x1406
	CASE(GL_FIXED)                                          // 0x140C

	/* PixelFormat */
	CASE(GL_DEPTH_COMPONENT)                                // 0x1902
	CASE(GL_ALPHA)                                          // 0x1906
	CASE(GL_RGB)                                            // 0x1907
	CASE(GL_RGBA)                                           // 0x1908
	CASE(GL_LUMINANCE)                                      // 0x1909
	CASE(GL_LUMINANCE_ALPHA)                                // 0x190A

	/* PixelType */
	CASE(GL_UNSIGNED_SHORT_4_4_4_4)                         // 0x8033
	CASE(GL_UNSIGNED_SHORT_5_5_5_1)                         // 0x8034
	CASE(GL_UNSIGNED_SHORT_5_6_5)                           // 0x8363

	/* Shaders */
	CASE(GL_FRAGMENT_SHADER)                                // 0x8B30
	CASE(GL_VERTEX_SHADER)                                  // 0x8B31
	CASE(GL_MAX_VERTEX_ATTRIBS)                             // 0x8869
	CASE(GL_MAX_VERTEX_UNIFORM_VECTORS)                     // 0x8DFB
	CASE(GL_MAX_VARYING_VECTORS)                            // 0x8DFC
	CASE(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)               // 0x8B4D
	CASE(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)                 // 0x8B4C
	CASE(GL_MAX_TEXTURE_IMAGE_UNITS)                        // 0x8872
	CASE(GL_MAX_FRAGMENT_UNIFORM_VECTORS)                   // 0x8DFD
	CASE(GL_SHADER_TYPE)                                    // 0x8B4F
	CASE(GL_DELETE_STATUS)                                  // 0x8B80
	CASE(GL_LINK_STATUS)                                    // 0x8B82
	CASE(GL_VALIDATE_STATUS)                                // 0x8B83
	CASE(GL_ATTACHED_SHADERS)                               // 0x8B85
	CASE(GL_ACTIVE_UNIFORMS)                                // 0x8B86
	CASE(GL_ACTIVE_UNIFORM_MAX_LENGTH)                      // 0x8B87
	CASE(GL_ACTIVE_ATTRIBUTES)                              // 0x8B89
	CASE(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)                    // 0x8B8A
	CASE(GL_SHADING_LANGUAGE_VERSION)                       // 0x8B8C
	CASE(GL_CURRENT_PROGRAM)                                // 0x8B8D

	/* StencilFunction */
	CASE(GL_NEVER)                                          // 0x0200
	CASE(GL_LESS)                                           // 0x0201
	CASE(GL_EQUAL)                                          // 0x0202
	CASE(GL_LEQUAL)                                         // 0x0203
	CASE(GL_GREATER)                                        // 0x0204
	CASE(GL_NOTEQUAL)                                       // 0x0205
	CASE(GL_GEQUAL)                                         // 0x0206
	CASE(GL_ALWAYS)                                         // 0x0207

	/* StencilOp */
	CASE(GL_KEEP)                                           // 0x1E00
	CASE(GL_REPLACE)                                        // 0x1E01
	CASE(GL_INCR)                                           // 0x1E02
	CASE(GL_DECR)                                           // 0x1E03
	CASE(GL_INVERT)                                         // 0x150A
	CASE(GL_INCR_WRAP)                                      // 0x8507
	CASE(GL_DECR_WRAP)                                      // 0x8508

	/* StringName */
	CASE(GL_VENDOR)                                         // 0x1F00
	CASE(GL_RENDERER)                                       // 0x1F01
	CASE(GL_VERSION)                                        // 0x1F02
	CASE(GL_EXTENSIONS)                                     // 0x1F03

	/* TextureMagFilter */
	CASE(GL_NEAREST)                                        // 0x2600
	CASE(GL_LINEAR)                                         // 0x2601

	/* TextureMinFilter */
	CASE(GL_NEAREST_MIPMAP_NEAREST)                         // 0x2700
	CASE(GL_LINEAR_MIPMAP_NEAREST)                          // 0x2701
	CASE(GL_NEAREST_MIPMAP_LINEAR)                          // 0x2702
	CASE(GL_LINEAR_MIPMAP_LINEAR)                           // 0x2703

	/* TextureParameterName */
	CASE(GL_TEXTURE_MAG_FILTER)                             // 0x2800
	CASE(GL_TEXTURE_MIN_FILTER)                             // 0x2801
	CASE(GL_TEXTURE_WRAP_S)                                 // 0x2802
	CASE(GL_TEXTURE_WRAP_T)                                 // 0x2803

	/* TextureTarget */
	CASE(GL_TEXTURE)                                        // 0x1702

	CASE(GL_TEXTURE_CUBE_MAP)                               // 0x8513
	CASE(GL_TEXTURE_BINDING_CUBE_MAP)                       // 0x8514
	CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_X)                    // 0x8515
	CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_X)                    // 0x8516
	CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Y)                    // 0x8517
	CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y)                    // 0x8518
	CASE(GL_TEXTURE_CUBE_MAP_POSITIVE_Z)                    // 0x8519
	CASE(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z)                    // 0x851A
	CASE(GL_MAX_CUBE_MAP_TEXTURE_SIZE)                      // 0x851C

	/* TextureUnit */
	CASE(GL_TEXTURE0)                                       // 0x84C0
	CASE(GL_TEXTURE1)                                       // 0x84C1
	CASE(GL_TEXTURE2)                                       // 0x84C2
	CASE(GL_TEXTURE3)                                       // 0x84C3
	CASE(GL_TEXTURE4)                                       // 0x84C4
	CASE(GL_TEXTURE5)                                       // 0x84C5
	CASE(GL_TEXTURE6)                                       // 0x84C6
	CASE(GL_TEXTURE7)                                       // 0x84C7
	CASE(GL_TEXTURE8)                                       // 0x84C8
	CASE(GL_TEXTURE9)                                       // 0x84C9
	CASE(GL_TEXTURE10)                                      // 0x84CA
	CASE(GL_TEXTURE11)                                      // 0x84CB
	CASE(GL_TEXTURE12)                                      // 0x84CC
	CASE(GL_TEXTURE13)                                      // 0x84CD
	CASE(GL_TEXTURE14)                                      // 0x84CE
	CASE(GL_TEXTURE15)                                      // 0x84CF
	CASE(GL_TEXTURE16)                                      // 0x84D0
	CASE(GL_TEXTURE17)                                      // 0x84D1
	CASE(GL_TEXTURE18)                                      // 0x84D2
	CASE(GL_TEXTURE19)                                      // 0x84D3
	CASE(GL_TEXTURE20)                                      // 0x84D4
	CASE(GL_TEXTURE21)                                      // 0x84D5
	CASE(GL_TEXTURE22)                                      // 0x84D6
	CASE(GL_TEXTURE23)                                      // 0x84D7
	CASE(GL_TEXTURE24)                                      // 0x84D8
	CASE(GL_TEXTURE25)                                      // 0x84D9
	CASE(GL_TEXTURE26)                                      // 0x84DA
	CASE(GL_TEXTURE27)                                      // 0x84DB
	CASE(GL_TEXTURE28)                                      // 0x84DC
	CASE(GL_TEXTURE29)                                      // 0x84DD
	CASE(GL_TEXTURE30)                                      // 0x84DE
	CASE(GL_TEXTURE31)                                      // 0x84DF
	CASE(GL_ACTIVE_TEXTURE)                                 // 0x84E0

	/* TextureWrapMode */
	CASE(GL_REPEAT)                                         // 0x2901
	CASE(GL_CLAMP_TO_EDGE)                                  // 0x812F
	CASE(GL_MIRRORED_REPEAT)                                // 0x8370

	/* Uniform Types */
	CASE(GL_FLOAT_VEC2)                                     // 0x8B50
	CASE(GL_FLOAT_VEC3)                                     // 0x8B51
	CASE(GL_FLOAT_VEC4)                                     // 0x8B52
	CASE(GL_INT_VEC2)                                       // 0x8B53
	CASE(GL_INT_VEC3)                                       // 0x8B54
	CASE(GL_INT_VEC4)                                       // 0x8B55
	CASE(GL_BOOL)                                           // 0x8B56
	CASE(GL_BOOL_VEC2)                                      // 0x8B57
	CASE(GL_BOOL_VEC3)                                      // 0x8B58
	CASE(GL_BOOL_VEC4)                                      // 0x8B59
	CASE(GL_FLOAT_MAT2)                                     // 0x8B5A
	CASE(GL_FLOAT_MAT3)                                     // 0x8B5B
	CASE(GL_FLOAT_MAT4)                                     // 0x8B5C
	CASE(GL_SAMPLER_2D)                                     // 0x8B5E
	CASE(GL_SAMPLER_CUBE)                                   // 0x8B60

	/* Vertex Arrays */
	CASE(GL_VERTEX_ATTRIB_ARRAY_ENABLED)                    // 0x8622
	CASE(GL_VERTEX_ATTRIB_ARRAY_SIZE)                       // 0x8623
	CASE(GL_VERTEX_ATTRIB_ARRAY_STRIDE)                     // 0x8624
	CASE(GL_VERTEX_ATTRIB_ARRAY_TYPE)                       // 0x8625
	CASE(GL_VERTEX_ATTRIB_ARRAY_NORMALIZED)                 // 0x886A
	CASE(GL_VERTEX_ATTRIB_ARRAY_POINTER)                    // 0x8645
	CASE(GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING)             // 0x889F

	/* Read Format */
	CASE(GL_IMPLEMENTATION_COLOR_READ_TYPE)                 // 0x8B9A
	CASE(GL_IMPLEMENTATION_COLOR_READ_FORMAT)               // 0x8B9B

	/* Shader Source */
	CASE(GL_COMPILE_STATUS)                                 // 0x8B81
	CASE(GL_INFO_LOG_LENGTH)                                // 0x8B84
	CASE(GL_SHADER_SOURCE_LENGTH)                           // 0x8B88
	CASE(GL_SHADER_COMPILER)                                // 0x8DFA

	/* Shader Binary */
	CASE(GL_SHADER_BINARY_FORMATS)                          // 0x8DF8
	CASE(GL_NUM_SHADER_BINARY_FORMATS)                      // 0x8DF9

	/* Shader Precision-Specified Types */
	CASE(GL_LOW_FLOAT)                                      // 0x8DF0
	CASE(GL_MEDIUM_FLOAT)                                   // 0x8DF1
	CASE(GL_HIGH_FLOAT)                                     // 0x8DF2
	CASE(GL_LOW_INT)                                        // 0x8DF3
	CASE(GL_MEDIUM_INT)                                     // 0x8DF4
	CASE(GL_HIGH_INT)                                       // 0x8DF5

	/* Framebuffer Object. */
	CASE(GL_FRAMEBUFFER)                                    // 0x8D40
	CASE(GL_RENDERBUFFER)                                   // 0x8D41

	CASE(GL_RGBA4)                                          // 0x8056
	CASE(GL_RGB5_A1)                                        // 0x8057
	CASE(GL_RGB565)                                         // 0x8D62
	CASE(GL_DEPTH_COMPONENT16)                              // 0x81A5
	CASE(GL_STENCIL_INDEX8)                                 // 0x8D48

	CASE(GL_RENDERBUFFER_WIDTH)                             // 0x8D42
	CASE(GL_RENDERBUFFER_HEIGHT)                            // 0x8D43
	CASE(GL_RENDERBUFFER_INTERNAL_FORMAT)                   // 0x8D44
	CASE(GL_RENDERBUFFER_RED_SIZE)                          // 0x8D50
	CASE(GL_RENDERBUFFER_GREEN_SIZE)                        // 0x8D51
	CASE(GL_RENDERBUFFER_BLUE_SIZE)                         // 0x8D52
	CASE(GL_RENDERBUFFER_ALPHA_SIZE)                        // 0x8D53
	CASE(GL_RENDERBUFFER_DEPTH_SIZE)                        // 0x8D54
	CASE(GL_RENDERBUFFER_STENCIL_SIZE)                      // 0x8D55

	CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE)             // 0x8CD0
	CASE(GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME)             // 0x8CD1
	CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL)           // 0x8CD2
	CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE)   // 0x8CD3

	CASE(GL_COLOR_ATTACHMENT0)                              // 0x8CE0
	CASE(GL_DEPTH_ATTACHMENT)                               // 0x8D00
	CASE(GL_STENCIL_ATTACHMENT)                             // 0x8D20

	CASE(GL_FRAMEBUFFER_COMPLETE)                           // 0x8CD5
	CASE(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)              // 0x8CD6
	CASE(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)      // 0x8CD7
	CASE(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)              // 0x8CD9
	CASE(GL_FRAMEBUFFER_UNSUPPORTED)                        // 0x8CDD

	CASE(GL_FRAMEBUFFER_BINDING)                            // 0x8CA6
	CASE(GL_RENDERBUFFER_BINDING)                           // 0x8CA7
	CASE(GL_MAX_RENDERBUFFER_SIZE)                          // 0x84E8

	CASE(GL_INVALID_FRAMEBUFFER_OPERATION)                  // 0x0506

	CASE(GL_READ_BUFFER)                                    // 0x0C02
	CASE(GL_UNPACK_ROW_LENGTH)                              // 0x0CF2
	CASE(GL_UNPACK_SKIP_ROWS)                               // 0x0CF3
	CASE(GL_UNPACK_SKIP_PIXELS)                             // 0x0CF4
	CASE(GL_PACK_ROW_LENGTH)                                // 0x0D02
	CASE(GL_PACK_SKIP_ROWS)                                 // 0x0D03
	CASE(GL_PACK_SKIP_PIXELS)                               // 0x0D04
	CASE(GL_COLOR)                                          // 0x1800
	CASE(GL_DEPTH)                                          // 0x1801
	CASE(GL_STENCIL)                                        // 0x1802
	CASE(GL_RED)                                            // 0x1903
	CASE(GL_RGB8)                                           // 0x8051
	CASE(GL_RGBA8)                                          // 0x8058
	CASE(GL_RGB10_A2)                                       // 0x8059
	CASE(GL_TEXTURE_BINDING_3D)                             // 0x806A
	CASE(GL_UNPACK_SKIP_IMAGES)                             // 0x806D
	CASE(GL_UNPACK_IMAGE_HEIGHT)                            // 0x806E
	CASE(GL_TEXTURE_3D)                                     // 0x806F
	CASE(GL_TEXTURE_WRAP_R)                                 // 0x8072
	CASE(GL_MAX_3D_TEXTURE_SIZE)                            // 0x8073
	CASE(GL_UNSIGNED_INT_2_10_10_10_REV)                    // 0x8368
	CASE(GL_MAX_ELEMENTS_VERTICES)                          // 0x80E8
	CASE(GL_MAX_ELEMENTS_INDICES)                           // 0x80E9
	CASE(GL_TEXTURE_MIN_LOD)                                // 0x813A
	CASE(GL_TEXTURE_MAX_LOD)                                // 0x813B
	CASE(GL_TEXTURE_BASE_LEVEL)                             // 0x813C
	CASE(GL_TEXTURE_MAX_LEVEL)                              // 0x813D
	CASE(GL_MIN)                                            // 0x8007
	CASE(GL_MAX)                                            // 0x8008
	CASE(GL_DEPTH_COMPONENT24)                              // 0x81A6
	CASE(GL_MAX_TEXTURE_LOD_BIAS)                           // 0x84FD
	CASE(GL_TEXTURE_COMPARE_MODE)                           // 0x884C
	CASE(GL_TEXTURE_COMPARE_FUNC)                           // 0x884D
	CASE(GL_CURRENT_QUERY)                                  // 0x8865
	CASE(GL_QUERY_RESULT)                                   // 0x8866
	CASE(GL_QUERY_RESULT_AVAILABLE)                         // 0x8867
	CASE(GL_BUFFER_MAPPED)                                  // 0x88BC
	CASE(GL_BUFFER_MAP_POINTER)                             // 0x88BD
	CASE(GL_STREAM_READ)                                    // 0x88E1
	CASE(GL_STREAM_COPY)                                    // 0x88E2
	CASE(GL_STATIC_READ)                                    // 0x88E5
	CASE(GL_STATIC_COPY)                                    // 0x88E6
	CASE(GL_DYNAMIC_READ)                                   // 0x88E9
	CASE(GL_DYNAMIC_COPY)                                   // 0x88EA
	CASE(GL_MAX_DRAW_BUFFERS)                               // 0x8824
	CASE(GL_DRAW_BUFFER0)                                   // 0x8825
	CASE(GL_DRAW_BUFFER1)                                   // 0x8826
	CASE(GL_DRAW_BUFFER2)                                   // 0x8827
	CASE(GL_DRAW_BUFFER3)                                   // 0x8828
	CASE(GL_DRAW_BUFFER4)                                   // 0x8829
	CASE(GL_DRAW_BUFFER5)                                   // 0x882A
	CASE(GL_DRAW_BUFFER6)                                   // 0x882B
	CASE(GL_DRAW_BUFFER7)                                   // 0x882C
	CASE(GL_DRAW_BUFFER8)                                   // 0x882D
	CASE(GL_DRAW_BUFFER9)                                   // 0x882E
	CASE(GL_DRAW_BUFFER10)                                  // 0x882F
	CASE(GL_DRAW_BUFFER11)                                  // 0x8830
	CASE(GL_DRAW_BUFFER12)                                  // 0x8831
	CASE(GL_DRAW_BUFFER13)                                  // 0x8832
	CASE(GL_DRAW_BUFFER14)                                  // 0x8833
	CASE(GL_DRAW_BUFFER15)                                  // 0x8834
	CASE(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS)                // 0x8B49
	CASE(GL_MAX_VERTEX_UNIFORM_COMPONENTS)                  // 0x8B4A
	CASE(GL_SAMPLER_3D)                                     // 0x8B5F
	CASE(GL_SAMPLER_2D_SHADOW)                              // 0x8B62
	CASE(GL_FRAGMENT_SHADER_DERIVATIVE_HINT)                // 0x8B8B
	CASE(GL_PIXEL_PACK_BUFFER)                              // 0x88EB
	CASE(GL_PIXEL_UNPACK_BUFFER)                            // 0x88EC
	CASE(GL_PIXEL_PACK_BUFFER_BINDING)                      // 0x88ED
	CASE(GL_PIXEL_UNPACK_BUFFER_BINDING)                    // 0x88EF
	CASE(GL_FLOAT_MAT2x3)                                   // 0x8B65
	CASE(GL_FLOAT_MAT2x4)                                   // 0x8B66
	CASE(GL_FLOAT_MAT3x2)                                   // 0x8B67
	CASE(GL_FLOAT_MAT3x4)                                   // 0x8B68
	CASE(GL_FLOAT_MAT4x2)                                   // 0x8B69
	CASE(GL_FLOAT_MAT4x3)                                   // 0x8B6A
	CASE(GL_SRGB)                                           // 0x8C40
	CASE(GL_SRGB8)                                          // 0x8C41
	CASE(GL_SRGB8_ALPHA8)                                   // 0x8C43
	CASE(GL_COMPARE_REF_TO_TEXTURE)                         // 0x884E
	CASE(GL_MAJOR_VERSION)                                  // 0x821B
	CASE(GL_MINOR_VERSION)                                  // 0x821C
	CASE(GL_NUM_EXTENSIONS)                                 // 0x821D
	CASE(GL_RGBA32F)                                        // 0x8814
	CASE(GL_RGB32F)                                         // 0x8815
	CASE(GL_RGBA16F)                                        // 0x881A
	CASE(GL_RGB16F)                                         // 0x881B
	CASE(GL_VERTEX_ATTRIB_ARRAY_INTEGER)                    // 0x88FD
	CASE(GL_MAX_ARRAY_TEXTURE_LAYERS)                       // 0x88FF
	CASE(GL_MIN_PROGRAM_TEXEL_OFFSET)                       // 0x8904
	CASE(GL_MAX_PROGRAM_TEXEL_OFFSET)                       // 0x8905
	CASE(GL_MAX_VARYING_COMPONENTS)                         // 0x8B4B
	CASE(GL_TEXTURE_2D_ARRAY)                               // 0x8C1A
	CASE(GL_TEXTURE_BINDING_2D_ARRAY)                       // 0x8C1D
	CASE(GL_R11F_G11F_B10F)                                 // 0x8C3A
	CASE(GL_UNSIGNED_INT_10F_11F_11F_REV)                   // 0x8C3B
	CASE(GL_RGB9_E5)                                        // 0x8C3D
	CASE(GL_UNSIGNED_INT_5_9_9_9_REV)                       // 0x8C3E
	CASE(GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH)          // 0x8C76
	CASE(GL_TRANSFORM_FEEDBACK_BUFFER_MODE)                 // 0x8C7F
	CASE(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS)     // 0x8C80
	CASE(GL_TRANSFORM_FEEDBACK_VARYINGS)                    // 0x8C83
	CASE(GL_TRANSFORM_FEEDBACK_BUFFER_START)                // 0x8C84
	CASE(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE)                 // 0x8C85
	CASE(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN)          // 0x8C88
	CASE(GL_RASTERIZER_DISCARD)                             // 0x8C89
	CASE(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS)  // 0x8C8A
	CASE(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS)        // 0x8C8B
	CASE(GL_INTERLEAVED_ATTRIBS)                            // 0x8C8C
	CASE(GL_SEPARATE_ATTRIBS)                               // 0x8C8D
	CASE(GL_TRANSFORM_FEEDBACK_BUFFER)                      // 0x8C8E
	CASE(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING)              // 0x8C8F
	CASE(GL_RGBA32UI)                                       // 0x8D70
	CASE(GL_RGB32UI)                                        // 0x8D71
	CASE(GL_RGBA16UI)                                       // 0x8D76
	CASE(GL_RGB16UI)                                        // 0x8D77
	CASE(GL_RGBA8UI)                                        // 0x8D7C
	CASE(GL_RGB8UI)                                         // 0x8D7D
	CASE(GL_RGBA32I)                                        // 0x8D82
	CASE(GL_RGB32I)                                         // 0x8D83
	CASE(GL_RGBA16I)                                        // 0x8D88
	CASE(GL_RGB16I)                                         // 0x8D89
	CASE(GL_RGBA8I)                                         // 0x8D8E
	CASE(GL_RGB8I)                                          // 0x8D8F
	CASE(GL_RED_INTEGER)                                    // 0x8D94
	CASE(GL_RGB_INTEGER)                                    // 0x8D98
	CASE(GL_RGBA_INTEGER)                                   // 0x8D99
	CASE(GL_SAMPLER_2D_ARRAY)                               // 0x8DC1
	CASE(GL_SAMPLER_2D_ARRAY_SHADOW)                        // 0x8DC4
	CASE(GL_SAMPLER_CUBE_SHADOW)                            // 0x8DC5
	CASE(GL_UNSIGNED_INT_VEC2)                              // 0x8DC6
	CASE(GL_UNSIGNED_INT_VEC3)                              // 0x8DC7
	CASE(GL_UNSIGNED_INT_VEC4)                              // 0x8DC8
	CASE(GL_INT_SAMPLER_2D)                                 // 0x8DCA
	CASE(GL_INT_SAMPLER_3D)                                 // 0x8DCB
	CASE(GL_INT_SAMPLER_CUBE)                               // 0x8DCC
	CASE(GL_INT_SAMPLER_2D_ARRAY)                           // 0x8DCF
	CASE(GL_UNSIGNED_INT_SAMPLER_2D)                        // 0x8DD2
	CASE(GL_UNSIGNED_INT_SAMPLER_3D)                        // 0x8DD3
	CASE(GL_UNSIGNED_INT_SAMPLER_CUBE)                      // 0x8DD4
	CASE(GL_UNSIGNED_INT_SAMPLER_2D_ARRAY)                  // 0x8DD7
	CASE(GL_BUFFER_ACCESS_FLAGS)                            // 0x911F
	CASE(GL_BUFFER_MAP_LENGTH)                              // 0x9120
	CASE(GL_BUFFER_MAP_OFFSET)                              // 0x9121
	CASE(GL_DEPTH_COMPONENT32F)                             // 0x8CAC
	CASE(GL_DEPTH32F_STENCIL8)                              // 0x8CAD
	CASE(GL_FLOAT_32_UNSIGNED_INT_24_8_REV)                 // 0x8DAD
	CASE(GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING)          // 0x8210
	CASE(GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE)          // 0x8211
	CASE(GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE)                // 0x8212
	CASE(GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE)              // 0x8213
	CASE(GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE)               // 0x8214
	CASE(GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE)              // 0x8215
	CASE(GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE)              // 0x8216
	CASE(GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE)            // 0x8217
	CASE(GL_FRAMEBUFFER_DEFAULT)                            // 0x8218
	CASE(GL_FRAMEBUFFER_UNDEFINED)                          // 0x8219
	CASE(GL_DEPTH_STENCIL_ATTACHMENT)                       // 0x821A
	CASE(GL_DEPTH_STENCIL)                                  // 0x84F9
	CASE(GL_UNSIGNED_INT_24_8)                              // 0x84FA
	CASE(GL_DEPTH24_STENCIL8)                               // 0x88F0
	CASE(GL_UNSIGNED_NORMALIZED)                            // 0x8C17
	CASE(GL_READ_FRAMEBUFFER)                               // 0x8CA8
	CASE(GL_DRAW_FRAMEBUFFER)                               // 0x8CA9
	CASE(GL_READ_FRAMEBUFFER_BINDING)                       // 0x8CAA
	CASE(GL_RENDERBUFFER_SAMPLES)                           // 0x8CAB
	CASE(GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER)           // 0x8CD4
	CASE(GL_MAX_COLOR_ATTACHMENTS)                          // 0x8CDF
	CASE(GL_COLOR_ATTACHMENT1)                              // 0x8CE1
	CASE(GL_COLOR_ATTACHMENT2)                              // 0x8CE2
	CASE(GL_COLOR_ATTACHMENT3)                              // 0x8CE3
	CASE(GL_COLOR_ATTACHMENT4)                              // 0x8CE4
	CASE(GL_COLOR_ATTACHMENT5)                              // 0x8CE5
	CASE(GL_COLOR_ATTACHMENT6)                              // 0x8CE6
	CASE(GL_COLOR_ATTACHMENT7)                              // 0x8CE7
	CASE(GL_COLOR_ATTACHMENT8)                              // 0x8CE8
	CASE(GL_COLOR_ATTACHMENT9)                              // 0x8CE9
	CASE(GL_COLOR_ATTACHMENT10)                             // 0x8CEA
	CASE(GL_COLOR_ATTACHMENT11)                             // 0x8CEB
	CASE(GL_COLOR_ATTACHMENT12)                             // 0x8CEC
	CASE(GL_COLOR_ATTACHMENT13)                             // 0x8CED
	CASE(GL_COLOR_ATTACHMENT14)                             // 0x8CEE
	CASE(GL_COLOR_ATTACHMENT15)                             // 0x8CEF
	CASE(GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE)             // 0x8D56
	CASE(GL_MAX_SAMPLES)                                    // 0x8D57
	CASE(GL_HALF_FLOAT)                                     // 0x140B
	CASE(GL_MAP_READ_BIT)                                   // 0x0001
	CASE(GL_MAP_WRITE_BIT)                                  // 0x0002
	CASE(GL_MAP_INVALIDATE_RANGE_BIT)                       // 0x0004
	CASE(GL_MAP_INVALIDATE_BUFFER_BIT)                      // 0x0008
	CASE(GL_MAP_FLUSH_EXPLICIT_BIT)                         // 0x0010
	CASE(GL_MAP_UNSYNCHRONIZED_BIT)                         // 0x0020
	CASE(GL_RG)                                             // 0x8227
	CASE(GL_RG_INTEGER)                                     // 0x8228
	CASE(GL_R8)                                             // 0x8229
	CASE(GL_RG8)                                            // 0x822B
	CASE(GL_R16F)                                           // 0x822D
	CASE(GL_R32F)                                           // 0x822E
	CASE(GL_RG16F)                                          // 0x822F
	CASE(GL_RG32F)                                          // 0x8230
	CASE(GL_R8I)                                            // 0x8231
	CASE(GL_R8UI)                                           // 0x8232
	CASE(GL_R16I)                                           // 0x8233
	CASE(GL_R16UI)                                          // 0x8234
	CASE(GL_R32I)                                           // 0x8235
	CASE(GL_R32UI)                                          // 0x8236
	CASE(GL_RG8I)                                           // 0x8237
	CASE(GL_RG8UI)                                          // 0x8238
	CASE(GL_RG16I)                                          // 0x8239
	CASE(GL_RG16UI)                                         // 0x823A
	CASE(GL_RG32I)                                          // 0x823B
	CASE(GL_RG32UI)                                         // 0x823C
	CASE(GL_VERTEX_ARRAY_BINDING)                           // 0x85B5
	CASE(GL_R8_SNORM)                                       // 0x8F94
	CASE(GL_RG8_SNORM)                                      // 0x8F95
	CASE(GL_RGB8_SNORM)                                     // 0x8F96
	CASE(GL_RGBA8_SNORM)                                    // 0x8F97
	CASE(GL_SIGNED_NORMALIZED)                              // 0x8F9C
	CASE(GL_PRIMITIVE_RESTART_FIXED_INDEX)                  // 0x8D69
	CASE(GL_COPY_READ_BUFFER)                               // 0x8F36
	CASE(GL_COPY_WRITE_BUFFER)                              // 0x8F37
	CASE(GL_UNIFORM_BUFFER)                                 // 0x8A11
	CASE(GL_UNIFORM_BUFFER_BINDING)                         // 0x8A28
	CASE(GL_UNIFORM_BUFFER_START)                           // 0x8A29
	CASE(GL_UNIFORM_BUFFER_SIZE)                            // 0x8A2A
	CASE(GL_MAX_VERTEX_UNIFORM_BLOCKS)                      // 0x8A2B
	CASE(GL_MAX_FRAGMENT_UNIFORM_BLOCKS)                    // 0x8A2D
	CASE(GL_MAX_COMBINED_UNIFORM_BLOCKS)                    // 0x8A2E
	CASE(GL_MAX_UNIFORM_BUFFER_BINDINGS)                    // 0x8A2F
	CASE(GL_MAX_UNIFORM_BLOCK_SIZE)                         // 0x8A30
	CASE(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS)         // 0x8A31
	CASE(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS)       // 0x8A33
	CASE(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT)                // 0x8A34
	CASE(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)           // 0x8A35
	CASE(GL_ACTIVE_UNIFORM_BLOCKS)                          // 0x8A36
	CASE(GL_UNIFORM_TYPE)                                   // 0x8A37
	CASE(GL_UNIFORM_SIZE)                                   // 0x8A38
	CASE(GL_UNIFORM_NAME_LENGTH)                            // 0x8A39
	CASE(GL_UNIFORM_BLOCK_INDEX)                            // 0x8A3A
	CASE(GL_UNIFORM_OFFSET)                                 // 0x8A3B
	CASE(GL_UNIFORM_ARRAY_STRIDE)                           // 0x8A3C
	CASE(GL_UNIFORM_MATRIX_STRIDE)                          // 0x8A3D
	CASE(GL_UNIFORM_IS_ROW_MAJOR)                           // 0x8A3E
	CASE(GL_UNIFORM_BLOCK_BINDING)                          // 0x8A3F
	CASE(GL_UNIFORM_BLOCK_DATA_SIZE)                        // 0x8A40
	CASE(GL_UNIFORM_BLOCK_NAME_LENGTH)                      // 0x8A41
	CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS)                  // 0x8A42
	CASE(GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES)           // 0x8A43
	CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER)      // 0x8A44
	CASE(GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER)    // 0x8A46
	CASE(GL_MAX_VERTEX_OUTPUT_COMPONENTS)                   // 0x9122
	CASE(GL_MAX_FRAGMENT_INPUT_COMPONENTS)                  // 0x9125
	CASE(GL_MAX_SERVER_WAIT_TIMEOUT)                        // 0x9111
	CASE(GL_OBJECT_TYPE)                                    // 0x9112
	CASE(GL_SYNC_CONDITION)                                 // 0x9113
	CASE(GL_SYNC_STATUS)                                    // 0x9114
	CASE(GL_SYNC_FLAGS)                                     // 0x9115
	CASE(GL_SYNC_FENCE)                                     // 0x9116
	CASE(GL_SYNC_GPU_COMMANDS_COMPLETE)                     // 0x9117
	CASE(GL_UNSIGNALED)                                     // 0x9118
	CASE(GL_SIGNALED)                                       // 0x9119
	CASE(GL_ALREADY_SIGNALED)                               // 0x911A
	CASE(GL_TIMEOUT_EXPIRED)                                // 0x911B
	CASE(GL_CONDITION_SATISFIED)                            // 0x911C
	CASE(GL_WAIT_FAILED)                                    // 0x911D
	CASE(GL_VERTEX_ATTRIB_ARRAY_DIVISOR)                    // 0x88FE
	CASE(GL_ANY_SAMPLES_PASSED)                             // 0x8C2F
	CASE(GL_ANY_SAMPLES_PASSED_CONSERVATIVE)                // 0x8D6A
	CASE(GL_SAMPLER_BINDING)                                // 0x8919
	CASE(GL_RGB10_A2UI)                                     // 0x906F
	CASE(GL_TEXTURE_SWIZZLE_R)                              // 0x8E42
	CASE(GL_TEXTURE_SWIZZLE_G)                              // 0x8E43
	CASE(GL_TEXTURE_SWIZZLE_B)                              // 0x8E44
	CASE(GL_TEXTURE_SWIZZLE_A)                              // 0x8E45
	CASE(GL_GREEN)                                          // 0x1904
	CASE(GL_BLUE)                                           // 0x1905
	CASE(GL_INT_2_10_10_10_REV)                             // 0x8D9F
	CASE(GL_TRANSFORM_FEEDBACK)                             // 0x8E22
	CASE(GL_TRANSFORM_FEEDBACK_PAUSED)                      // 0x8E23
	CASE(GL_TRANSFORM_FEEDBACK_ACTIVE)                      // 0x8E24
	CASE(GL_TRANSFORM_FEEDBACK_BINDING)                     // 0x8E25
	CASE(GL_PROGRAM_BINARY_RETRIEVABLE_HINT)                // 0x8257
	CASE(GL_PROGRAM_BINARY_LENGTH)                          // 0x8741
	CASE(GL_NUM_PROGRAM_BINARY_FORMATS)                     // 0x87FE
	CASE(GL_PROGRAM_BINARY_FORMATS)                         // 0x87FF
	CASE(GL_COMPRESSED_R11_EAC)                             // 0x9270
	CASE(GL_COMPRESSED_SIGNED_R11_EAC)                      // 0x9271
	CASE(GL_COMPRESSED_RG11_EAC)                            // 0x9272
	CASE(GL_COMPRESSED_SIGNED_RG11_EAC)                     // 0x9273
	CASE(GL_COMPRESSED_RGB8_ETC2)                           // 0x9274
	CASE(GL_COMPRESSED_SRGB8_ETC2)                          // 0x9275
	CASE(GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2)       // 0x9276
	CASE(GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2)      // 0x9277
	CASE(GL_COMPRESSED_RGBA8_ETC2_EAC)                      // 0x9278
	CASE(GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC)               // 0x9279
	CASE(GL_TEXTURE_IMMUTABLE_FORMAT)                       // 0x912F
	CASE(GL_MAX_ELEMENT_INDEX)                              // 0x8D6B
	CASE(GL_NUM_SAMPLE_COUNTS)                              // 0x9380
	CASE(GL_TEXTURE_IMMUTABLE_LEVELS)                       // 0x8D63

	default:
		return "(unrecognized enum)";
	}

#undef CASE
}

const char *
piglit_get_prim_name(GLenum prim)
{
	switch (prim) {
	case GL_POINTS:
		return "GL_POINTS";
	case GL_LINES:
		return "GL_LINES";
	case GL_LINE_STRIP:
		return "GL_LINE_STRIP";
	case GL_LINE_LOOP:
		return "GL_LINE_LOOP";
	case GL_TRIANGLES:
		return "GL_TRIANGLES";
	case GL_TRIANGLE_STRIP:
		return "GL_TRIANGLE_STRIP";
	case GL_TRIANGLE_FAN:
		return "GL_TRIANGLE_FAN";
	default:
		return "(unrecognized enum)";
	}
}
