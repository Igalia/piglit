add_definitions(-DSOURCE_DIR="${piglit_SOURCE_DIR}/")

if(PIGLIT_HAS_POSIX_CLOCK_MONOTONIC AND HAVE_LIBRT)
    link_libraries(rt)
endif()

# CMake requires that source file properties be set in the same directory where
# the property is used.
set_source_files_properties(
	${piglit_dispatch_gen_outputs}
	PROPERTIES GENERATED 1
	)

if(HAVE_LIBCACA)
    link_libraries(caca)
endif()

set(UTIL_INCLUDES
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

set(UTIL_SOURCES
	piglit-log.c
	piglit-util.c
	)

set(UTIL_GL_INCLUDES
	${UTIL_INCLUDES}
	${GLEXT_INCLUDE_DIR}
	${OPENGL_INCLUDE_PATH}
	${WAFFLE_INCLUDE_DIRS}
	)

set(UTIL_GL_SOURCES
	fdo-bitmap.c
	minmax-test.c
	piglit-dispatch.c
	piglit-dispatch-init.c
	piglit-fbo.cpp
	piglit-matrix.c
	piglit-test-pattern.cpp
	piglit-util-gl.c
	piglit-vbo.cpp
	piglit-framework-gl.c
	piglit-framework-gl/piglit_gl_framework.c
	piglit-shader.c
	piglit_ktx.c
	rgb9e5.c
	r11g11b10f.c
	sized-internalformats.c
	${CMAKE_BINARY_DIR}/tests/util/piglit-util-gl-enum-gen.c
	)

set(UTIL_GL_LIBS
	piglitutil
	)

if(PIGLIT_USE_WAFFLE)
	list(APPEND UTIL_GL_SOURCES
		piglit-framework-gl/piglit_fbo_framework.c
		piglit-framework-gl/piglit_wfl_framework.c
		piglit-framework-gl/piglit_winsys_framework.c
		piglit-util-waffle.c
	)

	if(PIGLIT_HAS_GBM)
		list(APPEND UTIL_GL_SOURCES
			piglit-framework-gl/piglit_gbm_framework.c
		)
	endif()
	if(PIGLIT_HAS_WAYLAND)
		list(APPEND UTIL_GL_SOURCES
			piglit-framework-gl/piglit_wl_framework.c
		)
	endif()
	if(PIGLIT_HAS_X11)
		list(APPEND UTIL_GL_SOURCES
			piglit-framework-gl/piglit_x11_framework.c
		)
	endif()

	list(APPEND UTIL_GL_LIBS
		${WAFFLE_LDFLAGS}
	)
	IF(PIGLIT_BUILD_GLX_TESTS)
		list(APPEND UTIL_GL_LIBS
			${OPENGL_gl_LIBRARY}
		)
	ENDIF(PIGLIT_BUILD_GLX_TESTS)
else()
	list(APPEND UTIL_GL_SOURCES
		piglit-framework-gl/piglit_glut_framework.c
	)
endif()

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
