#!/bin/bash

set -eu

glslc="../../../../../shaderc/build/glslc/glslc"

shaders=(
	vs_two_sets_ifc_text \
	vs_two_sets_named_ifc_text \
	vs_two_sets_struct_text \
	vs_two_sets_text \
	vs_double_text \
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
