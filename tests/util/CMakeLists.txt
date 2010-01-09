add_definitions ( -DSOURCE_DIR="\\\"${piglit_SOURCE_DIR}/\\\"" )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

add_library (piglitutil
	    piglit-util.c
	    shader-load.c
	    piglit-framework.c
)
