#!/bin/bash

# Emit global variable declarations for either the vertex shader or the
# fragment shader.
function emit_globals
{
    matrix_dim=$1
    array_dim=$2
    mode=$3
    index_value=$4
    col="[$5]"
    value_type=$6

    v=${version/./}
    if [ $v -ge 120 ]; then
	base_type="mat${matrix_dim}x${matrix_dim}"
    else
	base_type="mat${matrix_dim}"
    fi

    type=$base_type
    dim=""
    if [ $array_dim -ne 0 ]; then
	if [ $v -ge 120 ]; then
	    type="${type}[$array_dim]"
	else
	    dim="[${array_dim}]"
	fi
    fi

    echo "uniform ${base_type} src_matrix;"
    echo "uniform vec${matrix_dim} v;"
    echo "uniform vec${matrix_dim} expect;"

    if [ $array_dim -ne 0 ]; then
	echo "uniform int index;"
    fi

    if [ "x$value_type" = "xfloat" ]; then
	echo "uniform int row;"
    fi

    echo "uniform int col;"
    echo "uniform ${value_type} value;"

    if [ "x$mode" = "xvarying" ]; then
	echo "varying ${type} dst_matrix${dim};"
    fi
    echo
}


function emit_distanceSqr_function
{
    dim=$1
    echo "float distanceSqr(vec${dim} a, vec${dim} b) { vec${dim} diff = a - b; return dot(diff, diff); }"
    echo
}

function emit_set_matrix
{
    matrix_dim=$1
    array_dim=$2
    mode=$3
    index_value=$4
    col="[$5]"
    value_type=$6

    v=${version/./}
    if [ $v -ge 120 ]; then
	base_type="mat${matrix_dim}x${matrix_dim}"
    else
	base_type="mat${matrix_dim}"
    fi

    type=$base_type
    dim=""
    if [ $array_dim -ne 0 ]; then
	if [ $v -ge 120 ]; then
	    type="${type}[$array_dim]"
	else
	    dim="[${array_dim}]"
	fi
    fi

    if [ "x$mode" = "xtemp" ]; then
	echo -n "    ${type} dst_matrix${dim}"
	lhs=""
    else
	lhs="    dst_matrix"
    fi

    if [ $array_dim -ne 0 ]; then
	if [ $v -ge 120 ]; then
	    echo -n "${lhs} = ${type}("
	    for i in $(seq $array_dim); do
		if [ $i -ne 1 ]; then
		    echo -n ", "
		fi
		echo -n "${base_type}(0.0)"
	    done
	    echo ");"
	else
	    if [ "x$mode" = "xtemp" ]; then
		echo ";"
	    fi

	    for i in $(seq 0 $((array_dim - 1))); do
		echo "    dst_matrix[$i] = ${base_type}(0.0);"
	    done
	fi
    elif [ "x$mode" = "xtemp" ]; then
	echo ";"
    fi

    echo
}


function emit_transform
{
    matrix_dim=$1
    array_dim=$2
    mode=$3
    index_value=$4
    col="[$5]"
    value_type=$6

    if [ "x$value_type" = "xfloat" ]; then
	row="[row]"
    else
	row=""
    fi

    cat <<EOF
    /* Patch the supplied matrix with the supplied value.  If the resulting
     * matrix is correct, it will transform the input vector to the expected
     * value.  Verify that the distance between the result and the expected
     * vector is less than epsilon.
EOF

    if [ $array_dim -ne 0 ]; then
	cat <<EOF
     *
     * NOTE: This test assumes that reads of arrays using non-constant
     * indicies works correctly.  If reads and writes happen to fail in an
     * identical manner, this test may give false positives.
EOF
	idx="[${index_value}]"
    else
	idx=""
    fi

    echo "     */"
    echo "    dst_matrix${idx} = src_matrix;"
    echo "    dst_matrix${idx}${col}${row} = value;"
}


function emit_fs
{
    matrix_dim=$1
    array_dim=$2
    mode=$3
    index_value=$4
    col="[$5]"
    value_type=$6

    v=${version/./}
    if [ $v -ge 120 ]; then
	base_type="mat${matrix_dim}x${matrix_dim}"
    else
	base_type="mat${matrix_dim}"
    fi

    type=$base_type
    dim=""
    if [ $array_dim -ne 0 ]; then
	if [ $v -ge 120 ]; then
	    type="${type}[$array_dim]"
	else
	    dim="[${array_dim}]"
	fi
    fi

    echo "[fragment shader]"

    emit_globals $*
    emit_distanceSqr_function $matrix_dim

    echo "void main()"
    echo "{"

    # Assume for now that fragment shaders can only write to temporaries.
    # There usually aren't enough MRT outputs for an array of matrices
    # anyway.  Just like with vertex shader array inputs (attribute arrays),
    # we'll probably need a non-shader_runner test to exercise that path.
    if [ "x$mode" = "xtemp" ]; then
	emit_set_matrix $*
	emit_transform $*
    fi

    echo "    gl_FragColor = (distanceSqr(dst_matrix${idx} * v, expect) < 4e-9)"
    echo "        ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);"
    echo "}"
    echo
}


function emit_test_vectors
{
    matrix_dim=$1
    array_dim=$2
    mode=$3
    index_value=$4
    col=$5
    value_type=$6

    cat <<EOF
[test]
clear color 0.5 0.5 0.5 0.5
clear
ortho

EOF

    # NOTE: shader_runner uses the matCxR names even for GLSL 1.10
    type="mat${matrix_dim}x${matrix_dim}"

    # Random input vector, a random matrix, and the result of multiplying that
    # vector by that matrix.  Naturally, the matrices are listed in OpenGL
    # column-major order
    case "${matrix_dim}x${matrix_dim}" in
	"2x2")
	    vec="0.803161418975390 0.852987140792140"
	    exp="0.708718134966688 1.452243795483797"
	    mat="0.241498998195656 0.861223395812970
                 0.603473877011433 0.891622340451180"
	    ;;
	"3x3")
	    vec="0.681652305322399 0.210426138878113 0.185916924650237"
	    exp="0.610649606928364 0.711906885823636 0.312244778977868"
	    mat="0.493944462129466 0.722190133917966 0.239853948232558
                 0.550143078409278 0.591962645398579 0.467616286531193
                 0.850846377186973 0.511303112962423 0.270815003356504"
	    ;;
	"4x4")
	    vec="0.0394868046587045 0.8922408276905568 0.3337495624366961 0.8732295730825839"
	    exp="1.03935908892461 1.18846180713529 1.10078681232072 1.72434439561820"
	    mat="0.922040144261674 0.158053783109488 0.357016429866574 0.836368810383957
                 0.560251913703792 0.171634921595771 0.602494709909111 0.693273570571311
                 0.350720358904176 0.912192627475775 0.688544081259531 0.913891056231967
                 0.442058176039301 0.829835836794679 0.365674411003021 0.879197364462782"
	    ;;
    esac

    # Trim out the extra whitespace from the matrix data.  The extra space
    # (above) makes it easier to read, but it makes it harder to generate
    # sensible test scripts.
    mat=$(echo $mat | tr '\n' ' ' | sed 's/[[:space:]]\+/ /g;s/[[:space:]]*$//g')

    if [ $array_dim -eq 0 ]; then
	sizes="1"
    elif [ "x$index_value" = "xindex" ]; then
	sizes=$(seq $array_dim)
    else
	sizes="2"
    fi

    if [ "x$value_type" = "xfloat" ]; then
	rows=$(seq $matrix_dim)
	bad="666.0"
    else
	rows=1
	case $matrix_dim in
	    2) bad="666.0 777.0";;
	    3) bad="666.0 777.0 888.0";;
	    4) bad="666.0 777.0 888.0 999.0";;
	esac
    fi

    if [ "x$col" = "xcol" ]; then
	columns=$(seq $matrix_dim)
    else
	columns="2"
    fi

    for i in $sizes; do
	if [ $array_dim -ne 0 -a "x$index_value" = "xindex" ]; then
	    echo "uniform int index $((i-1))"
	fi

	x_base=$(((i - 1) * (15 * matrix_dim + 10)))
	for c in $columns; do
	    if [ "x$col" = "xcol" ]; then
		echo "uniform int col $((c-1))"
	    fi

            for r in $rows; do
		if [ "x$value_type" = "xfloat" ]; then
		    echo "uniform int row $((r-1))"
		fi

		d=$((matrix_dim * (c-1) + (r-1) + 1))
		if [ "x$value_type" = "xfloat" ]; then
		    v=$(echo $mat | cut -d' ' -f$d)
		else
		    v=$(echo $mat | cut -d' ' -f$d-$((d+matrix_dim-1)))
		fi

		echo "uniform vec${matrix_dim} v ${vec}"
		echo "uniform vec${matrix_dim} expect ${exp}"
		echo "uniform ${type} src_matrix ${mat/$v/$bad}"
		echo "uniform ${value_type} value $v"

		x=$((x_base + 15 * c - 10))
		y=$((15 * r - 10))
		echo "draw rect $x $y 10 10"

		x=$(($x + 5))
		y=$(($y + 5))
		echo "probe rgb $x $y 0.0 1.0 0.0"
		echo
	    done
	done
    done
}


function emit_fs_wr_test
{
    echo "# Test generated by:"
    echo "# ${cmd}"
    echo
    echo "[require]"
    echo "GLSL >= ${version}"
    echo

    echo "[vertex shader]"
    echo "void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; }"
    echo

    emit_fs $*
    emit_test_vectors $*
}


function emit_vs_wr_test
{
    matrix_dim=$1
    array_dim=$2
    mode=$3

    echo "# Test generated by:"
    echo "# ${cmd}"
    echo
    echo "[require]"
    echo "GLSL >= ${version}"
    if [ "x$mode" = "xvarying" ]; then
       echo "GL_MAX_VARYING_COMPONENTS >= $varying_comps"
    fi
    echo

    echo "[vertex shader]"
    emit_globals $*
    if [ "x$mode" != "xvarying" ] ; then
        emit_distanceSqr_function $matrix_dim
    fi

    echo "void main()"
    echo "{"
    echo "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
    echo

    emit_set_matrix $*
    emit_transform $*

    if [ "x$mode" != "xvarying" ] ; then
	echo "    gl_FrontColor = (distanceSqr(dst_matrix${idx} * v, expect) < 4e-9)"
	echo "        ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);"
    fi

    echo "}"
    echo

    if [ "x$mode" != "xvarying" ];then
	echo "[fragment shader]"
	echo "void main() { gl_FragColor = gl_Color; }"
	echo
    else
	emit_fs $*
    fi

    emit_test_vectors $*
}


cmd="$0 $*"

if [ "x$1" = "x" ]; then
    version="1.10"
else
    case "$1" in
	1.[12]0) version="$1";;
	*)
	    echo "Bogus GLSL version \"$1\" specified."
	    exit 1
	    ;;
    esac
fi

for mode in temp varying; do
    # More than 3 is unlikely to work for the varying tests due to using too
    # many varying vectors.  mat4[3] uses 12 varying vectors by itself.
    for array_dim in 0 3; do
	if [ $array_dim -ne 0 ]; then
	    arr="array-"
	    idx_txt="index-"
	else
	    arr=""
	    idx_txt=""
	fi

	for matrix_dim in 2 3 4; do
	    if [ $array_dim -ne 0 ]; then
		varying_comps=$((matrix_dim * matrix_dim * array_dim))
	    else
		varying_comps=$((matrix_dim * matrix_dim))
	    fi

	    # Fragment shaders cannot write varyings
	    if [ "x$mode" != "xvarying" ]; then
		name="fs-${mode}-${arr}mat${matrix_dim}-${idx_txt}col-row-wr.shader_test"
		emit_fs_wr_test $matrix_dim $array_dim $mode index col float \
		    > $name
		echo $name

		name="fs-${mode}-${arr}mat${matrix_dim}-${idx_txt}row-wr.shader_test"
		emit_fs_wr_test $matrix_dim $array_dim $mode index 1   float \
		    > $name
		echo $name

		name="fs-${mode}-${arr}mat${matrix_dim}-${idx_txt}col-wr.shader_test"
		emit_fs_wr_test $matrix_dim $array_dim $mode index col vec${matrix_dim} \
		    > $name
		echo $name

		name="fs-${mode}-${arr}mat${matrix_dim}-${idx_txt}wr.shader_test"
		emit_fs_wr_test $matrix_dim $array_dim $mode index 1   vec${matrix_dim} \
		    > $name
		echo $name

		if [ $array_dim -ne 0 ]; then
		    name="fs-${mode}-${arr}mat${matrix_dim}-col-row-wr.shader_test"
		    emit_fs_wr_test $matrix_dim $array_dim $mode 1 col float \
			> $name
		    echo $name

		    name="fs-${mode}-${arr}mat${matrix_dim}-row-wr.shader_test"
		    emit_fs_wr_test $matrix_dim $array_dim $mode 1 1   float \
			> $name
		    echo $name

		    name="fs-${mode}-${arr}mat${matrix_dim}-col-wr.shader_test"
		    emit_fs_wr_test $matrix_dim $array_dim $mode 1 col vec${matrix_dim} \
			> $name
		    echo $name

		    name="fs-${mode}-${arr}mat${matrix_dim}-wr.shader_test"
		    emit_fs_wr_test $matrix_dim $array_dim $mode 1 1   vec${matrix_dim} \
			> $name
		    echo $name
		fi
	    fi

	    name="vs-${mode}-${arr}mat${matrix_dim}-${idx_txt}col-row-wr.shader_test"
	    emit_vs_wr_test $matrix_dim $array_dim $mode index col float \
		> $name
	    echo $name

	    name="vs-${mode}-${arr}mat${matrix_dim}-${idx_txt}row-wr.shader_test"
	    emit_vs_wr_test $matrix_dim $array_dim $mode index 1   float \
		> $name
	    echo $name

	    name="vs-${mode}-${arr}mat${matrix_dim}-${idx_txt}col-wr.shader_test"
	    emit_vs_wr_test $matrix_dim $array_dim $mode index col vec${matrix_dim} \
		> $name
	    echo $name

	    name="vs-${mode}-${arr}mat${matrix_dim}-${idx_txt}wr.shader_test"
	    emit_vs_wr_test $matrix_dim $array_dim $mode index 1   vec${matrix_dim} \
		> $name
	    echo $name

	    if [ $array_dim -ne 0 ]; then
		name="vs-${mode}-${arr}mat${matrix_dim}-col-row-wr.shader_test"
		emit_vs_wr_test $matrix_dim $array_dim $mode 1 col float \
		    > $name
		echo $name

		name="vs-${mode}-${arr}mat${matrix_dim}-row-wr.shader_test"
		emit_vs_wr_test $matrix_dim $array_dim $mode 1 1   float \
		    > $name
		echo $name

		name="vs-${mode}-${arr}mat${matrix_dim}-col-wr.shader_test"
		emit_vs_wr_test $matrix_dim $array_dim $mode 1 col vec${matrix_dim} \
		    > $name
		echo $name

		name="vs-${mode}-${arr}mat${matrix_dim}-wr.shader_test"
		emit_vs_wr_test $matrix_dim $array_dim $mode 1 1   vec${matrix_dim} \
		    > $name
		echo $name
	    fi
	done
    done
done
