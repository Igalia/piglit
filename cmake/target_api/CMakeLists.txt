# Copyright (c) 2010-2012 Intel Corporation
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

# The no_api directory must precede the GL directories because it may build
# api-independent libraries on which the api-dependent targets rely.
add_subdirectory(no_api)

if(PIGLIT_BUILD_GL_TESTS)
	add_subdirectory(gl)
endif(PIGLIT_BUILD_GL_TESTS)

if(PIGLIT_BUILD_GLES1_TESTS)
	add_subdirectory(gles1)
endif(PIGLIT_BUILD_GLES1_TESTS)

if(PIGLIT_BUILD_GLES2_TESTS)
	add_subdirectory(gles2)
endif(PIGLIT_BUILD_GLES2_TESTS)

if(PIGLIT_BUILD_GLES3_TESTS)
	add_subdirectory(gles3)
endif(PIGLIT_BUILD_GLES3_TESTS)

if(PIGLIT_BUILD_CL_TESTS)
	add_subdirectory(cl)
endif(PIGLIT_BUILD_CL_TESTS)

if(PIGLIT_BUILD_VULKAN_TESTS)
	add_subdirectory(vulkan)
endif(PIGLIT_BUILD_VULKAN_TESTS)
