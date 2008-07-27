6 2 0
tc
 -1 -1 -1 -1
 -1 -1 -1 -1
tex
expected
 0 0 1 0

tc
 1 1 -1 -1
 -1 -1 1 1
tex
expected
 1 0 0 0

tc
 1 1 -1 -1
 -1 -1 -1 1
tex
expected
 0 0 1 0

tc
 1 1 -1 -1
 -1 -1 1 -1
tex
expected
 0 0 1 0

tc
 1 -1 -1 -1
 -1 -1 1 1
tex
expected
 0 0 1 0

tc
 -1 1 -1 -1
 -1 -1 -1 1
tex
expected
 0 0 1 0

!!ARBfp1.0
KIL fragment.texcoord[0].xxyy;
KIL fragment.texcoord[1].zwwz;
MOV result.color, { 1.0, 0.0, 0.0, 0.0 };
END
