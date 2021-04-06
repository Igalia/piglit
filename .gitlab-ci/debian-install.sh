#!/bin/bash
set -eux

export DEBIAN_FRONTEND=noninteractive

apt-get install -y \
  ca-certificates

sed -i -e 's/http:\/\/deb/https:\/\/deb/g' /etc/apt/sources.list
echo 'deb https://deb.debian.org/debian buster-backports main' >/etc/apt/sources.list.d/backports.list

# Use newer packages from backports by default
cat >/etc/apt/preferences <<EOF
Package: *
Pin: release a=buster-backports
Pin-Priority: 500
EOF

apt-get update

apt-get install -y \
  bison \
  bzip2 \
  ccache \
  cmake \
  curl \
  flex \
  freeglut3-dev \
  g++-multilib \
  gcc-multilib \
  gettext \
  git \
  jq \
  libdrm-dev \
  libdrm2 \
  libegl1-mesa-dev \
  libglvnd-dev \
  libpciaccess-dev \
  libvulkan-dev \
  libwaffle-dev \
  libwayland-dev \
  libxkbcommon-dev \
  libxrender-dev \
  meson \
  mingw-w64 \
  ninja-build \
  opencl-dev \
  pkg-config \
  python3 \
  python3-dev \
  python3-jsonschema \
  python3-mako \
  python3-mock \
  python3-numpy \
  python3-packaging \
  python3-pil \
  python3-pip \
  python3-psutil \
  python3-pytest \
  python3-pytest-mock \
  python3-pytest-timeout \
  python3-requests \
  python3-requests-mock \
  python3-setuptools \
  python3-wheel \
  python3-yaml \
  tox \
  waffle-utils \
  unzip

pip3 install pytest-pythonpath
pip3 install pytest-raises

# Download Waffle artifacts.  See also
# https://gitlab.freedesktop.org/mesa/waffle/-/merge_requests/89
# https://docs.gitlab.com/ee/ci/pipelines/job_artifacts.html#downloading-the-latest-artifacts
for target in mingw32 mingw64
do
    mkdir -p /opt/waffle/$target
    curl -s -L "https://gitlab.freedesktop.org/mesa/waffle/-/jobs/artifacts/${WAFFLE_BRANCH:-maint-1.7}/raw/publish/$target/waffle-$target.zip?job=cmake-mingw" -o /tmp/waffle-$target.zip
    unzip -qo /tmp/waffle-$target.zip -d /opt/waffle/$target
    test -d /opt/waffle/$target/waffle
    rm /tmp/waffle-$target.zip
done

curl -s -L "https://dri.freedesktop.org/libdrm/libdrm-2.4.98.tar.bz2" -o /tmp/libdrm-2.4.98.tar.bz2

apt-get purge -y \
  curl \
  unzip
