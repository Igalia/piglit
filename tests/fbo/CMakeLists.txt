
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
add_executable (fbo-d24s8 fbo-d24s8.c)
add_executable (fbo-3d fbo-3d.c)
add_executable (fbo-alpha fbo-alpha.c)
add_executable (fbo-luminance-alpha fbo-luminance-alpha.c)
add_executable (fbo-blit fbo-blit.c)
add_executable (fbo-blit-d24s8 fbo-blit-d24s8.c)
add_executable (fbo-copypix fbo-copypix.c)
add_executable (fbo-readdrawpix fbo-readdrawpix.c)
add_executable (fbo-clearmipmap fbo-clearmipmap.c)
add_executable (fbo-drawbuffers fbo-drawbuffers.c)
add_executable (fbo-drawbuffers-fragcolor fbo-drawbuffers-fragcolor.c)
add_executable (fbo-drawbuffers-maxtargets fbo-drawbuffers-maxtargets.c)
add_executable (fbo-drawbuffers2-blend fbo-drawbuffers2-blend.c)
add_executable (fbo-drawbuffers2-colormask fbo-drawbuffers2-colormask.c)
add_executable (fbo-flushing fbo-flushing.c)
add_executable (fbo-fragcoord fbo-fragcoord.c)
add_executable (fbo-generatemipmap fbo-generatemipmap.c)
add_executable (fbo-generatemipmap-formats fbo-generatemipmap-formats.c)
add_executable (fbo-generatemipmap-scissor fbo-generatemipmap-scissor.c)
add_executable (fbo-generatemipmap-nonsquare fbo-generatemipmap-nonsquare.c)
add_executable (fbo-generatemipmap-npot fbo-generatemipmap-npot.c)
add_executable (fbo-generatemipmap-viewport fbo-generatemipmap-viewport.c)
add_executable (fbo-getframebufferattachmentparameter-01 fbo-getframebufferattachmentparameter-01.c)
add_executable (fbo-incomplete-texture-01 fbo-incomplete-texture-01.c)
add_executable (fbo-incomplete-texture-02 fbo-incomplete-texture-02.c)
add_executable (fbo-incomplete-texture-03 fbo-incomplete-texture-03.c)
add_executable (fbo-integer fbo-integer.c)
add_executable (fbo-maxsize fbo-maxsize.c)
add_executable (fbo-nodepth-test fbo-nodepth-test.c)
add_executable (fbo-nostencil-test fbo-nostencil-test.c)
add_executable (fbo-readpixels fbo-readpixels.c)
add_executable (fbo-rg fbo-rg.c)
add_executable (fbo-pbo-readpixels-small fbo-pbo-readpixels-small.c)
add_executable (fbo-copyteximage fbo-copyteximage.c)
add_executable (fbo-copyteximage-simple fbo-copyteximage-simple.c)
add_executable (fbo-cubemap fbo-cubemap.c)
add_executable (fbo-scissor-bitmap fbo-scissor-bitmap.c)
