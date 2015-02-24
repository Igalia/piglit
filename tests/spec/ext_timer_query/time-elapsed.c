/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file time-elapsed.c
 *
 * Test TIME_ELAPSED and TIMESTAMP queries.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static char *vs_text =
	"#version 110\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"}\n";

/**
 * Time-wasting fragment shader.  This fragment shader computes:
 *
 *   x = (sum(i=0..(iters-1): 2*i) % iters) / iters
 *
 * This value should always work out to 0.0, but it's highly unlikely
 * that an optimizer will figure this out.  Hence we can use this
 * shader to waste an arbitrary amount of time (by suitable choice of
 * the value of iters).
 *
 * The shader outputs a color of (x, 1.0, 0.0, 0.0).
 */
static char *fs_text =
	"#version 110\n"
	"uniform int iters;\n"
	"void main()\n"
	"{\n"
	"  int cumulative_sum = 0;\n"
	"  for (int i = 0; i < iters; ++i) {\n"
	"    cumulative_sum += i;\n"
	"    if (cumulative_sum >= iters)\n"
	"      cumulative_sum -= iters;\n"
	"    cumulative_sum += i;\n"
	"    if (cumulative_sum >= iters)\n"
	"      cumulative_sum -= iters;\n"
	"  }\n"
	"  float x = float(cumulative_sum) / float(iters);\n"
	"  gl_FragColor = vec4(x, 1.0, 0.0, 0.0);\n"
	"}\n";

enum {
	TIME_ELAPSED,
	TIMESTAMP
} test = TIME_ELAPSED;

GLuint prog;
GLint iters_loc;

static float
draw(GLuint *q, int iters)
{
	int64_t start_time, end_time;

	glUseProgram(prog);
	glUniform1i(iters_loc, iters);

	start_time = piglit_time_get_nano();

	if (test == TIMESTAMP) {
		glQueryCounter(q[0], GL_TIMESTAMP);
	} else {
		glBeginQuery(GL_TIME_ELAPSED, q[0]);
	}
	piglit_draw_rect(-1, -1, 2, 2);
	if (test == TIMESTAMP) {
		glQueryCounter(q[1], GL_TIMESTAMP);
	} else {
		glEndQuery(GL_TIME_ELAPSED);
	}

	/* This glFinish() is important, since this is used in a
	 * timing loop.
	 */
	glFinish();

	end_time = piglit_time_get_nano();

	return (end_time - start_time)/ 1000.0 / 1000.0 / 1000.0;
}

static float
get_gpu_time(GLuint *q)
{
	GLint64EXT elapsed;

	if (test == TIMESTAMP) {
		GLint64 start, end;
		glGetQueryObjecti64vEXT(q[0], GL_QUERY_RESULT, &start);
		glGetQueryObjecti64vEXT(q[1], GL_QUERY_RESULT, &end);
		elapsed = end - start;
	} else {
		glGetQueryObjecti64vEXT(q[0], GL_QUERY_RESULT, &elapsed);
	}

	return elapsed / 1000.0 / 1000.0 / 1000.0;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	GLuint q[2];
	int iters;
#define	NUM_RESULTS 5
	float cpu_time[NUM_RESULTS];
	float gpu_time[NUM_RESULTS];
	float delta[NUM_RESULTS];
	float cpu_time_mean;
	float delta_mean, delta_stddev;
	float cpu_overhead;
	float t, t_cutoff;
	int i;

	glColor4f(0.0, 1.0, 0.0, 0.0);
	glGenQueries(2, q);

	/* Prime the drawing pipe before we start measuring time,
	 * since the first draw call is likely to be slower than all
	 * others.
	 */
	draw(q, 1);

	/* Figure out some baseline difference between GPU time
	 * elapsed and CPU time elapsed for a single draw call (CPU
	 * overhead of timer query and glFinish()).
	 *
	 * Note that this doesn't take into account any extra CPU time
	 * elapsed from start to finish if multiple batchbuffers are
	 * accumulated by the driver in getting to our 1/10th of a
	 * second elapsed time goal, and some other client sneaks
	 * rendering in in between those batches.
	 *
	 * Part of the rendering size being relatively large is to
	 * hopefully avoid that, though it might be better to have
	 * some time-consuming shader with a single draw call instead.
	 */
	cpu_overhead = 0;
	for (i = 0; i < NUM_RESULTS; i++) {
		cpu_time[i] = draw(q, 1);
		gpu_time[i] = get_gpu_time(q);

		cpu_overhead += cpu_time[i] - gpu_time[i];
	}
	cpu_overhead /= NUM_RESULTS;

	/* Find a number of draw calls that takes about 1/10th of a
	 * second.
	 */
retry:
	for (iters = 1; ; iters *= 2) {
		if (draw(q, iters) > 0.1)
			break;
		if (iters * 2 <= iters) {
			printf("Couldn't find appropriate number of iterations\n");
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	/* Now, do several runs like this so we can determine if the
	 * timer matches up with wall time.
	 */
	for (i = 0; i < NUM_RESULTS; i++) {
		cpu_time[i] = draw(q, iters);
		gpu_time[i] = get_gpu_time(q);
	}

	cpu_time_mean = 0;
	delta_mean = 0;
	for (i = 0; i < NUM_RESULTS; i++) {
		delta[i] = cpu_time[i] - cpu_overhead - gpu_time[i];
		cpu_time_mean += cpu_time[i];
		delta_mean += delta[i];
	}
	cpu_time_mean /= NUM_RESULTS;
	delta_mean /= NUM_RESULTS;

	/* There's some risk of our "get to 0.1 seconds" loop deciding
	 * that a small number of iters was sufficient if we got
	 * scheduled out for a while.  Re-run if so.
	 *
	 * We wouldn't have that problem if we could rely on the GPU
	 * time elapsed query, but that's the thing we're testing.
	 */
	if (cpu_time_mean < 0.05)
		goto retry;

	/* Calculate stddevs. */
	delta_stddev = 0;
	for (i = 0; i < NUM_RESULTS; i++) {
		float d = delta[i] - delta_mean;
		delta_stddev += d * d / (NUM_RESULTS - 1);
	}
	delta_stddev = sqrt(delta_stddev);

	/* Dependent t-test for paired samples.
	 *
	 * This is a good test, because we expect the two times (cpu
	 * and gpu) of the samples to be correlated, and we expect the
	 * stddev to match (since time it should arise from system
	 * variables like scheduling of other tasks and state of the
	 * caches).  Unless maybe the variance of cpu time is greater
	 * than gpu time, because we may see scheduling accounted for
	 * in our CPU (wall) time, while scheduling other tasks
	 * doesn't end up counted toward our GPU time.
	 */
	t = delta_mean / (delta_stddev / sqrt(NUM_RESULTS));

	/* Integral of Student's t distribution for 4 degrees of
	 * freedom (NUM_RESULTS = 5), two-tailed (we care about
	 * difference above or below 0, not just one direction), at
	 * p = .05.
	 */
	t_cutoff = 2.776;

	/* Now test that our sampled distribution (rate of clock
	 * advance between CPU and GPU) was within expectations for a
	 * delta of 0.  I actually want to be testing the likelihood
	 * that the real difference is enough that we actually care.
	 * I didn't find an easy way to account for that after a bunch
	 * of wikipedia browsing, so I'll punt on proper analysis for
	 * now and just check that the sampled delta isn't too small
	 * to care about.
	 */
	if (t > t_cutoff && fabs(delta_mean) > .05 * cpu_time_mean) {
		fprintf(stderr, "GPU time didn't match CPU time\n");
		printf("Estimated CPU overhead: %f\n", cpu_overhead);
		printf("Difference: %f secs (+/- %f secs)\n",
		       delta_mean, delta_stddev);
		printf("t = %f\n", t);

		printf("%20s %20s %20s\n",
		       "gpu_time", "cpu_time", "delta");
		for (i = 0; i < NUM_RESULTS; i++) {
			printf("%20f %20f %20f\n",
			       gpu_time[i], cpu_time[i], delta[i]);
		}

		pass = false;
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	piglit_present_results();

	glDeleteQueries(2, q);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);

	prog = piglit_build_simple_program(vs_text, fs_text);
	iters_loc = glGetUniformLocation(prog, "iters");

	piglit_require_extension("GL_EXT_timer_query");

	if (argc == 2 && strcmp(argv[1], "timestamp") == 0) {
		piglit_require_extension("GL_ARB_timer_query");
		test = TIMESTAMP;
	}
}
