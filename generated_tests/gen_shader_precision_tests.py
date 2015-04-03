# coding=utf-8
#
# Copyright © 2014 Intel Corporation
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


"""Generate a set of shader_runner tests for overloaded versions of every
 built-in function specified in arb_shader_precision, based on the test
 vectors computed by builtin_function.py, and structured according to the mako
 templates in templates/gen_shader_precision_tests.

 The vertex, geometry, and fragment shader types are exercised by each test.
 In all cases, the inputs to the built-in functions come from uniforms, so
 that the effectiveness of the test won't be circumvented by constant folding
 in the GLSL compiler.

 The tests operate by invoking the built-in function in the appropriate
 shader, calculating any deviance from the expected value (in ulps), comparing
 the deviance to a supplied tolerance (according to those specified in
 arb_shader_precision), and then outputting the pass/fail result as a solid
 rgba color which is then checked using shader_runner's "probe rgba" command.

 For built-in functions whose result type is multi-valued (vec or mat), the
 tests calculate the error in ulps for each element separately, but compare
 only the largest error value to the tolerance.  This accounts for cases where
 error varies among the elements of a vec or mat result.

 This program updates the tolerance values in the test vectors with tolerances
 sourced from the GLSL spec (v4.5) and adds a few new tests as well.

 This program outputs, to stdout, the name of each file it generates.
"""

from __future__ import print_function, division, absolute_import
from builtin_function import *
import os
import numpy
import struct

import six
from six.moves import range

from templates import template_file



tolerances = {'pow': 16.0,
              'exp': 3.0,
              'exp2': 3.0,
              'log': 3.0,
              'log2': 3.0,
              'sqrt': 3.0,
              'inversesqrt': 2.0,
              'op-div': 2.5,
              'op-assign-div': 2.5,
              }

shader_precision_spec_fns = ('op-add', 'op-assign-add',
                             'op-sub', 'op-assign-sub',
                             'op-mult', 'op-assign-mult',
                             'op-div', 'op-assign-div',
                             'degrees', 'radians',
                             'pow',
                             'exp',
                             'exp2',
                             'log', 'log2',
                             'sqrt', 'inversesqrt')

def _is_sequence(arg):
    return isinstance(arg, (collections.Sequence, numpy.ndarray))

def _floatToBits(f):
    s = struct.pack('>f', f)
    return struct.unpack('>L', s)[0]

# The precision for log and log2 from the GLSL spec:
# 3 ULP outside the range [0.5, 2.0].
# Absolute error < 2^-21 inside the range [0.5, 2.0].
def _log_tol(compcnt, args, i, j):
    arg = args[j][i] if compcnt > 1 else args[0]
    if arg < 0.5 or arg > 2.0:
        return 3.0
    elif arg >= 0.5 and arg < 1.0:
        # in the range [0.5, 1.0), one ulp is 5.960464478e-08,
        # so 2^-21 is 8 ulps
        return 8.0
    else:
        # in the range [1.0, 2.0), one ulp is 1.192092896e-07,
        # so 2^-21 is 4 ulps
        return 4.0

# The precision for exp and exp2 from the GLSL spec:
# (3 + 2 * |x|) ULP.
def _exp_tol(compcnt, args, i, j):
    arg = args[j][i] if compcnt > 1 else args[0]
    return 3.0 + 2.0 * abs(arg)

# The precision for division from the GLSL spec:
# 2.5 ULP for b in the range [2^-126, 2^126].
def _div_tol(compcnt, args, i, j):
    divisor = args[1][i] if _is_sequence(args[1]) else args[1]
    assert (divisor >= pow(2,-126) and divisor <= pow(2,126)) or \
           (divisor <= -pow(2,-126) and divisor >= -pow(2,126))
    return 2.5

# The precision for pow from the GLSL spec:
# Inherited from exp2 (x * log2 (y)).
def _pow_tol(compcnt, args, i, j):
    x = args[0][i] if _is_sequence(args[0]) else args[0]
    y = args[1][i] if _is_sequence(args[1]) else args[1]
    return(_exp_tol(0, [x * _log_tol(0, [y], 0, 0)], 0, 0))


_tol_fns = { 'op-add': (lambda *args: 0.0),
             'op-assign-add': (lambda *args: 0.0),
             'op-sub': (lambda *args: 0.0),
             'op-assign-sub': (lambda *args: 0.0),
             'op-mult': (lambda *args: 0.0),
             'op-assign-mult': (lambda *args: 0.0),
             'op-div': (lambda compcnt, args, i, j: _div_tol(compcnt, args, i, j)),
             'op-assign-div': (lambda compcnt, args, i, j: _div_tol(compcnt, args, i, j)),
             'degrees': (lambda *args: 2.5),
             'radians': (lambda *args: 2.5),
             'pow': (lambda compcnt, args, i, j: _pow_tol(compcnt, args, i, j)),
             'exp': _exp_tol,
             'exp2': _exp_tol,
             'log': _log_tol,
             'log2': _log_tol,
             'sqrt': (lambda *args: 2.5),
             'inversesqrt': (lambda *args: 2.0) }


def _gen_tolerance(name, rettype, args):
    ret = []
    compcnt = rettype.num_cols * rettype.num_rows

    for j in range(rettype.num_cols):
        for i in range(rettype.num_rows):
            assert _tol_fns[name]
            ret.append(_tol_fns[name](compcnt, args, i, j))
    return ret

def make_indexers(signature):
   """Build a list of strings which index into every possible
   value of the result.  For example, if the result is a vec2,
   then build the indexers ['[0]', '[1]'].
   """
   if signature.rettype.num_cols == 1:
      col_indexers = ['']
   else:
      col_indexers = ['[{0}]'.format(i) for i in range(signature.rettype.num_cols)]
   if signature.rettype.num_rows == 1:
      row_indexers = ['']
   else:
      row_indexers = ['[{0}]'.format(i) for i in range(signature.rettype.num_rows)]
   return [col_indexer + row_indexer
           for col_indexer in col_indexers for row_indexer in row_indexers]

def shader_runner_type(glsl_type):
    """Return the appropriate type name necessary for binding a
    uniform of the given type using shader_runner's "uniform" command.
    Boolean values and vectors are converted to ints, and square
    matrices are written in "matNxN" form.
    """
    if glsl_type.base_type == glsl_bool:
        if glsl_type.is_scalar:
            return 'int'
        else:
            return 'ivec{0}'.format(glsl_type.num_rows)
    elif glsl_type.is_matrix:
        return 'mat{0}x{1}'.format(glsl_type.num_cols, glsl_type.num_rows)
    else:
        return str(glsl_type)

def shader_runner_format(values):
    """Format the given values for use in a shader_runner "uniform" or
    "probe rgba" command.  Sequences of values are separated by spaces.
    """

    if _is_sequence(values):
        retval = ''
        for x in values:
            assert isinstance(x, (float, np.float32))
            retval += ' {0:#08x}'.format(_floatToBits(x))
    else:
        assert isinstance(values, (float, np.float32))
        retval = '{0:#08x}'.format(_floatToBits(values))

    return retval


def main():
    """ Main function """

    for signature, test_vectors in six.iteritems(test_suite):
        arg_float_check = all(arg.base_type == glsl_float for arg in signature.argtypes)
        arg_mat_check = any(arg.is_matrix for arg in signature.argtypes)
        # Filter the test vectors down to only those which deal exclusively in
        # non-matrix float types and are specified in the spec
        if (signature.rettype.base_type == glsl_float and
            arg_float_check and
            signature.name in shader_precision_spec_fns and
            not arg_mat_check):
            # replace the tolerances in each test_vector with
            # our own tolerances specified in ulps
            refined_test_vectors = []
            complex_tol_type = signature.rettype
            for test_vector in test_vectors:
                tolerance = _gen_tolerance(signature.name, signature.rettype, test_vector.arguments)
                refined_test_vectors.append(TestVector(test_vector.arguments, test_vector.result, tolerance))
            # Then generate the shader_test scripts
            for shader_stage in ('vs', 'fs', 'gs'):
                template = template_file('gen_shader_precision_tests', '{0}.mako'.format(shader_stage))
                output_filename = os.path.join( 'spec', 'arb_shader_precision',
                                                '{0}-{1}-{2}.shader_test'.format(
                                                shader_stage, signature.name,
                                                '-'.join(str(argtype)
                                                for argtype in signature.argtypes)))
                print(output_filename)
                dirname = os.path.dirname(output_filename)
                if not os.path.exists(dirname):
                    os.makedirs(dirname)
                indexers = make_indexers(signature)
                num_elements = signature.rettype.num_cols * signature.rettype.num_rows
                invocation = signature.template.format( *['arg{0}'.format(i)
                                                        for i in range(len(signature.argtypes))])
                with open(output_filename, 'w') as f:
                    f.write(template.render_unicode( signature=signature,
                                                     is_complex_tolerance=_is_sequence(tolerance),
                                                     complex_tol_type=signature.rettype,
                                                     test_vectors=refined_test_vectors,
                                                     invocation=invocation,
                                                     num_elements=num_elements,
                                                     indexers=indexers,
                                                     shader_runner_type=shader_runner_type,
                                                     shader_runner_format=shader_runner_format,
                                                     column_major_values=column_major_values ))

if __name__ == "__main__":
    main()
