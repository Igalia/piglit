set(piglit_target_api "gles3")

add_definitions(
	-DPIGLIT_USE_OPENGL_ES3
	)
add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)
