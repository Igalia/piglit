1 3 0
tc
 0.5 0.0 1.0 0.0
 1.5 4.5 2.0 1.0
-0.5 4.5 3.5 1.0
tex
expected
 0.25 0.5 0.5 1.0

!!ARBfp1.0
TEMP r0;
LRP_SAT r0, fragment.texcoord[0], fragment.texcoord[1], fragment.texcoord[2];
MUL result.color, 0.5, r0;
END
; comment
