#!/bin/bash
#
# Copyright © 2009 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Generate a bunch of vertex program texture tests
#
# Authors: Ian Romanick <ian.d.romanick@intel.com>

function emit_target_require
{
    t=$1
    if echo $t | grep -q ^SHADOW ; then
	echo "# REQUIRE GL_ARB_fragment_program_shadow"
	echo "OPTION	ARB_fragment_program_shadow;"
	t=$(echo $t | sed 's/^SHADOW//')
    fi
    if [ "$t" = "RECT" ]; then
	echo "# REQUIRE GL_ARB_texture_rectangle"
    fi
    if [ "$t" = "CUBE" ]; then
	echo "# REQUIRE GL_ARB_texture_cube_map"
    fi
    if [ "$t" = "3D" ]; then
	echo "# REQUIRE GL_EXT_texture3D"
    fi
}


function emit_fail_NVvp3
{
    if ! echo "$1" | egrep -q '(TEX|TX[BLP])'; then
	printf '# FAIL - %s not supported by GL_NV_vertex_program3\n' "$1"
    fi
}

function emit_shader_ARBvp
{
    echo '!!ARBvp1.0'

    emit_target_require $2

    echo '# FAIL - texture instructions not supported by GL_ARB_vertex_program'
    printf '%s	result.color, vertex.texcoord[0], texture[0], %s;\n' "$1" "$2"
    echo 'END'
}


function emit_shader_NVvp3
{
    echo '!!ARBvp1.0'
    echo "# REQUIRE GL_NV_vertex_program3"

    echo "OPTION	NV_vertex_program3;"
    emit_target_require $2
    emit_fail_NVvp3 $1

    echo ""
    printf '%s	result.color, vertex.texcoord[0], texture[0], %s;\n' "$1" "$2"
    echo 'END'
}



function emit_shader_NVvp3_alt
{
    echo '!!ARBvp1.0'
    echo "# REQUIRE GL_NV_vertex_program3"

    echo "OPTION	NV_vertex_program3;"
    emit_target_require $2
    emit_fail_NVvp3 $1

    echo ""
    printf 'OUTPUT	%s = result.color;\n' "$2"
    printf '%s	%s, vertex.texcoord[0], texture[0], %s;\n' "$1" "$2" "$2"
    echo 'END'
}


path=shaders/ARBvp1.0
#           VP3 VP3 FP  GP4 VP3 VP3 GP4
for inst in TEX TXB TXD TXF TXL TXP TXQ; do
    inst_low=$(echo $inst | awk '{print tolower($1);}')

    i=1
    for target in 1D 2D 3D CUBE RECT SHADOW1D SHADOW2D; do
	file=$(printf "%s-%02d.txt" $inst_low $i)

	emit_shader_ARBvp $inst $target > $path/$file
	i=$((i + 1))
    done

    for target in 1D 2D 3D CUBE RECT SHADOW1D SHADOW2D; do
	file=$(printf "%s-%02d.txt" $inst_low $i)

	emit_shader_NVvp3 $inst $target > $path/$file
	i=$((i + 1))
    done

    for target in CUBE RECT; do
	file=$(printf "%s-%02d.txt" $inst_low $i)

	emit_shader_NVvp3_alt $inst $target > $path/$file
	i=$((i + 1))
    done

    # Add this set of tests cases here so that the tests from the previous
    # don't get re-numbered.  This prevents unnecessary churn in the diffs.
    for target in SHADOWRECT; do
	file=$(printf "%s-%02d.txt" $inst_low $i)

	emit_shader_NVvp3 $inst $target > $path/$file
	i=$((i + 1))
    done

    for target in SHADOW1D SHADOW2D SHADOWRECT; do
	file=$(printf "%s-%02d.txt" $inst_low $i)

	emit_shader_NVvp3_alt $inst $target > $path/$file
	i=$((i + 1))
    done
done
