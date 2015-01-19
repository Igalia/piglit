[require]
GLSL >= 4.00

[vertex shader]
#extension GL_ARB_shader_precision : require
% if signature.extension:
#extension GL_${signature.extension} : require
% endif
in vec4 piglit_vertex;
flat out vec4 color;
% for i, arg in enumerate(signature.argtypes):
uniform ${arg} arg${i};
% endfor
uniform float tolerance;
uniform ${signature.rettype} expected;

void main()
{
  gl_Position = piglit_vertex;
  ##
  ## perform the operation being tested
  ##
  ${signature.rettype} result = ${invocation};
  ##
  ## compare the result(s) to the expected value(s)
  ##
  % if signature.rettype.is_matrix or signature.rettype.is_vector:
    ##
    ## build an array of bit-level representations of the floating point results calculated above
    ##
  int resultbits[${num_elements}] = int[${num_elements}](\
${', '.join('floatBitsToInt(result{0})'.format(i) for i in indexers)}\
);
    ##
    ## build an array of bit-level representations of the passed-in floating point expected results
    ##
  int expectedbits[${num_elements}] = int[${num_elements}](\
${', '.join('floatBitsToInt(expected{0})'.format(i) for i in indexers)}\
);
  ##
  ## check for differences in the sign bit for each result
  ##
  bool signerr = \
${' || '.join('(resultbits[{0}]>>31 != expectedbits[{0}]>>31)'.format(i) for i in xrange(0, num_elements))}\
;
  ##
  ## calculate the difference between the generated value and the expected value in ulps
  ##
  ${signature.rettype} ulps = ${signature.rettype}(\
${', '.join('abs(resultbits[{0}] - expectedbits[{0}])'.format(i) for i in xrange(0, num_elements))}\
);
  ##
  ## find the maximum error in ulps of all the calculations using a nested max() sort
  ##
  float max_error = \
    ## start with the outermost max() if there are more than 2 elements
    ## (two element arrays, eg. vec2, are handled by the final max() below, only)
    % if num_elements > 2:
max( \
    % endif
    ## cat each value to compare, with an additional nested max() up until the final two values
    % for i, indexer in enumerate(indexers[:len(indexers)-2]):
ulps${indexer}, \
    % if i != len(indexers)-3:
max(\
    % endif
    ## cat the final, deepest, max comparison
    % endfor
max(ulps${indexers[len(indexers)-2]}, ulps${indexers[len(indexers)-1]})\
    ## fill in completing parens
    % for i in xrange(0, num_elements-2):
)\
    % endfor
;
  % else:
    ##
    ## if there is only a single result value generated, compare it directly
    ##
  int resultbits = floatBitsToInt(result);
  int expectedbits = floatBitsToInt(expected);
  bool signerr = resultbits>>31 != expectedbits>>31;
    % if signature.name != 'distance':
  float ulps = distance(resultbits, expectedbits);
    % else:
  float ulps = abs(resultbits - expectedbits);
    % endif
  % endif
  ##
  ## the test passes if there were no sign errors and the ulps are within tolerance
  ##
  color = \
  % if signature.rettype.is_matrix or signature.rettype.is_vector:
!signerr && max_error <= tolerance\
  % else:
!signerr && ulps <= tolerance\
  % endif
 ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
}

[fragment shader]
flat in vec4 color;

void main()
{
  gl_FragColor = color;
}

[vertex data]
piglit_vertex/float/2
-1.0 -1.0
 1.0 -1.0
 1.0  1.0
-1.0  1.0

[test]
% for test_num, test_vector in enumerate(test_vectors):
  % for i in xrange(len(test_vector.arguments)):
uniform ${shader_runner_type(signature.argtypes[i])} arg${i} ${shader_runner_format( column_major_values(test_vector.arguments[i]))}
  % endfor
uniform ${shader_runner_type(signature.rettype)} expected ${shader_runner_format(column_major_values(test_vector.result))}
uniform float tolerance \
${tolerances.get(signature.name, 0.0)}
draw arrays GL_TRIANGLE_FAN 0 4
## shader_runner uses a 250x250 window so we must ensure that test_num <= 250.
probe rgba ${test_num % 250} 0 0.0 1.0 0.0 1.0
% endfor
