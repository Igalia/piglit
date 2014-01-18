add_definitions(-DSOURCE_DIR="${piglit_SOURCE_DIR}/")

if(PIGLIT_HAS_POSIX_CLOCK_MONOTONIC AND HAVE_LIBRT)
    link_libraries(rt)
endif()

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
	# One needs to have at least one hardware driver present, otherwise
	# there is no point compiling just the dispatcher.
	if(PIGLIT_BUILD_DMA_BUF_TESTS)
		add_definitions(-DHAVE_LIBDRM)

		list(APPEND UTIL_GL_SOURCES
			piglit-framework-gl/piglit_drm_dma_buf.c
		)

		list(APPEND UTIL_GL_LIBS
			${LIBDRM_LDFLAGS}
			${XCB_DRI2_LDFLAGS}
		)

		list(APPEND UTIL_GL_INCLUDES
			${LIBDRM_INCLUDE_DIRS}
			${XCB_DRI2_INCLUDE_DIRS}
		)

		if(LIBDRM_INTEL_FOUND)
			add_definitions(-DHAVE_LIBDRM_INTEL)

			list(APPEND UTIL_GL_LIBS
				${LIBDRM_INTEL_LDFLAGS}
			)
		endif()
	endif()

	if(PIGLIT_HAS_X11)
		list(APPEND UTIL_GL_LIBS ${X11_X11_LIB})
	endif()
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

piglit_include_target_api()
