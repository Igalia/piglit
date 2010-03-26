
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
	piglitglxutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	${GLEW_glew_LIBRARY}
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable (glx-multithread glx-multithread.c)
	target_link_libraries(glx-multithread pthread)
	add_executable (glx-swap-exchange glx-swap-exchange.c)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
