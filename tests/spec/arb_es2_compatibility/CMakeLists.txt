
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

add_executable (arb_es2_compatibility-depthrangef arb_es2_compatibility-depthrangef.c)
add_executable (arb_es2_compatibility-drawbuffers arb_es2_compatibility-drawbuffers.c)
add_executable (arb_es2_compatibility-maxvectors arb_es2_compatibility-maxvectors.c)
add_executable (arb_es2_compatibility-releaseshadercompiler arb_es2_compatibility-releaseshadercompiler.c)
add_executable (arb_es2_compatibility-shadercompiler arb_es2_compatibility-shadercompiler.c)
add_executable (arb_es2_compatibility-getshaderprecisionformat arb_es2_compatibility-getshaderprecisionformat.c)
