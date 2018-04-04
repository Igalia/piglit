/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#ifndef PIGLIT_UTIL_H
#define PIGLIT_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#if defined(_WIN32)

#include <windows.h>

/* Another two macros provided by windows.h which conflict with piglit */
#undef near
#undef far

#endif

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>

#if defined(__APPLE__) || defined(__MINGW32__)
#  include "libgen.h" // for basename
#endif

#include "piglit-log.h"

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if (__GNUC__ >= 3)
#define PRINTFLIKE(f, a) __attribute__ ((format(__printf__, f, a)))
#else
#define PRINTFLIKE(f, a)
#endif

#if defined(__GNUC__) || __has_attribute(noreturn)
#define NORETURN __attribute__((noreturn))
#else
#define NORETURN
#endif

#ifndef HAVE_ASPRINTF
int asprintf(char **strp, const char *fmt, ...) PRINTFLIKE(2, 3);
#endif /* HAVE_ASPRINTF */

#ifndef HAVE_FFS
#ifdef __MINGW32__
#define ffs __builtin_ffs
#else /* !__MINGW32__ */

/**
 * Find the first bit set in i and return the index set of that bit.
 */
static inline int
ffs(int i)
{
	int bit;

	if (i == 0) {
		return 0;
	}

	for (bit = 1; !(i & 1); bit++) {
		i = i >> 1;
	}

	return bit;
}

#endif /* !__MINGW32__ */
#endif /* !HAVE_FFS*/

#ifdef _WIN32
#  define PIGLIT_PATH_SEP '\\'
#else
#  define PIGLIT_PATH_SEP '/'
#endif

enum piglit_result {
	PIGLIT_PASS,
	PIGLIT_FAIL,
	PIGLIT_SKIP,
	PIGLIT_WARN
};

/**
 * An individual subtest that makes up part of a test group.
 */
struct piglit_subtest {
	/** Name of the subtest as it will appear in the log. */
	const char *name;

	/** Command line name used to select this test. */
	const char *option;

	/** Function that implements the test. */
	enum piglit_result (*subtest_func)(void *data);

	/** Passed as the data parameter to subtest_func.*/
	void *data;
};

/**
 * Detect the end of an array of piglit_subtest structures
 *
 * The array of subtests is terminated by structure with a \c NULL \c
 * name pointer.
 */
#define PIGLIT_SUBTEST_END(s) ((s)->name == NULL)

const struct piglit_subtest*
piglit_find_subtest(const struct piglit_subtest *subtests, const char *name);

enum piglit_result
piglit_run_selected_subtests(const struct piglit_subtest *all_subtests,
			     const char **selected_subtests,
			     size_t num_selected_subtests,
			     enum piglit_result previous_result);

void
piglit_register_subtests(const char *names[]);

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )
#define MIN2(a, b) ((a) > (b) ? (b) : (a))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))
#define MIN3(a, b, c) MIN2(MIN2((a), (b)), (c))
#define MAX3(a, b, c) MAX2(MAX2((a), (b)), (c))
#define ALIGN(value, alignment) (((value) + alignment - 1) & ~(alignment - 1))
#define PIGLIT_STRINGIFY(macro_or_string) PIGLIT_STRINGIFY_ARG (macro_or_string)
#define PIGLIT_STRINGIFY_ARG(contents) #contents

/**
 * Utility macro that checks for a given opengl error, and report a
 * subtest result.
 */
#define PIGLIT_SUBTEST_ERROR(error, global, ...) \
do { \
	bool local = piglit_check_gl_error((error)); \
	global = global && local; \
	piglit_report_subtest_result(local ? PIGLIT_PASS : PIGLIT_FAIL, \
				     __VA_ARGS__);			\
} while (0)

/**
 * Utility macro that checks for a given condition, and report a
 * subtest result.
 */
#define PIGLIT_SUBTEST_CONDITION(condition, global, ...) \
do { \
	bool cond = (condition); \
	global = global && cond; \
	piglit_report_subtest_result(cond ? PIGLIT_PASS : PIGLIT_FAIL, \
				     __VA_ARGS__);		       \
} while (0)

static inline unsigned
log2u(unsigned v)
{
#ifdef __GCC__
	return v == 0 ? 0 : 31 - __builtin_clz(v);
#else
	unsigned res = 0;

	while (v >>= 1)
		res++;

	return res;
#endif
}

/**
 * Returns the smallest power-of-two integer greater than or equal to v
 */
static inline unsigned
next_power_of_two(unsigned v)
{
	/* Special case zero because 1U << 32 is undefined. */
	return v == 0 ? 1 : 1U << (log2u(v - 1) + 1);
}


/**
 * Return true if and only if two string are equal according to strcmp().
 */
static inline bool
streq(const char *a, const char *b)
{
	return strcmp(a, b) == 0;
}

/**
 * Wrapper for strtod() which also handles +/-inf with MSVC.
 * Note: we only check for "inf" and not "INF".
 */
static inline double
strtod_inf(const char *nptr, char **endptr)
{
	return strtod(nptr, endptr);
}

/**
 * Wrapper for strtod_inf() which allows using an exact hex bit
 * pattern to generate a float value.
 */
static inline float
strtof_hex(const char *nptr, char **endptr)
{
	/* skip spaces and tabs */
	while (*nptr == ' ' || *nptr == '\t')
		nptr++;

	if (strncmp(nptr, "0x", 2) == 0) {
		union {
			uint32_t u;
			float f;
		} x;

		x.u = strtoul(nptr, endptr, 16);
		return x.f;
	} else {
		return strtod_inf(nptr, endptr);
	}
}

/**
 * Wrapper for strtod_inf() which allows using an exact hex bit
 * pattern to generate a double value.
 */
static inline double
strtod_hex(const char *nptr, char **endptr)
{
	/* skip spaces and tabs */
	while (*nptr == ' ' || *nptr == '\t')
		nptr++;

	if (strncmp(nptr, "0x", 2) == 0) {
		union {
			uint64_t u64;
			double d;
		} x;

		x.u64 = strtoull(nptr, endptr, 16);
		return x.d;
	} else {
		return strtod_inf(nptr, endptr);
	}
}

/**
 * Wrapper for strtol() which allows using an exact hex bit pattern to
 * generate a signed int value.
 */
static inline int
strtol_hex(const char *nptr, char **endptr)
{
	/* skip spaces and tabs */
	while (*nptr == ' ' || *nptr == '\t')
		nptr++;

	if (strncmp(nptr, "0x", 2) == 0) {
		union {
			uint32_t u;
			int32_t i;
		} x;

		x.u = strtoul(nptr, endptr, 16);
		return x.i;
	} else {
		return strtol(nptr, endptr, 0);
	}
}

#ifndef HAVE_STRCHRNUL
static inline char *
strchrnul(const char *s, int c)
{
       const char *t = strchr(s, c);

       return (t == NULL) ? ((char *) s + strlen(s)) : (char *) t;
}
#endif


#ifndef HAVE_STRNDUP
static inline char *
strndup(const char *s, size_t n)
{
	const size_t len = strlen(s);
	const size_t size_to_copy = MIN2(n, len);

	char *const copy = (char *const) malloc(size_to_copy + 1);
	if (copy != NULL) {
		memcpy(copy, s, size_to_copy);
		copy[size_to_copy] = '\0';
	}

	return copy;
}
#endif

/**
 * Determine if an extension is listed in an extension string
 *
 * \param haystack   List of all extensions to be searched
 * \param needle     Extension whose presens is to be detected
 *
 * \precondition \c haystack is not null
 *
 * \sa piglit_is_extension_supported, piglit_is_glx_extension_supported
 */
bool piglit_is_extension_in_string(const char *haystack, const char *needle);

/**
 * Determine if an extension is listed in an extension string array
 *
 * \param haystack   Array of all extensions to be searched
 * \param needle     Extension whose presens is to be detected
 *
 * \precondition \c haystack is not null
 *
 * \sa piglit_is_extension_supported, piglit_is_glx_extension_supported
 */
bool piglit_is_extension_in_array(const char **haystack, const char *needle);

int piglit_find_line(const char *program, int position);
void piglit_merge_result(enum piglit_result *all, enum piglit_result subtest);
const char * piglit_result_to_string(enum piglit_result result);
NORETURN void piglit_report_result(enum piglit_result result);
void piglit_set_timeout(double seconds, enum piglit_result timeout_result);
void piglit_report_subtest_result(enum piglit_result result,
				  const char *format, ...) PRINTFLIKE(2, 3);

void piglit_general_init(void);

extern void piglit_set_rlimit(unsigned long lim);

char *piglit_load_text_file(const char *file_name, unsigned *size);

/**
 * \brief Read environment variable PIGLIT_SOURCE_DIR.
 *
 * If environment is not defined, then report failure. The intention is
 * that tests should use this to construct the path to any needed data files.
 */
const char*
piglit_source_dir(void);

/**
 * \brief Join paths together with the system path separator.
 *
 * On Unix, the path separator is '/'. On Windows, '\\'.
 *
 * The variable argument consists of \a n null-terminated strings.  The
 * resultant path is written to \a buf.  No more than \a buf_size bytes are
 * written to \a buf. The last byte written is always the null character.
 * Returned is the number of bytes written, including the terminating null.
 */
size_t
piglit_join_paths(char buf[], size_t buf_size, int n, ...);

/**
 * \brief Whether piglit_time_get* return monotonically increasing time.
 *
 * Can be used to determine how accurate/reliable the time returned by the
 * function(s) is.
 */
bool
piglit_time_is_monotonic();

/**
 * \brief Get the time in nanoseconds
 *
 * This time can be used for relative time measurements.
 *
 * A negative return value indicates an error.
 *
 * \sa piglit_time_is_monotonic
 */
int64_t
piglit_time_get_nano(void);

/**
 * \brief Try to delay for a given duration
 *
 * \param time_ns	The requested duration to wait for
 *
 * Returns the time elapsed which may be cut short or overrun the requested
 * duration, e.g. due to signal handling or scheduling.
 *
 * \sa on Linux nanosleep is used and restarted on SIGINT
 * \sa usleep() is use on other OSs
 * \sa the returned duration is timed using piglit_time_get_nano()
 */
int64_t
piglit_delay_ns(int64_t time_ns);

const char**
piglit_split_string_to_array(const char *string, const char *separators);

bool
piglit_strip_arg(int *argc, char *argv[], const char *arg);

void
piglit_parse_subtest_args(int *argc, char *argv[],
			  const struct piglit_subtest *subtests,
			  const char ***out_selected_subtests,
			  size_t *out_num_selected_subtests);

/**
 * \brief Return the thread id.
 *
 * On Linux, this functions wraps the gettid() syscall.
 * On unsupported systems, this returns 0.
 */
uint64_t
piglit_gettid(void);

size_t
piglit_get_page_size(void);

void *
piglit_alloc_aligned(size_t alignment, size_t size);

void
piglit_free_aligned(void *p);


#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* PIGLIT_UTIL_H */
