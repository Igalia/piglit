set(piglit_target_api "gles1")

add_definitions(
	-DUSE_OPENGL_ES1
	-DUSE_EGLUT
	)
add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)
