#!/bin/bash

trimname=$(echo $1 | sed 's/shader_test/trim.shader_test/')
workname=$(echo $1 | sed 's/shader_test/work.shader_test/')

cp $1 $trimname

typeset -i i
i=0

while true; do
    if ! python2 generated_tests/random_ubo_trim.py $trimname $workname ; then
	echo Test trimmer could not make progress.
	break
    fi


    if bin/shader_runner ./$workname -auto -fbo | grep -q pass; then
	# If the test passes now, increment a counter.
	echo No progress
	i=$((i + 1))
    else
	# The test still fails.  The working file is now the minimal
	# file.  Also, reset the counter.
	mv $workname $trimname
	i=0
    fi

    if [ $i -gt 100 ]; then
	echo "100 attempts to trim failed.  Done"
	break
    fi
done
