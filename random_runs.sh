#!/bin/bash

typeset -i i
typeset -i f

i=1
f=0

if [ -d pass ]; then
	rm -f pass/*.shader_test
else
	mkdir pass
fi

if [ -d fail ]; then
	rm -f fail/*.shader_test
else
	mkdir fail
fi

while true; do
    t=$(printf "0x%08x.shader_test" $i)

    python2 generated_tests/random_ubo.py 330 > $t

    if bin/shader_runner ./$t -auto -fbo | grep -q pass; then
	mv $t pass/
    else
	mv $t fail/
	echo $t
	f=$((f + 1))
    fi

    if [ $f -ge 1000 ]; then
	echo $i total tests
	exit 0
    fi

    i=$((i + 1))
done
