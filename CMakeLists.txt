cmake_minimum_required(VERSION 2.6)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckCSourceCompiles)
INCLUDE (CheckCXXCompilerFlag)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)
INCLUDE (FindPkgConfig)

project (piglit)

find_package(X11)
if(X11_FOUND)
	set(PIGLIT_HAS_X11 True)
	add_definitions(-DPIGLIT_HAS_X11)
endif()


option(PIGLIT_BUILD_GL_TESTS "Build tests for OpenGL" ON)
option(PIGLIT_BUILD_GLES1_TESTS "Build tests for OpenGL ES1" OFF)
option(PIGLIT_BUILD_GLES2_TESTS "Build tests for OpenGL ES2" OFF)
option(PIGLIT_BUILD_GLES3_TESTS "Build tests for OpenGL ES3" OFF)
option(PIGLIT_BUILD_CL_TESTS "Build tests for OpenCL" OFF)

if(PIGLIT_BUILD_GL_TESTS)
	find_package(OpenGL REQUIRED)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	option(PIGLIT_USE_WAFFLE "Use Waffle in place of GLUT" ON)
else()
	option(PIGLIT_USE_WAFFLE "Use Waffle in place of GLUT" OFF)
endif()

if(PIGLIT_USE_WAFFLE)
	pkg_check_modules(WAFFLE REQUIRED waffle-1)

	if(NOT WAFFLE_FOUND)
		message(FATAL_ERROR "Failed to find Waffle. If Waffle is not "
		"packaged for your distribution, you can get it at "
		"http://people.freedesktop.org/~chadversary/waffle."
	)
	endif()

	# Check the installed waffle version.
	#
	# We cannot reliably check the version with pkg_check_modules(), but
	# instead must check the version manually as below. The problem is that,
	# if one passes a required version to pkg_check_modules(), CMake
	# validates the required version at most once for the lifetime of the
	# source tree.  If someone changes the required version by editing the
	# CMakeLists, CMake fails to detect the new requirement.
	set(WAFFLE_REQUIRED_VERSION "1.3.0")
	if(WAFFLE_VERSION VERSION_LESS WAFFLE_REQUIRED_VERSION)
		message(FATAL_ERROR "Found waffle-${WAFFLE_VERSION}, but "
		"piglit requires waffle-${WAFFLE_REQUIRED_VERSION}")
	endif()

	# Check the installed waffle version.
	#
	# We cannot reliably check the version with pkg_check_modules(), but
	# instead must check the version manually as below. The problem is that,
	# if one passes a required version to pkg_check_modules(), CMake
	# validates the required version at most once for the lifetime of the
	# source tree.  If someone changes the required version by editing the
	# CMakeLists, CMake fails to detect the new requirement.
	set(WAFFLE_REQUIRED_VERSION "1.3.0")
	if(WAFFLE_VERSION VERSION_LESS WAFFLE_REQUIRED_VERSION)
		message(FATAL_ERROR "Found waffle-${WAFFLE_VERSION}, but "
		"piglit requires waffle-${WAFFLE_REQUIRED_VERSION}")
        endif()

	add_definitions(-DPIGLIT_USE_WAFFLE)
	add_definitions(-DWAFFLE_API_VERSION=0x0103)
	add_definitions(-DPIGLIT_HAS_WAYLAND)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WAFFLE_CFLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WAFFLE_CFLAGS}")
else()
	find_package(GLUT REQUIRED)

	# The 'REQUIRED' above correctly produces an error for
	# OpenGL, but there's a bug involving FindGLUT.cmake
	# that fails to produce the error as of CMake 2.8.5.
	#
	# Instead, CMake keeps going and eventually spams
	# the console with a message for every target that used
	# e.g. the ${GLUT_INCLUDE_DIR} variable. So it
	# prints a line for basically every single test in piglit.
	#
	# Work around the bug and error out quickly here instead.
	if (NOT GLUT_FOUND)
		message(FATAL_ERROR "GLUT library not found")
	endif()
endif(PIGLIT_USE_WAFFLE)

if(PIGLIT_BUILD_GLES1_TESTS AND NOT PIGLIT_USE_WAFFLE)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES1_TESTS requires PIGLIT_USE_WAFFLE")
endif(PIGLIT_BUILD_GLES1_TESTS AND NOT PIGLIT_USE_WAFFLE)

if(PIGLIT_BUILD_GLES2_TESTS AND NOT PIGLIT_USE_WAFFLE)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES2_TESTS requires PIGLIT_USE_WAFFLE")
endif(PIGLIT_BUILD_GLES2_TESTS AND NOT PIGLIT_USE_WAFFLE)

if(PIGLIT_BUILD_GLES3_TESTS AND NOT PIGLIT_USE_WAFFLE)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES3_TESTS requires PIGLIT_USE_WAFFLE")
endif(PIGLIT_BUILD_GLES3_TESTS AND NOT PIGLIT_USE_WAFFLE)

if(PIGLIT_BUILD_CL_TESTS)
	find_package(OpenCL REQUIRED)
endif(PIGLIT_BUILD_CL_TESTS)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(PIGLIT_HAS_GBM True)
	add_definitions(-DPIGLIT_HAS_GBM)

	set(PIGLIT_HAS_GLX True)
	add_definitions(-DPIGLIT_HAS_GLX)

	pkg_check_modules(LIBDRM QUIET libdrm)
	pkg_check_modules(LIBDRM_INTEL QUIET libdrm_intel)
	pkg_check_modules(XCB_DRI2 QUIET xcb-dri2)

	option(PIGLIT_BUILD_GLX_TESTS "Build tests that require GLX" ON)
ELSE()
	option(PIGLIT_BUILD_GLX_TESTS "Build tests that require GLX" OFF)
ENDIF()

# Choose to build tests that use dma_buf.
#
# Piglit's dma_buf utilities require xcb-dri2 to gain DRM authentication.
#
# The presence of libdrm is not sufficient. At least one libdrm_${hardware}
# library is also needed.
#
# When building for Intel, libdrm_intel>=2.4.38 is required because support for
# drm-prime arrived in that version.
#
if(LIBDRM_FOUND AND XCB_DRI2_FOUND AND
   (LIBDRM_INTEL_VERSION VERSION_GREATER "2.4.37"))
	set(PIGLIT_BUILD_DMA_BUF_TESTS_IS_VALID true)
else()
	set(PIGLIT_BUILD_DMA_BUF_TESTS_IS_VALID false)
endif()

if(PIGLIT_BUILD_DMA_BUF_TESTS_IS_VALID)
	option(PIGLIT_BUILD_DMA_BUF_TESTS "Build tests that use dma_buf" ON)
else()
	option(PIGLIT_BUILD_DMA_BUF_TESTS "Build tests that use dma_buf" OFF)
endif()

# If the users has updated PIGLIT_BUILD_DMA_BUF_TESTS in the CMakeCache, then
# we need to validate it.
if(PIGLIT_BUILD_DMA_BUF_TESTS AND NOT PIGLIT_BUILD_DMA_BUF_TESTS_IS_VALID)
	message(FATAL_ERROR
		"PIGLIT_BUILD_DMA_BUF_TESTS require libdrm, "
		"libdrm_intel>=2.4.38, and xcb-dri2")
endif()

IF(PIGLIT_BUILD_GLX_TESTS)
	pkg_check_modules(GLPROTO REQUIRED glproto)
ENDIF()

# Check for presence of Python 2.6 or greater.
foreach(python_cmd python2 python)
	execute_process(
		COMMAND ${python_cmd} -c "import sys; assert '2.6' <= sys.version < '3'"
		OUTPUT_QUIET
		ERROR_QUIET
		RESULT_VARIABLE python_version_check_error_code)
	if(python_version_check_error_code EQUAL 0)
		set(python ${python_cmd})
		break()
	endif(python_version_check_error_code EQUAL 0)
endforeach(python_cmd)

if(NOT DEFINED python)
	message(FATAL_ERROR "python version 2.x (where x >= 6) required")
endif(NOT DEFINED python)

# Check for the presence of several python packages, which are needed to build
# generated tests.
execute_process(
	COMMAND ${python} -c "import numpy"
	OUTPUT_QUIET
	ERROR_QUIET
	RESULT_VARIABLE import_numpy_error_code)
if(NOT import_numpy_error_code EQUAL 0)
	message(FATAL_ERROR "numpy python module not found")
endif(NOT import_numpy_error_code EQUAL 0)
execute_process(
	COMMAND ${python} -c "from mako.template import Template"
	OUTPUT_QUIET
	ERROR_QUIET
	RESULT_VARIABLE import_mako_error_code)
if(NOT import_mako_error_code EQUAL 0)
	message(FATAL_ERROR "mako.template python module not found")
endif(NOT import_mako_error_code EQUAL 0)

# Default to compiling with debug information (`gcc -g`):
if(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING
	"May be one of: None Debug RelWithDebInfo Release MinSizeRel" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

if (NOT MSVC)
	CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_FLAG_WALL)
	IF (C_COMPILER_FLAG_WALL)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	ENDIF (C_COMPILER_FLAG_WALL)
	CHECK_CXX_COMPILER_FLAG("-Wall" CXX_COMPILER_FLAG_WALL)
	IF (CXX_COMPILER_FLAG_WALL)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	ENDIF (CXX_COMPILER_FLAG_WALL)

	# Unfortunately MSVC does not support C99.  Among all features enabled
	# by C99, declarations after statements is the most frequently used.
	# For portability sake, we request gcc to warn when this is used.
	CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_FLAG_WDECL_AFTER_STMT)
	IF (C_COMPILER_FLAG_WDECL_AFTER_STMT)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wdeclaration-after-statement")
	ENDIF (C_COMPILER_FLAG_WDECL_AFTER_STMT)

	CHECK_CXX_COMPILER_FLAG("-Wno-narrowing" CXX_COMPILER_FLAG_WNO_NARROWING)
	IF (CXX_COMPILER_FLAG_WNO_NARROWING)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
	ENDIF (CXX_COMPILER_FLAG_WNO_NARROWING)
else ()
	include_directories("include/msvc/c99")

	# -Wall or (/Wall) is actually supported by MSVC and would be detected
	# by CHECK_C_COMPILER_FLAG above, but is very pedantic, causing
	# thousand of warnings when including windows.h.
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W4")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")

	add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif ()

if (MINGW)
	# Avoid depending on MinGW runtime DLLs
	check_cxx_compiler_flag (-static-libgcc HAVE_STATIC_LIBGCC_FLAG)
	if (HAVE_STATIC_LIBGCC_FLAG)
		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc")
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libgcc")
		set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libgcc")
	endif ()
	check_cxx_compiler_flag (-static-libstdc++ HAVE_STATIC_LIBSTDCXX_FLAG)
	if (HAVE_STATIC_LIBSTDCXX_FLAG)
		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libstdc++")
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static-libstdc++")
		set (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -static-libstdc++")
	endif ()
endif ()

if (${CMAKE_C_COMPILER_ID} STREQUAL "SunPro")
	# Use C++ to link C files.
	# http://developers.sun.com/solaris/articles/mixing.html#linking
	# Modified rule from Modules/CMakeCInformation.cmake.
	set (CMAKE_C_LINK_EXECUTABLE
		"<CMAKE_CXX_COMPILER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
endif()

# Always enable GNU C extensions.  Non-GNU platforms will need to
# define wrappers for them.
add_definitions(-D_GNU_SOURCE)

if (WIN32)
	# MSVC & MinGW only define & use APIENTRY
	add_definitions (-DGLAPIENTRY=__stdcall)

	# Avoid namespace pollution when including windows.h
	# http://support.microsoft.com/kb/166474
	add_definitions (-DWIN32_LEAN_AND_MEAN=1)

	# Don't define min/max macros
	add_definitions (-DNOMINMAX)

	# Define M_PI and others
	add_definitions (-D_USE_MATH_DEFINES)
endif (WIN32)

if (OPENGL_FOUND)
	if (APPLE)
		find_path(GLEXT_INCLUDE_DIR
			NAMES OpenGL/glext.h
			PATHS ${OPENGL_INCLUDE_DIR}
			DOC "Include for OpenGL/glext.h on OSX"
		)
	else (APPLE)
		find_path(GLEXT_INCLUDE_DIR
			NAMES GL/glext.h
			PATHS ${OPENGL_INCLUDE_DIR}
			DOC "Include for GL/glext.h"
		)
	endif (APPLE)
endif()

FIND_LIBRARY(HAVE_LIBRT NAMES rt)
if(HAVE_LIBRT)
	set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} rt)
endif()
check_c_source_compiles(
	"
	#define _POSIX_C_SOURCE 199309L
	#include <time.h>
	int main() { return clock_gettime(CLOCK_MONOTONIC, NULL); }
	"
	PIGLIT_HAS_POSIX_CLOCK_MONOTONIC
)
set(CMAKE_REQUIRED_LIBRARIES)

if(PIGLIT_HAS_POSIX_CLOCK_MONOTONIC)
	add_definitions(-DPIGLIT_HAS_POSIX_CLOCK_MONOTONIC)
endif()

if(PIGLIT_USE_WAFFLE AND ${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	pkg_check_modules(EGL egl)
endif()

if(EGL_FOUND)
	add_definitions(-DPIGLIT_HAS_EGL)
	include_directories(${EGL_INCLUDE_DIRS})
endif()

if(PIGLIT_BUILD_GLES1_TESTS AND NOT EGL_FOUND)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES1_TESTS requires EGL. "
			    "Failed to find EGL library.")
endif()

if(PIGLIT_BUILD_GLES2_TESTS AND NOT EGL_FOUND)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES2_TESTS requires EGL. "
			    "Failed to find EGL library.")
endif()

if(PIGLIT_BUILD_GLES3_TESTS AND NOT EGL_FOUND)
	message(FATAL_ERROR "Option PIGLIT_BUILD_GLES3_TESTS requires EGL. "
			    "Failed to find EGL library.")
endif()

find_library(OPENGL_gles1_LIBRARY NAMES GLESv1_CM)
find_library(OPENGL_gles2_LIBRARY NAMES GLESv2)

# Put all executables into the bin subdirectory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${piglit_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${piglit_BINARY_DIR}/lib)

# Do the same for MSVC, regardless of the build type. This only works correctly
# for CMake 2.8.1 and above.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${piglit_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${piglit_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${piglit_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${piglit_BINARY_DIR}/bin)

check_function_exists(asprintf  HAVE_ASPRINTF)
check_function_exists(strchrnul HAVE_STRCHRNUL)
check_function_exists(strndup   HAVE_STRNDUP)
check_function_exists(fopen_s   HAVE_FOPEN_S)
check_function_exists(setrlimit HAVE_SETRLIMIT)

check_include_file(sys/time.h  HAVE_SYS_TIME_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/resource.h  HAVE_SYS_RESOURCE_H)
check_include_file(sys/stat.h  HAVE_SYS_STAT_H)
check_include_file(unistd.h    HAVE_UNISTD_H)
check_include_file(fcntl.h     HAVE_FCNTL_H)

configure_file(
	"${piglit_SOURCE_DIR}/tests/util/config.h.in"
	"${piglit_BINARY_DIR}/tests/util/config.h"
)

include(cmake/piglit_util.cmake)
include(cmake/piglit_glapi.cmake)
include(cmake/piglit_dispatch.cmake)

include_directories(src)
add_subdirectory(cmake/target_api)
add_subdirectory(generated_tests)


##############################################################################
# Packaging

install (
	FILES
		COPYING
		README
		RELEASE
	DESTINATION share/doc
)

install (
	PROGRAMS
		piglit-merge-results.py
		piglit-print-commands.py
		piglit-run.py
		piglit-resume.py
		piglit-summary.py
		piglit-summary-html.py
		piglit-summary-junit.py
	DESTINATION .
)

install (
	DIRECTORY framework
	DESTINATION .
	FILES_MATCHING PATTERN "*.py"
)

install (
	DIRECTORY templates
	DESTINATION .
)

install (
	DIRECTORY tests
	DESTINATION .
	FILES_MATCHING REGEX ".*\\.(py|program_test|shader_test|frag|vert|geom|ktx|cl|txt)$"
	REGEX "CMakeFiles|CMakeLists" EXCLUDE
)

install (
	DIRECTORY ${CMAKE_BINARY_DIR}/generated_tests
	DESTINATION .
	FILES_MATCHING REGEX ".*\\.(shader_test|program_test|frag|vert|geom|cl|txt)$"
	REGEX "CMakeFiles|CMakeLists" EXCLUDE
)


set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")

# Use current date in YYYYMMDD format as patch number
execute_process (
	COMMAND ${python} -c "import time, sys; sys.stdout.write(time.strftime('%Y%m%d'))"
	OUTPUT_VARIABLE CPACK_PACKAGE_VERSION_PATCH
)

# cpack mistakenly detects Mingw-w64 as win32
if (MINGW)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set (CPACK_SYSTEM_NAME win64)
	endif ()
endif ()

# See http://www.vtk.org/Wiki/CMake:CPackPackageGenerators
if (WIN32)
	set (CPACK_GENERATOR "ZIP")
else ()
	set (CPACK_GENERATOR "TBZ2")
endif ()

include(CPack)
