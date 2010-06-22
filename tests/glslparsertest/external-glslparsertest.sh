#!/bin/sh

# This is the wrapper script for using the standalone GLSL2 compiler
# for the glslparsertest.  Drop a link named "glslcompiler" in this
# directory pointing at the compiler binary.

if test "x$2" != "xpass" -a "x$2" != "xfail"; then
    echo "usage: external-glslparsertest.sh filename [pass|fail]"
    exit 1;
fi

result=$(./tests/glslparsertest/glslcompiler $1)
status=$?

if test "x$2" = xpass; then
    if test "$status" = "0"; then
	echo PIGLIT: {\'result\': \'pass\' }
    else
	echo "shader source:"
	cat $1
	echo "compiler result:"
	echo "$result"
	echo PIGLIT: {\'result\': \'fail\' }
    fi
else
    if test "$status" != "1"; then
	echo "shader should have failed:"
	cat $1
	echo PIGLIT: {\'result\': \'fail\' }
    else
	echo PIGLIT: {\'result\': \'pass\' }
    fi
fi
