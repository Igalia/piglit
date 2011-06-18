#!/bin/sh

function get_coord_type
{
    case $1 in
	texture1D)     coord_type="float" ;;
	texture1DProj) coord_type="vec2"  ;;
	texture2D)     coord_type="vec2"  ;;
	texture2DProj) coord_type="vec3"  ;;
	texture3D)     coord_type="vec3"  ;;
	texture3DProj) coord_type="vec4"  ;;
	textureCube)   coord_type="vec3"  ;;
	texture2DRect) coord_type="vec2"  ;;
	texture2DRectProj) coord_type="vec3"  ;;
	shadow1D)      coord_type="vec3"  ;;
	shadow1DProj)  coord_type="vec4"  ;;
	shadow2D)      coord_type="vec3"  ;;
	shadow2DProj)  coord_type="vec4"  ;;
	shadow2DRect)  coord_type="vec3"  ;;
	shadow2DRectProj)  coord_type="vec4"  ;;
    esac

    echo $coord_type
}


function get_grad_type
{
    # The gradient type is the same of the Proj and non-Proj versions.
    # It is also the same for the shadow and non-shadow versions.
    get_coord_type $(echo $1 | sed 's/Proj//;s/shadow/texture/')
}


function get_sampler_dimensions
{
    # This expression does 3 steps:
    #   1. Moves the word 'texture' or 'shadow' to the end
    #   2. Deletes the word 'texture'
    #   3. Deletes the word 'Proj'
    #   4. Replaces 'shadow' with 'Shadow'
    echo $1 | sed 's/^\(shadow\|texture\)\(.*\)/\2\1/;s/texture//;s/Proj//;s/shadow/Shadow/'
}


function gen_frag_grad_test
{
    coord_type=$(get_coord_type $1)
    grad_type=$(get_grad_type $1)
    dim=$(get_sampler_dimensions $1)
    mode=$1

    if test x$2 != x ; then
	coord_type=$2
    fi

    if echo $1 | grep -q Rect ; then
	extensions="GL_ARB_shader_texture_lod GL_ARB_texture_rectangle"
    else
	extensions="GL_ARB_shader_texture_lod"
    fi

    cat <<EOF
/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: ${extensions}
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler${dim} s;
varying ${coord_type} coord;
varying ${grad_type} dPdx;
varying ${grad_type} dPdy;

void main()
{
  gl_FragColor = ${mode}GradARB(s, coord, dPdx, dPdy);
}
EOF
}


function gen_vert_grad_test
{
    coord_type=$(get_coord_type $1)
    grad_type=$(get_grad_type $1)
    dim=$(get_sampler_dimensions $1)
    mode=$1

    if test x$2 != x ; then
	coord_type=$2
    fi

    if echo $1 | grep -q Rect ; then
	extensions="GL_ARB_shader_texture_lod GL_ARB_texture_rectangle"
    else
	extensions="GL_ARB_shader_texture_lod"
    fi

    cat <<EOF
/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: ${extensions}
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

uniform sampler${dim} s;
attribute vec4 pos;
attribute ${coord_type} coord;
attribute ${grad_type} dPdx;
attribute ${grad_type} dPdy;
varying vec4 color;

void main()
{
  gl_Position = pos;
  color = ${mode}GradARB(s, coord, dPdx, dPdy);
}
EOF
}


function gen_frag_lod_test
{
    coord_type=$(get_coord_type $1)
    dim=$(get_sampler_dimensions $1)
    mode=$1

    if test x$2 != x ; then
	coord_type=$2
    fi

    cat <<EOF
/* [config]
 * expect_result: pass
 * glsl_version: 1.10
 * extension: GL_ARB_shader_texture_lod
 * [end config]
 */
#extension GL_ARB_shader_texture_lod: require

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
   gen_frag_lod_test $d > $name
   i=$((i + 1))


   if echo $d | egrep -q 'texture[12]DProj' ; then
       name=$(printf "tex_lod-%02d.frag" $i)
       gen_frag_lod_test $d vec4 > $name
       i=$((i + 1))
   fi
done

i=1
for d in texture1D texture1DProj shadow1D shadow1DProj \
    texture2D texture2DProj shadow2D shadow2DProj \
    texture3D texture3DProj textureCube \
    texture2DRect texture2DRectProj \
    shadow2DRect shadow2DRectProj
do
   name=$(printf "tex_grad-%02d.frag" $i)
   gen_frag_grad_test $d > $name
   i=$((i + 1))


   if echo $d | egrep -q 'texture[12]DProj' ; then
       name=$(printf "tex_grad-%02d.frag" $i)
       gen_frag_grad_test $d vec4 > $name
       i=$((i + 1))
   fi
done

for d in texture1D texture1DProj shadow1D shadow1DProj \
    texture2D texture2DProj shadow2D shadow2DProj \
    texture3D texture3DProj textureCube \
    texture2DRect texture2DRectProj \
    shadow2DRect shadow2DRectProj
do
   name=$(printf "tex_grad-%02d.vert" $i)
   gen_vert_grad_test $d > $name
   i=$((i + 1))


   if echo $d | egrep -q 'texture[12]DProj' ; then
       name=$(printf "tex_grad-%02d.vert" $i)
       gen_vert_grad_test $d vec4 > $name
       i=$((i + 1))
   fi
done
