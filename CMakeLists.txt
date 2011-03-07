cmake_minimum_required(VERSION 2.6)

INCLUDE (CheckCCompilerFlag)
INCLUDE (CheckFunctionExists)
INCLUDE (CheckIncludeFile)

project (piglit)

find_package(OpenGL REQUIRED)
find_package(TIFF REQUIRED)
find_package(GLUT REQUIRED)
find_package(PNG REQUIRED)
find_package(X11)
find_library(OPENGL_gles2_LIBRARY NAMES GLESv2)

CHECK_C_COMPILER_FLAG("-Wall" C_COMPILER_FLAG_WALL)
IF (C_COMPILER_FLAG_WALL)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
ENDIF (C_COMPILER_FLAG_WALL)

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
find_library(OPENGL_gles2_LIBRARY NAMES GLESv2)

set(EXECUTABLE_OUTPUT_PATH ${piglit_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${piglit_BINARY_DIR}/lib)

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

include_directories(src)
add_subdirectory(cmake/target_api)
