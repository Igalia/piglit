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

/** @file glsl-kwin-blur-1.c
 *
 * Tests the blur effect used by the KWin window manager,
 * with a 6 pixel blur radius (uses 7 varyings).
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
          with a 6 pixel blur radius. The code generator makes sure
          that the code doesn't exceed GL_MAX_VARYING_FLOATS.
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

    "void main(void)\n"
    "{\n"
    "    vec2 center = vec4(gl_TextureMatrix[0] * gl_MultiTexCoord0).st;\n"

    "    samplePos0 = center + pixelSize * vec2(-5.5);\n"
    "    samplePos1 = center + pixelSize * vec2(-3.5);\n"
    "    samplePos2 = center + pixelSize * vec2(-1.5);\n"
    "    samplePos3 = center;\n"
    "    samplePos4 = center + pixelSize * vec2(1.5);\n"
    "    samplePos5 = center + pixelSize * vec2(3.5);\n"
    "    samplePos6 = center + pixelSize * vec2(5.5);\n"

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

    "const vec4 kernel0 = vec4(0.0242836);\n"
    "const vec4 kernel1 = vec4(0.11585);\n"
    "const vec4 kernel2 = vec4(0.275987);\n"
    "const vec4 kernel3 = vec4(0.167758);\n"

    "void main(void)\n"
    "{\n"
    "    vec4 sum = texture2D(texUnit, samplePos0) * kernel0;\n"
    "    sum = sum + texture2D(texUnit, samplePos1) * kernel1;\n"
    "    sum = sum + texture2D(texUnit, samplePos2) * kernel2;\n"
    "    sum = sum + texture2D(texUnit, samplePos3) * kernel3;\n"
    "    sum = sum + texture2D(texUnit, samplePos4) * kernel2;\n"
    "    sum = sum + texture2D(texUnit, samplePos5) * kernel1;\n"
    "    sum = sum + texture2D(texUnit, samplePos6) * kernel0;\n"
    "    gl_FragColor = sum;\n"
    "}\n";

static const int expected_edge[] = {
    0x00, 0x03, 0x06, 0x15, 0x24, 0x47, 0x6a, 0x95, 0xb8, 0xdb, 0xea, 0xf9,
    0xfc, 0xff
};

static const int expected_corner[] = {
    0x00, 0x02, 0x05, 0x14, 0x2c, 0x57, 0x85, 0xbc, 0xd7, 0xf3, 0xf9, 0xff
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
    for (i = 0; i < 14; i++) {
        float color[3];
        color[0] = expected_edge[i] / 255.;
        color[1] = color[0];
        color[2] = color[0];
        pass = piglit_probe_pixel_rgb(50, 18 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(50, HEIGHT - 19 - i, color) && pass;
        pass = piglit_probe_pixel_rgb(18 + i, 50, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 19 - i, 50, color) && pass;
    }

    /* Test the corners */
    for (i = 0; i < 12; i++) {
        float color[3];
        color[0] = expected_corner[i] / 255.;
        color[1] = color[0];
        color[2] = color[0];
        pass = piglit_probe_pixel_rgb(20 + i, 20 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(20 + i, HEIGHT - 21 - i, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 21 - i, 20 + i, color) && pass;
        pass = piglit_probe_pixel_rgb(WIDTH - 21 - i, HEIGHT - 21 - i, color) && pass;
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
	piglit_require_gl_version(20);
}
