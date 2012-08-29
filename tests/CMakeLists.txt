include_directories(
	${piglit_BINARY_DIR}/tests/util
	${piglit_SOURCE_DIR}/tests/util
)

add_subdirectory (util)
add_subdirectory (bugs)
add_subdirectory (fbo)
add_subdirectory (general)
add_subdirectory (gles2)
add_subdirectory (glx)
add_subdirectory (glslparsertest)
add_subdirectory (hiz)
add_subdirectory (asmparsertest)
add_subdirectory (security)
add_subdirectory (shaders)
add_subdirectory (texturing)
add_subdirectory (spec)

if (TIFF_FOUND)
	add_subdirectory (glean)
endif (TIFF_FOUND)

IF(OPENGL_egl_LIBRARY)
	add_subdirectory (egl)
ENDIF(OPENGL_egl_LIBRARY)

IF(BUILD_CL_TESTS)
	add_subdirectory (cl)
ENDIF(BUILD_CL_TESTS)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_subdirectory (mesa)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
