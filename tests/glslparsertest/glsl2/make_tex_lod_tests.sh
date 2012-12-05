#!/bin/bash

function gen_frag_test
{
    case $1 in
	texture1D)     coord_type="float" ;;
	texture1DProj) coord_type="vec2"  ;;
	texture2D)     coord_type="vec2"  ;;
	texture2DProj) coord_type="vec3"  ;;
	texture3D)     coord_type="vec3"  ;;
	texture3DProj) coord_type="vec4"  ;;
	textureCube)   coord_type="vec3"  ;;
	shadow1D)      coord_type="vec3"  ;;
	shadow1DProj)  coord_type="vec4"  ;;
	shadow2D)      coord_type="vec3"  ;;
	shadow2DProj)  coord_type="vec4"  ;;
    esac

    # This expression does 3 steps:
    #   1. Moves the word 'texture' or 'shadow' to the end
    #   2. Deletes the word 'texture'
    #   3. Deletes the word 'Proj'
    #   4. Replaces 'shadow' with 'Shadow'
    dim=$(echo $1 | sed 's/^\(shadow\|texture\)\(.*\)/\2\1/;s/texture//;s/Proj//;s/shadow/Shadow/')
    mode=$1

    if test x$2 != x ; then
	coord_type=$2
    fi

    cat <<EOF
/* FAIL
 * Without GLSL 1.40 or GL_ARB_shader_texture_lod, the "Lod" versions of the
 * texture lookup functions are not available in fragment shaders.
 */
uniform sampler${dim} s;
varying ${coord_type} coord;
varying float lod;

void main()
{
  gl_FragColor = ${mode}Lod(s, coord, lod);
}
EOF
}


i=1
for d in texture1D texture1DProj shadow1D shadow1DProj \
    texture2D texture2DProj shadow2D shadow2DProj \
    texture3D texture3DProj textureCube
do
   name=$(printf "tex_lod-%02d.frag" $i)
   gen_frag_test $d > $name
   i=$((i + 1))


   if echo $d | egrep -q 'texture[12]DProj' ; then
       name=$(printf "tex_lod-%02d.frag" $i)
       gen_frag_test $d vec4 > $name
       i=$((i + 1))
   fi
done
