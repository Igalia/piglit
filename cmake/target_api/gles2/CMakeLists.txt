set(piglit_target_api "gles2")
add_definitions(
	-DUSE_OPENGL_ES2
	-DUSE_EGLUT
	)

# Don't add any targets, because Piglit doesn't support GLES2 yet.
