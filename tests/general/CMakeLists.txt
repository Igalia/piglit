
include_directories(
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${piglit_SOURCE_DIR}/tests/util
	${GLEW_INCLUDE_DIR}
)

link_directories (
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

add_executable (bgra-vert-attrib-pointer bgra-vert-attrib-pointer.c)
add_executable (clear-varray-2.0 clear-varray-2.0.c)
add_executable (dlist-clear dlist-clear.c)
add_executable (depthrange-clear depthrange-clear.c)
add_executable (linestipple linestipple.c)
add_executable (provoking-vertex provoking-vertex.c)
add_executable (read-front read-front.c)
add_executable (scissor-copypixels scissor-copypixels.c)
add_executable (scissor-depth-clear scissor-depth-clear.c)
add_executable (scissor-stencil-clear scissor-stencil-clear.c)
add_executable (stencil-drawpixels stencil-drawpixels.c)
add_executable (texgen texgen.c)
add_executable (varray-disabled varray-disabled.c)
add_executable (vbo-map-remap vbo-map-remap.c)
add_executable (windowoverlap windowoverlap.c)
add_executable (occlusion_query occlusion_query.c)
