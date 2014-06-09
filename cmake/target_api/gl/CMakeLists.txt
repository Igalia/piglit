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

	# Check if glutInitErrorFunc is available
	if (NOT APPLE)
		include(CheckSymbolExists)
		set(CMAKE_REQUIRED_INCLUDES "${GLUT_INCLUDE_DIR}")
		set(CMAKE_REQUIRED_LIBRARIES "${GLUT_glut_LIBRARY}")
		check_symbol_exists ("glutInitErrorFunc" "GL/glut.h;GL/freeglut_ext.h" PIGLIT_USE_GLUT_INIT_ERROR_FUNC)
		if (PIGLIT_USE_GLUT_INIT_ERROR_FUNC)
			add_definitions(-DPIGLIT_USE_GLUT_INIT_ERROR_FUNC)
		endif ()
	endif ()
endif()

add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)
