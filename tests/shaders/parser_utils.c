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
#include <string.h>
#include <ctype.h>
#include "parser_utils.h"

bool
parse_whitespace(const char *s, const char **rest)
{
	const char *end = s;
	for (; *end && *end != '\n' && isspace(*end); end++);

	if (rest)
		*rest = end;

	return end != s;
}

bool
parse_str(const char *s, const char *lit, const char **rest)
{
	const char *t;
	parse_whitespace(s, &t);
	const bool ret = strncmp(t, lit, strlen(lit)) == 0;

	if (rest)
		*rest = (ret ? t + strlen(lit) : s);

	return ret;
}

bool
parse_word(const char *s, const char **t, const char **rest)
{
	parse_whitespace(s, t);

	const char *end = *t;
	for (; *end && !isspace(*end); end++);

	if (rest)
		*rest = (*t != end ? end : s);

	return *t != end;
}

bool
parse_word_copy(const char *s, char *t, unsigned n, const char **rest)
{
	const char *start, *end;
	const bool ret = parse_word(s, &start, &end) && end - start < n;

	if (ret) {
		memcpy(t, start, end - start);
		t[end - start] = 0;
	}
	if (rest)
		*rest = (ret ? end : s);

	return ret;
}

bool
parse_enum_gl(const char *s, GLenum *e, const char **rest)
{
	char name[512];
	const bool ret = parse_word_copy(s, name, sizeof(name), rest);
	*e = (ret ? piglit_get_gl_enum_from_name(name) : GL_NONE);
	return ret;
}

/**
 * Skip over whitespace upto the end of line
 */
const char *
eat_whitespace(const char *src)
{
	while (isspace((int) *src) && (*src != '\n'))
		src++;

	return src;
}


/**
 * Skip over non-whitespace upto the end of line
 */
const char *
eat_text(const char *src)
{
	while (!isspace((int) *src) && (*src != '\0'))
		src++;

	return src;
}


bool
string_match(const char *string, const char *line)
{
	return (strncmp(string, line, strlen(string)) == 0);
}


/**
 * Copy a string until either whitespace or the end of the string
 */
const char *
strcpy_to_space(char *dst, const char *src)
{
	while (!isspace((int) *src) && (*src != '\0'))
		*(dst++) = *(src++);

	*dst = '\0';
	return src;
}
