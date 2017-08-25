/*
 * Copyright © 2015 Glenn Kennard
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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
 *
 */

/**
 * \file common.h
 * Common tools for generating queries.
 */

#ifndef ARB_QUERY_BUFFER_OBJECT_COMMON_H
#define ARB_QUERY_BUFFER_OBJECT_COMMON_H

#include "piglit-util-gl.h"

struct query_type_desc {
	GLenum type;
	const char *extensions[2];
};

extern const struct query_type_desc query_types[];

unsigned num_query_types();

void
get_query_values(const struct query_type_desc *desc, bool *exact, uint32_t *expected);

bool
is_query_supported(const struct query_type_desc *desc);

void
run_query(unsigned query, const struct query_type_desc *desc);

void
query_common_init();


#endif /* ARB_QUERY_BUFFER_OBJECT_COMMON_H */
