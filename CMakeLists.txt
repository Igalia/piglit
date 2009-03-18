
project (piglit)

include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
include(${CMAKE_ROOT}/Modules/FindTIFF.cmake)
include(${CMAKE_ROOT}/Modules/FindGLUT.cmake)
include(${CMAKE_ROOT}/Modules/FindPNG.cmake)

FIND_LIBRARY( GLEW_glew_LIBRARY GLEW
	/usr/lib
)

FIND_PATH( GLEW_INCLUDE_DIR GL/glew.h
      /usr/include/GL
)

set(EXECUTABLE_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)

add_subdirectory (tests)
