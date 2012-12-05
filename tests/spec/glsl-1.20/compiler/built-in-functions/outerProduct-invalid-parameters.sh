#!/bin/bash

function emit_vs
{
    t=$1

    cat > outerProduct-$t.vert <<EOF
/* [config]
 * expect_result: fail
 * glsl_version: 1.20
 * [end config]
 */
#version 120
void main () {
  gl_Position = vec4(0);
  outerProduct(${t}(0), ${t}(0));
}
EOF
}

for i in int float bool bvec2 bvec3 bvec4 mat2 mat2x2 mat2x3 mat2x4 mat3 mat3x2 mat3x3 mat3x4 mat4 mat4x2 mat4x3 mat4x4
do
    emit_vs $i
done
