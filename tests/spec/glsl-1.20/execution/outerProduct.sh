#!/bin/sh

function emit_conversion_info
{
cat <<EOF
/* From page 43 (page 49 of the PDF) of the GLSL 1.20 spec:
 *
 *     "If an exact match is found, the other signatures are ignored, and the
 *     exact match is used. Otherwise, if no exact match is found, then the
 *     implicit conversions in Section 4.1.10 "Implicit Conversions" will be
 *     applied to the calling arguments if this can make their types match a
 *     signature."
 *
 * From page 20 (page 26 of the PDF) of the GLSL 1.20 spec:
 *
 *     "In some situations, an expression and its type will be implicitly
 *     converted to a different type. The following table shows all allowed
 *     implicit conversions:
 *
 *         Type of expression    Can be implicitly converted to
 *               int                         float
 *              ivec2                         vec2
 *              ivec3                         vec3
 *              ivec4                         vec4"
 */
EOF
}

function emit_outerProduct_data
{
    c=$1
    r=$2
    vtype=$3

    cbase=$4
    cvec=""
    for i in $(seq $c); do
	v=$(($i + $cbase))
	cvec="${cvec} ${v}"
    done

    rbase=$(($cbase + $c))
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
    echo "uniform ${vtype}${c} c ${cvec}"
    echo "uniform ${vtype}${r} r ${rvec}"
    echo "uniform mat${r}x${c} expected ${mat}"

    x=$((20 * $4 - 10))
    echo "draw rect $x 10 10 10"
    echo "probe rgb $(($x + 5)) 15 0.0 1.0 0.0"
}

function emit_vs_test
{
    c=$1
    r=$2
    vtype=$3
    mat=$4

    if [ "$vtype" = "ivec" ]; then
	name="vs-outerProduct-${mat}-ivec.shader_test"
    else
	name="vs-outerProduct-${mat}.shader_test"
    fi

    cat > $name <<EOF
[require]
GLSL >= 1.20

[vertex shader]
EOF

    if [ "$vtype" = "ivec" ]; then
	emit_conversion_info >> $name
    fi

    cat >> $name <<EOF
#version 120
uniform ${vtype}${c} c;
uniform ${vtype}${r} r;
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

    emit_outerProduct_data $c $r $vtype 1 >> $name
    emit_outerProduct_data $c $r $vtype 2 >> $name
    emit_outerProduct_data $c $r $vtype 3 >> $name
    emit_outerProduct_data $c $r $vtype 4 >> $name
}

function emit_fs_test
{
    c=$1
    r=$2
    vtype=$3
    mat=$4

    if [ "$vtype" = "ivec" ]; then
	name="fs-outerProduct-${mat}-ivec.shader_test"
    else
	name="fs-outerProduct-${mat}.shader_test"
    fi

    cat > $name <<EOF
[require]
GLSL >= 1.20

[vertex shader]
#version 120
void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }

[fragment shader]
EOF

    if [ "$vtype" = "ivec" ]; then
	emit_conversion_info >> $name
    fi

    cat >> $name <<EOF
#version 120
uniform ${vtype}${c} c;
uniform ${vtype}${r} r;
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

    emit_outerProduct_data $c $r $vtype 1 >> $name
    emit_outerProduct_data $c $r $vtype 2 >> $name
    emit_outerProduct_data $c $r $vtype 3 >> $name
    emit_outerProduct_data $c $r $vtype 4 >> $name
}

for c in 2 3 4; do
    for r in 2 3 4; do
	emit_vs_test $c $r  vec "mat${r}x${c}"
	emit_vs_test $c $r ivec "mat${r}x${c}"
	emit_fs_test $c $r  vec "mat${r}x${c}"
	emit_fs_test $c $r ivec "mat${r}x${c}"
	if [ $c -eq $r ]; then
	    emit_vs_test $c $r  vec "mat${r}"
	    emit_vs_test $c $r ivec "mat${r}"
	    emit_fs_test $c $r  vec "mat${r}"
	    emit_fs_test $c $r ivec "mat${r}"
	fi
    done
done
