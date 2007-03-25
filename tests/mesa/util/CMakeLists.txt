
include_directories (
	${OPENGL_INCLUDE_PATH}
	${PNG_INCLUDE_DIR}
)
link_libraries (
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${TIFF_LIBRARY}
	${PNG_LIBRARIES}
)
add_definitions (
	${PNG_DEFINITIONS}
)

add_library (mesautil
	dumpstate.c
	glstate.c
	idproj.c
	matrix.c
	readtex.c
	trackball.c
	errcheck.c
	glutskel.c
	imagesgi.cpp
	showbuffer.c
	winpos.c
	writeimg.c
)
