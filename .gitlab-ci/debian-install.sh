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
apt-get dist-upgrade -y

apt-get install -y \
  bison \
  cmake \
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
  libwaffle-dev \
  libwayland-dev \
  libxkbcommon-dev \
  libxrender-dev \
  ninja-build \
  opencl-dev \
  pkg-config \
  python3 \
  python3-dev \
  python3-jsonschema \
  python3-mako \
  python3-mock \
  python3-numpy \
  python3-pip \
  python3-psutil \
  python3-pytest \
  python3-pytest-mock \
  python3-pytest-timeout \
  python3-setuptools \
  python3-six \
  python3-wheel \
  tox \
  waffle-utils

pip3 install pytest-pythonpath
pip3 install pytest-raises
