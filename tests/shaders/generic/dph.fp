2 2 0
tc
 -1.0 0.0 0.0 -0.5
 0.2 0.0 0.0 0.0
tex
expected
 0.2 0.2 0.2 1.0

tc
 -1.0 0.0 0.0 -0.5
 0.2 0.0 0.0 0.5
tex
expected
 0.7 0.7 0.7 1.0
!!ARBfp1.0
DPH result.color, -fragment.texcoord[0], fragment.texcoord[1];
MOV result.color.w, 1;
END
