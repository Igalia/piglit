# Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# This module defines the following variables:
#
#   OPENCL_FOUND
#       True if OpenCL is installed.
#
#   OPENCL_INCLUDE_PATH
#
#   OPENCL_opencl_LIBRARY
#       Path to OpenCL's library.

  if (APPLE)
    find_path(OPENCL_INCLUDE_PATH OpenCL/cl.h)
  else()
    find_path(OPENCL_INCLUDE_PATH CL/opencl.h)
  endif()

  find_library(OPENCL_opencl_LIBRARY
      NAMES OpenCL CL cl)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCL
    DEFAULT_MSG
    OPENCL_opencl_LIBRARY OPENCL_INCLUDE_PATH
    )
