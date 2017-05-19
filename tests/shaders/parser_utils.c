/*
 * Copyright © 2016 Intel Corporation
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

unsigned
parse_ints(const char *s, int *i, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		int v = strtoll(s = end, (char **)&end, 0);
		if (s == end)
			break;
		i[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
}

unsigned
parse_uints(const char *s, unsigned *u, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		unsigned v = strtoul(s = end, (char **)&end, 0);
		if (s == end)
			break;
		u[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
}

unsigned
parse_int64s(const char *s, int64_t *i, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		int64_t v = strtoll(s = end, (char **)&end, 0);
		if (s == end)
			break;
		i[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
}

unsigned
parse_uint64s(const char *s, uint64_t *u, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		uint64_t v = strtoull(s = end, (char **)&end, 0);
		if (s == end)
			break;
		u[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
}

unsigned
parse_floats(const char *s, float *f, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		float v = strtof_hex(s = end, (char **)&end);
		if (s == end)
			break;
		f[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
}

unsigned
parse_doubles(const char *s, double *d, unsigned n, const char **rest)
{
	const char *end = s;
	unsigned j;

	for (j = 0; j < n; j++) {
		double v = strtod_hex(s = end, (char **)&end);
		if (s == end)
			break;
		d[j] = v;
	}

	if (rest)
		*rest = end;

	return j;
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

bool
parse_enum_tab(const struct string_to_enum *tab,
	       const char *s, unsigned *e, const char **rest)
{
	const char *end = s;
	bool ret = parse_word(s, &s, &end);
	unsigned i = 0;

	if (ret) {
		for (i = 0; tab[i].name; i++) {
			if (!strncmp(tab[i].name, s, end - s) &&
			    !tab[i].name[end - s])
				break;
		}

		*e = tab[i].value;
		ret = tab[i].name;
	}

	if (rest)
		*rest = (ret ? end : s);

	return ret;
}

bool
parse_tex_target(const char *s, unsigned *t, const char **rest)
{
	static const struct string_to_enum tab[] = {
		{ "1D", GL_TEXTURE_1D },
		{ "2D", GL_TEXTURE_2D },
		{ "3D", GL_TEXTURE_3D },
		{ "Rect", GL_TEXTURE_RECTANGLE },
		{ "Cube", GL_TEXTURE_CUBE_MAP },
		{ "1DArray", GL_TEXTURE_1D_ARRAY },
		{ "2DArray", GL_TEXTURE_2D_ARRAY },
		{ "CubeArray", GL_TEXTURE_CUBE_MAP_ARRAY },
		{ NULL, 0 }
	};
	return parse_enum_tab(tab, s, t, rest);
}

bool
parse_comparison_op(const char *s, enum comparison *t, const char **rest)
{
	if (parse_str(s, "==", rest)) {
		*t = equal;
		return true;
	} else if (parse_str(s, "!=", rest)) {
		*t = greater;
		return true;
	} else if (parse_str(s, "<=", rest)) {
		*t = less_equal;
		return true;
	} else  if (parse_str(s, "<", rest)) {
		*t = less;
		return true;
	} else if (parse_str(s, ">=", rest)) {
		*t = greater_equal;
		return true;
	} else if (parse_str(s, ">", rest)) {
		*t = greater;
		return true;
	} else {
		return false;
	}
}
