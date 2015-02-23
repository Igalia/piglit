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
 templates in shader_precision_templates/.

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

 This program outputs, to stdout, the name of each file it generates.
"""

from __future__ import print_function, division
from builtin_function import *
import os 

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

trig_builtins = ('sin', 'cos', 'tan', 
                 'asin', 'acos', 'atan', 
                 'sinh', 'cosh', 'tanh', 
                 'asinh', 'acosh', 'atanh')

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
    "probe rgba" command.  Bools are converted to 0's and 1's, and
    values are separated by spaces.
    """
    transformed_values = []
    retval = ''
    for value in values:
        if isinstance(value, (bool, np.bool_)):
            transformed_values.append(int(value))
        else:
            transformed_values.append(value)
    for x in transformed_values:
        if isinstance(x,np.float32):
            retval+=' {0}'.format('{0:1.8e}'.format(x))
        else:
            retval+=' {0}'.format(repr(x))
    return retval

def main():
    """ Main function """

    for signature, test_vectors in sorted(six.iteritems(test_suite)):
        arg_float_check = tuple(
                        arg.base_type == glsl_float for arg in signature.argtypes)
        # Filter the test vectors down to only those which deal exclusively in float types
        #and are not trig functions or determinant()
        indexers = make_indexers(signature)
        num_elements = signature.rettype.num_cols*signature.rettype.num_rows
        invocation = signature.template.format( *['arg{0}'.format(i) 
                                                for i in range(len(signature.argtypes))])
        if (signature.rettype.base_type == glsl_float and
            arg_float_check and
            all(arg_float_check) and
            signature.name not in trig_builtins and
            signature.name != 'determinant'): 
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
                with open(output_filename, 'w') as f:
                    f.write(template.render_unicode( signature=signature, 
                                                     test_vectors=test_vectors,
                                                     tolerances=tolerances,
                                                     invocation=invocation,
                                                     num_elements=num_elements,
                                                     indexers=indexers,
                                                     shader_runner_type=shader_runner_type,
                                                     shader_runner_format=shader_runner_format,
                                                     column_major_values=column_major_values ))

if __name__ == "__main__":
    main()
