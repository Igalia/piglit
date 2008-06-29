
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
)

link_directories (
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	mesautil
	piglitutil
)

add_executable (cubemap cubemap.c)
add_executable (lodbias lodbias.c)
add_executable (texdepth texdepth.c)
add_executable (texrect-many texrect-many.c)
