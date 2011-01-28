include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/spec/arb_color_buffer_float
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	piglitutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
)

add_executable (ext_fog_coord-modes modes.c)
IF (UNIX)
	target_link_libraries (ext_fog_coord-modes m)
ENDIF (UNIX)
