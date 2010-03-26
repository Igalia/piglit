cmake_minimum_required(VERSION 2.6)

INCLUDE (CheckCCompilerFlag)

project (piglit)

include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
include(${CMAKE_ROOT}/Modules/FindTIFF.cmake)
include(${CMAKE_ROOT}/Modules/FindGLUT.cmake)
include(${CMAKE_ROOT}/Modules/FindPNG.cmake)

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

set(EXECUTABLE_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)

add_subdirectory (tests)
