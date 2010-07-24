cmake_minimum_required(VERSION 2.6)

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)

project (piglit)

include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
include(${CMAKE_ROOT}/Modules/FindTIFF.cmake)
include(${CMAKE_ROOT}/Modules/FindGLUT.cmake)
include(${CMAKE_ROOT}/Modules/FindPNG.cmake)
include(${CMAKE_ROOT}/Modules/FindX11.cmake)

CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_FLAG_WALL)
IF (C_COMPILER_FLAG_WALL)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
ENDIF (C_COMPILER_FLAG_WALL)

FIND_LIBRARY( GLEW_glew_LIBRARY GLEW
	/usr/lib
)

FIND_PATH( GLEW_INCLUDE_DIR GL/glew.h
      /usr/include/GL
)

FIND_LIBRARY(OPENGL_egl_LIBRARY
      NAMES EGL
      PATHS /usr/lib
)

set(EXECUTABLE_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)

add_subdirectory (tests)

check_function_exists(strchrnul HAVE_STRCHRNUL)
check_function_exists(fopen_s   HAVE_FOPEN_S)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/stat.h  HAVE_SYS_STAT_H)
check_include_file(unistd.h    HAVE_UNISTD_H)
check_include_file(fcntl.h     HAVE_FCNTL_H)

configure_file(
	"${piglit_SOURCE_DIR}/tests/util/config.h.in"
	"${piglit_SOURCE_DIR}/tests/util/config.h"
)
