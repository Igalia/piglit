set(piglit_target_api "gl")
add_definitions(
	-DUSE_OPENGL
	-DUSE_GLUT
	)
add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)
