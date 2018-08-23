/*
 * Copyright © 2018 Intel Corporation
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

#include "piglit-shader-test.h"

#include <ctype.h>

/* FIXME: C&P from parse_utils, that is local to shader_runner. At
 * this point it is not worth to move all parse_utils to a common
 * library. Perhaps in the future if we start to use all the utils
 */
bool
parse_whitespace(const char *s, const char **rest)
{
	const char *end = s;
	for (; *end && *end != '\n' && isspace(*end); end++);

	if (rest)
		*rest = end;

	return end != s;
}

/* FIXME: C&P from parse_utils, that is local to shader_runner. At
 * this point it is not worth to move all parse_utils to a common
 * library. Perhaps in the future if we start to use all the utils
 */
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

static void
group_name_for_stage(GLenum shader_type,
		     bool spirv,
		     char *group_name)
{
	char *stage_name;

	switch(shader_type) {
	case GL_VERTEX_SHADER:
		stage_name = "vertex";
		break;
	case GL_FRAGMENT_SHADER:
		stage_name = "fragment";
		break;
	case GL_TESS_CONTROL_SHADER:
		stage_name = "tessellation control";
		break;
	case GL_TESS_EVALUATION_SHADER:
		stage_name = "tessellation evaluation";
		break;
	case GL_GEOMETRY_SHADER:
		stage_name = "geometry";
		break;
	case GL_COMPUTE_SHADER:
		stage_name = "compute";
		break;
	default: stage_name = "error";
	}

	if (!spirv)
		sprintf(group_name, "[%s shader]", stage_name);
	else
		sprintf(group_name, "[%s shader spirv]", stage_name);
}

bool
piglit_load_source_from_shader_test(const char *filename,
				    GLenum shader_type,
				    bool spirv,
				    char **output_source,
				    unsigned *output_source_size)
{
	char group_name[4096];
	unsigned text_size;
	char *line = NULL;
	char *first_line = NULL;

	group_name_for_stage(shader_type, spirv, group_name);

	char *text = piglit_load_text_file(filename, &text_size);
	line = text;

	if (line == NULL) {
		fprintf(stderr, "Could not read file \"%s\"\n", filename);
		return false;
	}

	while (line[0] != '\0') {
		if (line[0] == '[' && first_line != NULL) {
			break;
		}

		if (line[0] == '[' && first_line == NULL) {
			if (parse_str(line, group_name, NULL)) {
				first_line = strchrnul(line, '\n');
				if (first_line[0] != '\0')
					first_line++;
			}
		}

		line = strchrnul(line, '\n');
		if (line[0] != '\0')
			line++;
	}

	if (first_line == NULL) {
		fprintf(stderr, "Could not find groupname \"%s\" on file \"%s\"\n",
			group_name, filename);
		free(text);
		return false;
	}

	text_size = line - first_line + 1;

	if (output_source) {
		*output_source = malloc(sizeof(char) * text_size);
		snprintf(*output_source, line - first_line + 1, "%s", first_line);
	}

	if (output_source_size)
		*output_source_size = text_size;

	free(text);

	return true;
}
