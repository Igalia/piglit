
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

link_directories (
	${piglit_SOURCE_DIR}/tests/mesa/util
)

link_libraries (
	piglitutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	${GLEW_glew_LIBRARY}
)

add_executable (crash-cubemap-order crash-cubemap-order.c)
add_executable (crash-texparameter-before-teximage crash-texparameter-before-teximage.c)
add_executable (fdo9833 fdo9833.c)
add_executable (fdo10370 fdo10370.c)
add_executable (fdo14575 fdo14575.c)
add_executable (r300-readcache r300-readcache.c)
add_executable (tex1d-2dborder tex1d-2dborder.c)
add_executable (fdo20701 fdo20701.c)
add_executable (point-sprite point-sprite.c)
add_executable (fdo22540 fdo22540.c)
add_executable (fdo23489 fdo23489.c)
add_executable (fdo23670-depth_test fdo23670-depth_test.c)
add_executable (fdo23670-drawpix_stencil fdo23670-drawpix_stencil.c)
add_executable (fdo24066 fdo24066.c)
add_executable (fdo25614-genmipmap fdo25614-genmipmap.c)
