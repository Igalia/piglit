# -------------------------------------------
# Temporarily disable building GLES2 targets.
# -------------------------------------------

#add_definitions ( -DSOURCE_DIR="${piglit_SOURCE_DIR}/" )
#
#include_directories(
#	${OPENGL_INCLUDE_PATH}
#	${piglit_SOURCE_DIR}/tests/util/
#	${piglit_SOURCE_DIR}/tests/util/glut_egl
#)
#
#link_libraries (
#	${OPENGL_gles2_LIBRARY}
#	${OPENGL_egl_LIBRARY}
#	piglitglutegl_x11
#	piglitegl_gles2util
#)
#
#add_executable (gles2_shader_runner gles2_shader_runner.c ../util/shader-load.c)
#target_link_libraries(gles2_shader_runner X11)
