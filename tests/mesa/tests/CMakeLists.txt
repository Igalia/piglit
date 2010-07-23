
#add_definitions ( -D__X11__ -D__UNIX__ )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${PNG_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
)

link_directories (
	${piglit_SOURCE_DIR}/tests/mesa/util
)

link_libraries (
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${PNG_LIBRARIES}
	${TIFF_LIBRARY}
	mesautil
)

add_definitions (
	${PNG_DEFINITIONS}
)

add_executable (texline texline.c)
IF (UNIX)
       target_link_libraries (texline m)
ENDIF (UNIX)

#add_executable (arbfptest1 arbfptest1.c)
