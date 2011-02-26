# "Make no_api. no_api... what kind of API is that? NonAPI. This is the
# essential art of CMake-zen."
#     -- Eihei Dogen. "Fukan Zazengi". circa 1228 CE.

set(piglit_target_api "no_api")
foreach(dir "src" "tests")
	add_subdirectory(${piglit_SOURCE_DIR}/${dir}
		${piglit_BINARY_DIR}/target_api/${piglit_target_api}/${dir}
		)
endforeach(dir)
