#!/bin/sh

function emit_outerProduct_data
{
    c=$1
    r=$2

    cbase=$3
    cvec=""
    for i in $(seq $c); do
	v=$(($i + $cbase))
	cvec="${cvec} ${v}"
    done

    rbase=$(($3 + $c))
    rvec=""
    for i in $(seq $r); do
	v=$(($i + $rbase))
	rvec="${rvec} ${v}"
    done

    mat=""
    for i in $(seq $r); do
	for j in $(seq $c); do
	    m=$((($i + $rbase) * ($j + $cbase)))
	    mat="${mat} ${m}"
	done
    done

    echo
    echo "uniform vec${c} c ${cvec}"
    echo "uniform vec${r} r ${rvec}"
    echo "uniform mat${r}x${c} expected ${mat}"

    x=$((20 * $3 - 10))
    echo "draw rect $x 10 10 10"
    echo "probe rgb $(($x + 5)) 15 0.0 1.0 0.0"
}

function emit_vs_test
{
    c=$1
    r=$2
    mat=$3

    name="vs-outerProduct-${mat}.shader_test"
    cat > $name <<EOF
[require]
GLSL >= 1.20

[vertex shader]
#version 120
uniform vec${c} c, r;
uniform ${mat} expected;
varying vec4 color;

void main() {
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  ${mat} result = outerProduct(c, r);
  color = (result == expected) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
}

[fragment shader]
#version 120
varying vec4 color;
void main() { gl_FragColor = color; }

[test]
clear color 0.5 0.5 0.5 0.0
clear
ortho
EOF

    emit_outerProduct_data $c $r 1 >> $name
    emit_outerProduct_data $c $r 2 >> $name
    emit_outerProduct_data $c $r 3 >> $name
    emit_outerProduct_data $c $r 4 >> $name
}

function emit_fs_test
{
    c=$1
    r=$2
    mat=$3

    name="fs-outerProduct-${mat}.shader_test"
    cat > $name <<EOF
[require]
GLSL >= 1.20

[vertex shader]
#version 120
void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }

[fragment shader]
#version 120
uniform vec${c} c, r;
uniform ${mat} expected;

void main() {
  ${mat} result = outerProduct(c, r);
  gl_FragColor = (result == expected) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
}

[test]
clear color 0.5 0.5 0.5 0.0
clear
ortho
EOF

    emit_outerProduct_data $c $r 1 >> $name
    emit_outerProduct_data $c $r 2 >> $name
    emit_outerProduct_data $c $r 3 >> $name
    emit_outerProduct_data $c $r 4 >> $name
}

for c in 2 3 4; do
    for r in 2 3 4; do
	emit_vs_test $c $r "mat${r}x${c}"
	emit_fs_test $c $r "mat${r}x${c}"
	if [ $c -eq $r ]; then
	    emit_vs_test $c $r "mat${r}"
	    emit_fs_test $c $r "mat${r}"
	fi
    done
done
