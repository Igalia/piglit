add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

set (UTIL_SOURCES
	fdo-bitmap.c
	piglit-util.c
	shader-load.c
	piglit-framework.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions ( -DUSE_GLX )
	add_library (piglitglxutil
		    piglit-util.c
		    piglit-glx-framework.c
		    piglit-glx-util.c
	)
	set (UTIL_SOURCES
		${UTIL_SOURCES}
		piglit-glx-util.c
	)

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library (piglitutil
	${UTIL_SOURCES}
)
