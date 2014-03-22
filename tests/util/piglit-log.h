/*
 * Copyright 2014 Intel Corporation
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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Piglit logging options
 *
 * Options can be set with the setter function piglit_log_set_opt() as well as
 * environment variables.  For each option, the environment variable of the
 * same name, if set to a non-empty value,  overrides any value set with the
 * setter function.
 */
enum piglit_log_opt {
	/**
	 * Print thread id in log messages.
	 * Option type: bool
	 */
	PIGLIT_LOG_PRINT_TID = 0,

	/** Fake option. This is the maximum value of piglit_log_opt. */
	PIGLIT_LOG_OPT_MAX = 0,
};

intptr_t
piglit_log_get_opt(enum piglit_log_opt);

void
piglit_log_set_opt(enum piglit_log_opt opt, intptr_t value);

/** Log an error.message. */
void
piglit_loge(const char *fmt, ...);

/** Log an info message. */
void
piglit_logi(const char *fmt, ...);

#ifdef __cplusplus
} /* end extern "C" */
#endif
