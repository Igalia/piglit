#!/bin/sh

function gen_frag_test
{
    case $1 in
	texture2DRect)     coord_type="vec2"  ;;
	texture2DRectProj) coord_type="vec3"  ;;
	shadow2DRect)      coord_type="vec3"  ;;
	shadow2DRectProj)  coord_type="vec4"  ;;
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
/* PASS */
uniform sampler${dim} s;
varying ${coord_type} coord;

void main()
{
  gl_FragColor = ${mode}(s, coord);
}
EOF
}


i=4
for d in texture2DRect texture2DRectProj \
    shadow2DRect shadow2DRectProj
do
   name=$(printf "tex_rect-%02d.frag" $i)
   gen_frag_test $d > $name
   i=$((i + 1))

   if test $d = 'texture2DRectProj'; then
       name=$(printf "tex_rect-%02d.frag" $i)
       gen_frag_test $d vec4 > $name
       i=$((i + 1))
   fi
done
