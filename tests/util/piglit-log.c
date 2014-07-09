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

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>

#include "piglit-log.h"
#include "piglit-util.h"

struct piglit_log_opt_list {
	intptr_t val;
	bool is_env_set;
};

/**
 * Array of logging options, one entry for each value of enum piglit_log_opt.
 */
static struct piglit_log_opt_list opts[PIGLIT_LOG_OPT_MAX + 1];

static void
get_env_overrides(void)
{
	static bool once = true;
	const char *env = NULL;

	if (!once) {
		return;
	}
	once = false;

	env = getenv("PIGLIT_LOG_PRINT_TID");
	if (env && !streq(env, "")) {
		opts[PIGLIT_LOG_PRINT_TID].is_env_set = true;
		opts[PIGLIT_LOG_PRINT_TID].val = atoi(env);
	}
}

/** Is option out of bounds?  */
static bool
is_opt_oob(enum piglit_log_opt opt)
{
	return opt > PIGLIT_LOG_OPT_MAX;
}

intptr_t
piglit_log_get_opt(enum piglit_log_opt opt)
{
	get_env_overrides();

	if (is_opt_oob(opt)) {
		return 0;
	}

	return opts[opt].val;
}

void
piglit_log_set_opt(enum piglit_log_opt opt, intptr_t value) {
	get_env_overrides();

	if (is_opt_oob(opt) || opts[opt].is_env_set) {
		return;
	}

	opts[opt].val = value;
}

static void
piglit_log_tagv(const char *tag, const char *fmt, va_list ap)
{
	printf("piglit");
	if (piglit_log_get_opt(PIGLIT_LOG_PRINT_TID)) {
		printf("(%"PRIu64")", piglit_gettid());
	}
	printf(": %s: ", tag);
	vprintf(fmt, ap);
	printf("\n");
}

void
piglit_loge(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	piglit_log_tagv("error", fmt, ap);
	va_end(ap);
}

void
piglit_logi(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	piglit_log_tagv("info", fmt, ap);
	va_end(ap);
}

void
piglit_logd(const char *fmt, ...)
{
	static bool once = true;
	static bool debug = false;
	va_list ap;

	if (once) {
		const char *env;

		once = false;
		env = getenv("PIGLIT_DEBUG");

		if (env == NULL
		    || streq(env, "")
		    || streq(env, "0")
		    || streq(env, "false")) {
			debug = false;
		} else if (streq(env, "1")
			   || streq(env, "true")) {
			debug = true;
		} else {
			piglit_loge("PIGLIT_DEBUG has invalid value: "
				    "%s\n", env);
			abort();
		}
	}

	if (!debug) {
		return;
	}

	va_start(ap, fmt);
	piglit_log_tagv("debug", fmt, ap);
	va_end(ap);
}
