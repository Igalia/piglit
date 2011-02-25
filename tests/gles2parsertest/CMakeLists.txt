add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${piglit_SOURCE_DIR}/tests/util/
	${piglit_SOURCE_DIR}/tests/util/glut_egl
)

link_directories (
	${piglit_SOURCE_DIR}/tests/util/
	${piglit_SOURCE_DIR}/tests/util/glut_egl
)

link_libraries (
	${OPENGLES_gles2_LIBRARY}
	${OPENGL_egl_LIBRARY}
	piglitglutegl_x11
	piglitegl_gles2util
)


add_executable (gles2parsertest gles2parsertest.c)
target_link_libraries(gles2parsertest X11)
