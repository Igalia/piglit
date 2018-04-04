set(piglit_target_api "vulkan")

add_definitions(
	-DUSE_VULKAN
	)

add_subdirectory(${piglit_SOURCE_DIR}/tests
	${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
	)

