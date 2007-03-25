
#add_definitions ( -D__X11__ -D__UNIX__ )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
)

link_directories (
	${piglit_SOURCE_DIR}/tests/mesa/util
)

link_libraries (
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	mesautil
)

add_executable (crossbar crossbar.c)
add_executable (texline texline.c)
#add_executable (arbfptest1 arbfptest1.c)
