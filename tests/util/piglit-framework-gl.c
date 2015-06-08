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
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "piglit-util-gl.h"
#include "piglit-framework-gl/piglit_gl_framework.h"

struct piglit_gl_framework *gl_fw;

const char *piglit_binary_name;
bool piglit_dump_png = false;
bool piglit_use_fbo = false;
int piglit_automatic = 0;
unsigned piglit_winsys_fbo = 0;

int piglit_width;
int piglit_height;

static void
process_args(int *argc, char *argv[], unsigned *force_samples,
	     struct piglit_gl_test_config *config);

static bool
override_size(struct piglit_gl_test_config *config)
{
	const char *default_size;
	unsigned int width;
	unsigned int height;

	default_size = getenv("PIGLIT_DEFAULT_SIZE");
	if (!default_size)
		return false;

	if (sscanf(default_size, "%ux%u", &width, &height) != 2)
		return false;

	if (width == 0 || height == 0)
		return false;

	config->window_width = width;
	config->window_height = height;
	return true;
}

void
piglit_gl_test_config_init(struct piglit_gl_test_config *config)
{
	memset(config, 0, sizeof(*config));

	if (!override_size(config)) {
		/* Default window size.  Note: Win8's min window width */
		/* seems to be 160 pixels.  When the window size is */
		/* unexpectedly resized, tests are marked as "WARN". */
		/* Let's use a larger default to avoid that. */
		config->window_width = 160;
		config->window_height = 160;
	}
}

static void
delete_arg(char *argv[], int argc, int arg)
{
	int i;

	for (i = arg + 1; i < argc; i++) {
		argv[i - 1] = argv[i];
	}
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

	piglit_binary_name = argv[0];

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
		} else if (!strcmp(argv[j], "-png")) {
			piglit_dump_png = true;
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
	if (piglit_dump_png) {
		static char *fileprefix = NULL;
		static int frame = 0;
		char *filename;
		GLenum base_format = GL_RGBA;
		GLubyte *image;
		if (fileprefix == NULL) {
			int i;
			fileprefix = strdup(piglit_binary_name);
			fileprefix = basename(fileprefix);
			/* Strip potentially bad characters */
			for (i = 0; fileprefix[i]; i++) {
				if (!isalnum(fileprefix[i]) && fileprefix[i] != '-')
					fileprefix[i] = '_';
			}
		}
		image = malloc(4 * piglit_width * piglit_height);
		glReadPixels(0, 0, piglit_width, piglit_height,
			     base_format, GL_UNSIGNED_BYTE, image);
		assert(glGetError() == GL_NO_ERROR);

		asprintf(&filename, "%s%03d.png", fileprefix, frame++);

		printf("Writing %s...\n", filename);
		piglit_write_png(filename, base_format, piglit_width,
				 piglit_height, image, true);
		free(filename);
		free(image);
	}

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

size_t
piglit_get_selected_tests(const char ***selected_subtests)
{
	*selected_subtests = gl_fw->test_config->selected_subtests;
	return gl_fw->test_config->num_selected_subtests;
}
