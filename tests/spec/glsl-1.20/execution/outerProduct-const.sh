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

function emit_outerProduct_const_data
{
    c=$1
    r=$2
    vtype=$3
    mtype=$4

    cbase=$5
    cvec=""
    for i in $(seq $c); do
	v=$(($i + $cbase))
	if [ "x$cvec" == "x" ]; then
	    cvec="${v}"
	else
	    cvec="${cvec}, ${v}"
	fi
    done

    rbase=$(($cbase + $c))
    rvec=""
    for i in $(seq $r); do
	v=$(($i + $rbase))
	if [ "x$rvec" == "x" ]; then
	    rvec="${v}"
	else
	    rvec="${rvec}, ${v}"
	fi
    done

    expected=""
    for i in $(seq $r); do
	for j in $(seq $c); do
	    m=$((($i + $rbase) * ($j + $cbase)))
	    if [ "x$expected" == "x" ]; then
		expected="${m}"
	    else
		expected="${expected}, ${m}"
	    fi
	done
    done

    echo
    echo "const ${vtype}${c} c = ${vtype}${c}(${cvec});"
    echo "const ${vtype}${r} r = ${vtype}${r}(${rvec});"
    echo "uniform ${mtype} expected = ${mtype}(${expected});"

}

function emit_vs_test
{
    c=$1
    r=$2
    vtype=$3
    mat=$4

    if [ "$vtype" = "ivec" ]; then
	name="vs-outerProduct-const-${mat}-ivec.shader_test"
    else
	name="vs-outerProduct-const-${mat}.shader_test"
    fi

    cat > $name <<EOF
[require]
GLSL >= 1.20

[vertex shader]
EOF

    if [ "$vtype" = "ivec" ]; then
	emit_conversion_info >> $name
    fi

    echo "#version 120" >> $name
    emit_outerProduct_const_data $c $r $vtype $mat 1 >> $name

    cat >> $name <<EOF
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

draw rect 10 10 10 10
probe rgb 15 15 0.0 1.0 0.0
EOF
}

function emit_fs_test
{
    c=$1
    r=$2
    vtype=$3
    mat=$4

    if [ "$vtype" = "ivec" ]; then
	name="fs-outerProduct-const-${mat}-ivec.shader_test"
    else
	name="fs-outerProduct-const-${mat}.shader_test"
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

    echo "#version 120" >> $name
    emit_outerProduct_const_data $c $r $vtype $mat 1 >> $name

    cat >> $name <<EOF

void main() {
  ${mat} result = outerProduct(c, r);
  gl_FragColor = (result == expected) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
}

[test]
clear color 0.5 0.5 0.5 0.0
clear
ortho

draw rect 10 10 10 10
probe rgb 15 15 0.0 1.0 0.0
EOF
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
