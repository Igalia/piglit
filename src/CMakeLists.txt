if(USE_WAFFLE)
	add_subdirectory(glut_waffle)
endif(USE_WAFFLE)

if(OPENGL_egl_LIBRARY)
	add_subdirectory(glut_egl)
endif(OPENGL_egl_LIBRARY)
