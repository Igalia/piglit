/*
 * Copyright © 2001 Brian Paul
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Based on the Mesa demo "texwrap" by Brian Paul.
 * Reworked and extended by Marek Olšák.
 *
 * This is more than just a test of wrap modes.
 *
 * Besides all the wrap modes, it tests:
 *
 * - 1D, 2D, 3D, and RECT texture targets.
 *
 * - Many formats, see the list below.
 *   Especially the border color might need to be set up differently
 *   for each format in hardware. Also, some hardware might not support
 *   clamp-to-border and clamp for some formats. We need to make sure all
 *   useful formats are appropriately covered here.
 *   The test is skipped if the format chosen by GL is not the same
 *   as the requested format.
 *
 * - Non-power-of-two textures.
 *   Some drivers have a special shader-based code path for NPOT textures.
 *
 * - Projective texture mapping.
 *   This is also useful to verify the correctness of shader-based wrap modes
 *   for some hardware.
 *   Both projective and non-projective mapping is tested in the automatic
 *   mode.
 *
 * - Texture borders (marked as deprecated in GL3, removed in the Core profile).
 *
 ****************************************************************************
 *
 * Parameters:
 *   One of: 1d, 2d, 3d, rect
 *   One of: See the list of formats below.
 *   Any of: npot, border
 *
 * Each parameter must begin with a hyphen and all parameters must be
 * concatenated into one string. The whole string must end with a hyphen
 * as well.
 *
 * Examples:
 *   -3d-rgba8-border-
 *   -2d-rgba16f-npot-
 *   -rect-rgb10a2-
 *
 * Default:
 *   -2d-rgba8-
 */

#include "piglit-util.h"

#ifndef GL_RGBA16F
#define GL_RGBA16F 0x881A
#endif
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif
#ifndef GL_TEXTURE_SWIZZLE_RGBA_EXT
#define GL_TEXTURE_SWIZZLE_RGBA_EXT 0x8E46
#endif

/* Formats. */

struct format {
    const char  *name;
    const char  *option;
    GLenum      internalformat;
    int         red, green, blue, alpha;
    unsigned    nearest_deltamax, linear_deltamax;
    float       version;
    const char  *extensions[2];
} formats[] = {
    {"RGBA8",    "-rgba8-",   GL_RGBA8,    8, 8, 8, 8,     1,  8,  1.1},
    {"RGBA4",    "-rgba4-",   GL_RGBA4,    4, 4, 4, 4,     17, 17, 1.1},
    {"RGB565",   "-rgb565",   GL_RGB5,     5, 6, 5, 0,     9,  9,  1.1},
    {"RGB5_A1",  "-rgb5a1-",  GL_RGB5_A1,  5, 5, 5, 1,     9,  9,  1.1},
    {"RGB10_A2", "-rgb10a2-", GL_RGB10_A2, 10, 10, 10, 2,  1,  8,  1.1},
    {"RGBA16",   "-rgba16-",  GL_RGBA16,   16, 16, 16, 16, 1,  8,  1.1},
    {"RGBA16F",  "-rgba16f-", GL_RGBA16F,  16, 16, 16, 16, 1,  8,  3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},
    {"RGBA32F",  "-rgba32f-", GL_RGBA32F,  32, 32, 32, 32, 1,  8,  3.0,
     {"GL_ARB_texture_float", "GL_ATI_texture_float"}},

    {NULL}
};

/* Wrap modes. */

struct wrap_mode {
    GLenum      mode;
    const char  *name;
    GLboolean   valid_for_rect;
    float       version;
    const char  *extensions[2];
    GLboolean   supported;
} wrap_modes[] = {
    {GL_REPEAT,                     "REPEAT",                     GL_FALSE,
     1.1, {NULL, NULL}},

    {GL_CLAMP,                      "CLAMP",                      GL_TRUE,
     1.1, {NULL, NULL}},

    {GL_CLAMP_TO_EDGE,              "CLAMP_TO_EDGE",              GL_TRUE,
     1.2, {"GL_EXT_texture_edge_clamp", "GL_SGIS_texture_edge_clamp"}},

    {GL_CLAMP_TO_BORDER,            "CLAMP_TO_BORDER",            GL_TRUE,
     1.3, {"GL_ARB_texture_border_clamp", "GL_SGIS_texture_border_clamp"}},

    {GL_MIRRORED_REPEAT,            "MIRRORED_REPEAT",            GL_FALSE,
     1.4, {"GL_ARB_texture_mirrored_repeat", "GL_IBM_texture_mirrored_repeat"}},

    {GL_MIRROR_CLAMP_EXT,           "MIRROR_CLAMP_EXT",           GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once"}},

    {GL_MIRROR_CLAMP_TO_EDGE_EXT,   "MIRROR_CLAMP_TO_EDGE_EXT",   GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", "GL_ATI_texture_mirror_once"}},

    {GL_MIRROR_CLAMP_TO_BORDER_EXT, "MIRROR_CLAMP_TO_BORDER_EXT", GL_FALSE,
     999.0, {"GL_EXT_texture_mirror_clamp", NULL}},

    {0}
};

/* Defines. */
#define BORDER_TEXTURE      1
#define NO_BORDER_TEXTURE   2
#define BIAS_INT            (texture_size+2)
#define BIAS                (BIAS_INT / (double)texture_size)
#define TEXEL_SIZE          3
#define TILE_SIZE           ((BIAS_INT*2 + texture_size) * TEXEL_SIZE)
#define TILE_SPACE          5
#define SIZE_POT            8
#define SIZE_NPOT           9
#define SIZEMAX             (SIZE_POT > SIZE_NPOT ? SIZE_POT : SIZE_NPOT)

/* Test parameters and state. */
static GLuint texture_id;
static GLenum texture_target;
static GLboolean texture_npot;
static GLboolean texture_proj;
static GLboolean texture_swizzle;
static int texture_size;
static struct format *texture_format;
static GLboolean has_texture_swizzle;

/* Image data. */
static const int swizzle[4] = {2, 0, 1, 3};
static const float borderf[4] = { 0.0, 1.0, 0.5, 1.0 };
static float border_image[SIZEMAX * SIZEMAX * SIZEMAX * 4];
static float no_border_image[(SIZEMAX+2) * (SIZEMAX+2) * (SIZEMAX+2) * 4];

/* Derived from texture_size, texture_target, and border. */
static int size_x = 1, size_y = 1, size_z = 1;          /* size */
static int bsize_x = 1, bsize_y = 1, bsize_z = 1;       /* size + 2*border */
static int border_x = 0, border_y = 0, border_z = 0;    /* 0 or 1 */

/* Piglit stuff. */
int piglit_width = 872, piglit_height = 230;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;
extern int piglit_automatic;


static void print_string(const char *s)
{
    while (*s) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, (int) *s);
        s++;
    }
}

static void sample_nearest(int x, int y, int z,
                           GLenum wrap_mode, GLenum filter,
                           unsigned char pixel[4])
{
    unsigned sample_border;
    float border_factor = 0;
    int coords[3] = {x, y, z};
    unsigned i;
    float result[4];

    /* Zero coords according to the texture target. */
    switch (texture_target) {
    case GL_TEXTURE_1D:
        coords[1] = 0;
        /* Pass through. */

    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE_NV:
        coords[2] = 0;
    }

    /* Resolve clamp mirroring. */
    switch (wrap_mode) {
    case GL_MIRROR_CLAMP_EXT:
    case GL_MIRROR_CLAMP_TO_EDGE_EXT:
    case GL_MIRROR_CLAMP_TO_BORDER_EXT:
        for (i = 0; i < 3; i++) {
            if (coords[i] < 0) {
                coords[i] = -coords[i] - 1;
            }
        }
    }

    /* Resolve border sampling. */
    switch (wrap_mode) {
    case GL_CLAMP:
    case GL_MIRROR_CLAMP_EXT:
        if (filter != GL_LINEAR) {
            break;
        }

    case GL_CLAMP_TO_BORDER:
    case GL_MIRROR_CLAMP_TO_BORDER_EXT:
        sample_border = 0;
        for (i = 0; i < 3; i++) {
            if (coords[i] >= texture_size || coords[i] < 0) {
                sample_border++;
            }
        }
    }

    /* Resolve wrapping. */
    switch (wrap_mode) {
    case GL_REPEAT:
        for (i = 0; i < 3; i++) {
            coords[i] = (coords[i] + texture_size*10) % texture_size;
        }
        break;

    case GL_CLAMP:
    case GL_MIRROR_CLAMP_EXT:
        if (filter == GL_LINEAR) {
            const double factor[] = {0, 0.5, 0.75, 0.875};
            border_factor = factor[sample_border];
        }
        /* Pass through. */

    case GL_CLAMP_TO_EDGE:
    case GL_MIRROR_CLAMP_TO_EDGE_EXT:
        for (i = 0; i < 3; i++) {
            coords[i] = coords[i] >= texture_size ? texture_size-1 :
                coords[i] < 0 ? 0 : coords[i];
        }
        break;

    case GL_CLAMP_TO_BORDER:
    case GL_MIRROR_CLAMP_TO_BORDER_EXT:
        if (sample_border) {
            border_factor = 1;
        }
        break;

    case GL_MIRRORED_REPEAT:
        for (i = 0; i < 3; i++) {
            coords[i] = (coords[i] + texture_size*10) % (texture_size * 2);
            if (coords[i] >= texture_size)
                coords[i] = 2*texture_size - coords[i] - 1;
        }
        break;
    }

    /* Sample the pixel. */
    if (texture_id == BORDER_TEXTURE) {
        /* Do not sample the per-pixel border. */
        switch (texture_target) {
        case GL_TEXTURE_3D:
            coords[2] += 1;
            /* Pass through. */
        case GL_TEXTURE_2D:
            coords[1] += 1;
            /* Pass through. */
        case GL_TEXTURE_1D:
            coords[0] += 1;
            break;
        default:
            assert(0);
        }

        memcpy(result,
               &border_image[(coords[2]*bsize_y*bsize_x +
                              coords[1]*bsize_x + coords[0])*4],
               sizeof(result));
    } else {
        memcpy(result,
               &no_border_image[(coords[2]*size_y*size_x +
                                 coords[1]*size_x + coords[0])*4],
               sizeof(result));
    }

    /* Sample the border.
     * This is actually the only place we care about linear filtering,
     * for CLAMP. Pixels are expected to be sampled at their center,
     * so we don't have to take 4 samples. */
    if (sample_border) {
        for (i = 0; i < 3; i++)
            result[i] = borderf[i] * border_factor +
                        result[i] * (1 - border_factor);
    }

    if (texture_swizzle) {
        for (i = 0; i < 4; i++) {
            pixel[i] = result[swizzle[i]] * 255.1;
        }
    } else {
        for (i = 0; i < 4; i++) {
            pixel[i] = result[i] * 255.1;
        }
    }
}

GLboolean probe_pixel_rgba(unsigned char *pixels, unsigned stride,
                           unsigned pixels_deltamax,
                           unsigned x, unsigned y, unsigned char *expected,
                           unsigned a, unsigned b)
{
    int deltamax = 0;
    unsigned i;
    unsigned char *probe = &pixels[(y * stride + x) * 4];

    for (i = 0; i < 3; ++i) {
        int delta = abs((int)probe[i] - (int)expected[i]);
        if (delta > deltamax)
            deltamax = delta;
    }

    if (deltamax <= pixels_deltamax)
        return GL_TRUE;

    printf("Probe at (%i,%i) @ %i,%i\n", x, y, a, b);
    printf("  Expected: %i %i %i\n", expected[0], expected[1], expected[2]);
    printf("  Observed: %i %i %i\n", probe[0], probe[1], probe[2]);
    return GL_FALSE;
}

static void update_swizzle()
{
    GLint iden[4] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
    GLint swiz[4] = {iden[swizzle[0]], iden[swizzle[1]], iden[swizzle[2]],
                     iden[swizzle[3]]};
    glBindTexture(texture_target, texture_id);
    if (texture_swizzle) {
        glTexParameteriv(texture_target, GL_TEXTURE_SWIZZLE_RGBA_EXT, swiz);
    } else {
        glTexParameteriv(texture_target, GL_TEXTURE_SWIZZLE_RGBA_EXT, iden);
    }
}

static void draw()
{
    unsigned i, j;
    int offset;

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(texture_target, texture_id);

    /* Loop over min/mag filters. */
    for (i = 0; i < 2; i++) {
        offset = 0;

        if (i) {
            glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(texture_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(texture_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        /* Loop over wrap modes. */
        for (j = 0; wrap_modes[j].mode != 0; j++) {
            float x0 = 0;
            float y0 = 0;
            float x1 = TILE_SIZE;
            float y1 = TILE_SIZE;
            float s0 = -BIAS;
            float t0 = -BIAS;
            float s1 = 1 + BIAS;
            float t1 = 1 + BIAS;
            float q = 1;
            float ts0 = s0, ts1 = s1, tt0 = t0, tt1 = t1, tr = 0.5;

            if (!wrap_modes[j].supported)
                continue;

            /* Projective texturing. */
            if (texture_proj) {
                q = 2.3;
                ts0 *= q;
                ts1 *= q;
                tt0 *= q;
                tt1 *= q;
                tr *= q;
            }

            /* Rectangles. */
            if (texture_target == GL_TEXTURE_RECTANGLE_NV) {
                ts0 *= texture_size;
                ts1 *= texture_size;
                tt0 *= texture_size;
                tt1 *= texture_size;
            }

            glTexParameteri(texture_target, GL_TEXTURE_WRAP_S,
                            wrap_modes[j].mode);
            glTexParameteri(texture_target, GL_TEXTURE_WRAP_T,
                            wrap_modes[j].mode);
            if (texture_target == GL_TEXTURE_3D) {
                glTexParameteri(texture_target, GL_TEXTURE_WRAP_R,
                                wrap_modes[j].mode);
            }

            glPushMatrix();
            glTranslatef(offset * (TILE_SIZE + TILE_SPACE) + 5,
                         i * (TILE_SIZE + TILE_SPACE) + 35,
                         0);
            offset++;

            glEnable(texture_target);
            glColor3f(1, 1, 1);
            glBegin(GL_POLYGON);
            glTexCoord4f(ts0, tt0, tr, q);  glVertex2f(x0, y0);
            glTexCoord4f(ts1, tt0, tr, q);  glVertex2f(x1, y0);
            glTexCoord4f(ts1, tt1, tr, q);  glVertex2f(x1, y1);
            glTexCoord4f(ts0, tt1, tr, q);  glVertex2f(x0, y1);
            glEnd();
            glDisable(texture_target);

            /* Draw red outline showing bounds of texture at s=0,1 and t=0,1. */
            if (!piglit_automatic) {
                glColor3f(1, 0, 0);
                glBegin(GL_LINE_LOOP);
                glVertex2f(x0 + BIAS * (x1-x0) / (s1-s0),
                           y0 + BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x1 - BIAS * (x1-x0) / (s1-s0),
                           y0 + BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x1 - BIAS * (x1-x0) / (s1-s0),
                           y1 - BIAS * (y1-y0) / (t1-t0));
                glVertex2f(x0 + BIAS * (x1-x0) / (s1-s0),
                           y1 - BIAS * (y1-y0) / (t1-t0));
                glEnd();
            }

            glPopMatrix();
        }
    }

    glDisable(texture_target);
    glColor3f(1, 1, 1);
    offset = 0;

    if (!piglit_automatic) {
        for (i = 0; wrap_modes[i].mode != 0; i++) {
            if (wrap_modes[i].supported) {
                glWindowPos2iARB(offset * (TILE_SIZE + TILE_SPACE) + 5,
                                 5 + ((offset & 1) * 15));
                print_string(wrap_modes[i].name);
                offset++;
            }
        }
    }
}

static GLboolean probe_pixels()
{
    unsigned i, j;
    unsigned char *pixels;
    GLboolean pass = GL_TRUE;

    pixels = malloc(piglit_width * piglit_height * 4);
    glReadPixels(0, 0, piglit_width, piglit_height,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // make slices different for 3D textures

    /* Loop over min/mag filters. */
    for (i = 0; i < 2; i++) {
        GLenum filter = i ? GL_LINEAR : GL_NEAREST;
        const char *sfilter = i ? "LINEAR" : "NEAREST";
        unsigned deltamax = i ? texture_format->linear_deltamax :
                                texture_format->nearest_deltamax;
        unsigned offset = 0;

        for (j = 0; wrap_modes[j].mode != 0; j++) {
            unsigned char expected[4];
            int x0 = offset * (TILE_SIZE + TILE_SPACE) + 5;
            int y0 = i * (TILE_SIZE + TILE_SPACE) + 35;
            int a, b;

            if (!wrap_modes[j].supported)
                continue;

            printf("Testing %s%s%s%s: %s\n",
                   sfilter,
                   texture_proj ? " with projective mapping" : "",
                   texture_swizzle ? (texture_proj ? " and" : " with") : "",
                   texture_swizzle ? " swizzling" : "",
                   wrap_modes[j].name);

            for (b = 0; b < (texture_size + BIAS_INT*2); b++) {
                for (a = 0; a < (texture_size + BIAS_INT*2); a++) {
                    double x = x0 + TEXEL_SIZE*(a+0.5);
                    double y = y0 + TEXEL_SIZE*(b+0.5);

                    sample_nearest(a - BIAS_INT, b - BIAS_INT,
                                   0, /* the slices are the same */
                                   wrap_modes[j].mode, filter, expected);

                    if (!probe_pixel_rgba(pixels, piglit_width, deltamax,
                                          x, y, expected, a, b)) {
                        pass = GL_FALSE;
                        goto tile_done;
                    }
                }
            }

        tile_done:
            offset++;
        }
    }

    free(pixels);
    return pass;
}

static GLboolean test_simple()
{
    GLboolean pass;
    draw();
    pass = probe_pixels();
    glutSwapBuffers();
    return pass;
}

static GLboolean test_proj()
{
    GLboolean pass = GL_TRUE;
    pass = test_simple() && pass;
    texture_proj = 1;
    pass = test_simple() && pass;
    texture_proj = 0;
    return pass;
}

static GLboolean test_swizzle()
{
    GLboolean pass = GL_TRUE;
    pass = test_proj() && pass;
    if (has_texture_swizzle) {
        texture_swizzle = 1;
        update_swizzle();
        pass = test_proj() && pass;
        texture_swizzle = 0;
        update_swizzle();
    }
    return pass;
}

enum piglit_result piglit_display()
{
    GLboolean pass = GL_TRUE;

    if (piglit_automatic) {
        pass = test_swizzle();
    } else {
        if (has_texture_swizzle) {
            update_swizzle();
        }
        pass = test_simple();
    }

    return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

static void key_func(unsigned char key, int x, int y)
{
    switch (key) {
    case 'b':
        if (texture_target == GL_TEXTURE_RECTANGLE_NV) {
            printf("The texture border cannot be used with rectangle "
                   "textures.\n");
            return;
        }

        texture_id = texture_id == NO_BORDER_TEXTURE ?
                     BORDER_TEXTURE : NO_BORDER_TEXTURE;

        printf(texture_id == NO_BORDER_TEXTURE ?
               "Without the border.\n" : "With the border.\n");
        break;

    case 'p':
        texture_proj = !texture_proj;
        break;

    case 's':
        texture_swizzle = !texture_swizzle;
        printf(texture_swizzle ?
               "Texture swizzle enabled.\n" : "Texture swizzle disabled.\n");
        break;
    }

    piglit_escape_exit_key(key, x, y);
}

static GLboolean check_support(float version, const char *extensions[2])
{
    float glversion = atof((char *)glGetString(GL_VERSION));

    if (version <= glversion ||
        (extensions[0] && glutExtensionSupported(extensions[0])) ||
        (extensions[1] && glutExtensionSupported(extensions[1]))) {
        return GL_TRUE;
    }

    return GL_FALSE;
}

static void init_textures()
{
    int x, y, z;

    switch (texture_target) {
    case GL_TEXTURE_3D:
        size_z = texture_size;
        bsize_z = texture_size+2;
        border_z = 1;
        /* Pass through. */
    case GL_TEXTURE_2D:
    case GL_TEXTURE_RECTANGLE_NV:
        size_y = texture_size;
        bsize_y = texture_size+2;
        border_y = 1;
        /* Pass through. */
    case GL_TEXTURE_1D:
        size_x = texture_size;
        bsize_x = texture_size+2;
        border_x = 1;
    }

    if ((!piglit_automatic || texture_id == BORDER_TEXTURE) &&
        texture_target != GL_TEXTURE_RECTANGLE_NV) {
        for (z = 0; z < bsize_z; z++) {
            for (y = 0; y < bsize_y; y++) {
                for (x = 0; x < bsize_x; x++) {
                    unsigned i = (z*bsize_y*bsize_x + y*bsize_x + x)*4;

                    if (y == border_y && x == border_x) {
                        /* lower-left texel = RED */
                        border_image[i + 0] = 1;
                        border_image[i + 1] = 0;
                        border_image[i + 2] = 0;
                        border_image[i + 3] = 1;
                    } else if (y == border_y && x == bsize_x-border_x-1) {
                        /* lower-right corner = CYAN */
                        border_image[i + 0] = 0;
                        border_image[i + 1] = 1;
                        border_image[i + 2] = 1;
                        border_image[i + 3] = 1;
                    } else if (y == bsize_y-border_y-1 && x == border_x) {
                        /* upper-left corner = BLUE */
                        border_image[i + 0] = 0;
                        border_image[i + 1] = 0;
                        border_image[i + 2] = 1;
                        border_image[i + 3] = 1;
                    } else if (y == bsize_y-border_y-1 &&
                               x == bsize_x-border_x-1) {
                        /* upper-right corner = ORANGE */
                        border_image[i + 0] = 1;
                        border_image[i + 1] = 0.6;
                        border_image[i + 2] = 0.3;
                        border_image[i + 3] = 1;
                    } else if ((z == 0 && border_z) ||
                               (z == bsize_z-1 && border_z) ||
                               (y == 0 && border_y) ||
                               (y == bsize_y-1 && border_y) ||
                               (x == 0 && border_x) ||
                               (x == bsize_x-1 && border_x)) {
                        /* border color */
                        border_image[i + 0] = borderf[0];
                        border_image[i + 1] = borderf[1];
                        border_image[i + 2] = borderf[2];
                        border_image[i + 3] = borderf[3];
                    } else if ((y + x - !border_y) & 1) {
                        /* white */
                        border_image[i + 0] = 1;
                        border_image[i + 1] = 1;
                        border_image[i + 2] = 1;
                        border_image[i + 3] = 1;
                    } else {
                        /* black */
                        border_image[i + 0] = 0;
                        border_image[i + 1] = 0;
                        border_image[i + 2] = 0;
                        border_image[i + 3] = 0;
                    }
                }
            }
        }

        glBindTexture(texture_target, BORDER_TEXTURE);
        switch (texture_target) {
        case GL_TEXTURE_1D:
            glTexImage1D(texture_target, 0, texture_format->internalformat,
                         bsize_x, 1,
                         GL_RGBA, GL_FLOAT, (void *) border_image);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE_NV:
            glTexImage2D(texture_target, 0, texture_format->internalformat,
                         bsize_x, bsize_y, 1,
                         GL_RGBA, GL_FLOAT, (void *) border_image);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(texture_target, 0, texture_format->internalformat,
                         bsize_x, bsize_y, bsize_z, 1,
                         GL_RGBA, GL_FLOAT, (void *) border_image);
            break;
        }
    }

    if (!piglit_automatic || texture_id == NO_BORDER_TEXTURE) {
        for (z = 0; z < size_z; z++) {
            for (y = 0; y < size_y; y++) {
                for (x = 0; x < size_x; x++) {
                    unsigned i = (z*size_y*size_x + y*size_x + x)*4;

                    if (y == 0 && x == 0) {
                        /* lower-left texel = RED */
                        no_border_image[i + 0] = 1;
                        no_border_image[i + 1] = 0;
                        no_border_image[i + 2] = 0;
                        no_border_image[i + 3] = 1;
                    } else if (y == 0 && x == size_x-1) {
                        /* lower-right corner = CYAN */
                        no_border_image[i + 0] = 0;
                        no_border_image[i + 1] = 1;
                        no_border_image[i + 2] = 1;
                        no_border_image[i + 3] = 1;
                    } else if (y == size_y-1 && x == 0) {
                        /* upper-left corner = BLUE */
                        no_border_image[i + 0] = 0;
                        no_border_image[i + 1] = 0;
                        no_border_image[i + 2] = 1;
                        no_border_image[i + 3] = 1;
                    } else if (y == size_y-1 && x == size_x-1) {
                        /* upper-right corner = ORANGE */
                        no_border_image[i + 0] = 1;
                        no_border_image[i + 1] = 0.6;
                        no_border_image[i + 2] = 0.3;
                        no_border_image[i + 3] = 1;
                    } else if ((y + x) & 1) {
                        /* white */
                        no_border_image[i + 0] = 1;
                        no_border_image[i + 1] = 1;
                        no_border_image[i + 2] = 1;
                        no_border_image[i + 3] = 1;
                    } else {
                        /* black */
                        no_border_image[i + 0] = 0;
                        no_border_image[i + 1] = 0;
                        no_border_image[i + 2] = 0;
                        no_border_image[i + 3] = 0;
                    }
                }
            }
        }

        glBindTexture(texture_target, NO_BORDER_TEXTURE);
        glTexParameterfv(texture_target, GL_TEXTURE_BORDER_COLOR, borderf);
        switch (texture_target) {
        case GL_TEXTURE_1D:
            glTexImage1D(texture_target, 0, texture_format->internalformat,
                         size_x, 0,
                         GL_RGBA, GL_FLOAT, (void *) no_border_image);
            break;

        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE_NV:
            glTexImage2D(texture_target, 0, texture_format->internalformat,
                         size_x, size_y, 0,
                         GL_RGBA, GL_FLOAT, (void *) no_border_image);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(texture_target, 0, texture_format->internalformat,
                         size_x, size_y, size_z, 0,
                         GL_RGBA, GL_FLOAT, (void *) no_border_image);
            break;
        }
    }
}

GLboolean is_format_supported(struct format *f)
{
    GLuint id;
    float p[4] = {0};
    int r, g, b, a, iformat;
    GLboolean res;

    if (!check_support(f->version, f->extensions))
        return GL_FALSE;

    /* A quick and dirty way to check if we get the format we want. */
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, f->internalformat, 1, 1, 0,
                 GL_RGBA, GL_FLOAT, &p);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &r);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &g);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &b);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &a);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT,
                             &iformat);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &id);

    res = r == f->red && g == f->green && b == f->blue && a == f->alpha &&
          iformat == f->internalformat;

    printf("%s is R%iG%iB%iA%i. The internal format is 0x%04X.\n",
           f->name, r, g, b, a, iformat);
    if (!res) {
        printf("The real format appears to be different from the requested "
               "format.\n"
               "Skipping.\n");
    }

    return res;
}

void piglit_init(int argc, char **argv)
{
    unsigned i;
    const char *ext_swizzle[] = {
        "GL_ARB_texture_swizzle",
        "GL_EXT_texture_swizzle"
    };

    texture_target = GL_TEXTURE_2D;
    texture_id = NO_BORDER_TEXTURE;
    texture_npot = 0;
    texture_proj = 0;
    texture_swizzle = 0;
    texture_format = &formats[0];
    has_texture_swizzle = check_support(3.3, ext_swizzle);

    if (argc == 2) {
        printf("Parameter: %s\n", argv[1]);

        /* Texture targets. */
        if (strstr(argv[1], "-1d-")) {
            texture_target = GL_TEXTURE_1D;
            printf("Using TEXTURE_1D.\n");
    
        } else if (strstr(argv[1], "-3d-")) {
            const char *extensions[2] = {"GL_EXT_texture3D", NULL};
            if (!check_support(1.2, extensions))
                piglit_report_result(PIGLIT_SKIP);
    
            texture_target = GL_TEXTURE_3D;
            printf("Using TEXTURE_3D.\n");
    
        } else if (strstr(argv[1], "-rect-")) {
            const char *extensions[2] = {"GL_ARB_texture_rectangle",
                                         "GL_NV_texture_rectangle"};
            if (!check_support(3.1, extensions))
                piglit_report_result(PIGLIT_SKIP);
    
            texture_target = GL_TEXTURE_RECTANGLE_NV;
            texture_npot = GL_TRUE; /* Enforce NPOT dimensions. */
            printf("Using TEXTURE_RECTANGLE.\n");
    
        } else {
            texture_target = GL_TEXTURE_2D;
            printf("Using TEXTURE_2D.\n");
        }
    
        /* Parameters: npot, border */
        if (strstr(argv[1], "-npot-")) {
            piglit_require_extension("GL_ARB_texture_non_power_of_two");
            texture_npot = 1;
            printf("Using NPOT dimensions.\n");
        }
        if (strstr(argv[1], "-border-")) {
            assert(texture_target != GL_TEXTURE_RECTANGLE_NV);
            texture_id = BORDER_TEXTURE;
            printf("Using the border.\n");
        }

        /* Formats. */
        for (i = 0; formats[i].name; i++) {
            if (strstr(argv[1], formats[i].option)) {
                if (!is_format_supported(&formats[i]))
                    piglit_report_result(PIGLIT_SKIP);

                texture_format = &formats[i];
                printf("Using %s.\n", formats[i].name);
            }
        }
    }

    texture_size = texture_npot ? SIZE_NPOT : SIZE_POT;

    /* Check wrap extensions. */
    for (i = 0 ; wrap_modes[i].mode != 0 ; i++) {
        if (texture_target == GL_TEXTURE_RECTANGLE_NV &&
            !wrap_modes[i].valid_for_rect) {
            wrap_modes[i].supported = GL_FALSE;
        } else {
            wrap_modes[i].supported =
                check_support(wrap_modes[i].version, wrap_modes[i].extensions);
        }
    }

    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    glutKeyboardFunc(key_func);

    glClearColor(0.5, 0.5, 0.5, 1.0);

    init_textures();
}
