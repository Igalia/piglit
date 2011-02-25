add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )

include_directories(
	${OPENGL_INCLUDE_PATH}
)

link_libraries (
	${OPENGL_egl_LIBRARY}
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_library (piglitglutegl_x11
		    glut_egl.c
		    glut_egl_x11.c
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
