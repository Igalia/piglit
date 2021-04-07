#!/bin/bash

set -ex

cmake . \
      -D CMAKE_BUILD_TYPE=Debug \
      -D PIGLIT_BUILD_CL_TESTS=on \
      -D PIGLIT_BUILD_DMA_BUF_TESTS=on \
      -D PIGLIT_BUILD_GLES1_TESTS=on \
      -D PIGLIT_BUILD_GLES2_TESTS=on \
      -D PIGLIT_BUILD_GLX_TESTS=on \
      -D PIGLIT_BUILD_GL_TESTS=on \
      -D PIGLIT_BUILD_WGL_TESTS=off \
      -GNinja

ninja -j${FDO_CI_CONCURRENT:-4}
