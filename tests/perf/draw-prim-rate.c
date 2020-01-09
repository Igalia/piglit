/*
 * Copyright (C) 2018  Advanced Micro Devices, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VMWARE BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Measure primitive rate under various circumstances.
 *
 * Culling methods:
 * - none
 * - rasterizer discard
 * - face culling
 * - view culling
 * - degenerate primitives
 * - subpixel primitives
 */

#include "common.h"
#include <stdbool.h>
#undef NDEBUG
#include <assert.h>
#include "piglit-util-gl.h"

/* this must be a power of two to prevent precision issues */
#define WINDOW_SIZE 1024

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_width = WINDOW_SIZE;
	config.window_height = WINDOW_SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static unsigned gpu_freq_mhz;
static GLint progs[3];

void
piglit_init(int argc, char **argv)
{
	for (unsigned i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-freq=", 6) == 0)
			sscanf(argv[i] + 6, "%u", &gpu_freq_mhz);
	}

	piglit_require_gl_version(32);

	progs[0] = piglit_build_simple_program(
			  "#version 120 \n"
			  "void main() { \n"
			  "  gl_Position = gl_Vertex; \n"
			  "}",

			  "#version 120 \n"
			  "void main() { \n"
			  "  gl_FragColor = vec4(1.0); \n"
			  "}");

	progs[1] = piglit_build_simple_program(
			  "#version 150 compatibility \n"
			  "varying vec4 v[4]; \n"
			  "attribute vec4 a[4]; \n"
			  "void main() { \n"
			  "  for (int i = 0; i < 4; i++) v[i] = a[i]; \n"
			  "  gl_Position = gl_Vertex; \n"
			  "}",

			  "#version 150 compatibility \n"
			  "varying vec4 v[4]; \n"
			  "void main() { \n"
			  "  gl_FragColor = vec4(dot(v[0] + v[1] + v[2] + v[3], vec4(1.0)) == 1.0 ? 0.0 : 1.0); \n"
			  "}");

	progs[2] = piglit_build_simple_program(
			  "#version 150 compatibility \n"
			  "varying vec4 v[8]; \n"
			  "attribute vec4 a[8]; \n"
			  "void main() { \n"
			  "  for (int i = 0; i < 8; i++) v[i] = a[i]; \n"
			  "  gl_Position = gl_Vertex; \n"
			  "}",

			  "#version 150 compatibility \n"
			  "varying vec4 v[8]; \n"
			  "void main() { \n"
			  "  gl_FragColor = vec4(dot(v[0] + v[1] + v[2] + v[3] + v[4] + v[5] + v[6] + v[7], vec4(1.0)) == 1.0 ? 0.0 : 1.0); \n"
			  "}");

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnable(GL_CULL_FACE);
}

static void
gen_triangle_tile(unsigned num_quads_per_dim, double prim_size_in_pixels,
		  unsigned cull_percentage,
		  bool back_face_culling, bool view_culling, bool degenerate_prims,
		  unsigned max_vertices, unsigned *num_vertices, float *vertices,
		  unsigned max_indices, unsigned *num_indices, unsigned *indices)
{
	/* clip space coordinates in both X and Y directions: */
	const double first = -1;
	const double max_length = 2;
	const double d = prim_size_in_pixels * 2.0 / WINDOW_SIZE;

	assert(d * num_quads_per_dim <= max_length);
	assert(*num_vertices == 0);

	/* the vertex ordering is counter-clockwise */
	for (unsigned ty = 0; ty < num_quads_per_dim; ty++) {
		bool cull;

		if (cull_percentage == 0)
			cull = false;
		else if (cull_percentage == 25)
			cull = ty % 4 == 0;
		else if (cull_percentage == 50)
			cull = ty % 2 == 0;
		else if (cull_percentage == 75)
			cull = ty % 4 != 0;
		else if (cull_percentage == 100)
			cull = true;
		else
			assert(!"wrong cull_percentage");

		for (unsigned tx = 0; tx < num_quads_per_dim; tx++) {
			unsigned x = tx;
			unsigned y = ty;

			/* view culling in different directions */
			double xoffset = 0, yoffset = 0, zoffset = 0;

			if (cull && view_culling) {
				unsigned side = (ty / 2) % 4;

				if (side == 0)		xoffset = -2;
				else if (side == 1)	xoffset =  2;
				else if (side == 2)	yoffset = -2;
				else if (side == 3)	yoffset =  2;
			}

			if (indices) {
				unsigned elem = *num_vertices * 3;

				/* generate horizontal stripes with maximum reuse */
				if (x == 0) {
					*num_vertices += 2;
					assert(*num_vertices <= max_vertices);

					vertices[elem++] = xoffset + first + d * x;
					vertices[elem++] = yoffset + first + d * y;
					vertices[elem++] = zoffset;

					vertices[elem++] = xoffset + first + d * x;
					vertices[elem++] = yoffset + first + d * (y + 1);
					vertices[elem++] = zoffset;
				}

				int base_index = *num_vertices;

				*num_vertices += 2;
				assert(*num_vertices <= max_vertices);

				vertices[elem++] = xoffset + first + d * (x + 1);
				vertices[elem++] = yoffset + first + d * y;
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * (x + 1);
				vertices[elem++] = yoffset + first + d * (y + 1);
				vertices[elem++] = zoffset;

				/* generate indices */
				unsigned idx = *num_indices;
				*num_indices += 6;
				assert(*num_indices <= max_indices);

				indices[idx++] = base_index - 2;
				indices[idx++] = base_index;
				indices[idx++] = base_index - 1;

				indices[idx++] = base_index - 1;
				indices[idx++] = base_index;
				indices[idx++] = base_index + 1;

				if (cull && back_face_culling) {
					/* switch the winding order */
					unsigned tmp = indices[idx - 6];
					indices[idx - 6] = indices[idx - 5];
					indices[idx - 5] = tmp;

					tmp = indices[idx - 3];
					indices[idx - 3] = indices[idx - 2];
					indices[idx - 2] = tmp;
				}

				if (cull && degenerate_prims) {
					indices[idx - 5] = indices[idx - 4];
					indices[idx - 2] = indices[idx - 1];
				}
			} else {
				unsigned elem = *num_vertices * 3;
				*num_vertices += 6;
				assert(*num_vertices <= max_vertices);

				vertices[elem++] = xoffset + first + d * x;
				vertices[elem++] = yoffset + first + d * y;
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * (x + 1);
				vertices[elem++] = yoffset + first + d * y;
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * x;
				vertices[elem++] = yoffset + first + d * (y + 1);
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * x;
				vertices[elem++] = yoffset + first + d * (y + 1);
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * (x + 1);
				vertices[elem++] = yoffset + first + d * y;
				vertices[elem++] = zoffset;

				vertices[elem++] = xoffset + first + d * (x + 1);
				vertices[elem++] = yoffset + first + d * (y + 1);
				vertices[elem++] = zoffset;

				if (cull && back_face_culling) {
					/* switch the winding order */
					float old[6*3];
					memcpy(old, vertices + elem - 6*3, 6*3*4);

					for (unsigned i = 0; i < 6; i++) {
						vertices[elem - 6*3 + i*3 + 0] = old[(5 - i)*3 + 0];
						vertices[elem - 6*3 + i*3 + 1] = old[(5 - i)*3 + 1];
						vertices[elem - 6*3 + i*3 + 2] = old[(5 - i)*3 + 2];
					}
				}

				if (cull && degenerate_prims) {
					/* use any previously generated vertices */
					unsigned v0 = rand() % *num_vertices;
					unsigned v1 = rand() % *num_vertices;

					memcpy(&vertices[elem - 5*3], &vertices[v0*3], 12);
					memcpy(&vertices[elem - 4*3], &vertices[v0*3], 12);

					memcpy(&vertices[elem - 2*3], &vertices[v1*3], 12);
					memcpy(&vertices[elem - 1*3], &vertices[v1*3], 12);
				}
			}
		}
	}
}

static void
gen_triangle_strip_tile(unsigned num_quads_per_dim, double prim_size_in_pixels,
			unsigned cull_percentage,
			bool back_face_culling, bool view_culling, bool degenerate_prims,
			unsigned max_vertices, unsigned *num_vertices, float *vertices)
{
	/* clip space coordinates in both X and Y directions: */
	const double first = -1;
	const double max_length = 2;
	const double d = prim_size_in_pixels * 2.0 / WINDOW_SIZE;

	assert(d * num_quads_per_dim <= max_length);
	assert(*num_vertices == 0);

	/* the vertex ordering is counter-clockwise */
	for (unsigned y = 0; y < num_quads_per_dim; y++) {
		bool cull;

		if (cull_percentage == 0)
			cull = false;
		else if (cull_percentage == 25)
			cull = y % 4 == 0;
		else if (cull_percentage == 50)
			cull = y % 2 == 0;
		else if (cull_percentage == 75)
			cull = y % 4 != 0;
		else if (cull_percentage == 100)
			cull = true;
		else
			assert(!"wrong cull_percentage");

		/* view culling in different directions */
		double xoffset = 0, yoffset = 0, zoffset = 0;

		if (cull && view_culling) {
			unsigned side = (y / 2) % 4;

			if (side == 0)		xoffset = -2;
			else if (side == 1)	xoffset =  2;
			else if (side == 2)	yoffset = -2;
			else if (side == 3)	yoffset =  2;
		}

		if (cull && degenerate_prims) {
			unsigned elem = *num_vertices * 3;
			*num_vertices += 2 + num_quads_per_dim * 2;
			assert(*num_vertices <= max_vertices);

			for (unsigned x = 0; x < 2 + num_quads_per_dim * 2; x++) {
				vertices[elem++] = 0;
				vertices[elem++] = 0;
				vertices[elem++] = 0;
			}
			continue;
		}

		unsigned elem = *num_vertices * 3;
		bool add_degenerates = y > 0;
		*num_vertices += (add_degenerates ? 4 : 0) + 2 + num_quads_per_dim * 2;
		assert(*num_vertices <= max_vertices);

		unsigned x = 0;
		unsigned y0 = y;
		unsigned y1 = y + 1;

		if (cull && back_face_culling) {
			y0 = y + 1;
			y1 = y;
		}

		/* Add degenerated triangles to connect with the previous triangle strip. */
		if (add_degenerates) {
			unsigned base = elem;

			vertices[elem++] = vertices[base - 3];
			vertices[elem++] = vertices[base - 2];
			vertices[elem++] = vertices[base - 1];
		}

		for (unsigned i = 0; i < (add_degenerates ? 4 : 1); i++) {
			vertices[elem++] = xoffset + first + d * x;
			vertices[elem++] = yoffset + first + d * y1;
			vertices[elem++] = zoffset;
		}

		vertices[elem++] = xoffset + first + d * x;
		vertices[elem++] = yoffset + first + d * y0;
		vertices[elem++] = zoffset;

		for (; x < num_quads_per_dim; x++) {
			vertices[elem++] = xoffset + first + d * (x + 1);
			vertices[elem++] = yoffset + first + d * y1;
			vertices[elem++] = zoffset;

			vertices[elem++] = xoffset + first + d * (x + 1);
			vertices[elem++] = yoffset + first + d * y0;
			vertices[elem++] = zoffset;
		}
	}
}

enum draw_method {
	INDEXED_TRIANGLES,
	TRIANGLES,
	TRIANGLE_STRIP,
	NUM_DRAW_METHODS,
};

static enum draw_method global_draw_method;
static unsigned count;
static unsigned num_duplicates;
static unsigned duplicate_index;
static unsigned vb_size, ib_size;

static void
run_draw(unsigned iterations)
{
	for (unsigned i = 0; i < iterations; i++) {
		if (global_draw_method == INDEXED_TRIANGLES) {
			glDrawElements(GL_TRIANGLES, count,
				       GL_UNSIGNED_INT,
				       (void*)(long)(ib_size * duplicate_index));
		} else if (global_draw_method == TRIANGLES) {
			glDrawArrays(GL_TRIANGLES, (vb_size / 12) * duplicate_index, count);
		} else if (global_draw_method == TRIANGLE_STRIP) {
			glDrawArrays(GL_TRIANGLE_STRIP, (vb_size / 12) * duplicate_index, count);
		}

		duplicate_index = (duplicate_index + 1) % num_duplicates;
	}
}

enum cull_method {
	NONE,
	BACK_FACE_CULLING,
	VIEW_CULLING,
	SUBPIXEL_PRIMS,
	RASTERIZER_DISCARD,
	DEGENERATE_PRIMS,
	NUM_CULL_METHODS,
};

static double
run_test(unsigned debug_num_iterations, enum draw_method draw_method,
	 enum cull_method cull_method, unsigned num_quads_per_dim,
	 double quad_size_in_pixels, unsigned cull_percentage)
{
	const unsigned max_indices = 8100000 * 3;
	const unsigned max_vertices = max_indices;

	while (num_quads_per_dim * quad_size_in_pixels >= WINDOW_SIZE)
		quad_size_in_pixels *= 0.5;

	/* Generate vertices. */
	float *vertices = (float*)malloc(max_vertices * 12);
	unsigned *indices = NULL;

	if (draw_method == INDEXED_TRIANGLES)
		indices = (unsigned*)malloc(max_indices * 4);

	unsigned num_vertices = 0, num_indices = 0;
	if (draw_method == TRIANGLE_STRIP) {
		gen_triangle_strip_tile(num_quads_per_dim, quad_size_in_pixels,
					cull_percentage,
					cull_method == BACK_FACE_CULLING,
					cull_method == VIEW_CULLING,
					cull_method == DEGENERATE_PRIMS,
					max_vertices, &num_vertices, vertices);
	} else {
		gen_triangle_tile(num_quads_per_dim, quad_size_in_pixels,
				  cull_percentage,
				  cull_method == BACK_FACE_CULLING,
				  cull_method == VIEW_CULLING,
				  cull_method == DEGENERATE_PRIMS,
				  max_vertices, &num_vertices, vertices,
				  max_indices, &num_indices, indices);
	}

	vb_size = num_vertices * 12;
	ib_size = num_indices * 4;

	/* Duplicate buffers and switch between them, so that no data is cached
	 * between draws. 32 MB should be greater than any cache.
	 */
	num_duplicates = MAX2(1, 32*1024*1024 / vb_size);

	/* Create buffers. */
	GLuint vb, ib;
	glGenBuffers(1, &vb);
	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glBufferData(GL_ARRAY_BUFFER,
		     vb_size * num_duplicates, NULL, GL_STATIC_DRAW);
	for (unsigned i = 0; i < num_duplicates; i++)
		glBufferSubData(GL_ARRAY_BUFFER, vb_size * i, vb_size, vertices);
	free(vertices);

	if (draw_method == INDEXED_TRIANGLES) {
		glGenBuffers(1, &ib);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			     ib_size * num_duplicates, NULL,
			     GL_STATIC_DRAW);
		for (unsigned i = 0; i < num_duplicates; i++) {
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, ib_size * i,
					ib_size, indices);
		}
		free(indices);
	}
	/* Make sure all uploads are finished. */
	glFinish();

	/* Test */
	if (cull_method == RASTERIZER_DISCARD)
		glEnable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, vb);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	if (draw_method == INDEXED_TRIANGLES)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib);

	global_draw_method = draw_method;
	count = draw_method == INDEXED_TRIANGLES ? num_indices : num_vertices;
	duplicate_index = 0;

	double rate = 0;

	if (debug_num_iterations)
		run_draw(debug_num_iterations);
	else
		rate = perf_measure_rate(run_draw, 0.15);

	if (cull_method == RASTERIZER_DISCARD)
		glDisable(GL_RASTERIZER_DISCARD);

	/* Cleanup. */
	glDeleteBuffers(1, &vb);
	if (draw_method == INDEXED_TRIANGLES)
		glDeleteBuffers(1, &ib);
	return rate;
}

static void
run(enum draw_method draw_method, enum cull_method cull_method,
    const unsigned *num_quads_per_dim, const unsigned *num_prims,
    unsigned num_prim_sets)
{
	unsigned num_subtests = 1;
	static unsigned cull_percentages[] = {100, 75, 50, 25};
	static double quad_sizes_in_pixels[] = {1.0 / 7, 0.25, 0.5};

	if (cull_method == BACK_FACE_CULLING ||
	    cull_method == VIEW_CULLING ||
	    cull_method == DEGENERATE_PRIMS) {
		num_subtests = ARRAY_SIZE(cull_percentages);
	} else if (cull_method == SUBPIXEL_PRIMS) {
		num_subtests = ARRAY_SIZE(quad_sizes_in_pixels);
	}

	for (unsigned subtest = 0; subtest < num_subtests; subtest++) {
		/* 2 is the maximum prim size when everything fits into the window */
		double quad_size_in_pixels;
		unsigned cull_percentage;

		if (cull_method == SUBPIXEL_PRIMS) {
			quad_size_in_pixels = quad_sizes_in_pixels[subtest];
			cull_percentage = 0;
		} else {
			quad_size_in_pixels = 2;
			cull_percentage = cull_percentages[subtest];
		}

		printf("  %-14s, ",
		       draw_method == INDEXED_TRIANGLES ? "glDrawElements" :
		       draw_method == TRIANGLES ? "glDrawArraysT" : "glDrawArraysTS");

		if (cull_method == NONE ||
		    cull_method == RASTERIZER_DISCARD) {
			printf("%-21s",
			       cull_method == NONE ? "none" : "rasterizer discard");
		} else if (cull_method == SUBPIXEL_PRIMS) {
			printf("%2u small prims/pixel ",
			       (unsigned)((1.0 / quad_size_in_pixels) *
					  (1.0 / quad_size_in_pixels) * 2));
		} else {
			printf("%3u%% %-16s", cull_percentage,
			       cull_method == BACK_FACE_CULLING ? "back faces" :
				cull_method == VIEW_CULLING ?	  "culled by view" :
				cull_method == DEGENERATE_PRIMS ? "degenerate prims" :
								  "(error)");
		}
		fflush(stdout);

		for (unsigned prog = 0; prog < ARRAY_SIZE(progs); prog++) {
			glUseProgram(progs[prog]);

			if (prog)
				printf("   ");

			for (int i = 0; i < num_prim_sets; i++) {
				double rate = run_test(0, draw_method, cull_method,
						       num_quads_per_dim[i],
						       quad_size_in_pixels, cull_percentage);
				rate *= num_prims[i];

				if (gpu_freq_mhz) {
					rate /= gpu_freq_mhz * 1000000.0;
					printf(",%6.2f", rate);
				} else {
					printf(",%6.2f", rate / 1000000000);
				}
				fflush(stdout);
			}
		}
		printf("\n");
	}
}

enum piglit_result
piglit_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* for debugging */
	if (getenv("ONE")) {
		glUseProgram(progs[0]);
		run_test(1, TRIANGLE_STRIP, BACK_FACE_CULLING, ceil(sqrt(0.5 * 512000)), 2, 50);
		piglit_swap_buffers();
		return PIGLIT_PASS;
	}

	const unsigned num_quads_per_dim[] = {
		/* The second number is the approx. number of primitives. */
		ceil(sqrt(0.5 * 1000)),
		ceil(sqrt(0.5 * 2000)),
		ceil(sqrt(0.5 * 4000)),
		ceil(sqrt(0.5 * 6000)),
		ceil(sqrt(0.5 * 8000)),
		ceil(sqrt(0.5 * 16000)),
		ceil(sqrt(0.5 * 32000)),
		ceil(sqrt(0.5 * 128000)),
		ceil(sqrt(0.5 * 512000)),
		/* 512000 is the maximum number when everything fits into the window */
		/* After that, the prim size decreases, so you'll get subpixel prims. */
		ceil(sqrt(0.5 * 2000000)),
		ceil(sqrt(0.5 * 8000000)),
	};

	unsigned num_prims[ARRAY_SIZE(num_quads_per_dim)];
	for (int i = 0; i < ARRAY_SIZE(num_quads_per_dim); i++)
		num_prims[i] = num_quads_per_dim[i] * num_quads_per_dim[i] * 2;

	printf("  Measuring %-27s,    0 Varying                                                                       4 Varyings                                                                      8 Varyings\n",
	       gpu_freq_mhz ? "Prims/clock," : "GPrims/second,");
	printf("  Draw Call     ,  Cull Method         ");

	for (unsigned prog = 0; prog < ARRAY_SIZE(progs); prog++) {
		if (prog)
			printf("   ");
		for (int i = 0; i < ARRAY_SIZE(num_prims); i++)
			printf(", %4uK", num_prims[i] / 1000);
	}
	printf("\n");

	for (int draw_method = 0; draw_method < NUM_DRAW_METHODS; draw_method++) {
		for (int cull_method = 0; cull_method < NUM_CULL_METHODS; cull_method++)
			run(draw_method, cull_method, num_quads_per_dim, num_prims, ARRAY_SIZE(num_prims));
	}

	exit(0);
	return PIGLIT_SKIP;
}
