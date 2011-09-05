add_definitions(-DSOURCE_DIR="${piglit_SOURCE_DIR}/")

include_directories(
	${OPENGL_INCLUDE_PATH}
	${piglit_SOURCE_DIR}/tests/util
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	link_libraries(${X11_X11_LIB})
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(UTIL_SOURCES
	fdo-bitmap.c
	piglit-util.c
	piglit-shader.c
	shader-load.c
	piglit-framework.c
	rgb9e5.c
	)

piglit_include_target_api()
