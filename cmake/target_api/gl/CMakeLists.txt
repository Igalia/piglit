set(piglit_target_api "gl")
add_definitions(
	-DPIGLIT_USE_OPENGL
	)

if(NOT PIGLIT_USE_WAFFLE)
	add_definitions(-DPIGLIT_USE_GLUT)
	include_directories(
		${GLUT_INCLUDE_DIR}
		)
	link_libraries(
		${GLUT_glut_LIBRARY}
		)
endif()

add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)
