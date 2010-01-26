
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

link_directories (
	${piglit_SOURCE_DIR}/tests/mesa/util
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	piglitutil
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${GLUT_glut_LIBRARY}
	${TIFF_LIBRARY}
	${GLEW_glew_LIBRARY}
)

add_executable (fbo-1d fbo-1d.c)
add_executable (fbo-3d fbo-3d.c)
add_executable (fbo-clearmipmap fbo-clearmipmap.c)
add_executable (fbo-drawbuffers fbo-drawbuffers.c)
add_executable (fbo-flushing fbo-flushing.c)
add_executable (fbo-generatemipmap fbo-generatemipmap.c)
add_executable (fbo-generatemipmap-npot fbo-generatemipmap-npot.c)
add_executable (fbo-generatemipmap-viewport fbo-generatemipmap-viewport.c)
add_executable (fbo-nodepth-test fbo-nodepth-test.c)
add_executable (fbo-nostencil-test fbo-nostencil-test.c)
add_executable (fbo-readpixels fbo-readpixels.c)
add_executable (fbo-copyteximage fbo-copyteximage.c)
add_executable (fbo-cubemap fbo-cubemap.c)
