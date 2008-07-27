
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
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
	piglitutil
)

add_executable (crash-texparameter-before-teximage crash-texparameter-before-teximage.c)
add_executable (fdo9833 fdo9833.c)
add_executable (fdo10370 fdo10370.c)
add_executable (fdo14575 fdo14575.c)
add_executable (r300-readcache r300-readcache.c)
add_executable (tex1d-2dborder tex1d-2dborder.c)
