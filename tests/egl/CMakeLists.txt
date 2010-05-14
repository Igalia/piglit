
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

link_directories (
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	piglitutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${OPENGL_egl_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	${GLEW_glew_LIBRARY}
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable (egl-nok-swap-region egl-util.c egl-nok-swap-region.c)
	target_link_libraries(egl-nok-swap-region pthread X11)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
