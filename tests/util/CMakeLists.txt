add_definitions(-DSOURCE_DIR="${piglit_SOURCE_DIR}/")

include_directories(
	${OPENGL_INCLUDE_PATH}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	link_libraries(${X11_X11_LIB})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(UTIL_SOURCES
	fdo-bitmap.c
	piglit-util.c
	piglit-util-enum.c
	shader-load.c
	piglit-framework.c
	rgb9e5.c
	)

if(OPENGL_egl_LIBRARY)
	set(UTIL_SOURCES
	    ${UTIL_SOURCES}
	    piglit-util-egl.c
	    )
endif(OPENGL_egl_LIBRARY)

piglit_include_target_api()
