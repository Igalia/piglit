#!/bin/bash

set -eu

glslc="../../../../../../shaderc/build/glslc/glslc"

shaders=( \
	vs_pass_through \
	gs_text \
)

for shader in "${shaders[@]}"; do
    prefix=$(echo "$shader" | sed 's/_.*//')
    case "$prefix" in
	gs) stage=geom ;;
	vs) stage=vert ;;
	fs) stage=frag ;;
	*) echo "unknown shader prefix: $prefix" >&2; exit 1;;
    esac
    "$glslc" --target-env=opengl -fshader-stage="$stage" \
	     -o "$shader.spirv" \
	     "$shader.glsl"
done
