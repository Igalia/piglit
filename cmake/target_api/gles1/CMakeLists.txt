set(piglit_target_api "gles1")

if(OPENGL_gles1_LIBRARY AND OPENGL_egl_LIBRARY)
	add_definitions(
		-DUSE_OPENGL_ES1
		-DUSE_EGLUT
		)
	add_subdirectory(${piglit_SOURCE_DIR}/tests
		${piglit_BINARY_DIR}/target_api/${piglit_target_api}/tests
		)
endif(OPENGL_gles1_LIBRARY AND OPENGL_egl_LIBRARY)
