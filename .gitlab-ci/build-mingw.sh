#!/bin/bash

if [ $# -lt 1 ]
then
  echo "usage: $0 mingw32|mingw64 [...]" 2>&1
  exit 1
fi

if [ -d /usr/lib/ccache ]
then
  export PATH=/usr/lib/ccache:$PATH
fi

set -e -x

for target
do

  if [ "$GITLAB_CI" = "true" ]
  then
    # See debian-install.sh
    waffleDir=/opt/waffle/$target/waffle
  else
    # For local testing purposes only
    waffleDir=$PWD/external/$target/waffle
    if [ ! -d $waffleDir ]
    then
      mkdir -p external/$target
      if [ ! -f external/waffle-$target.zip ]
      then
        curl -s -L "https://gitlab.freedesktop.org/mesa/waffle/-/jobs/artifacts/${WAFFLE_BRANCH:-maint-1.7}/raw/publish/$target/waffle-$target.zip?job=cmake-mingw" -o external/waffle-$target.zip
      fi
      unzip -qo external/waffle-$target.zip -d external/$target
    fi
  fi

  test -d $waffleDir

  if [ -n "$CI_COMMIT_TAG" -a "$target" = "mingw32" ]
  then
    buildType=MinSizeRel
    packageTarget=package
  else
    buildType=Debug
    packageTarget=install
  fi

  cmake \
    -S . \
    -B build/$target \
    -G "Ninja" \
    -DCMAKE_TOOLCHAIN_FILE=.gitlab-ci/$target.cmake \
    -DCMAKE_BUILD_TYPE=$buildType \
    -DCMAKE_INSTALL_PREFIX=publish/$target \
    -DPIGLIT_USE_WAFFLE=TRUE \
    -DWaffle_INCLUDE_DIRS=$waffleDir/include/waffle-1 \
    -DWaffle_LDFLAGS=$waffleDir/lib/libwaffle-1.dll.a \
    -DWaffle_DLL=$waffleDir/bin/waffle-1.dll

  cmake --build build/$target

  cmake --build build/$target --target $packageTarget

done
