cmake_minimum_required(VERSION 2.6)

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckCXXCompilerFlag)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)

project (piglit)

find_package(OpenGL REQUIRED)
find_package(TIFF)
find_package(GLUT REQUIRED)
find_package(PNG REQUIRED)
find_package(X11)

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

# Check for the presence of numpy, which is needed to build generated
# tests.
execute_process(
	COMMAND ${python} -c "import numpy"
	OUTPUT_QUIET
	ERROR_QUIET
	RESULT_VARIABLE import_numpy_error_code)
if(NOT import_numpy_error_code EQUAL 0)
	message(FATAL_ERROR "numpy library not found")
endif(NOT import_numpy_error_code EQUAL 0)

if (NOT MSVC)
	CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_FLAG_WALL)
	IF (C_COMPILER_FLAG_WALL)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	ENDIF (C_COMPILER_FLAG_WALL)
	CHECK_CXX_COMPILER_FLAG("-Wall" CXX_COMPILER_FLAG_WALL)
	IF (CXX_COMPILER_FLAG_WALL)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
	ENDIF (CXX_COMPILER_FLAG_WALL)
else ()
	# -Wall or (/Wall) is actually supported by MSVC and would be detected
	# by CHECK_C_COMPILER_FLAG above, but is very pedantic, causing
	# thousand of warnings when including windows.h.
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W4")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W4")

	add_definitions (-D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_WARNINGS)
	add_definitions (-D_SCL_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_WARNINGS)
endif ()

if (WIN32)
	# MSVC & MinGW only define & use APIENTRY
	add_definitions (-DGLAPIENTRY=__stdcall)

	# Don't define min/max macros
	add_definitions (-DNOMINMAX)

	# Define M_PI and others
	add_definitions (-D_USE_MATH_DEFINES)
endif (WIN32)

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

FIND_LIBRARY(OPENGL_egl_LIBRARY
      NAMES EGL
      PATHS /usr/lib
)
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

check_function_exists(strchrnul HAVE_STRCHRNUL)
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
	"${piglit_SOURCE_DIR}/tests/util/config.h"
)

include_directories(src)
add_subdirectory(cmake/target_api)
add_subdirectory(generated_tests)
