# Copyright 2012 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

set(piglit_dispatch_gen_output_dir ${CMAKE_BINARY_DIR}/tests/util)

file(MAKE_DIRECTORY ${piglit_dispatch_gen_output_dir})

set(piglit_dispatch_gen_outputs
	${piglit_dispatch_gen_output_dir}/piglit-dispatch-gen.c
	${piglit_dispatch_gen_output_dir}/piglit-dispatch-gen.h
	)

set(piglit_dispatch_gen_inputs
	${CMAKE_SOURCE_DIR}/tests/util/gen_dispatch.py
	${CMAKE_BINARY_DIR}/glapi/glapi.json
	)

add_custom_command(
	OUTPUT ${piglit_dispatch_gen_outputs}
	DEPENDS ${piglit_dispatch_gen_inputs}
	COMMAND ${python} ${piglit_dispatch_gen_inputs} ${piglit_dispatch_gen_outputs}
	)

add_custom_target(piglit_dispatch_gen
	DEPENDS ${piglit_dispatch_gen_outputs}
	)
