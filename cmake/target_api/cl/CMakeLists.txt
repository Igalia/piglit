set(piglit_target_api "cl")

add_definitions(
	-DUSE_OPENCL
	)

add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)

