
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

add_executable (array-texture array-texture.c)
add_executable (copytexsubimage copytexsubimage.c)
IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable (crossbar crossbar.c)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
add_executable (cubemap cubemap.c)
add_executable (depth-level-clamp depth-level-clamp.c)
add_executable (gen-compressed-teximage gen-compressed-teximage.c)
add_executable (gen-nonzero-unit gen-nonzero-unit.c)
add_executable (gen-teximage gen-teximage.c)
add_executable (gen-texsubimage gen-texsubimage.c)
add_executable (getteximage-simple getteximage-simple.c)
add_executable (levelclamp levelclamp.c)
add_executable (lodbias lodbias.c)
add_executable (lodclamp lodclamp.c)
add_executable (lodclamp-between lodclamp-between.c)
add_executable (lodclamp-between-max lodclamp-between-max.c)
add_executable (mipmap-setup mipmap-setup.c)
add_executable (rg-draw-pixels rg-draw-pixels.c)
add_executable (s3tc-teximage s3tc-teximage.c)
add_executable (fxt1-teximage fxt1-teximage.c)
add_executable (s3tc-texsubimage s3tc-texsubimage.c)
add_executable (streaming-texture-leak streaming-texture-leak.c)
add_executable (tex3d tex3d.c)
add_executable (tex3d-npot tex3d-npot.c)
add_executable (texdepth texdepth.c)
add_executable (texrect-many texrect-many.c)
add_executable (texredefine texredefine.c)
add_executable (texture-integer texture-integer.c)
add_executable (depth-tex-modes      depth-tex-modes.c depth-tex-modes-common.c)
add_executable (depth-tex-modes-rg   depth-tex-modes-rg.c depth-tex-modes-common.c)
add_executable (depth-tex-modes-glsl depth-tex-modes-glsl.c)
add_executable (depth-tex-compare depth-tex-compare.c)
add_executable (tex-border-1 tex-border-1.c)
add_executable (tex-swizzle tex-swizzle.c)
add_executable (texwrap texwrap.c)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_executable (tfp tfp.c)
	target_link_libraries(tfp X11)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
