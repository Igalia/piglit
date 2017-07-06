/*
 * Copyright © 2016 Intel Corporation
 *
 * Based on blending-in-shader.c, by Marek Olšák
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

/** @file blending-in-shader-arb.c
 *
 * Test programmable blending with GL_ARB_texture_barrier.
 *
 * This test is equivalent to blending-in-shader.c, but:
 *
 * * Tests GL_ARB_texture_barrier instead of GL_NV_texture_barrier,
 *   that are totally equivalent.
 *
 * * Uses GL_ARB_framebuffer_object instead of
 *   GL_EXT_framebuffer_object. Those are slightly different, and
 *   several drivers doesn't support the EXT one.
 *
 * * This tests switches to use an integer texture, because that would
 *   allow us to do actual vs reference comparisons without the need
 *   of a tolerance.
 *
 * * Allows to parametrize several aspects: resolution, number of
 *   blending passes, number of drawing passes, square granularity
 *   (triangle count) and number of textures.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 31;
        config.supports_gl_core_version = 31;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

/* Texture stuff */
static GLuint fbo, prog;
static GLuint *tex;
static GLuint *texloc;

static GLuint **tex_data;
static GLuint **reference_data;

/* vertex data */
typedef struct {
        float data[4];
}Vertex;

unsigned int total_indices;
GLuint buf = 0;
GLuint vao = 0;
GLuint buf_index = 0;

#define MAX_NUM_TEXTURES 8
/* Command line parameters */
static int width = 0;
static int height = 0;
static int num_textures = 0;
static int blend_passes = 0;
static int granularity = 0;
static int draw_passes = 0;

static const char *vs_text = {
        "#version 130\n"
        "in vec4 piglit_vertex;"
        "void main() {\n"
        "  gl_Position = piglit_vertex;\n"
        "}\n"
};

static const char *texel_fetch_template = {
          "  color[%u] = texelFetch(fb[%u], ivec2(gl_FragCoord.xy), 0);\n"
};

static const char *fs_text_template = {
        "#version 130\n"
        "#define NUM_TEXTURES %i\n"
        "uniform usampler2D fb[NUM_TEXTURES];\n"
        "out uvec4 color[NUM_TEXTURES];\n"
        "void main() {\n"
        "%s"
        "  for (int t = 0; t < NUM_TEXTURES; t++){\n"
        "    color[t]++;\n"
        "  }\n"
        "}\n"
};

static void
allocate_data()
{
        unsigned int t;

        tex_data = malloc(sizeof(GLuint *) * num_textures);
        reference_data = malloc(sizeof(GLuint *) * num_textures);

        for (t = 0; t < num_textures; t++) {
                tex_data[t] = malloc(sizeof(GLuint) * width * height);
                reference_data[t] = malloc(sizeof(GLuint) * width * height);
        }
}

static bool
compare_with_reference()
{
        bool outcome = true;
        unsigned int t;
        GLuint *actual;

        actual = malloc(sizeof(GLuint) * width * height);

        for (t = 0; t < num_textures; t++) {
                unsigned int count = 0;
                unsigned int i;

                glActiveTexture(GL_TEXTURE0 + t);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, actual);

                for (i = 0; i < width * height; i++) {
                        if (reference_data[t][i] != actual[i]) {
                                count++;
                                outcome = false;
                        }
                }

                if (count > 0) {
                        fprintf(stderr, "Error on texture %u: %u texels out of "
                                "%u are different.\n", t, count, width * height);
                }
        }

        free(actual);

        return outcome;
}

static void
initialize_data()
{
        unsigned int i,j;
        unsigned int t;

        allocate_data();

        for (t = 0; t < num_textures; t++) {
                for (i = 0; i < width * height; i++) {
                        tex_data[t][i] = reference_data[t][i] = rand();
                        for (j = 0; j < blend_passes; j++)
                                reference_data[t][i] = reference_data[t][i] + 1;
                }

                glActiveTexture(GL_TEXTURE0 + t);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED_INTEGER,
                                GL_UNSIGNED_INT, tex_data[t]);
                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + t, tex[t], 0);
        }
        assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

static void
interpolate_square(const Vertex square[4],
                   unsigned int granularity,
                   Vertex *result)
{
        unsigned int x;
        unsigned int y;
        unsigned int b = granularity - 1;
        unsigned int k = 0;

        for (y = 0; y < granularity; y++) {
                for (x = 0; x < granularity; x++) {
                        result[k].data[0] = (square[0].data[0] * (b - x) + square[2].data[0] * x)/b;
                        result[k].data[1] = (square[0].data[1] * (b - y) + square[1].data[1] * y)/b;
                        result[k].data[2] = 0;
                        result[k].data[3] = 1;

                        k++;
                }
        }
}

/*
 * Tesselates @square_pos, returning a set of vertex information and
 * indices compatible to draw with glDrawElements at (@tesselated_pos,
 * @indices). @granularity is the number of vertexes per side. So the
 * minimum should be 2.
 *
 * Is assumed that the vertex at @square_pos are in the order
 * (bottom-left, top-left, top-right, bottom-right)
 *
 * Probably not the most optimal code to do this, but it works
 *
 * Returns: newly allocated memory at @tesselated_pos, and @indices.
 */
static void
util_tesselate_square(const Vertex square_pos[4],
                      unsigned int granularity,
                      Vertex **tesselated_pos,
                      unsigned int **indices)
{
        unsigned int x;
        unsigned int y;
        unsigned int k;
        Vertex *result_pos;
        unsigned int *result_indices;

        if (granularity < 2) {
                fprintf(stderr, "Granularity should be equal or greater to 2"
                        " in order to tesselate a square. Setting it to 2.\n");
                granularity = 2;
        }

        result_pos = (Vertex*) malloc(sizeof(Vertex) * granularity * granularity);
        total_indices = (granularity - 1) * (granularity - 1) * 6;
        result_indices = (unsigned int*) malloc(sizeof(unsigned int) * total_indices);

        interpolate_square(square_pos, granularity, result_pos);

        k = 0;
        for (y = 0; y < granularity - 1; y++) {
                for (x = 0; x < granularity - 1; x++) {
                        result_indices[k++] = y * granularity + x;
                        result_indices[k++] = y * granularity + x + 1;
                        result_indices[k++] = (y + 1) * granularity + x;

                        result_indices[k++] = y * granularity + x + 1;
                        result_indices[k++] = (y + 1) * granularity + x +1;
                        result_indices[k++] = (y + 1) * granularity + x;
                }
        }

        *tesselated_pos = result_pos;
        *indices = result_indices;
}

static GLvoid
initialize_vertex_data(float x, float y, float w, float h)
{
        Vertex verts[4];
        Vertex *tesselated_pos;
        unsigned int *indices;

        verts[0].data[0] = x;
        verts[0].data[1] = y;
        verts[1].data[0] = x;
        verts[1].data[1] = y + h;
        verts[2].data[0] = x + w;
        verts[2].data[1] = y + h;
        verts[3].data[0] = x + w;
        verts[3].data[1] = y;

        util_tesselate_square(verts, granularity,
                              &tesselated_pos,
                              &indices);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &buf);
        glBindBuffer(GL_ARRAY_BUFFER, buf);

        glBufferData(GL_ARRAY_BUFFER,
                     (sizeof(GLfloat) * 4 * granularity * granularity),
                     tesselated_pos,
                     GL_STATIC_DRAW);

        glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT,
                              GL_FALSE, 0,
                              0);
        glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);

        glGenBuffers(1, &buf_index);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf_index);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * total_indices,
                     indices, GL_STATIC_DRAW);

        free(tesselated_pos);
        free(indices);
}

static void
clean_resources()
{
        unsigned int t;

        glDeleteTextures(num_textures, tex);
        glDeleteProgram(prog);
        glDeleteFramebuffers(1, &fbo);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &buf);
        glDeleteBuffers(1, &buf_index);

        for (t = 0; t < num_textures; t++) {
                free(tex_data[t]);
                free(reference_data[t]);
        }

        free(tex_data);
        free(reference_data);
        free(texloc);
        free(tex);
}

static GLvoid
draw_rect_tex()
{
        /* This multiply and divide by three is a trick to ensure that
         * basic_count is a multiple of three */
        unsigned int basic_count = 3 * (total_indices / (3 * draw_passes));
        unsigned int first = 0;

        while (first < total_indices) {
                unsigned int count = MIN2(total_indices - first, basic_count);

                glDrawRangeElements(GL_TRIANGLES, first, first + count, count,
                                    GL_UNSIGNED_INT, BUFFER_OFFSET(first * sizeof(GLuint)));
                first += count;
        }
}

enum piglit_result
piglit_display(void)
{
        unsigned int i;
        bool outcome = false;

        glViewport(0, 0, width, height);

        for (i = 0; i < blend_passes; i++) {
                if (i != 0)
                        glTextureBarrier();
                draw_rect_tex();
        }

        outcome = compare_with_reference() ? PIGLIT_PASS : PIGLIT_FAIL;

        clean_resources();

        return outcome;
}

static void
initialize_textures()
{
        unsigned int t;

        tex = malloc(sizeof(GLuint) * num_textures);
        glGenTextures(num_textures, tex);
        for (t = 0; t < num_textures; t++) {
                glActiveTexture(GL_TEXTURE0 + t);
                glBindTexture(GL_TEXTURE_2D, tex[t]);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, width, height);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
}

static void
initialize_fbo()
{
        unsigned int t;
        GLenum draw_buffers[MAX_NUM_TEXTURES];

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        for (t = 0; t < num_textures; t++) {
                draw_buffers[t] = GL_COLOR_ATTACHMENT0 + t;
        }
        glDrawBuffers(num_textures, draw_buffers);
}

static void
initialize_program()
{
        char *whole_fetch_string = NULL;
        char *fs_text;
        int result;
        unsigned int t;

        for (t = 0; t < num_textures; t++) {
                char *one_fetch_string;

                result = asprintf(&one_fetch_string, texel_fetch_template, t, t);
                if (result == -1) {
                        fprintf(stderr, "Error creating fetch string from template.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }
                if (whole_fetch_string != NULL) {
                        char *tmp;
                        result = asprintf(&tmp, "%s%s", whole_fetch_string,
                                          one_fetch_string);
                        if (result == -1) {
                                fprintf(stderr, "Error concatenating fetch string "
                                        "from template.\n");
                                piglit_report_result(PIGLIT_FAIL);
                        }
                        free(whole_fetch_string);
                        free(one_fetch_string);
                        whole_fetch_string = tmp;
                } else {
                        whole_fetch_string = one_fetch_string;
                }
        }

        result = asprintf(&fs_text, fs_text_template, num_textures,
                          whole_fetch_string);
        if (result == -1) {
                fprintf(stderr, "Error creating shader from template.\n");
                piglit_report_result(PIGLIT_FAIL);
        }
        prog = piglit_build_simple_program(vs_text, fs_text);
        piglit_check_gl_error(GL_NO_ERROR);

        texloc = malloc(sizeof(GLuint) * num_textures);
        glUseProgram(prog);
        for (t = 0; t < num_textures; t++) {
                const char name_template[] = "fb[%u]";
                char *name;

                result = asprintf(&name, name_template, t);
                if (result == -1) {
                        fprintf(stderr, "Error creating uniform name from template.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }
                texloc[t] = glGetUniformLocation(prog, name);
                if (texloc[t] == -1) {
                        fprintf(stderr, "Error getting uniform %s.\n", name);
                        piglit_report_result(PIGLIT_FAIL);
                }
                glUniform1i(texloc[t], t);
                free(name);
        }

        free(fs_text);
        free(whole_fetch_string);
}

static void
print_usage()
{
        printf("Usage: arb_texture_barrier-blending-in-shader ");
        printf("<resolution> <blend_passes> <num_textures> <granularity> <common piglit args>\n");
        printf("\tresolution valid range is [1, 1024]\n");
        printf("\tblend_passes valid range is [1,42]\n");
        printf("\tnum_textures valid range is [1, %i]\n", MAX_NUM_TEXTURES);
        printf("\tgranularity (the number of vertices per side) valid range is [2, 256]\n");
        printf("\tdraw_passes valid range is [1, 10]\n");
}

static void
parse_args(int argc, char **argv)
{
        if (argc != 6) {
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        width = height = atoi(argv[1]);
        if (width < 1 || width > 1024) {
                fprintf(stderr, "Wrong value for resolution: %s\n", argv[1]);
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        blend_passes = atoi(argv[2]);
        if (blend_passes < 1 || blend_passes > 42) {
                fprintf(stderr, "Wrong value for blend_passes: %s\n", argv[2]);
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        num_textures = atoi(argv[3]);
        if (num_textures < 1 || num_textures > MAX_NUM_TEXTURES) {
                fprintf(stderr, "Wrong value for num_textures: %s\n", argv[3]);
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        granularity = atoi(argv[4]);
        if (granularity < 2 || granularity > 256) {
                fprintf(stderr, "Wrong value for granularity: %s\n", argv[4]);
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        draw_passes = atoi(argv[5]);
        if (draw_passes < 1 || draw_passes > 10) {
                fprintf(stderr, "Wrong value for draw_passes: %s\n", argv[5]);
                print_usage();
                piglit_report_result(PIGLIT_FAIL);
        }

        fprintf(stdout, "Executing test with the following parameters:\n"
                "resolution = %i\n"
                "blend_passes = %i\n"
                "num_textures = %i\n"
                "granularity = %i\n"
                "draw_passes = %i\n",
                width, blend_passes, num_textures, granularity, draw_passes);
}

void
piglit_init(int argc, char **argv)
{
        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_texture_barrier");
        piglit_require_GLSL_version(130);

        parse_args(argc, argv);

        srand(0);

        initialize_program();
        initialize_textures();
        initialize_fbo();
        initialize_data();
        initialize_vertex_data(-1, -1, 2, 2);
}
