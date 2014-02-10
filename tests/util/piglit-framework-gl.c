/*
 * Copyright © 2009 Intel Corporation
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

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "piglit-util-gl-common.h"
#include "piglit-framework-gl/piglit_gl_framework.h"

struct piglit_gl_framework *gl_fw;

bool piglit_use_fbo = false;
int piglit_automatic = 0;
unsigned piglit_winsys_fbo = 0;

int piglit_width;
int piglit_height;

static void
process_args(int *argc, char *argv[], unsigned *force_samples,
	     struct piglit_gl_test_config *config);

void
piglit_gl_test_config_init(struct piglit_gl_test_config *config)
{
	memset(config, 0, sizeof(*config));
}

static void
delete_arg(char *argv[], int argc, int arg)
{
	int i;

	for (i = arg + 1; i < argc; i++) {
		argv[i - 1] = argv[i];
	}
}

static void
piglit_parse_subtest_args(int *argc, char *argv[],
			  const struct piglit_gl_subtest *subtests,
			  const char ***out_selected_subtests,
			  size_t *out_num_selected_subtests)
{
	int j;
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	for (j = 1; j < *argc; j++) {
		if (streq(argv[j], "-subtest")) {
			int i;

			++j;
			if (j >= *argc) {
				fprintf(stderr,
					"-subtest requires an argument\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			selected_subtests =
				realloc(selected_subtests,
					(num_selected_subtests + 1)
					* sizeof(char*));
			selected_subtests[num_selected_subtests] = argv[j];
			++num_selected_subtests;

			/* Remove 2 arguments from the command line. */
			for (i = j + 1; i < *argc; i++) {
				argv[i - 2] = argv[i];
			}
			*argc -= 2;
			j -= 2;
		} else if (streq(argv[j], "-list-subtests")) {
			int i;

			if (subtests == NULL) {
				fprintf(stderr, "Test defines no subtests!\n");
				exit(EXIT_FAILURE);
			}

			for (i = 0; !PIGLIT_GL_SUBTEST_END(&subtests[i]); ++i) {
				printf("%s: %s\n",
				       subtests[i].option,
				       subtests[i].name);
			}

			exit(EXIT_SUCCESS);
		}
	}

	*out_selected_subtests = selected_subtests;
	*out_num_selected_subtests = num_selected_subtests;
}

/**
 * Recognized arguments are removed from @a argv. The updated array
 * length is returned in @a argc.
 */
static void
process_args(int *argc, char *argv[], unsigned *force_samples,
	     struct piglit_gl_test_config *config)
{
	int j;

	piglit_parse_subtest_args(argc, argv, config->subtests,
				  &config->selected_subtests,
				  &config->num_selected_subtests);

	/* Find/remove "-auto" and "-fbo" from the argument vector.
	 */
	for (j = 1; j < *argc; j++) {
		if (!strcmp(argv[j], "-auto")) {
			piglit_automatic = 1;
			delete_arg(argv, *argc, j--);
			*argc -= 1;
		} else if (!strcmp(argv[j], "-fbo")) {
			piglit_use_fbo = true;
			delete_arg(argv, *argc, j--);
			*argc -= 1;
		} else if (!strcmp(argv[j], "-rlimit")) {
			char *ptr;
			unsigned long lim;
			int i;

			j++;
			if (j >= *argc) {
				fprintf(stderr,
					"-rlimit requires an argument\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			lim = strtoul(argv[j], &ptr, 0);
			if (ptr == argv[j]) {
				fprintf(stderr,
					"-rlimit requires an argument\n");
				piglit_report_result(PIGLIT_FAIL);
			}

			piglit_set_rlimit(lim);

			/* Remove 2 arguments (hence the 'i - 2') from the
			 * command line.
			 */
			for (i = j + 1; i < *argc; i++) {
				argv[i - 2] = argv[i];
			}
			*argc -= 2;
			j -= 2;
		} else if (!strncmp(argv[j], "-samples=", 9)) {
			*force_samples = atoi(argv[j]+9);
			delete_arg(argv, *argc, j--);
			*argc -= 1;
		}
	}
}

void
piglit_gl_process_args(int *argc, char *argv[],
		       struct piglit_gl_test_config *config)
{
	unsigned force_samples = 0;

	process_args(argc, argv, &force_samples, config);

	if (force_samples > 1)
		config->window_samples = force_samples;

}

void
piglit_gl_test_run(int argc, char *argv[],
		   const struct piglit_gl_test_config *config)
{
	piglit_width = config->window_width;
	piglit_height = config->window_height;

	gl_fw = piglit_gl_framework_factory(config);
	if (gl_fw == NULL) {
		printf("piglit: error: failed to create "
		       "piglit_gl_framework\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	gl_fw->run_test(gl_fw, argc, argv);
	assert(false);
}

void
piglit_post_redisplay(void)
{
	if (gl_fw->post_redisplay)
		gl_fw->post_redisplay(gl_fw);
}

void
piglit_set_keyboard_func(void (*func)(unsigned char key, int x, int y))
{
	if (gl_fw->set_keyboard_func)
		gl_fw->set_keyboard_func(gl_fw, func);
}

void
piglit_swap_buffers(void)
{
	if (gl_fw->swap_buffers)
		gl_fw->swap_buffers(gl_fw);
}

void
piglit_present_results(void)
{
	if (!piglit_automatic)
		piglit_swap_buffers();
}

void
piglit_set_reshape_func(void (*func)(int w, int h))
{
	if (!gl_fw->set_reshape_func)
		gl_fw->set_reshape_func(gl_fw, func);
}


enum piglit_result
piglit_create_dma_buf(unsigned w, unsigned h, unsigned cpp,
		      const void *src_data, unsigned src_stride,
		      struct piglit_dma_buf **buf, int *fd,
		      unsigned *stride, unsigned *offset)
{
	*fd = 0;
	*stride = 0;
	*offset = 0;

	if (!gl_fw->create_dma_buf)
		return PIGLIT_SKIP;

	return gl_fw->create_dma_buf(w, h, cpp, src_data, src_stride, buf, fd,
				stride, offset);
}

void
piglit_destroy_dma_buf(struct piglit_dma_buf *buf)
{
	if (gl_fw->destroy_dma_buf)
		gl_fw->destroy_dma_buf(buf);
}

const struct piglit_gl_subtest *
piglit_find_subtest(const struct piglit_gl_subtest *subtests, const char *name)
{
	unsigned i;

	for (i = 0; !PIGLIT_GL_SUBTEST_END(&subtests[i]); i++) {
		if (strcmp(subtests[i].option, name) == 0)
			return &subtests[i];
	}

	return NULL;
}

enum piglit_result
piglit_run_selected_subtests(const struct piglit_gl_subtest *all_subtests,
			     const char **selected_subtests,
			     size_t num_selected_subtests,
			     enum piglit_result previous_result)
{
	enum piglit_result result = previous_result;

	if (num_selected_subtests) {
		unsigned i;

		for (i = 0; i < num_selected_subtests; i++) {
			enum piglit_result subtest_result;
			const char *const name = selected_subtests[i];
			const struct piglit_gl_subtest *subtest =
				piglit_find_subtest(all_subtests, name);

			if (subtest == NULL) {
				fprintf(stderr,
					"Unknown subtest \"%s\".\n",
					name);
				piglit_report_result(PIGLIT_FAIL);
			}

			subtest_result = subtest->subtest_func(subtest->data);
			piglit_report_subtest_result(subtest_result, "%s",
						     subtest->name);

			piglit_merge_result(&result, subtest_result);
		}
	} else {
		unsigned i;

		for (i = 0; !PIGLIT_GL_SUBTEST_END(&all_subtests[i]); i++) {
			const enum piglit_result subtest_result =
				all_subtests[i].subtest_func(all_subtests[i].data);
			piglit_report_subtest_result(subtest_result, "%s",
						     all_subtests[i].name);

			piglit_merge_result(&result, subtest_result);
		}
	}

	return result;
}

size_t
piglit_get_selected_tests(const char ***selected_subtests)
{
	*selected_subtests = gl_fw->test_config->selected_subtests;
	return gl_fw->test_config->num_selected_subtests;
}
