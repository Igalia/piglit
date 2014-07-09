/**
 * ${warning}
 *
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

<%block filter='fake_whitespace'>\
#include "piglit-util-gl.h"

const char*
piglit_get_gl_enum_name(GLenum param)
{
>-------switch (param) {
% for enum in sorted_unique_enums_in_default_namespace:
>-------case ${enum.c_num_literal}: return "${enum.name}";
% endfor
>-------default: return "(unrecognized enum)";
>-------}
}

const char*
piglit_get_prim_name(GLenum prim)
{
<% gl_patches = gl_registry.enums['GL_PATCHES'] %>\
>-------switch (prim) {
% for enum in sorted_unique_enums_in_default_namespace:
% if enum.num_value <= gl_patches.num_value:
>-------case ${enum.c_num_literal}: return "${enum.name}";
% endif
% endfor
>-------default: return "(unrecognized enum)";
>-------}
}
</%block>\
