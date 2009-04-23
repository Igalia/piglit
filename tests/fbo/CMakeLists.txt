
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
)

link_directories (
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	mesautil
	piglitutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
)

add_executable (fbo-3d fbo-3d.c)
add_executable (fbo-readpixels fbo-readpixels.c)
add_executable (fbo-cubemap fbo-cubemap.c)
