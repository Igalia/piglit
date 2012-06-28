add_definitions(-DSOURCE_DIR="${piglit_SOURCE_DIR}/")

set(UTIL_INCLUDES
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

set(UTIL_SOURCES
	piglit-util.c
	)

set(UTIL_GL_INCLUDES
	${UTIL_INCLUDES}
	${GLEXT_INCLUDE_DIR}
	${OPENGL_INCLUDE_PATH}
	)

set(UTIL_GL_SOURCES
	fdo-bitmap.c
	piglit-util-gl-common.c
	piglit-util-gl-enum.c
	piglit-framework.c
	piglit-framework-fbo.c
	piglit-framework-glut.c
        piglit_ktx.c
	rgb9e5.c
	)

set(UTIL_GL_LIBS
	piglitutil
	)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(UTIL_GL_LIBS
		${UTIL_GL_LIBS}
		${X11_X11_LIB}
	)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(OPENGL_egl_LIBRARY)
	set(UTIL_GL_SOURCES
	    ${UTIL_GL_SOURCES}
	    piglit-util-egl.c
	    )
	set(UTIL_GL_LIBS
		${UTIL_GL_LIBS}
		${OPENGL_egl_LIBRARY}
	)
endif(OPENGL_egl_LIBRARY)

if(USE_WAFFLE)
	set(UTIL_GL_LIBS
		${UTIL_GL_LIBS}
		"${WAFFLE_waffle_LIBRARY}"
	)
endif(USE_WAFFLE)

piglit_include_target_api()
