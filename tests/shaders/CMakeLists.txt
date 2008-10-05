
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
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	mesautil
	piglitutil
)

add_executable (trinity-fp1 trinity-fp1.c)
add_executable (fp-lit-mask fp-lit-mask.c)
add_executable (fp-fragment-position fp-fragment-position.c)
add_executable (fp-generic fp-generic.c)
add_executable (fp-kil fp-kil.c)
add_executable (fp-incomplete-tex fp-incomplete-tex.c)
add_executable (fp-indirections fp-indirections.c)
add_executable (vp-bad-program vp-bad-program.c)
