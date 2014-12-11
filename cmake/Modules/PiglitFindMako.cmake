# Copyright 2014 Intel Corporation
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

# This module sets the following variables:
#
#   MAKO_REQUIRED_VERSION
#
#   MAKO_FOUND (CACHE)
#       True if and only if the installed Mako version is at least
#       MAKO_REQUIRED_VERSION.
#
#    MAKO_VERSION (CACHE)
#       If MAKO_FOUND, then this is the installed Mako's full version string,
#       given by the Python value ``mako.__version__``. Otherwise,
#       "MAKO_VERSION-NOTFOUND".
#
# This module avoids checking the installed Mako version when not needed, by
# performing the check the check only if the cached MAKO_VERSION does not
# satisfy the current value of MAKO_REQUIRED_VERSION.

set(MAKO_REQUIRED_VERSION "0.7.3")

set(__MAKO_CHECK_VERSION_PY "
try:
	import mako
except ImportError as err:
	import sys
	sys.exit(err)
else:
	print(mako.__version__)
")

set(__MAKO_INSTALL_HINT "Hint: Try installing Mako with `pip install --user --upgrade Mako`")

if(MAKO_VERSION VERSION_LESS MAKO_REQUIRED_VERSION)
	message(STATUS "Looking for Mako >= ${MAKO_REQUIRED_VERSION}")

	set(MAKO_FOUND false)
	set(MAKO_VERSION "MAKO_VERSION-NOTFOUND")

	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} -c "${__MAKO_CHECK_VERSION_PY}"
		OUTPUT_VARIABLE __MAKO_ACTUAL_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_VARIABLE __MAKO_STDERR
		ERROR_STRIP_TRAILING_WHITESPACE
		RESULT_VARIABLE __MAKO_EXIT_STATUS
	)

	if(NOT __MAKO_EXIT_STATUS EQUAL 0)
		message(SEND_ERROR
			"  Failed to find Mako\n"
			"  ${__MAKO_INSTALL_HINT}\n")
	elseif(__MAKO_ACTUAL_VERSION VERSION_LESS MAKO_REQUIRED_VERSION)
		message(SEND_ERROR
			"  Found Mako ${__MAKO_ACTUAL_VERSION}, but Mako >= ${MAKO_REQUIRED_VERSION} is required\n"
			"  ${__MAKO_INSTALL_HINT}\n")
	else()
		message(STATUS "Found Mako ${__MAKO_ACTUAL_VERSION}")
		set(MAKO_FOUND true)
		set(MAKO_VERSION "${__MAKO_ACTUAL_VERSION}")
	endif()
endif()

if(NOT MAKO_FOUND)
endif()

set(MAKO_FOUND "${MAKO_FOUND}"
    CACHE INTERNAL "Was Mako >= ${MAKO_REQUIRED_VERSION} found?"
    FORCE
)
set(MAKO_VERSION "${MAKO_VERSION}"
    CACHE INTERNAL "Full version string of installed Mako, if MAKO_FOUND."
    FORCE
)
