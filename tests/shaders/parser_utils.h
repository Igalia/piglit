/*
 * Copyright © 2010-2016 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file parser_utils.h
 *
 * These are a bunch of plain-text parsing utilities, most of them
 * have the form:
 *   boolean-like parse_foo(input-string, output-foo, output-string)
 *
 * If the input is a well-formed string representation of a "foo"
 * value, as many characters will be read from the string as they are
 * needed to initialize the "foo" object returned via the first output
 * argument, and the boolean return value will evaluate to true.  If
 * the output string argument is not NULL, a pointer one past the last
 * character consumed to parse a "foo" value will be returned to the
 * caller so that the rest of the document can be processed (e.g. by
 * passing the output string as input string of another parse
 * function).
 *
 * If the input cannot be parsed as a "foo" object, the boolean return
 * value will evaluate to false and the input string will be returned
 * as output string as-is (which mimicks the behavior of the C
 * standard library strto* functions).  The "foo" output argument will
 * be left in an undefined state in that case.
 */
#ifndef PIGLIT_PARSER_UTILS_H
#define PIGLIT_PARSER_UTILS_H

#include <stdbool.h>
#include "piglit-util-gl.h"

/**
 * Parse one or more whitespace characters (other than newline) from
 * the input string.
 */
bool
parse_whitespace(const char *s, const char **rest);

/**
 * Parse an exact match of string \p lit, optionally preceded by
 * whitespace.
 */
bool
parse_str(const char *s, const char *lit, const char **rest);

/**
 * Parse a single non-empty whitespace-separated token.  On success \p
 * t and \p rest will respectively point at the first and one past the
 * last character of the result.
 */
bool
parse_word(const char *s, const char **t, const char **rest);

/**
 * Like parse_word(), but the result is copied into the fixed-size
 * buffer pointed to by \p t and null-terminated.
 *
 * The parse is considered to fail if the size of the result
 * (including the terminating null character) would have exceded the
 * number of characters allocated for it in the buffer as given by the
 * \p n argument.
 */
bool
parse_word_copy(const char *s, char *t, unsigned n, const char **rest);

/**
 * Parse a GL_* symbolic constant.
 */
bool
parse_enum_gl(const char *s, GLenum *e, const char **rest);

const char *eat_whitespace(const char *src);
const char *eat_text(const char *src);
bool string_match(const char *string, const char *line);
const char *strcpy_to_space(char *dst, const char *src);

/**
 * Abort the Piglit test with failure status if the boolean expression
 * (typically the result of a chain of parse function calls) evaluates
 * to false.
 */
#define REQUIRE(b, ...) do {					\
		if (!(b)) {					\
			fprintf(stderr, __VA_ARGS__);		\
			piglit_report_result(PIGLIT_FAIL);	\
		}						\
	} while (0)

#endif
