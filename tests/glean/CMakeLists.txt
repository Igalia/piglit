
add_definitions ( -D__X11__ -D__UNIX__ )

include_directories( ${OPENGL_INCLUDE_PATH} )

add_executable (glean
	codedid.cpp
	dsconfig.cpp
	dsfilt.cpp
	dsurf.cpp
	environ.cpp
	geomrend.cpp
	geomutil.cpp
	glutils.cpp
	main.cpp
	misc.cpp
	options.cpp
	rc.cpp
	tapi2.cpp
	tbasic.cpp
	tbasicperf.cpp
	tbinding.cpp
	tblend.cpp
	tchgperf.cpp
	tdepthstencil.cpp
	test.cpp
	tfbo.cpp
	tfpexceptions.cpp
	tfragprog1.cpp
	tgetstr.cpp
	tglsl1.cpp
	tlogicop.cpp
	tmaskedclear.cpp
	tmultitest.cpp
	toccluqry.cpp
	torthpos.cpp
	tpaths.cpp
	tpbo.cpp
	tpgos.cpp
	tpixelformats.cpp
	tpointatten.cpp
	tpointsprite.cpp
	treadpix.cpp
	treadpixperf.cpp
	trgbtris.cpp
	tscissor.cpp
	tteapot.cpp
	ttexcombine.cpp
	ttexcube.cpp
	ttexenv.cpp
	ttexgen.cpp
	ttexrect.cpp
	ttexture_srgb.cpp
	tvertattrib.cpp
	tvertprog1.cpp
	tvtxperf.cpp
	winsys.cpp
	gl.cpp
	image_misc.cpp
	pack.cpp
	rdtiff.cpp
	reg.cpp
	unpack.cpp
	wrtiff.cpp
	basic.cpp
	lex.cpp
	timer.cpp
)

target_link_libraries (glean
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${TIFF_LIBRARY}
)

