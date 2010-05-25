
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
add_executable (bgra-sec-color-pointer bgra-sec-color-pointer.c)
add_executable (clear-varray-2.0 clear-varray-2.0.c)
add_executable (depth-clamp-range depth-clamp-range.c)
add_executable (dlist-clear dlist-clear.c)
add_executable (dlist-fdo3129-01 dlist-fdo3129-01.c)
add_executable (dlist-fdo3129-02 dlist-fdo3129-02.c)
add_executable (depth_clamp depth_clamp.c)
add_executable (depthfunc depthfunc.c)
add_executable (depthrange-clear depthrange-clear.c)
add_executable (draw-elements draw-elements.c)
add_executable (draw-elements-base-vertex draw-elements-base-vertex.c)
add_executable (draw-elements-vs-inputs draw-elements-vs-inputs.c)
add_executable (draw-vertices draw-vertices.c)
add_executable (draw-vertices-half-float draw-vertices-half-float.c)
add_executable (linestipple linestipple.c)
add_executable (pbo-drawpixels pbo-drawpixels.c)
add_executable (pbo-read-argb8888 pbo-read-argb8888.c)
add_executable (pbo-readpixels-small pbo-readpixels-small.c)
add_executable (pbo-teximage pbo-teximage.c)
add_executable (pbo-teximage-tiling pbo-teximage-tiling.c)
add_executable (pbo-teximage-tiling-2 pbo-teximage-tiling-2.c)
add_executable (point-line-no-cull point-line-no-cull.c)
add_executable (provoking-vertex provoking-vertex.c)
add_executable (oes-read-format oes-read-format.c)
add_executable (read-front read-front.c)
add_executable (scissor-bitmap scissor-bitmap.c)
add_executable (scissor-clear scissor-clear.c)
add_executable (scissor-copypixels scissor-copypixels.c)
add_executable (scissor-depth-clear scissor-depth-clear.c)
add_executable (scissor-many scissor-many.c)
add_executable (scissor-offscreen scissor-offscreen.c)
add_executable (scissor-stencil-clear scissor-stencil-clear.c)
add_executable (stencil-drawpixels stencil-drawpixels.c)
add_executable (sync_api sync_api.c)
add_executable (texgen texgen.c)
add_executable (texunits texunits.c)
add_executable (timer_query timer_query.c)
add_executable (user-clip user-clip.c)
add_executable (varray-disabled varray-disabled.c)
add_executable (vbo-map-remap vbo-map-remap.c)
add_executable (vbo-subdata-sync vbo-subdata-sync.c)
add_executable (windowoverlap windowoverlap.c)
add_executable (object_purgeable-api-pbo object_purgeable-api-pbo.c object_purgeable.c)
add_executable (object_purgeable-api-texture object_purgeable-api-texture.c object_purgeable.c)
add_executable (object_purgeable-api-vbo object_purgeable-api-vbo.c object_purgeable.c)
add_executable (occlusion_query occlusion_query.c)
add_executable (quad-invariance quad-invariance.c)
