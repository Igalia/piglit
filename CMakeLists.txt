
project (piglit)

include(${CMAKE_ROOT}/Modules/FindOpenGL.cmake)
include(${CMAKE_ROOT}/Modules/FindTIFF.cmake)
include(${CMAKE_ROOT}/Modules/FindGLUT.cmake)

add_subdirectory (tests)
