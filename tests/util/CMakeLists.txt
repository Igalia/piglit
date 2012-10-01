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
	piglit-framework-gl/piglit_gl_framework.c
	piglit-framework-gl.c
        piglit_ktx.c
	rgb9e5.c
	)

if(PIGLIT_USE_WAFFLE)
	list(APPEND UTIL_GL_SOURCES
		piglit-framework-gl/piglit_gbm_framework.c
		piglit-framework-gl/piglit_fbo_framework.c
		piglit-framework-gl/piglit_wfl_framework.c
		piglit-framework-gl/piglit_winsys_framework.c
		piglit-framework-gl/piglit_wl_framework.c
		piglit-util-waffle.c
	)

	if(PIGLIT_HAS_X11)
		list(APPEND UTIL_GL_SOURCES
			piglit-framework-gl/piglit_x11_framework.c
		)
	endif()
else()
	list(APPEND UTIL_GL_SOURCES
		piglit-framework-gl/piglit_glut_framework.c
	)
endif()

set(UTIL_GL_LIBS
	piglitutil
        ${WAFFLE_LDFLAGS}
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

piglit_include_target_api()
