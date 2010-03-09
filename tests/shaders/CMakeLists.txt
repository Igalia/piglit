
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

add_executable (ati-fs-bad-delete ati-fs-bad-delete.c)
add_executable (trinity-fp1 trinity-fp1.c)
add_executable (fp-abs-01 fp-abs-01.c)
add_executable (fp-abs-02 fp-abs-02.c)
add_executable (fp-condition_codes-01 fp-condition_codes-01.c)
add_executable (fp-lit-mask fp-lit-mask.c)
add_executable (fp-fog fp-fog.c)
add_executable (fp-fragment-position fp-fragment-position.c)
add_executable (fp-generic fp-generic.c)
add_executable (fp-kil fp-kil.c)
add_executable (fp-incomplete-tex fp-incomplete-tex.c)
add_executable (fp-indirections fp-indirections.c)
add_executable (fp-indirections2 fp-indirections2.c)
add_executable (fp-long-alu fp-long-alu.c)
add_executable (fp-rfl fp-rfl.c)
add_executable (fp-set-01 fp-set-01.c)
add_executable (fp-set-02 fp-set-02.c)
add_executable (fp-unpack-01 fp-unpack-01.c)
add_executable (glsl-arb-fragment-coord-conventions glsl-arb-fragment-coord-conventions.c)
add_executable (glsl-arb-fragment-coord-conventions-define glsl-arb-fragment-coord-conventions-define.c)
add_executable (glsl-bug-22603 glsl-bug-22603.c)
add_executable (glsl-dlist-getattriblocation glsl-dlist-getattriblocation.c)
add_executable (glsl-reload-source glsl-reload-source.c)
add_executable (glsl-unused-varying glsl-unused-varying.c)
add_executable (glsl-uniform-update glsl-uniform-update.c)
add_executable (glsl-uniform-out-of-bounds glsl-uniform-out-of-bounds.c)
add_executable (glsl-fs-exp2 glsl-fs-exp2.c)
add_executable (glsl-fs-fragcoord glsl-fs-fragcoord.c)
add_executable (glsl-fs-log2 glsl-fs-log2.c)
add_executable (glsl-fs-sqrt-zero glsl-fs-sqrt-zero.c)
add_executable (glsl-vs-arrays glsl-vs-arrays.c)
add_executable (glsl-vs-mov-after-deref glsl-vs-mov-after-deref.c)
add_executable (glsl-vs-if glsl-vs-if.c)
add_executable (glsl-vs-if-bool glsl-vs-if-bool.c)
add_executable (glsl-vs-loop glsl-vs-loop.c)
add_executable (glsl-vs-loop-nested glsl-vs-loop-nested.c)
add_executable (glsl-vs-sqrt-zero glsl-vs-sqrt-zero.c)
add_executable (vp-address-01 vp-address-01.c)
add_executable (vp-address-02 vp-address-02.c)
add_executable (vp-address-03 vp-address-03.c)
add_executable (vp-address-04 vp-address-04.c)
add_executable (vp-address-05 vp-address-05.c)
add_executable (vp-address-06 vp-address-06.c)
add_executable (vp-bad-program vp-bad-program.c)
add_executable (vp-combined-image-units vp-combined-image-units.c)
add_executable (vp-clipdistance-01 vp-clipdistance-01.c)
add_executable (vp-clipdistance-02 vp-clipdistance-02.c)
add_executable (vp-clipdistance-03 vp-clipdistance-03.c)
add_executable (vp-clipdistance-04 vp-clipdistance-04.c)
add_executable (vpfp-generic vpfp-generic.cpp)
add_executable (vp-max-array vp-max-array.c)
add_executable (glsl-derivs glsl-derivs.c)
add_executable (glsl-deriv-varyings glsl-deriv-varyings.c)
add_executable (glsl-fwidth glsl-fwidth.c)
add_executable (glsl-lod-bias glsl-lod-bias.c)
add_executable (glsl-preprocessor-comments glsl-preprocessor-comments.c)
add_executable (vp-ignore-input vp-ignore-input.c)
add_executable (glsl-empty-vs-no-fs glsl-empty-vs-no-fs.c)
add_executable (glsl-useprogram-displaylist glsl-useprogram-displaylist.c)
