add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

add_library (piglitutil
	    fdo-bitmap.c
	    piglit-util.c
	    shader-load.c
	    piglit-framework.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_library (piglitglxutil
		    piglit-util.c
		    piglit-glx-util.c
	)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
