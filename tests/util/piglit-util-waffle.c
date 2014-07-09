/*
 * Copyright © 2012 Intel Corporation
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
#include <stdio.h>

#include "piglit-util-gl.h"
#include "piglit-util-waffle.h"

static void
wfl_log(const char *tag, const char *func_name)
{
	const struct waffle_error_info *info = waffle_error_get_info();

	assert(tag != NULL);
	assert(info->code != WAFFLE_NO_ERROR);

	fflush(stdout);
	fprintf(stderr, "piglit: %s: %s failed due to %s",
	        tag, func_name, waffle_error_to_string(info->code));
	if (info->message_length > 0)
		fprintf(stderr, ": %s", info->message);
	fprintf(stderr, "\n");
}

void
wfl_log_debug(const char *func_name)
{
	static int debug = -1;

	if (debug == -1) {
		const char *env = getenv("PIGLIT_DEBUG");
		if (env == NULL) {
			debug = 0;
		} else if (strcmp(env, "0") == 0) {
			debug = 0;
		} else if (strcmp(env, "1") == 0) {
			debug = 1;
		} else {
			fprintf(stderr, "PIGLIT_DEBUG has invalid value:"
				" %s\n", env);
			abort();
		}
	}

	if (debug == 1)
		wfl_log("debug", func_name);
}

void
wfl_log_error(const char *func_name)
{
	wfl_log("error", func_name);
}

void
wfl_fatal_error(const char *func_name)
{
	const struct waffle_error_info *info = waffle_error_get_info();

	assert(info->code != WAFFLE_NO_ERROR);

	wfl_log_error(func_name);

	if (info->code == WAFFLE_ERROR_UNSUPPORTED_ON_PLATFORM ||
	    info->code == WAFFLE_ERROR_BUILT_WITHOUT_SUPPORT)
		piglit_report_result(PIGLIT_SKIP);
	else
		piglit_report_result(PIGLIT_FAIL);
}
