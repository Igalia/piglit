#!/bin/bash

set -ex

EXTRA_MESON_ARGS=

# Build the minimum required version of libdrm if the system version is not
# new enough.
if pkg-config --max-version=2.4.98 libdrm; then
    export LIBDRM_VERSION=libdrm-2.4.98

    tar -xvf /tmp/$LIBDRM_VERSION.tar.bz2 && rm /tmp/$LIBDRM_VERSION.tar.bz2
    cd $LIBDRM_VERSION
    meson build -D vc4=false -D freedreno=false -D etnaviv=false $EXTRA_MESON_ARGS
    ninja -C build install
    cd ..
    rm -rf $LIBDRM_VERSION

    export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig

    LIBDRM_FLAGS="-D LIBDRM_CFLAGS=$(pkg-config --cflags libdrm | sed 's/ /;/g') \
      -D LIBDRM_LDFLAGS=$(pkg-config --libs libdrm | sed 's/ /;/g') \
      -D LIBDRM_FOUND=1 \
      -D LIBDRM_INTEL_CFLAGS=$(pkg-config --cflags libdrm_intel | sed 's/ /;/g') \
      -D LIBDRM_INTEL_LDFLAGS=$(pkg-config --libs libdrm_intel | sed 's/ /;/g') \
      -D LIBDRM_INTEL_FOUND=1"
else
    LIBDRM_FLAGS=""
fi

cmake . \
      -D CMAKE_BUILD_TYPE=Debug \
      -D PIGLIT_BUILD_CL_TESTS=on \
      -D PIGLIT_BUILD_DMA_BUF_TESTS=on \
      -D PIGLIT_BUILD_GLES1_TESTS=on \
      -D PIGLIT_BUILD_GLES2_TESTS=on \
      -D PIGLIT_BUILD_GLX_TESTS=on \
      -D PIGLIT_BUILD_GL_TESTS=on \
      -D PIGLIT_BUILD_WGL_TESTS=off \
      $LIBDRM_FLAGS \
      -GNinja

ninja -j4
