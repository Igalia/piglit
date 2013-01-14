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
#endif

#if defined(_MSC_VER)
#define log2(x) (log(x) / log(2))
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

#if defined(_MSC_VER)

#define snprintf sprintf_s

static __inline double
round(double x) {
	return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static __inline float
roundf(float x) {
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}

static __inline long
lround(double x) {
	return (long)round(x);
}

#ifndef va_copy
#ifdef __va_copy
#define va_copy(dest, src) __va_copy((dest), (src))
#else
#define va_copy(dest, src) (dest) = (src)
#endif
#endif

#endif /* defined(_MSC_VER) */

#if (__GNUC__ >= 3)
#define PRINTFLIKE(f, a) __attribute__ ((format(__printf__, f, a)))
#else
#define PRINTFLIKE(f, a)
#endif

#ifdef _WIN32
int asprintf(char **strp, const char *fmt, ...) PRINTFLIKE(2, 3);
#endif /* _WIN32 */

// Trick from http://tdistler.com/2011/03/24/how-to-define-nan-not-a-number-on-windows
#ifndef INFINITY
#  define INFINITY (FLT_MAX + FLT_MAX)
#endif
#ifndef NAN
#  define NAN (INFINITY - INFINITY)
#endif

enum piglit_result {
	PIGLIT_PASS,
	PIGLIT_FAIL,
	PIGLIT_SKIP,
	PIGLIT_WARN
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define CLAMP( X, MIN, MAX )  ( (X)<(MIN) ? (MIN) : ((X)>(MAX) ? (MAX) : (X)) )
#define MIN2(a, b) ((a) > (b) ? (b) : (a))
#define MAX2(a, b) ((a) > (b) ? (a) : (b))

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
void piglit_report_result(enum piglit_result result);
void piglit_report_subtest_result(enum piglit_result result,
				  const char *format, ...) PRINTFLIKE(2, 3);

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c);
#endif

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

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif /* PIGLIT_UTIL_H */
