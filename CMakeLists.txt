
project (piglit)

include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
include(${CMAKE_ROOT}/Modules/FindTIFF.cmake)
include(${CMAKE_ROOT}/Modules/FindGLUT.cmake)
include(${CMAKE_ROOT}/Modules/FindPNG.cmake)

set(EXECUTABLE_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${piglit_SOURCE_DIR}/bin)

add_subdirectory (tests)
