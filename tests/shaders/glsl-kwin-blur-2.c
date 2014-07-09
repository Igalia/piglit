/*
 * Copyright © 2010 Fredrik Höglund (fredrik@kde.org)
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
 * Authors:
 *    Fredrik Höglund (fredrik@kde.org)
 */

/** @file glsl-kwin-blur-2.c
 *
 * Tests the blur effect used by the KWin window manager,
 * with a 12 pixel blur radius (uses 13 varyings).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


/* Size of viewport and test region.  Note that there are pixel probes at
 * specific locations.
 */
#define WIDTH 100
#define HEIGHT 100


/*
    Note: In KWin, the code for these shaders is generated at runtime,
          based on the blur radius. This is what the code looks like
          with the default radius (12 pixels). The code generator makes
          sure that the code doesn't exceed GL_MAX_VARYING_FLOATS.
*/
static const char vs_code[] =
    "uniform vec2 pixelSize;\n"

    "varying vec2 samplePos0;\n"
    "varying vec2 samplePos1;\n"
    "varying vec2 samplePos2;\n"
    "varying vec2 samplePos3;\n"
    "varying vec2 samplePos4;\n"
    "varying vec2 samplePos5;\n"
    "varying vec2 samplePos6;\n"
    "varying vec2 samplePos7;\n"
    "varying vec2 samplePos8;\n"
    "varying vec2 samplePos9;\n"
    "varying vec2 samplePos10;\n"
    "varying vec2 samplePos11;\n"
    "varying vec2 samplePos12;\n"

    "void main(void)\n"
    "{\n"
    "    vec2 center = vec4(gl_TextureMatrix[0] * gl_MultiTexCoord0).st;\n"

    "    samplePos0  = center + pixelSize * vec2(-11.5);\n"
    "    samplePos1  = center + pixelSize * vec2(-9.5);\n"
    "    samplePos2  = center + pixelSize * vec2(-7.5);\n"
    "    samplePos3  = center + pixelSize * vec2(-5.5);\n"
    "    samplePos4  = center + pixelSize * vec2(-3.5);\n"
    "    samplePos5  = center + pixelSize * vec2(-1.5);\n"
    "    samplePos6  = center;\n"
    "    samplePos7  = center + pixelSize * vec2(1.5);\n"
    "    samplePos8  = center + pixelSize * vec2(3.5);\n"
    "    samplePos9  = center + pixelSize * vec2(5.5);\n"
    "    samplePos10 = center + pixelSize * vec2(7.5);\n"
    "    samplePos11 = center + pixelSize * vec2(9.5);\n"
    "    samplePos12 = center + pixelSize * vec2(11.5);\n"

    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "}\n";

/*
    This shader doesn't use the += operator because the old GLSL
    compiler in Mesa didn't emit MAD's when it was used.
    This isn't an issue with the new GLSL2 compiler.
*/
static const char fs_code[] =
    "uniform sampler2D texUnit;\n"

    "varying vec2 samplePos0;\n"
    "varying vec2 samplePos1;\n"
    "varying vec2 samplePos2;\n"
    "varying vec2 samplePos3;\n"
    "varying vec2 samplePos4;\n"
    "varying vec2 samplePos5;\n"
    "varying vec2 samplePos6;\n"
    "varying vec2 samplePos7;\n"
    "varying vec2 samplePos8;\n"
    "varying vec2 samplePos9;\n"
    "varying vec2 samplePos10;\n"
    "varying vec2 samplePos11;\n"
    "varying vec2 samplePos12;\n"

    "const vec4 kernel0 = vec4(0.00951198);\n"
    "const vec4 kernel1 = vec4(0.0236653);\n"
    "const vec4 kernel2 = vec4(0.0494943);\n"
    "const vec4 kernel3 = vec4(0.0870162);\n"
    "const vec4 kernel4 = vec4(0.128602);\n"
    "const vec4 kernel5 = vec4(0.15977);\n"
    "const vec4 kernel6 = vec4(0.0838822);\n"

    "void main(void)\n"
    "{\n"
    "    vec4 sum = texture2D(texUnit, samplePos0) * kernel0;\n"
    "    sum = sum + texture2D(texUnit, samplePos1) * kernel1;\n"
    "    sum = sum + texture2D(texUnit, samplePos2) * kernel2;\n"
    "    sum = sum + texture2D(texUnit, samplePos3) * kernel3;\n"
    "    sum = sum + texture2D(texUnit, samplePos4) * kernel4;\n"
    "    sum = sum + texture2D(texUnit, samplePos5) * kernel5;\n"
    "    sum = sum + texture2D(texUnit, samplePos6) * kernel6;\n"
    "    sum = sum + texture2D(texUnit, samplePos7) * kernel5;\n"
    "    sum = sum + texture2D(texUnit, samplePos8) * kernel4;\n"
    "    sum = sum + texture2D(texUnit, samplePos9) * kernel3;\n"
    "    sum = sum + texture2D(texUnit, samplePos10) * kernel2;\n"
    "    sum = sum + texture2D(texUnit, samplePos11) * kernel1;\n"
    "    sum = sum + texture2D(texUnit, samplePos12) * kernel0;\n"
    "    gl_FragColor = sum;\n"
    "}\n";

static const int expected_edge[] = {
    0x00, 0x01, 0x02, 0x05, 0x08, 0x0f, 0x15, 0x20, 0x2b, 0x3c, 0x4c, 0x60,
    0x75, 0x8a, 0x9f, 0xb3, 0xc3, 0xd4, 0xdf, 0xea, 0xf0, 0xf7, 0xfa, 0xfd,
    0xfe, 0xff
};

static const int expected_corner[] = {
    0x00, 0x01, 0x02, 0x04, 0x07, 0x0e, 0x17, 0x24, 0x36, 0x4b, 0x63, 0x7e,
    0x95, 0xb0, 0xc3, 0xd7, 0xe2, 0xef, 0xf5, 0xfb, 0xfd, 0xff
};

static GLuint setup_shaders()
{
    GLuint vs, fs, prog;

    vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
    fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
    prog = piglit_link_simple_program(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

static GLboolean test()
{
    GLboolean pass = GL_TRUE;

    /* Prepare the shaders */
    GLint prog       = setup_shaders();
    GLint uPixelSize = glGetUniformLocation(prog, "pixelSize");
    GLint uTexUnit   = glGetUniformLocation(prog, "texUnit");
    GLuint scratchTex;
    int i;

    /* Pixel sizes in texture coordinates for the horizontal and vertical passes */
    const float horizontal[2] = { 1.0 / WIDTH, 0 };
    const float vertical[2]   = { 0, 1.0 / HEIGHT };

    /* Texture and vertex coordinates */
    const float tc[] = { 0,1, 1,1, 0,0, 0,0, 1,1, 1,0 };
    const float vc[] = { -1,1, 1,1, -1,-1, -1,-1, 1,1, 1,-1 };

    /* Draw the rectangle that we're going to blur */
    piglit_draw_rect(-.5, -.5, 1, 1);

    /* Create a scratch texture */
    glGenTextures(1, &scratchTex);
    glBindTexture(GL_TEXTURE_2D, scratchTex);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);

    glUseProgram(prog);
    glUniform1i(uTexUnit, 0);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glTexCoordPointer(2, GL_FLOAT, 0, tc);
    glVertexPointer(2, GL_FLOAT, 0, vc);

    /* Horizontal pass */
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
    glUniform2fv(uPixelSize, 1, horizontal);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* Vertical pass */
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WIDTH, HEIGHT);
    glUniform2fv(uPixelSize, 1, vertical);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    /* Clean up */
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &scratchTex);
    glDeleteProgram(prog);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    assert(glGetError() == 0);

    /* Test the sides */
    for (i = 0; i < 26; i++) {
        float color[3];
        color[0] = expected_edge[i] / 255.;
        color[1] = color[0];
        color[2] = color[0];
        pass = piglit_probe_pixel_rgb(50, 12 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(50, HEIGHT - 13 - i, color) && pass;
        pass = piglit_probe_pixel_rgb(12 + i, 50, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 13 - i, 50, color) && pass;
    }

    /* Test the corners */
    for (i = 0; i < 22; i++) {
        float color[3];
        color[0] = expected_corner[i] / 255.;
        color[1] = color[0];
        color[2] = color[0];
        pass = piglit_probe_pixel_rgb(16 + i, 16 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(16 + i, HEIGHT - 17 - i, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 17 - i, 16 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 17 - i, HEIGHT - 17 - i, color) && pass;
    }

    return pass;
}

enum piglit_result piglit_display(void)
{
    GLboolean pass;

    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    pass = test();

    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
    int value;

    piglit_require_gl_version(20);

    glGetIntegerv(GL_MAX_VARYING_FLOATS, &value);
    if (value < (13 * 4)) {
        printf("Requires at least 13 varyings\n");
        piglit_report_result(PIGLIT_SKIP);
    }
}
