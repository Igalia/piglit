add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )

include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
)

set (UTIL_SOURCES
	fdo-bitmap.c
	piglit-shader.c
	piglit-util.c
	piglit-shared-util.c
	shader-load.c
	piglit-framework.c
	glew.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions ( -DUSE_GLX )
	add_library (piglitglxutil
		    piglit-shader.c
		    piglit-util.c
		    piglit-shared-util.c
		    piglit-glx-framework.c
		    piglit-glx-util.c
		    glew.c
	)
	set (UTIL_SOURCES
		${UTIL_SOURCES}
		piglit-glx-util.c
	)

	link_libraries (${X11_X11_LIB})

ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

add_library (piglitutil
	${UTIL_SOURCES}
)

if(UNIX)
	target_link_libraries(piglitutil m)
endif(UNIX)

IF(OPENGL_egl_LIBRARY)
	add_subdirectory (glut_egl)
ENDIF(OPENGL_egl_LIBRARY)
