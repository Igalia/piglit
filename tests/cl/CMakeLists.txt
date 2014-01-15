include_directories(
	${OPENCL_INCLUDE_PATH}
	${piglit_SOURCE_DIR}/tests/util
)

link_libraries (
	piglitutil_${piglit_target_api}
	${OPENCL_opencl_LIBRARY}
)

if(PIGLIT_HAS_POSIX_CLOCK_MONOTONIC)
	link_libraries(rt)
endif()

function(piglit_cl_add_test)
	add_executable(${ARGV})
endfunction(piglit_cl_add_test)

function(piglit_cl_add_custom_test name)

	list(REMOVE_AT ARGV 0)
	piglit_cl_add_test(
		cl-custom-${name}
		${ARGV}
	)

endfunction(piglit_cl_add_custom_test)

function(piglit_cl_add_api_test name)

	list(REMOVE_AT ARGV 0)
	piglit_cl_add_test(
		cl-api-${name}
		${ARGV}
	)

endfunction(piglit_cl_add_api_test)

function(piglit_cl_add_program_test name)

	list(REMOVE_AT ARGV 0)
	piglit_cl_add_test(
		cl-program-${name}
		${ARGV}
	)

endfunction(piglit_cl_add_program_test)

add_subdirectory (custom)
add_subdirectory (api)
add_subdirectory (program)
