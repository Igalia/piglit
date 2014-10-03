/*
 * Copyright (C) 2014 Intel Corporation
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
 */

/** @file grid.c
 *
 * Utility code for running a grid of shader invocations abstracting
 * out the details of the specific shader stage it's run on.
 */

#include "common.h"

char *
concat(char *hunk0, ...)
{
        char *s = hunk0;
        char *hunk;
        va_list ap;

        va_start(ap, hunk0);

        while ((hunk = va_arg(ap, char *))) {
                char *t = s;
                asprintf(&s, "%s\n%s", t, hunk);
                free(t);
                free(hunk);
        }

        va_end(ap);
        return s;
}

char *
image_hunk(const struct image_info img, const char *prefix)
{
        char *s = NULL;

        asprintf(&s,
                 "#define %sBASE_T %s\n"
                 "#define %sDATA_T %s\n"
                 "#define %sSCALE vec4(%.8e, %.8e, %.8e, %.8e)\n"
                 "#define %sIMAGE_ADDR_(addr_t, ext, i) %s\n"
                 "#define %sIMAGE_ADDR(idx)"
                 "        %sIMAGE_ADDR_(%s, ivec4(%d, %d, %d, %d),"
                 "                      ((idx).x + W * (idx).y))\n"
                 "#define %sIMAGE_LAYOUT_Q layout(%s)\n"
                 "#define %sIMAGE_BARE_T %s%s\n"
                 "#define %sIMAGE_T %sIMAGE_LAYOUT_Q %sIMAGE_BARE_T\n",
                 prefix, image_scalar_type_name(img.format),
                 prefix, image_vector_type_name(img.format),
                 prefix, image_format_scale(img.format).x,
                 image_format_scale(img.format).y,
                 image_format_scale(img.format).z,
                 image_format_scale(img.format).w,
                 prefix, (image_target_samples(img.target) > 1 ?
                          "addr_t(ivec3(i / ext.x % ext.y,"
                          "             i / ext.x / ext.y % ext.z,"
                          "             i / ext.x / ext.y / ext.z)),"
                          "(i % ext.x)" :
                          "addr_t(ivec3(i % ext.x,"
                          "             i / ext.x % ext.y,"
                          "             i / ext.x / ext.y))"),
                 prefix, prefix, img.target->addr_type_name,
                 img.size.x, img.size.y, img.size.z, img.size.w,
                 prefix, img.format->name,
                 prefix, image_type_name(img.format), img.target->name,
                 prefix, prefix, prefix);
        return s;
}

static char *
header_hunk(const struct grid_info grid)
{
        char *s = NULL;

        asprintf(&s, "#version 150\n"
                 "#extension GL_ARB_shader_image_load_store : enable\n"
                 "#define W %d\n"
                 "#define H %d\n"
                 "#define N %d\n"
                 "#define GRID_T %s\n"
                 "#define RET_IMAGE_T layout(%s) %s2D\n",
                 grid.size.x, grid.size.y, product(grid.size),
                 image_vector_type_name(grid.format),
                 grid.format->name, image_type_name(grid.format));
        return s;
}

static char *
generate_stage_source(const struct grid_info grid,
                      unsigned stage, const char *_body)
{
        char *header = header_hunk(grid);
        char *body = hunk(_body ? _body :
                          "GRID_T op(ivec2 idx, GRID_T x) {\n"
                          "        return x;\n"
                          "}\n");

        switch (stage) {
        case GL_VERTEX_SHADER:
                return concat(
                        header, body,
                        hunk("in vec4 piglit_vertex;\n"
                             "out ivec2 vidx;\n"
                             "flat out GRID_T vcolor;\n"
                             "\n"
                             "void main() {\n"
                             "        ivec2 idx = ivec2((piglit_vertex + 1.0).xy *"
                             "                          vec2(W, H) / 2);\n"
                             "\n"
                             "        vcolor = op(idx, GRID_T(0));\n"
                             "        vidx = idx;\n"
                             "        gl_Position = piglit_vertex;\n"
                             "}\n"), NULL);

        case GL_TESS_CONTROL_SHADER:
                return concat(
                        header,
                        hunk("#extension GL_ARB_tessellation_shader : enable\n"),
                        body,
                        hunk("layout(vertices=4) out;\n"
                             "\n"
                             "in ivec2 vidx[];\n"
                             "flat in GRID_T vcolor[];\n"
                             "out ivec2 tcidx[];\n"
                             "out GRID_T tccolor[];\n"
                             "\n"
                             "void main() {\n"
                             "        if (gl_InvocationID == 0) {\n"
                             "                /* No subdivisions, thanks. */\n"
                             "                gl_TessLevelInner[0] = 1;\n"
                             "                gl_TessLevelInner[1] = 1;\n"
                             "                gl_TessLevelOuter[0] = 1;\n"
                             "                gl_TessLevelOuter[1] = 1;\n"
                             "                gl_TessLevelOuter[2] = 1;\n"
                             "                gl_TessLevelOuter[3] = 1;\n"
                             "        }\n"
                             "        tccolor[gl_InvocationID] ="
                             "               op(vidx[gl_InvocationID],"
                             "                  vcolor[gl_InvocationID]);\n"
                             "        tcidx[gl_InvocationID] = vidx[gl_InvocationID];\n"
                             "        gl_out[gl_InvocationID].gl_Position ="
                             "               gl_in[gl_InvocationID].gl_Position;\n"
                             "}\n"), NULL);

        case GL_TESS_EVALUATION_SHADER:
                return concat(
                        header,
                        hunk("#extension GL_ARB_tessellation_shader : enable\n"),
                        body,
                        hunk("layout(quads, point_mode) in;\n"
                             "\n"
                             "in ivec2 tcidx[];\n"
                             "in GRID_T tccolor[];\n"
                             "out ivec2 teidx;\n"
                             "flat out GRID_T tecolor;\n"
                             "\n"
                             "void main() {\n"
                             "        int idx = ((gl_TessCoord.x > 0.5 ? 1 : 0) +"
                             "                   (gl_TessCoord.y > 0.5 ? 2 : 0));\n"
                             "\n"
                             "        tecolor = op(tcidx[idx], tccolor[idx]);\n"
                             "        teidx = tcidx[idx];\n"
                             "        gl_Position = gl_in[idx].gl_Position;\n"
                             "}\n"), NULL);

        case GL_GEOMETRY_SHADER:
                return concat(
                        header,
                        hunk(grid.stages & (GL_TESS_CONTROL_SHADER_BIT |
                                            GL_TESS_EVALUATION_SHADER_BIT) ?
                             "#define IN(name) te##name\n" :
                             "#define IN(name) v##name\n"),
                        body,
                        hunk("layout(points) in;\n"
                             "layout(points, max_vertices=1) out;\n"
                             "\n"
                             "in ivec2 IN(idx)[];\n"
                             "flat in GRID_T IN(color)[];\n"
                             "flat out GRID_T gcolor;\n"
                             "\n"
                             "void main() {\n"
                             "        gcolor = op(IN(idx)[0], IN(color)[0]);\n"
                             "        gl_Position = gl_in[0].gl_Position;\n"
                             "        EmitVertex();\n"
                             "}\n"), NULL);

        case GL_FRAGMENT_SHADER:
                return concat(
                        header,
                        hunk(grid.stages & (GL_TESS_CONTROL_SHADER_BIT |
                                            GL_TESS_EVALUATION_SHADER_BIT |
                                            GL_GEOMETRY_SHADER_BIT) ?
                             "#define IN(name) g##name\n" :
                             "#define IN(name) v##name\n"),
                        body,
                        hunk("flat in GRID_T IN(color);\n"
                             "out GRID_T fcolor;\n"
                             "\n"
                             "void main() {\n"
                             "        fcolor = op(ivec2(gl_FragCoord), IN(color));\n"
                             "}\n"), NULL);

        case GL_COMPUTE_SHADER:
                return concat(
                        header,
                        hunk("#extension GL_ARB_compute_shader : enable\n"),
                        body,
                        hunk("layout (local_size_x = W) in;\n"
                             "\n"
                             "uniform RET_IMAGE_T ret_img;\n"
                             "\n"
                             "void main() {\n"
                             "       ivec2 idx = ivec2(gl_GlobalInvocationID);\n"
                             "       GRID_T x = op(idx, GRID_T(0));\n"
                             "       imageStore(ret_img, idx, x);\n"
                             "}\n"), NULL);

        default:
                abort();
        }
}

static inline unsigned
get_stage_idx(const struct image_stage_info *stage)
{
        return stage - image_stages();
}

/**
 * Generate a full program pipeline using the shader code provided in
 * the \a sources array.
 */
static GLuint
generate_program_v(const struct grid_info grid, const char **sources)
{
        const unsigned basic_stages = (GL_FRAGMENT_SHADER_BIT |
                                       GL_VERTEX_SHADER_BIT);
        const unsigned tess_stages = (GL_TESS_CONTROL_SHADER_BIT |
                                      GL_TESS_EVALUATION_SHADER_BIT);
        const unsigned graphic_stages = (basic_stages | tess_stages |
                                         GL_GEOMETRY_SHADER_BIT);
        const unsigned stages =
                (grid.stages |
                 /* Make a full pipeline if a tesselation shader was
                  * requested. */
                 (grid.stages & tess_stages ? graphic_stages : 0) |
                 /* Make sure there is always a vertex and fragment
                  * shader if we're doing graphics. */
                 (grid.stages & graphic_stages ? basic_stages : 0));
        GLuint prog = glCreateProgram();
        const struct image_stage_info *stage;

        for (stage = image_stages(); stage->stage; ++stage) {
                if (stages & stage->bit) {
                        char *source = generate_stage_source(
                                grid, stage->stage,
                                sources[get_stage_idx(stage)]);
                        GLuint shader = piglit_compile_shader_text_nothrow(
                                stage->stage, source);

                        free(source);

                        if (!shader) {
                                glDeleteProgram(prog);
                                return 0;
                        }

                        glAttachShader(prog, shader);
                        glDeleteShader(shader);
                }
        }

        glBindAttribLocation(prog, PIGLIT_ATTRIB_POS, "piglit_vertex");
        glBindAttribLocation(prog, PIGLIT_ATTRIB_TEX, "piglit_texcoord");
        glLinkProgram(prog);

        if (!piglit_link_check_status(prog)) {
                glDeleteProgram(prog);
                return 0;
        }

        return prog;
}

GLuint
generate_program(const struct grid_info grid, ...)
{
        char *sources[6] = { NULL };
        va_list ap;
        unsigned stages, i;
        GLuint prog;

        va_start(ap, grid);

        for (stages = grid.stages; stages;) {
                const struct image_stage_info *stage =
                        get_image_stage(va_arg(ap, GLenum));
                char *source = va_arg(ap, char *);

                if (stage) {
                        assert(get_stage_idx(stage) < ARRAY_SIZE(sources));
                        sources[get_stage_idx(stage)] = source;
                        stages &= ~stage->bit;
                }
        }

        va_end(ap);

        prog = generate_program_v(grid, (const char **)sources);

        for (i = 0; i < ARRAY_SIZE(sources); ++i)
                free(sources[i]);

        return prog;
}

bool
draw_grid(const struct grid_info grid, GLuint prog)
{
        static GLuint lprog;

        if (lprog != prog) {
                glUseProgram(prog);
                lprog = prog;
        }

        if (grid.stages & GL_COMPUTE_SHADER_BIT) {
                set_uniform_int(prog, "ret_img", 7);
                glDispatchCompute(1, grid.size.y, 1);

        } else if (grid.stages & (GL_TESS_CONTROL_SHADER_BIT |
                                  GL_TESS_EVALUATION_SHADER_BIT)) {
                static struct image_extent size;
                static GLuint vao, vbo;

                if (size.x != grid.size.x || size.y != grid.size.y) {
                        size = grid.size;

                        if (!generate_grid_arrays(
                                    &vao, &vbo,
                                    1.0 / size.x - 1.0, 1.0 / size.y - 1.0,
                                    2.0 / size.x, 2.0 / size.y,
                                    size.x, size.y))
                                return false;
                }

                glBindVertexArray(vao);
                glPatchParameteri(GL_PATCH_VERTICES, 4);
                glDrawArrays(GL_PATCHES, 0, product(size));

        } else if (grid.stages & (GL_VERTEX_SHADER_BIT |
                                  GL_GEOMETRY_SHADER_BIT)) {
                static struct image_extent size;
                static GLuint vao, vbo;

                if (size.x != grid.size.x || size.y != grid.size.y) {
                        size = grid.size;

                        if (!generate_grid_arrays(
                                    &vao, &vbo,
                                    1.0 / size.x - 1.0, 1.0 / size.y - 1.0,
                                    2.0 / size.x, 2.0 / size.y,
                                    size.x, size.y))
                                return false;
                }

                glBindVertexArray(vao);
                glDrawArrays(GL_POINTS, 0, product(size));

        } else {
                static struct image_extent size;
                static GLuint vao, vbo;

                if (size.x != grid.size.x || size.y != grid.size.y) {
                        float vp[4];

                        glGetFloati_v(GL_VIEWPORT, 0, vp);
                        size = grid.size;

                        if (!generate_grid_arrays(
                                    &vao, &vbo, -1.0, -1.0,
                                    2.0 * size.x / vp[2], 2.0 * size.y / vp[3],
                                    2, 2))
                                return false;
                }


                glBindVertexArray(vao);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        return piglit_check_gl_error(GL_NO_ERROR);
}

bool
generate_grid_arrays(GLuint *vao, GLuint *vbo,
                     float x, float y, float dx, float dy,
                     unsigned nx, unsigned ny)
{
        float (*verts)[4] = malloc(sizeof(*verts) * nx * ny);
        int i, j;

        for (i = 0; i < nx; ++i) {
                for (j = 0; j < ny; ++j) {
                        const unsigned k = (nx * (j & ~1) + 2 * (i & ~1) +
                                            (i & 1) + 2 * (j & 1));
                        verts[k][0] = x + i * dx;
                        verts[k][1] = y + j * dy;
                        verts[k][2] = 0.0;
                        verts[k][3] = 1.0;
                }
        }

        if (!*vao) {
                glGenVertexArrays(1, vao);
                glGenBuffers(1, vbo);
        }

        glBindVertexArray(*vao);
        glBindBuffer(GL_ARRAY_BUFFER, *vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(*verts) * nx * ny,
                     verts, GL_STATIC_DRAW);

        glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT,
                              GL_FALSE, 0, 0);
        glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);

        free(verts);
        return piglit_check_gl_error(GL_NO_ERROR);
}
