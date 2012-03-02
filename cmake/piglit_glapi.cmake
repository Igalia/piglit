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

# Note: we're outputting the generated file to a subdirectory of
# ${CMAKE_SOURCE_DIR} so that we can check it back in to source
# control.
set(piglit_glapi_src_dir ${CMAKE_SOURCE_DIR}/glapi)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/glapi)

set(piglit_glapi_output ${CMAKE_BINARY_DIR}/glapi/glapi.json)

set(piglit_glapi_inputs
	${piglit_glapi_src_dir}/parse_glspec.py
	${piglit_glapi_src_dir}/gl.tm
	${piglit_glapi_src_dir}/gl.spec
	${piglit_glapi_src_dir}/enumext.spec
	)

add_custom_command(
	OUTPUT ${piglit_glapi_output}
	DEPENDS ${piglit_glapi_inputs}
	COMMAND ${python} ${piglit_glapi_inputs} ${piglit_glapi_output}
	)
