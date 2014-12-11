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

set(piglit_dispatch_gen_script ${CMAKE_SOURCE_DIR}/tests/util/gen_dispatch.py)
set(piglit_dispatch_gen_output_dir ${CMAKE_BINARY_DIR}/tests/util)

file(MAKE_DIRECTORY ${piglit_dispatch_gen_output_dir})

set(piglit_dispatch_gen_outputs
	${piglit_dispatch_gen_output_dir}/piglit-dispatch-gen.c
	${piglit_dispatch_gen_output_dir}/piglit-dispatch-gen.h
	${piglit_dispatch_gen_output_dir}/piglit-util-gl-enum-gen.c
	)

set(piglit_dispatch_gen_depends
	${CMAKE_SOURCE_DIR}/registry/__init__.py
	${CMAKE_SOURCE_DIR}/registry/gl.py
	${CMAKE_SOURCE_DIR}/registry/gl.xml
	${CMAKE_SOURCE_DIR}/tests/util/gen_dispatch.py
	${CMAKE_SOURCE_DIR}/tests/util/piglit-dispatch-gen.c.mako
	${CMAKE_SOURCE_DIR}/tests/util/piglit-dispatch-gen.h.mako
	${CMAKE_SOURCE_DIR}/tests/util/piglit-util-gl-enum-gen.c.mako
	)

add_custom_command(
	OUTPUT ${piglit_dispatch_gen_outputs}
	DEPENDS ${piglit_dispatch_gen_depends}
	COMMAND ${PYTHON_EXECUTABLE} ${piglit_dispatch_gen_script} --out-dir ${piglit_dispatch_gen_output_dir}
	)

add_custom_target(piglit_dispatch_gen
	DEPENDS ${piglit_dispatch_gen_outputs}
	)
