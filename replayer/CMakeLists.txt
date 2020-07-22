# coding=utf-8
#
# Copyright 2012 Intel Corporation
# Copyright © 2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

set(piglit_replayer_script ${piglit_SOURCE_DIR}/replayer/replayer.py)
set(piglit_replayer_output_dir ${piglit_BINARY_DIR}/bin)

set(piglit_replayer_outputs
	${piglit_replayer_output_dir}/replayer.py
	)

set(piglit_replayer_depends
	${CMAKE_SOURCE_DIR}/replayer/replayer.py
	)

add_custom_command(
	OUTPUT ${piglit_replayer_outputs}
	DEPENDS ${piglit_replayer_depends}
	COMMAND ${CMAKE_COMMAND} -E copy ${piglit_replayer_script} ${piglit_replayer_output_dir}
	)

add_custom_target(piglit_replayer ALL
	DEPENDS ${piglit_replayer_outputs}
	)

install(PROGRAMS ${piglit_replayer_outputs} DESTINATION ${PIGLIT_INSTALL_LIBDIR}/bin)
