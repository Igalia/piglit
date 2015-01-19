/*
 * Copyright 2014 VMware, Inc.
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
 * Test resource limits given the maximum supported by the implementation.
 *
 * Each component of the fragment output is derived using the following
 * expression (indexed at the scalar level):
 *
 *   result[i] = texture[i] * texture[i] * ... * attrib[i + l * numOutputs]
 *
 * Depending on the limits for vertex/fragment image units, the texture
 * contribution will vary. See the generation of g_expected at the end of
 * piglit_init.
 *
 * Since the scalar inputs are primes, multiplication will yield a unique
 * result. Results can be diagnosed by evaluating for their missing factor(s).
 *
 * Matthew McClure
 * 7 July 2014
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 30;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END


/*
 * Definitions for the runtime behavior.
 */
#define GLSL_VERSION                    "#version 130"
#define BUFFER_WIDTH                    32
#define BUFFER_HEIGHT                   32
#define MAX_SHADER_LINE_CHARS           256
#define MAX_SHADER_TEXT_CHARS           16*1024
#define MAX_COMPONENTS                  4
#define MAX_EXPRESSION_OUTPUTS          1
#define MAX_EXPRESSION_ARGUMENTS        2
#define MAX_EXPRESSION_TEMPS            1
#define TEMP_TEXT_SIZE                  (MAX_EXPRESSION_OUTPUTS +  \
                                         MAX_EXPRESSION_ARGUMENTS +  \
                                         MAX_EXPRESSION_ARGUMENTS)

#define NUM_PRIMES                      512
#define NUM_VERTICES                    3

#define DEBUG_INPUT                     0x01
#define DEBUG_READBACK                  0x02
#define DEBUG_SHADERS                   0x04
#define DEBUG_DRAW                      0x08
#define DEBUG_DONT_CLAMP_MAX_VARYINGS   0x10

/*
 * Type definitions for our working set.
 */
enum {
        DRAW_ARRAYS_VBO = 0,
        DRAW_ELEMENTS_VBO = 1,
        DRAW_IMMEDIATE = 2,
};

typedef struct MyVector4 {
        float x, y, z, w;
} MyVector4;

typedef struct PackedTypeDesc {
        char *typeName;
        unsigned numComponents;
        char *componentNames[MAX_COMPONENTS];
        char *defaultValue;
} PackedTypeDesc;

typedef struct PackedDesc {
        char *semanticName;
        char *variableName;
        bool isArray;
        unsigned count;
        const PackedTypeDesc *typeDesc;
} PackedDesc;


/*
 * Global variables for the test's state.
 */
static GLint g_maxVaryingFloats = 0;
static GLint g_maxVertexAttribs = 0;
static GLint g_maxVertexTextureImageUnits = 0;
static GLint g_maxTextureImageUnits = 0;
static GLint g_maxCombinedTextureImageUnits = 0;

static GLint g_maxAuxBuffers = 0;
static GLint g_maxDrawBuffers = 0;
static GLint g_maxColorAttachments = 0;

static GLint g_debugMask = 0x0;
static GLint g_drawMode = DRAW_IMMEDIATE;

static char *g_fragColor = "gl_FragColor";
static char *g_fragData = "gl_FragData";

static const GLfloat g_primes[NUM_PRIMES] = {
2.0, 3.0, 5.0, 7.0, 11.0, 13.0, 17.0, 19.0,
23.0, 29.0, 31.0, 37.0, 41.0, 43.0, 47.0, 53.0,
59.0, 61.0, 67.0, 71.0, 73.0, 79.0, 83.0, 89.0,
97.0, 101.0, 103.0, 107.0, 109.0, 113.0, 127.0, 131.0,
137.0, 139.0, 149.0, 151.0, 157.0, 163.0, 167.0, 173.0,
179.0, 181.0, 191.0, 193.0, 197.0, 199.0, 211.0, 223.0,
227.0, 229.0, 233.0, 239.0, 241.0, 251.0, 257.0, 263.0,
269.0, 271.0, 277.0, 281.0, 283.0, 293.0, 307.0, 311.0,
313.0, 317.0, 331.0, 337.0, 347.0, 349.0, 353.0, 359.0,
367.0, 373.0, 379.0, 383.0, 389.0, 397.0, 401.0, 409.0,
419.0, 421.0, 431.0, 433.0, 439.0, 443.0, 449.0, 457.0,
461.0, 463.0, 467.0, 479.0, 487.0, 491.0, 499.0, 503.0,
509.0, 521.0, 523.0, 541.0, 547.0, 557.0, 563.0, 569.0,
571.0, 577.0, 587.0, 593.0, 599.0, 601.0, 607.0, 613.0,
617.0, 619.0, 631.0, 641.0, 643.0, 647.0, 653.0, 659.0,
661.0, 673.0, 677.0, 683.0, 691.0, 701.0, 709.0, 719.0,
727.0, 733.0, 739.0, 743.0, 751.0, 757.0, 761.0, 769.0,
773.0, 787.0, 797.0, 809.0, 811.0, 821.0, 823.0, 827.0,
829.0, 839.0, 853.0, 857.0, 859.0, 863.0, 877.0, 881.0,
883.0, 887.0, 907.0, 911.0, 919.0, 929.0, 937.0, 941.0,
947.0, 953.0, 967.0, 971.0, 977.0, 983.0, 991.0, 997.0,
1009.0, 1013.0, 1019.0, 1021.0, 1031.0, 1033.0, 1039.0, 1049.0,
1051.0, 1061.0, 1063.0, 1069.0, 1087.0, 1091.0, 1093.0, 1097.0,
1103.0, 1109.0, 1117.0, 1123.0, 1129.0, 1151.0, 1153.0, 1163.0,
1171.0, 1181.0, 1187.0, 1193.0, 1201.0, 1213.0, 1217.0, 1223.0,
1229.0, 1231.0, 1237.0, 1249.0, 1259.0, 1277.0, 1279.0, 1283.0,
1289.0, 1291.0, 1297.0, 1301.0, 1303.0, 1307.0, 1319.0, 1321.0,
1327.0, 1361.0, 1367.0, 1373.0, 1381.0, 1399.0, 1409.0, 1423.0,
1427.0, 1429.0, 1433.0, 1439.0, 1447.0, 1451.0, 1453.0, 1459.0,
1471.0, 1481.0, 1483.0, 1487.0, 1489.0, 1493.0, 1499.0, 1511.0,
1523.0, 1531.0, 1543.0, 1549.0, 1553.0, 1559.0, 1567.0, 1571.0,
1579.0, 1583.0, 1597.0, 1601.0, 1607.0, 1609.0, 1613.0, 1619.0,
1621.0, 1627.0, 1637.0, 1657.0, 1663.0, 1667.0, 1669.0, 1693.0,
1697.0, 1699.0, 1709.0, 1721.0, 1723.0, 1733.0, 1741.0, 1747.0,
1753.0, 1759.0, 1777.0, 1783.0, 1787.0, 1789.0, 1801.0, 1811.0,
1823.0, 1831.0, 1847.0, 1861.0, 1867.0, 1871.0, 1873.0, 1877.0,
1879.0, 1889.0, 1901.0, 1907.0, 1913.0, 1931.0, 1933.0, 1949.0,
1951.0, 1973.0, 1979.0, 1987.0, 1993.0, 1997.0, 1999.0, 2003.0,
2011.0, 2017.0, 2027.0, 2029.0, 2039.0, 2053.0, 2063.0, 2069.0,
2081.0, 2083.0, 2087.0, 2089.0, 2099.0, 2111.0, 2113.0, 2129.0,
2131.0, 2137.0, 2141.0, 2143.0, 2153.0, 2161.0, 2179.0, 2203.0,
2207.0, 2213.0, 2221.0, 2237.0, 2239.0, 2243.0, 2251.0, 2267.0,
2269.0, 2273.0, 2281.0, 2287.0, 2293.0, 2297.0, 2309.0, 2311.0,
2333.0, 2339.0, 2341.0, 2347.0, 2351.0, 2357.0, 2371.0, 2377.0,
2381.0, 2383.0, 2389.0, 2393.0, 2399.0, 2411.0, 2417.0, 2423.0,
2437.0, 2441.0, 2447.0, 2459.0, 2467.0, 2473.0, 2477.0, 2503.0,
2521.0, 2531.0, 2539.0, 2543.0, 2549.0, 2551.0, 2557.0, 2579.0,
2591.0, 2593.0, 2609.0, 2617.0, 2621.0, 2633.0, 2647.0, 2657.0,
2659.0, 2663.0, 2671.0, 2677.0, 2683.0, 2687.0, 2689.0, 2693.0,
2699.0, 2707.0, 2711.0, 2713.0, 2719.0, 2729.0, 2731.0, 2741.0,
2749.0, 2753.0, 2767.0, 2777.0, 2789.0, 2791.0, 2797.0, 2801.0,
2803.0, 2819.0, 2833.0, 2837.0, 2843.0, 2851.0, 2857.0, 2861.0,
2879.0, 2887.0, 2897.0, 2903.0, 2909.0, 2917.0, 2927.0, 2939.0,
2953.0, 2957.0, 2963.0, 2969.0, 2971.0, 2999.0, 3001.0, 3011.0,
3019.0, 3023.0, 3037.0, 3041.0, 3049.0, 3061.0, 3067.0, 3079.0,
3083.0, 3089.0, 3109.0, 3119.0, 3121.0, 3137.0, 3163.0, 3167.0,
3169.0, 3181.0, 3187.0, 3191.0, 3203.0, 3209.0, 3217.0, 3221.0,
3229.0, 3251.0, 3253.0, 3257.0, 3259.0, 3271.0, 3299.0, 3301.0,
3307.0, 3313.0, 3319.0, 3323.0, 3329.0, 3331.0, 3343.0, 3347.0,
3359.0, 3361.0, 3371.0, 3373.0, 3389.0, 3391.0, 3407.0, 3413.0,
3433.0, 3449.0, 3457.0, 3461.0, 3463.0, 3467.0, 3469.0, 3491.0,
3499.0, 3511.0, 3517.0, 3527.0, 3529.0, 3533.0, 3539.0, 3541.0,
3547.0, 3557.0, 3559.0, 3571.0, 3581.0, 3583.0, 3593.0, 3607.0,
3613.0, 3617.0, 3623.0, 3631.0, 3637.0, 3643.0, 3659.0, 3671.0,
};

static const PackedTypeDesc g_rgbaDesc = {
        "vec4", 4, { ".r", ".g", ".b", ".a" },
        "vec4(1.0, 1.0, 1.0, 1.0)"
};

static const PackedTypeDesc g_vec4Desc = {
        "vec4", 4, { ".x", ".y", ".z", ".w" },
        "vec4(1.0, 1.0, 1.0, 1.0)"
};

static const PackedTypeDesc g_floatDesc = {
        "float", 1, { "" },
        "1.0"
};

static const PackedTypeDesc g_sampler2DDesc = {
        "sampler2D", 4, { "" },
        "vec4(1.0, 1.0, 1.0, 1.0)"
};

static const char *g_vectorComponents[MAX_COMPONENTS] = {
        ".x", ".y", ".z", ".w"
};

//static const MyVector4 one = { 1.0f, 1.0f, 1.0f, 1.0f };
//static const MyVector4 zero = { 0.0f, 0.0f, 0.0f, 0.0f };

static const MyVector4 g_positionBuffer[NUM_VERTICES] = {
        { -1.0, -1.0, 0.0, 0.0 },
        { -1.0, 1.0, 0.0, 0.0 },
        { 1.0, 1.0, 0.0, 0.0 }
};

static GLuint g_elementVBO;
static const GLushort g_elementBuffer[NUM_VERTICES] = { 0, 1, 2 };
static const char *g_vertexShaderText =
        "#version 110 \n"
        " \n"
        "void main() \n"
        "{ \n"
        "   gl_Position = gl_Vertex; \n"
        "} \n";

//static char *g_geometryShaderText = NULL;

static const char *g_fragmentShaderText =
        "#version 110 \n"
        " \n"
        "void main()\n"
        "{ \n"
        "    gl_FragData[0] = vec4(1.0, 2.0, 3.0, 4.0);\n"
        "    gl_FragData[1] = vec4(5.0, 6.0, 7.0, 8.0);\n"
        "} \n";

static GLuint g_program = 0;
static GLfloat *g_expected;


/**
 * Format a string which contains the requested declaration for the variable.
 * The desc structure will describe the variable, i.e. type, if it is an array,
 * and how large the array is.
 *
 * \param desc Descriptor of the variable referenced. See PackedDesc.
 * \param tmpStrSize Temporary string size for safe string formatting.
 * \param tmpStr Temporary staging string.
 * \param strSize Size of the formatted string.
 * \param str An allocated temporary string for this function to populate with
 *            the reference expression.
 * \return Pointer to str.
 */

static char *
get_packed_decl(const PackedDesc *desc,
                const unsigned tmpStrSize,
                char *tmpStr,
                const unsigned strSize,
                char *str)
{
        int i = 0;

        if (NULL == str) {
                return "";
        }

        if (desc->isArray) {
                snprintf(tmpStr, tmpStrSize,
                         "%s %s %s%s%u%s;\n",
                         desc->semanticName,
                         desc->typeDesc->typeName,
                         desc->variableName,
                         desc->isArray ? "[" : "",
                         desc->count,
                         desc->isArray ? "]" : "");
                strncat(str, tmpStr, strSize - strlen(str));
        } else if (desc->count > 1) {
                for (i = 0; i < desc->count; i++) {
                        snprintf(tmpStr, tmpStrSize,
                                 "%s %s %s%u;\n",
                                 desc->semanticName,
                                 desc->typeDesc->typeName,
                                 desc->variableName, i);
                        strncat(str, tmpStr, strSize - strlen(str));
                }
        } else {
                snprintf(tmpStr, tmpStrSize,
                         "%s %s %s;\n",
                         desc->semanticName,
                         desc->typeDesc->typeName,
                         desc->variableName);
                strncat(str, tmpStr, strSize - strlen(str));
        }

        return str;
}


/**
 * Format a string which contains the requested referenced value. The desc
 * structure will describe the variable which is sub-indexed by arrayIndex.
 * To address individual components, a componentIndex translated to (x,y,z,w)
 * is applied to the textual representation.
 *
 * \param desc Descriptor of the variable referenced. See PackedDesc.
 * \param arrayIndex Sub-index into the variable described by desc.
 * \param componentIndex Component of the variable, depends on the desc
 *                       configuration for the variable.
 * \param str An allocated temporary string for this function to populate with
 *            the reference expression.
 * \return Pointer to str.
 */

static char *
get_packed_reference(const PackedDesc *desc,
                     const unsigned arrayIndex,
                     const unsigned componentIndex,
                     const unsigned strSize,
                     char *str)
{
        if (g_debugMask & DEBUG_INPUT) {
                printf("desc=%p componentIndex=%u\n", desc, componentIndex);
                printf("desc->typeDesc=%p\n", desc->typeDesc);
                printf("desc->typeDesc->componentNames=%p\n",
                       desc->typeDesc->componentNames);
        }

        if (componentIndex > desc->typeDesc->numComponents) {
                snprintf(str, strSize, "%s", g_floatDesc.defaultValue);
                return str;
        }

        if (arrayIndex > desc->count) {
                snprintf(str, strSize, "%s%s",
                         desc->typeDesc->defaultValue,
                         desc->typeDesc->componentNames[componentIndex]);
                return str;
        }

        if (desc->count > 1) {
                snprintf(str, strSize, "%s%s%u%s%s",
                         desc->variableName,
                         desc->isArray ? "[" : "",
                         arrayIndex,
                         desc->isArray ? "]" : "",
                         desc->typeDesc->componentNames[componentIndex]);
        } else {
                snprintf(str, strSize, "%s%s",
                         desc->variableName,
                         desc->typeDesc->componentNames[componentIndex]);
        }

        return str;
}


/**
 * Set up an FBO for the test.
 *
 * \param colorTarget Color attachment index relative to GL_COLOR_ATTACHMENT0
 * \param internalFormat Internal GL format for the pixel data.
 * \param format GL format for the pixel data.
 * \param formatType Data type for the pixel data. 
 * \param width Width of the requested FBO backing store.
 * \param height Height of the requested FBO backing store.
 * \param fbo FBO object allocated to bind with this texture.
 * \param pTexture A pointer to the GLuint identifier storage for the allocated
 *                 texture.
 * \return true if the setup succeeded without error
 *         false otherwise.
 */

static bool
setup_fbo_2d(const GLuint colorTarget,
             const GLenum internalFormat,
             const GLenum format,
             const GLenum formatType,
             const GLuint width,
             const GLuint height,
             const GLuint fbo,
             GLuint *pTexture)
{
        GLenum status;

        glGenTextures(1, pTexture);
        glBindTexture(GL_TEXTURE_2D, *pTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0,
                     format, formatType, NULL);

        /*
         * Framebuffer object is implied to be bound.
         */
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + colorTarget,
                               GL_TEXTURE_2D, *pTexture, 0);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                fprintf(stderr, "Failed to create FBO %u.\n", colorTarget);
                piglit_report_result(PIGLIT_FAIL);
                return false;
        }

        status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
                fprintf(stderr, "Incomplete fbo for format %s.%s (status %s)\n",
                        piglit_get_gl_enum_name(internalFormat),
                        piglit_get_gl_enum_name(format),
                        piglit_get_gl_enum_name(status));
                piglit_report_result(PIGLIT_FAIL);
                return false;
        }

        return true;
}


/**
 * Set up the vertex buffer objects and element buffer objects for the
 * test. To ensure constant values across the primitive, populate the same
 * prime attribute value in each vertex.
 *
 * \return true if the setup succeeded without error
 *         false otherwise.
 */

static bool
setup_vertex_element_buffers(void)
{
        MyVector4 attrib[NUM_VERTICES];
        GLuint buf;
        GLuint attribLoc;
        int i;

        /*
         * Setup the gl_Position attribute buffer.
         */
        glGenBuffers(1, &buf);
        glBindBuffer(GL_ARRAY_BUFFER, buf);
        glBufferData(GL_ARRAY_BUFFER, NUM_VERTICES*sizeof(MyVector4),
                     g_positionBuffer, GL_STATIC_DRAW);

        attribLoc = glGetAttribLocation(g_program, "InPosition");
        glEnableVertexAttribArray(attribLoc);
        glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        /*
         * Setup the vertex buffer objects.
         */
        for (i = 0; i < g_maxVertexAttribs - 1; i++) {
                char strTemp[16];

                if ((i + 1) * MAX_COMPONENTS < NUM_PRIMES) {
                        attrib[0].x = attrib[1].x =
                        attrib[2].x = g_primes[i*MAX_COMPONENTS + 0];
                        attrib[0].y = attrib[1].y =
                        attrib[2].y = g_primes[i*MAX_COMPONENTS + 1];
                        attrib[0].z = attrib[1].z =
                        attrib[2].z = g_primes[i*MAX_COMPONENTS + 2];
                        attrib[0].w = attrib[1].w =
                        attrib[2].w = g_primes[i*MAX_COMPONENTS + 3];
                } else {
                        attrib[0].x = attrib[1].x = attrib[2].x = 1.0;
                        attrib[0].y = attrib[1].y = attrib[2].y = 1.0;
                        attrib[0].z = attrib[1].z = attrib[2].z = 1.0;
                        attrib[0].w = attrib[1].w = attrib[2].w = 1.0;
                }

                glGenBuffers(1, &buf);
                glBindBuffer(GL_ARRAY_BUFFER, buf);
                glBufferData(GL_ARRAY_BUFFER, sizeof(MyVector4)*NUM_VERTICES,
                             attrib, GL_STATIC_DRAW);

                snprintf(strTemp, sizeof(strTemp), "InValue%u", i);
                attribLoc = glGetAttribLocation(g_program, strTemp);
                glEnableVertexAttribArray(attribLoc);
                glVertexAttribPointer(attribLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                         fprintf(stderr, "Failed to create VBO %u.\n", i);
                         piglit_report_result(PIGLIT_FAIL);
                         return false;
                }
        }

        /*
         * Setup the element buffers.
         */
        glGenBuffers(1, &g_elementVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_elementVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_elementBuffer),
                     g_elementBuffer, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                fprintf(stderr, "Failed to create IBO.\n");
                piglit_report_result(PIGLIT_FAIL);
                return false;
        }

        return true;
}


/**
 * Build a GLSL shader for the given packedInput, packedUniform, packedOutput
 * descriptors. Each GLSL shader will have inputs, uniforms, and outputs. To
 * ensure all bound resources are made resident, this shader must use all the
 * inputs, uniforms, and outputs requested by the describing PackedDesc
 * structures. If the number of outputs are smaller than the number of inputs,
 * the inputs will map as follows:
 *
 *   outputs[0] = uniforms[0] inputs[0] * ... * inputs[(i % numOutputs == 0)]
 *
 * where i >= numOutputs and represents a subsequent pass of inputs up
 * until i == numInputs.
 *
 * If the number of outputs is larger than the number of inputs, the inputs
 * and uniforms are referenced until exhausted. Once exhausted, an input
 * or uniform will default to 1.0 to provide a multiplicative identity.
 *
 * \param packedInput A descriptor of the type and number of inputs to this
 *                    shader.
 * \param packedUniform A descriptor of the type and number of uniforms bound
 *                      to the shader.
 * \param packedOutput A descriptor of the type and number of outputs for this
 *                     shader.
 * \param outputDefaultSystemValue A required output for the GLSL shader.
 *                                 i.e. gl_Position for a vertex shader.
 * \param defaultSystemValueDecl Pre-defined output declaration.
 * \param defaultSystemValue Pre-defined output value.
 * \param pShaderText Output pointer for the allocated and generated shader
 *                    text.
 * \return true if the setup succeeded without error
 *         false otherwise.
 */

static bool
build_reduce_glsl_shader(const PackedDesc *packedInput,
                         const PackedDesc *packedUniform,
                         const PackedDesc *packedOutput,
                         const char *outputDefaultSystemValue,
                         const char *defaultSystemValueDecl,
                         const char *defaultSystemValue,
                         char **pShaderText)
{
        char *shaderTempText[TEMP_TEXT_SIZE];
        char *shaderText = NULL;
        unsigned shaderTextFree = 0;
        unsigned numScalarInputs =
                packedInput->count * packedInput->typeDesc->numComponents;
        unsigned numScalarUniforms =
                packedUniform->count * packedUniform->typeDesc->numComponents;
        unsigned numScalarOutputs =
                packedOutput->count * packedOutput->typeDesc->numComponents;

        /* Prepare for reduction of input contributions to the output slots */
        unsigned di_do = numScalarInputs / numScalarOutputs;
        unsigned r = numScalarInputs % numScalarOutputs;
        int i, j, k, l;

        if (g_debugMask & DEBUG_INPUT) {
                printf("di_do=%u r=%u\n", di_do, r);
        }

        for (i = 0; i < sizeof(shaderTempText) / sizeof(char *); i++) {
                shaderTempText[i] = calloc(MAX_SHADER_LINE_CHARS, sizeof(char));

                if (NULL == shaderTempText[i]) {
                        fprintf(stderr,
                                "Failed to allocate shader temporary text area\n");
                        piglit_report_result(PIGLIT_FAIL);
                        return false;
                }
        }

        shaderText = calloc(MAX_SHADER_TEXT_CHARS, sizeof(char));
        if (NULL == shaderText) {
                fprintf(stderr, "Failed to allocate space for shader text\n");
                piglit_report_result(PIGLIT_FAIL);
                return false;
        }

        shaderTextFree = MAX_SHADER_TEXT_CHARS - 1;

        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
        "%s\n", GLSL_VERSION);
        strncat(shaderText, shaderTempText[0], shaderTextFree);
        shaderTextFree -= strlen(shaderTempText[0]);

        /*
         * Declare the default system value input.
         */
        if (outputDefaultSystemValue) {
                snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                         "%s\n", defaultSystemValueDecl);
                strncat(shaderText, shaderTempText[0], shaderTextFree);
                shaderTextFree -= strlen(defaultSystemValueDecl);
        }

        /*
         * Declare the input attributes.
         */
        get_packed_decl(packedInput,
        MAX_SHADER_LINE_CHARS, shaderTempText[0],
        MAX_SHADER_TEXT_CHARS, shaderText);

        /*
         * Declare the uniform samplers using the vertex texture functionality.
         */
        get_packed_decl(packedUniform,
        MAX_SHADER_LINE_CHARS, shaderTempText[0],
        MAX_SHADER_TEXT_CHARS, shaderText);

        /*
         * Declare the outputs.
         */
        if (packedOutput->semanticName) {
                get_packed_decl(packedOutput,
                                MAX_SHADER_LINE_CHARS, shaderTempText[0],
                                MAX_SHADER_TEXT_CHARS, shaderText);
        }

        shaderTextFree = MAX_SHADER_TEXT_CHARS - strlen(shaderText);

        /*
         * Begin main program block.
         */
        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                 "void main()\n{\n");
        strncat(shaderText, shaderTempText[0], shaderTextFree);
        shaderTextFree -= strlen(shaderTempText[0]);

        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                 "  vec4 texel;\n");
        strncat(shaderText, shaderTempText[0], shaderTextFree);
        shaderTextFree -= strlen(shaderTempText[0]);

        /*
         * Passthru the attributes to the outputs.
         */
        i = j = k = l = 0;

        while (i < numScalarOutputs)  {
                char *resultStr = " ", *srcStr = " ";

                if ((i < packedUniform->count*packedUniform->typeDesc->numComponents) &&
                    (j % packedUniform->typeDesc->numComponents) == 0) {
                        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                                 "  texel = texture2D(Texture[%u], vec2(0.0, 0.0));\n",
                                 i / packedUniform->typeDesc->numComponents);
                        strncat(shaderText, shaderTempText[0], shaderTextFree);
                        shaderTextFree -= strlen(shaderTempText[0]);
                }

                /*
                 * Iterate through the contents of the texel
                 */
                resultStr = get_packed_reference(packedOutput,
                                (i / packedOutput->typeDesc->numComponents),
                                (i % packedOutput->typeDesc->numComponents),
                                MAX_SHADER_LINE_CHARS,
                                shaderTempText[1]);

                /* Reset the temp string to avoid cumulative contributions. */
                memset(shaderTempText[2], 0, MAX_SHADER_LINE_CHARS);

                if (k < numScalarInputs) {
                        /* Reduce the scalar multiple of the input contributions. */
                        for (l = 0; l < di_do; l++) {
                                srcStr = get_packed_reference(packedInput,
                                                (i + (l * numScalarOutputs)) /
                                                packedInput->typeDesc->numComponents,
                                                (i + (l * numScalarOutputs)) %
                                                packedInput->typeDesc->numComponents,
                                                MAX_SHADER_LINE_CHARS,
                                                shaderTempText[3]);

                                if (l > 0) {
                                        strncat(shaderTempText[2], " * ",
                                                MAX_SHADER_LINE_CHARS - strlen(shaderTempText[2]));
                                }
                                strncat(shaderTempText[2], srcStr,
                                        MAX_SHADER_LINE_CHARS - strlen(shaderTempText[2]));

                                k++;
                        }

                        /* Reduce the remaining scalar contributions until exhausted */
                        if (r) {
                                srcStr = get_packed_reference(packedInput,
                                                (i + (l * numScalarOutputs)) /
                                                packedInput->typeDesc->numComponents,
                                                (i + (l * numScalarOutputs)) %
                                                packedInput->typeDesc->numComponents,
                                                MAX_SHADER_LINE_CHARS,
                                                shaderTempText[3]);
                                if (l > 0) {
                                        strncat(shaderTempText[2], " * ",
                                                MAX_SHADER_LINE_CHARS - strlen(shaderTempText[2]));
                                }
                                strncat(shaderTempText[2], srcStr,
                                        MAX_SHADER_LINE_CHARS - strlen(shaderTempText[2]));
                                k++;
                                r--;
                        }
                        srcStr = shaderTempText[2];
                } else {
                        snprintf(shaderTempText[2], MAX_SHADER_LINE_CHARS,
                                 "%s", g_floatDesc.defaultValue);
                        srcStr = shaderTempText[2];
                }

                if (j < numScalarUniforms) {
                        if ((g_debugMask & DEBUG_INPUT) && outputDefaultSystemValue) {
                                snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                                         "  %s = texel%s;\n", resultStr,
                                         g_vectorComponents[j %
                                                packedUniform->typeDesc->numComponents]);
                        } else {
                                snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                                         "  %s = texel%s * %s;\n", resultStr,
                                         g_vectorComponents[j %
                                                packedUniform->typeDesc->numComponents],
                                         srcStr);
                        }
                        strncat(shaderText, shaderTempText[0], shaderTextFree);
                        shaderTextFree -= strlen(shaderTempText[0]);
                        j++;
                } else {
                        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                                 "  %s = %s%s * %s;\n", resultStr,
                                 packedUniform->typeDesc->defaultValue,
                                 g_vectorComponents[0],
                                 srcStr);
                        strncat(shaderText, shaderTempText[0], shaderTextFree);
                        shaderTextFree -= strlen(shaderTempText[0]);
                }

                /* Increment to the next output. */
                i++;
        }

        /*
         * End the main program block.
         */
        if (outputDefaultSystemValue) {
                snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS,
                         "  %s = %s;\n",
                         outputDefaultSystemValue, defaultSystemValue);
                strncat(shaderText, shaderTempText[0], shaderTextFree);
                shaderTextFree -= strlen(shaderTempText[0]);
        }

        snprintf(shaderTempText[0], MAX_SHADER_LINE_CHARS, "}\n");
        strncat(shaderText, shaderTempText[0], shaderTextFree);
        shaderTextFree -= strlen(shaderTempText[0]);

        for (i = 0; i < sizeof(shaderTempText) / sizeof(char *); i++) {
                if (shaderTempText[i]) {
                        free(shaderTempText[i]);
                }
        }

        *pShaderText = shaderText;
        return true;
}


/**
 * Core piglit_display callback. This function will bind the FBOs, Textures,
 * VBOs, IBOs, and draw a primitive using the GLSL shader generated by
 * build_reduce_glsl_shader.
 *
 * Once rendered, a pixel from each of the color attachments is read back and
 * compared to the final expression value stored in g_expected given the
 * associated color attachment index.
 *
 * \return PIGLIT_PASS if the setup succeeded without error
 *         PIGLIT_FAIL otherwise.
 */

enum piglit_result
piglit_display(void)
{
        GLuint vao = 0;
        GLuint fbo = 0;
        GLuint *fboTextures = NULL;
        GLenum *colorBuffers = NULL;
        GLuint attribLoc;
        GLfloat result[4];
        int i = 0;

        /*
         * Reserve memory for the FBO texture objects.
         */
        fboTextures = calloc(g_maxColorAttachments, sizeof(GLuint));

        if (NULL == fboTextures) {
                fprintf(stderr, "Failed to create FBO texture object container.\n");
                goto fail;
        }

        /*
         * Build the color attachments
         */
        colorBuffers = calloc(g_maxColorAttachments, sizeof(GLenum));

        if (NULL == colorBuffers) {
                fprintf(stderr, "Failed to create draw buffers descriptor.\n");
                goto fail;
        }

        /*
         * Generate an FBO container to hold the color attachment hierarchy.
         */
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        for (i = 0; i < g_maxColorAttachments; i++) {
                setup_fbo_2d(i, GL_RGBA32F, GL_RGBA, GL_FLOAT,
                             BUFFER_WIDTH, BUFFER_HEIGHT,
                             fbo, &fboTextures[i]);
                colorBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        /*
         * Build the textures sampled by the shaders
         */
        for (i = 0; i < g_maxCombinedTextureImageUnits; i++) {
                GLuint tex;
                GLuint uniformLoc;
                char strTemp[16];

                glGenTextures(1, &tex);
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, tex);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                if (((i+1) * MAX_COMPONENTS) > sizeof(g_primes)) {
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0,
                                     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                } else {
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0,
                                     GL_RGBA, GL_FLOAT,
                                     &g_primes[i * MAX_COMPONENTS]);
                }

                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr, "Failed to create texture %u.\n", i);
                        goto fail;
                }

                snprintf(strTemp, sizeof(strTemp), "Texture[%u]", i);
                uniformLoc = glGetUniformLocation(g_program, strTemp);
                glUniform1i(uniformLoc, i);

                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr, "Unable to assign texture %u uniform.\n", i);
                        goto fail;
                }
        }

        /*
         * Setup the vertex and element buffers for drawing our points.
         */
        if (g_drawMode != DRAW_IMMEDIATE) {
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);

                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr, "Unable to create VAO.\n");
                        goto fail;
                }

                setup_vertex_element_buffers();
        }

        /*
         * Setup the raster state.
         */
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDrawBuffers(g_maxColorAttachments, colorBuffers);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                fprintf(stderr, "Unable to assign draw buffers.\n");
                goto fail;
        }

        glClearColor(0.0, 1.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);

        /*
         * Bind the vertex array and enable each attribute.
         */
        if (g_drawMode != DRAW_IMMEDIATE) {
                glBindVertexArray(vao);
                attribLoc = glGetAttribLocation(g_program, "InPosition");
                glEnableVertexAttribArray(attribLoc);

                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr,
                                "Unable to enable vertex array attribute %u.\n",
                                attribLoc);
                        goto fail;
                }

                /*
                 * Enable the rest of the attributes.
                 */
                for (i = 0; i < g_maxVertexAttribs - 1; i++) {
                        char strTemp[16];

                        snprintf(strTemp, sizeof(strTemp), "InValue%u", i);
                        attribLoc = glGetAttribLocation(g_program, strTemp);
                        glEnableVertexAttribArray(attribLoc);

                        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                                fprintf(stderr,
                                        "Unable to enable vertex array attribute %u.\n",
                                        attribLoc);
                                goto fail;
                        }
                }

                if (g_drawMode == DRAW_ARRAYS_VBO) {
                        if (g_debugMask & DEBUG_DRAW) {
                                fprintf(stderr, "Draw mode DRAW_ARRAYS_VBO\n");
                        }
                        glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
                }

                if (g_drawMode == DRAW_ELEMENTS_VBO) {
                        if (g_debugMask & DEBUG_DRAW) {
                                fprintf(stderr,
                                        "Draw mode DRAW_ELEMENTS_VBO\n");
                        }
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_elementVBO);
                        glDrawElements(GL_TRIANGLES, NUM_VERTICES,
                                       GL_UNSIGNED_SHORT, 0);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                }

                /*
                 * Blindly reset all the attributes.
                 */
                for (i = 0; i < g_maxVertexAttribs; i++) {
                        glDisableVertexAttribArray(i);
                }
        } else {
                glBegin(GL_TRIANGLES);
                glVertex3f(-1.0, -1.0, 0.0);
                glVertex3f(-1.0, 1.0, 0.0);
                glVertex3f(1.0, 1.0, 0.0);
                glEnd();
        }

        if (!piglit_check_gl_error(GL_NO_ERROR))
                goto fail;

        /*
         * Read back the FBO contents.
         */

        /*
         * Disable color clamping so we don't encounter result
         * collisions attempting to use a normalized color space.
         *
         * Requires OpenGL 3.0
         */
        glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);

        for (i = 0; i < g_maxColorAttachments; i++) {
                if (g_debugMask & DEBUG_READBACK) {
                        printf("GL_READ_FRAMEBUFFER <- fbo=%u\n", fbo);
                }

                glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

                if (!piglit_check_gl_error(GL_NO_ERROR))
                        goto fail;

                glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
                                       GL_COLOR_ATTACHMENT0,
                                       GL_TEXTURE_2D,
                                       fboTextures[i],
                                       0);

                if (!piglit_check_gl_error(GL_NO_ERROR))
                        goto fail;

                glPixelStorei(GL_PACK_ALIGNMENT, 1);
                glReadBuffer(GL_COLOR_ATTACHMENT0);

                if (!piglit_check_gl_error(GL_NO_ERROR))
                        goto fail;

                glReadPixels(0, BUFFER_HEIGHT - 1,
                             1, 1, GL_RGBA, GL_FLOAT, result);

                if ((g_expected[(i * MAX_COMPONENTS) + 0] != result[0]) ||
                    (g_expected[(i * MAX_COMPONENTS) + 1] != result[1]) ||
                    (g_expected[(i * MAX_COMPONENTS) + 2] != result[2]) ||
                    (g_expected[(i * MAX_COMPONENTS) + 3] != result[3])) {
                        fprintf(stderr, "GL_COLOR_ATTACHMENT%u: expected "
                                        "(%f, %f, %f, %f) != (%f, %f, %f, %f)\n",
                                i, g_expected[(i * MAX_COMPONENTS) + 0],
                                g_expected[(i * MAX_COMPONENTS) + 1],
                                g_expected[(i * MAX_COMPONENTS) + 2],
                                g_expected[(i * MAX_COMPONENTS) + 3],
                                result[0], result[1], result[2], result[3]);

                        goto fail;
                }
        }

        piglit_present_results();

        /*
         * Cleanup the allocations.
         */
        free(colorBuffers);
        free(fboTextures);

        piglit_report_result(PIGLIT_PASS);

        return PIGLIT_PASS;

fail:

        if (colorBuffers)
                free(colorBuffers);

        if (fboTextures)
                free(fboTextures);

        piglit_report_result(PIGLIT_FAIL);

        return PIGLIT_FAIL;
}


/**
 * Core piglit_init callback. This function will query the implementation for
 * the maximum number of supported resources. We attempt to create a GLSL
 * shader which references all supported resources bindable within the GL 3.0
 * specification.
 *
 * Each resource will be populated with a unique prime value. This function is
 * also responsible for calculating the expected value and storing this in
 * the g_expected array.
 *
 * Note: If the number of primes is insufficient to assign a unique prime to
 *       each resource, we will exit with PIGLIT_SKIP.
 *
 * Results: PIGLIT_PASS if the setup succeeded without error
 *          PIGLIT_FAIL otherwise.
 */

void
piglit_init(int argc, char **argv)
{
        GLfloat *var;
        PackedDesc vsInput = { "in", "InValue", false, 0, &g_vec4Desc };
        PackedDesc vsUniform = { "uniform", "Texture", true, 0, &g_sampler2DDesc };
        PackedDesc vsOutput = { "out", "Variable", true, 0, &g_floatDesc };
        PackedDesc fsInput = { "in", "Variable", true, 0, &g_floatDesc };
        PackedDesc fsUniform = { "uniform", "Texture", true, 0, &g_sampler2DDesc };
        PackedDesc fsOutput = { NULL, g_fragData, true, 0, &g_rgbaDesc };
        char shaderTempText[MAX_SHADER_LINE_CHARS];
        char *vertexShaderText = (char *)g_vertexShaderText;
        char *fragmentShaderText = (char *)g_fragmentShaderText;
        int i = 0;

        piglit_require_gl_version(30);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-drawArraysVBO") == 0) {
                        g_drawMode = DRAW_ARRAYS_VBO;
                }
                if (strcmp(argv[i], "-drawElementsVBO") == 0) {
                        g_drawMode = DRAW_ELEMENTS_VBO;
                }
                if (strcmp(argv[i], "-drawImmediate") == 0) {
                        g_drawMode = DRAW_IMMEDIATE;
                }
                if (strcmp(argv[i], "-debugInput") == 0) {
                        g_debugMask |= DEBUG_INPUT;
                }
                if (strcmp(argv[i], "-debugReadback") == 0) {
                        g_debugMask |= DEBUG_READBACK;
                }
                if (strcmp(argv[i], "-debugShaders") == 0) {
                        g_debugMask |= DEBUG_SHADERS;
                }
                if (strcmp(argv[i], "-debugDraw") == 0) {
                        g_debugMask |= DEBUG_DRAW;
                }
                if (strcmp(argv[i], "-dontClampMaxVaryings") == 0) {
                        g_debugMask |= DEBUG_DONT_CLAMP_MAX_VARYINGS;
                }
        }

        /*
         * Query the sampler capabilities.
         */
        glGetIntegerv(GL_MAX_VARYING_FLOATS, &g_maxVaryingFloats);
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &g_maxVertexAttribs);
        glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
                      &g_maxVertexTextureImageUnits);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,
                      &g_maxTextureImageUnits);
        glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                      &g_maxCombinedTextureImageUnits);

        printf("GL_MAX_VARYING_FLOATS: %d\n", g_maxVaryingFloats);
        printf("GL_MAX_VERTEX_ATTRIBS: %d\n", g_maxVertexAttribs);
        printf("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: %d\n",
               g_maxVertexTextureImageUnits);
        printf("GL_MAX_TEXTURE_IMAGE_UNITS: %d\n",
               g_maxTextureImageUnits);
        printf("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d\n",
               g_maxCombinedTextureImageUnits);

        /*
         * Query the render target capabilities.
         */
        glGetIntegerv(GL_AUX_BUFFERS, &g_maxAuxBuffers);
        glGetIntegerv(GL_MAX_DRAW_BUFFERS, &g_maxDrawBuffers);
        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &g_maxColorAttachments);

        if (g_maxColorAttachments == 1) {
                fsOutput.variableName = g_fragColor;
        }

        printf("GL_AUX_BUFFERS: %d\n", g_maxAuxBuffers);
        printf("GL_MAX_DRAW_BUFFERS: %d\n", g_maxDrawBuffers);
        printf("GL_MAX_COLOR_ATTACHMENTS: %d\n", g_maxColorAttachments);

        if ((g_maxColorAttachments * MAX_COMPONENTS > NUM_PRIMES) ||
            (g_maxVaryingFloats > NUM_PRIMES) ||
            (g_maxVertexAttribs * MAX_COMPONENTS > NUM_PRIMES) ||
            (g_maxVertexTextureImageUnits * MAX_COMPONENTS > NUM_PRIMES) ||
            (g_maxTextureImageUnits * MAX_COMPONENTS > NUM_PRIMES)) {
                fprintf(stderr, "Unable to uniquely represent a result path.\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        /*
         * Clamp the max varyings by default to work around large array
         * issues with some GLSL implementations.
         */
        if ((g_debugMask & DEBUG_DONT_CLAMP_MAX_VARYINGS) == 0) {
                g_maxVaryingFloats = MIN2(g_maxVaryingFloats, 32);
                fprintf(stderr, "Clamped max varying floats to %u.\n",
                g_maxVaryingFloats);
        }

        if (g_drawMode == DRAW_IMMEDIATE) {
                g_maxVertexAttribs = 1;
                fprintf(stderr,
                        "Immediate mode selected, using only one vertex attrib.\n");
        }

        /*
         * Build the shaders based upon the queried limits.
         */
        vsInput.count = g_maxVertexAttribs - 1;
        vsUniform.count = g_maxVertexTextureImageUnits;
        vsOutput.count = g_maxVaryingFloats;
        if (!build_reduce_glsl_shader(&vsInput,
                                      &vsUniform,
                                      &vsOutput,
                                      "gl_Position",
                                      "in vec4 InPosition;",
                                      "InPosition",
                                      &vertexShaderText)) {
                fprintf(stderr, "Failed to build GLSL vertex shader\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        if (g_debugMask & DEBUG_SHADERS) {
                printf("vertexShaderText:\n%s", vertexShaderText);
        }

        fsInput.count = g_maxVaryingFloats;
        fsUniform.count = g_maxTextureImageUnits;
        fsOutput.count = g_maxColorAttachments;
        if (!build_reduce_glsl_shader(&fsInput,
                                      &fsUniform,
                                      &fsOutput,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &fragmentShaderText)) {
                fprintf(stderr, "Failed to build GLSL vertex shader\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        if (g_debugMask & DEBUG_SHADERS) {
                printf("fragmentShaderText:\n%s", fragmentShaderText);
        }

        /*
         * Build the vertex and fragment shaders.
         */
        g_program = piglit_build_simple_program(vertexShaderText,
                                                fragmentShaderText);
        if (!g_program) {
                fprintf(stderr, "Failed to compile/link program\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        snprintf(shaderTempText, sizeof(shaderTempText), "InPosition");
        glBindAttribLocation(g_program, 0, shaderTempText);

        for (i = 0; i < g_maxVertexAttribs - 1; i++) {
                snprintf(shaderTempText, sizeof(shaderTempText), "InValue%u", i);
                glBindAttribLocation(g_program, i+1, shaderTempText);
        }

        if (g_debugMask & DEBUG_SHADERS) {
                printf("Linking program...\n");
        }

        glLinkProgram(g_program);
        glUseProgram(g_program);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                fprintf(stderr, "Failure to link shaders\n");
                fprintf(stderr, "vertexShaderText:\n%s", vertexShaderText);
                fprintf(stderr, "fragmentShaderText:\n%s", fragmentShaderText);
                free(vertexShaderText);
                free(fragmentShaderText);
                piglit_report_result(PIGLIT_FAIL);
        }

        if (g_debugMask & DEBUG_SHADERS) {
                printf("Using program %u...\n", g_program);
        }

        /* Delete the interim copies generated by build_reduce_glsl_shader */ 
        free(vertexShaderText);
        free(fragmentShaderText);

        /*
         * Calculate the expected results for the bound resource limits.
         */
        var = calloc(g_maxVaryingFloats, sizeof(GLfloat));

        if (NULL == var) {
                fprintf(stderr,
                        "Failed to allocate temporary array for expected calculation.\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        g_expected = calloc(g_maxColorAttachments, sizeof(GLfloat)*MAX_COMPONENTS);

        if (NULL == g_expected) {
                fprintf(stderr, "Failed to allocate array for expected calculation.\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        /*
         * Sampler coverage up to the total number of varying floats.
         */
        for (i = 0; i < g_maxVaryingFloats; i++) {
                if ((i < g_maxVertexTextureImageUnits*MAX_COMPONENTS) &&
                    (i < NUM_PRIMES)) {
                        var[i] = g_primes[i];
                } else {
                        var[i] = 1.0;
                }
        }

        /*
         * Multiply in all vertex attributes.
         */
        i = 0;
        while (i < (g_maxVertexAttribs-1)*MAX_COMPONENTS) {
                if (i < NUM_PRIMES) {
                        var[i % g_maxVaryingFloats] *= g_primes[i];
                } else {
                        var[i % g_maxVaryingFloats] *= 1.0;
                }
                i++;
        }

        /*
         * Calculate the expected values for the FS stage.
         *
         * Sampler cover up to the total number of varying floats.
         */
        for (i = 0; i < g_maxColorAttachments*MAX_COMPONENTS; i++) {
                if ((i < g_maxVertexTextureImageUnits*MAX_COMPONENTS) &&
                    (i < NUM_PRIMES)) {
                        g_expected[i] = g_primes[i];
                } else {
                        g_expected[i] = 1.0;
                }
        }

        /*
         * Multiply in all the varying contributions generated by the VS.
         */
        i = 0;
        while (i < g_maxVaryingFloats) {
                g_expected[i % (g_maxColorAttachments*MAX_COMPONENTS)] *= var[i];
                i++;
        }

        for (i = 0; i < g_maxColorAttachments; i++) {
                printf("g_expected[%u]=(%f, %f, %f, %f)\n", i,
                       g_expected[(i * MAX_COMPONENTS) + 0],
                       g_expected[(i * MAX_COMPONENTS) + 1],
                       g_expected[(i * MAX_COMPONENTS) + 2],
                       g_expected[(i * MAX_COMPONENTS) + 3]);
        }

        free(var);
}
