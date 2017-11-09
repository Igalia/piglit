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

# function piglit_include_target_api
#
# If the file CMakeLists.${piglit_target_api}.txt exists in the current source
# directory, then include it.
#
function(piglit_include_target_api)

    # Verify that the value of `piglit_target_api` is valid.
    set(valid_api FALSE)

    foreach(api "gl" "gles1" "gles2" "gles3" "cl" "no_api")
        if(piglit_target_api STREQUAL ${api})
            set(valid_api TRUE)
            break()
        endif(piglit_target_api STREQUAL ${api})
    endforeach(api)

    if(NOT valid_api)
        message(FATAL_ERROR "Invalid value for piglit_target_api: ${piglit_target_api}")
    endif(NOT valid_api)

    # Include CMakeLists.${piglit_target_api}.txt` if it exists.
    set(api_file ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.${piglit_target_api}.txt)
    if(EXISTS ${api_file})
        include(${api_file})
    endif(EXISTS ${api_file})

endfunction(piglit_include_target_api)

#
# function piglit_add_executable
#
# This function wraps `add_executable` and has the same signature.
#
# In addition to calling `add_executable`, it adds to each object file
# a dependency on piglit_dispatch's generated files.
#
function(piglit_add_executable name)

    piglit_create_manifest_file(${name})

    list(REMOVE_AT ARGV 0)
    add_executable(${name} ${ARGV})
    add_dependencies(${name} piglit_dispatch_gen)

    install(TARGETS ${name} DESTINATION ${PIGLIT_INSTALL_LIBDIR}/bin)

endfunction(piglit_add_executable)

#
# function piglit_add_library
#
# This function wraps `add_library` and has the same signature.
#
# In addition to calling `add_library`, it adds to each object file
# a dependency on piglit_dispatch's generated files.
#
function(piglit_add_library name)

    list(REMOVE_AT ARGV 0)
    if(WIN32)
        add_library(${name} STATIC ${ARGV})
    else(WIN32)
        add_library(${name} SHARED ${ARGV})
        install(TARGETS ${name} DESTINATION ${PIGLIT_INSTALL_LIBDIR}/lib)
    endif(WIN32)
    add_dependencies(${name} piglit_dispatch_gen)
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
        set_target_properties(${name} PROPERTIES SOVERSION "0")
    endif()

endfunction(piglit_add_library)


# This is lame, but Windows 10 (and maybe Win8) asks for confirmation
# before running .exe files containing the strings "patch", "setup",
# "update", etc.  In Cygwin, we simply get "Permission Denied".
# This causes the Piglit test to fail.
# The work-around is to create a "manifest" file for such executables.
# This function examines the target name and creates the manifest file
# if needed.  The file will be named "${target}.exe.manifest".
# See https://answers.microsoft.com/en-us/windows/forum/windows_7-security/uac-prompts-on-any-program-with-the-word-patch-or/c5359497-d16e-43c6-99f2-db3d8eecc9c0?auth=1
function(piglit_create_manifest_file target)
   if (WIN32)
      # look for known strings
      string(FIND ${target} "patch" r1)
      string(FIND ${target} "setup" r2)
      string(FIND ${target} "update" r3)

      # if any of those strings is in the target filename
      if((${r1} GREATER -1) OR (${r2} GREATER -1) OR (${r3} GREATER -1))
	    # XXX we should probably use add_custom_command() here to copy
	    # the manifest file, but I've been unsuccessful in getting
	    # that to work.
	    file(GENERATE
		 OUTPUT $<TARGET_FILE:${target}>.manifest
		 INPUT ${CMAKE_SOURCE_DIR}/cmake/win10-manifest.txt)

	    install(FILES $<TARGET_FILE:${target}>.manifest
		    DESTINATION ${PIGLIT_INSTALL_LIBDIR}/bin)
       endif()
   endif()
endfunction(piglit_create_manifest_file)
