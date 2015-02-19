# coding=utf-8
#
# Copyright © 2011 Intel Corporation
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

# Generate a set of shader_runner tests for every overloaded version
# of every built-in function, based on the test vectors computed by
# builtin_function.py.
#
# In each set of generated tests, one test exercises the built-in
# function in each type of shader (vertex, geometry, and fragment).
# In all cases, the inputs to the built-in function come from
# uniforms, so that the effectiveness of the test won't be
# circumvented by constant folding in the GLSL compiler.
#
# The tests operate by invoking the built-in function in the
# appropriate shader, applying a scale and offset so that the expected
# values are in the range [0.25, 0.75], and then outputting the result
# as a solid rgba color, which is then checked using shader_runner's
# "probe rgba" command.
#
# For built-in functions whose result type is a matrix, the test
# checks one column at a time.
#
# This program outputs, to stdout, the name of each file it generates.
# With the optional argument --names-only, it only outputs the names
# of the files; it doesn't generate them.

from __future__ import print_function
from builtin_function import *
import abc
import numpy
import optparse
import os
import os.path
import sys

from modules import utils


def compute_offset_and_scale(test_vectors):
    """Compute scale and offset values such that for each result in
    test_vectors, (result - offset) * scale is in the range [0.25,
    0.75], and scale is less than or equal to 1.0.  These values are
    used to transform the test vectors so that their outputs can be
    stored in gl_FragColor without overflow.
    """
    low = min(numpy.min(tv.result) for tv in test_vectors)
    hi = max(numpy.max(tv.result) for tv in test_vectors)
    span = hi - low
    center = (hi + low)/2.0
    span *= 2.0
    if span < 1.0:
        span = 1.0
    offset = center - span/2.0
    scale = 1.0/span
    return offset, scale


def shader_runner_format(values):
    """Format the given values for use in a shader_runner "uniform" or
    "probe rgba" command.  Bools are converted to 0's and 1's, and
    values are separated by spaces.
    """
    transformed_values = []
    for value in values:
        if isinstance(value, (bool, np.bool_)):
            transformed_values.append(int(value))
        else:
            transformed_values.append(value)
    return ' '.join(repr(x) for x in transformed_values)


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


class Comparator(object):
    """Base class which abstracts how we compare expected and actual
    values.
    """
    __metaclass__ = abc.ABCMeta

    def make_additional_declarations(self):
        """Return additional declarations, if any, that are needed in
        the shader program.
        """
        return ''

    @abc.abstractmethod
    def make_result_handler(self, invocation, output_var):
        """Return the shader code that is needed to produce the result
        and store it in output_var.

        invocation is the GLSL code to compute the output of the
        built-in function.
        """

    @abc.abstractmethod
    def make_result_test(self, test_num, test_vector):
        """Return the shader_runner test code that is needed to test a
        single test vector.
        """

    def testname_suffix(self):
        """Return a string to be used as a suffix on the test name to
        distinguish it from tests using other comparators."""
        return ''


class BoolComparator(Comparator):
    """Comparator that tests functions returning bools and bvecs by
    converting them to floats.

    This comparator causes code to be generated in the following form:

        rettype result = func(args);
        output_var = vec4(result, 0.0, ...);
    """
    def __init__(self, signature):
        assert not signature.rettype.is_matrix
        self.__signature = signature
        self.__padding = 4 - signature.rettype.num_rows

    def make_result_handler(self, invocation, output_var):
        statements = '  {0} result = {1};\n'.format(
            self.__signature.rettype, invocation)
        statements += '  {0} = vec4(result{1});\n'.format(
            output_var, ', 0.0' * self.__padding)
        return statements

    def convert_to_float(self, value):
        """Convert the given vector or scalar value to a list of
        floats representing the expected color produced by the test.
        """
        value = value*1.0  # convert bools to floats
        value = column_major_values(value)
        value += [0.0] * self.__padding
        return value

    def make_result_test(self, test_num, test_vector, draw):
        test = draw
        test += 'probe rgba {0} 0 {1}\n'.format(
            test_num,
            shader_runner_format(self.convert_to_float(test_vector.result)))
        return test


class BoolIfComparator(Comparator):
    """Comparator that tests functions returning bools by evaluating
    them inside an if statement.

    This comparator causes code to be generated in the following form:

        if (func(args))
          output_var = vec4(1.0, 1.0, 0.0, 1.0);
        else
          output_var = vecp(0.0, 0.0, 1.0, 1.0);
    """
    def __init__(self, signature):
        assert signature.rettype == glsl_bool
        self.__padding = 4 - signature.rettype.num_rows

    def make_result_handler(self, invocation, output_var):
        statements = '  if({0})\n'.format(invocation)
        statements += '    {0} = vec4(1.0, 1.0, 0.0, 1.0);\n'.format(
            output_var)
        statements += '  else\n'
        statements += '    {0} = vec4(0.0, 0.0, 1.0, 1.0);\n'.format(
            output_var)
        return statements

    def convert_to_float(self, value):
        """Convert the given vector or scalar value to a list of
        floats representing the expected color produced by the test.
        """
        if value:
            return [1.0, 1.0, 0.0, 1.0]
        else:
            return [0.0, 0.0, 1.0, 1.0]

    def make_result_test(self, test_num, test_vector, draw):
        test = draw
        test += 'probe rgba {0} 0 {1}\n'.format(
            test_num,
            shader_runner_format(self.convert_to_float(test_vector.result)))
        return test

    def testname_suffix(self):
        return '-using-if'


class IntComparator(Comparator):
    """Comparator that tests functions returning ints or ivecs using a
    strict equality test.

    This comparator causes code to be generated in the following form:

        rettype result = func(args);
        output_var = result == expected ? vec4(0.0, 1.0, 0.0, 1.0)
                                        : vec4(1.0, 0.0, 0.0, 1.0);
    """
    def __init__(self, signature):
        self.__signature = signature

    def make_additional_declarations(self):
        return 'uniform {0} expected;\n'.format(self.__signature.rettype)

    def make_result_handler(self, invocation, output_var):
        statements = '  {0} result = {1};\n'.format(
            self.__signature.rettype, invocation)
        statements += '  {v} = {cond} ? {green} : {red};\n'.format(
            v=output_var, cond='result == expected',
            green='vec4(0.0, 1.0, 0.0, 1.0)',
            red='vec4(1.0, 0.0, 0.0, 1.0)')
        return statements

    def make_result_test(self, test_num, test_vector, draw):
        test = 'uniform {0} expected {1}\n'.format(
            shader_runner_type(self.__signature.rettype),
            shader_runner_format(column_major_values(test_vector.result)))
        test += draw
        test += 'probe rgba {0} 0 0.0 1.0 0.0 1.0\n'.format(test_num)
        return test


class FloatComparator(Comparator):
    """Comparator that tests functions returning floats or vecs using a
    strict equality test.

    This comparator causes code to be generated in the following form:

        rettype result = func(args);
        output_var = distance(result, expected) <= tolerance
                     ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    """
    def __init__(self, signature):
        self.__signature = signature

    def make_additional_declarations(self):
        decls = 'uniform float tolerance;\n'
        decls += 'uniform {0} expected;\n'.format(self.__signature.rettype)
        return decls

    def make_indexers(self):
        """Build a list of strings which index into every possible
        value of the result.  For example, if the result is a vec2,
        then build the indexers ['[0]', '[1]'].
        """
        if self.__signature.rettype.num_cols == 1:
            col_indexers = ['']
        else:
            col_indexers = ['[{0}]'.format(i)
                            for i in xrange(self.__signature.rettype.num_cols)]
        if self.__signature.rettype.num_rows == 1:
            row_indexers = ['']
        else:
            row_indexers = ['[{0}]'.format(i)
                            for i in xrange(self.__signature.rettype.num_rows)]
        return [col_indexer + row_indexer
                for col_indexer in col_indexers
                for row_indexer in row_indexers]

    def make_result_handler(self, invocation, output_var):
        statements = '  {0} result = {1};\n'.format(
            self.__signature.rettype, invocation)
        # Can't use distance when testing itself, or when the rettype
        # is a matrix.
        if self.__signature.name == 'distance' or \
                self.__signature.rettype.is_matrix:
            statements += '  {0} residual = result - expected;\n'.format(
                self.__signature.rettype)
            statements += '  float error_sq = {0};\n'.format(
                ' + '.join(
                    'residual{0} * residual{0}'.format(indexer)
                    for indexer in self.make_indexers()))
            condition = 'error_sq <= tolerance * tolerance'
        else:
            condition = 'distance(result, expected) <= tolerance'
        statements += '  {v} = {cond} ? {green} : {red};\n'.format(
            v=output_var, cond=condition, green='vec4(0.0, 1.0, 0.0, 1.0)',
            red='vec4(1.0, 0.0, 0.0, 1.0)')
        return statements

    def make_result_test(self, test_num, test_vector, draw):
        test = 'uniform {0} expected {1}\n'.format(
            shader_runner_type(self.__signature.rettype),
            shader_runner_format(column_major_values(test_vector.result)))
        test += 'uniform float tolerance {0}\n'.format(
            shader_runner_format([test_vector.tolerance]))
        test += draw
        test += 'probe rgba {0} 0 0.0 1.0 0.0 1.0\n'.format(test_num)
        return test


class ShaderTest(object):
    """Class used to build a test of a single built-in.  This is an
    abstract base class--derived types should override test_prefix(),
    make_vertex_shader(), make_fragment_shader(), and other functions
    if necessary.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, signature, test_vectors, use_if):
        """Prepare to build a test for a single built-in.  signature
        is the signature of the built-in (a key from the
        builtin_function.test_suite dict), and test_vectors is the
        list of test vectors for testing the given builtin (the
        corresponding value from the builtin_function.test_suite
        dict).

        If use_if is True, then the generated test checks the result
        by using it in an if statement--this only works for builtins
        returning bool.
        """
        self._signature = signature
        self._test_vectors = test_vectors
        if use_if:
            self._comparator = BoolIfComparator(signature)
        elif signature.rettype.base_type == glsl_bool:
            self._comparator = BoolComparator(signature)
        elif signature.rettype.base_type == glsl_float:
            self._comparator = FloatComparator(signature)
        elif signature.rettype.base_type in (glsl_int, glsl_uint):
            self._comparator = IntComparator(signature)
        else:
            raise Exception('Unexpected rettype {0}'.format(signature.rettype))

    def glsl_version(self):
        return self._signature.version_introduced

    def draw_command(self):
        if self.glsl_version() >= 140:
            return 'draw arrays GL_TRIANGLE_FAN 0 4\n'
        else:
            return 'draw rect -1 -1 2 2\n'

    def make_additional_requirements(self):
        """Return a string that should be included in the test's
        [require] section.
        """
        return ''

    @abc.abstractmethod
    def test_prefix(self):
        """Return the prefix that should be used in the test file name
        to identify the type of test, e.g. "vs" for a vertex shader
        test.
        """

    def make_vertex_shader(self):
        """Return the vertex shader for this test (or None if this
        test doesn't require a vertex shader).  No need to
        reimplement this function in classes that don't use vertex
        shaders.
        """
        return None

    def make_geometry_shader(self):
        """Return the geometry shader for this test (or None if this
        test doesn't require a geometry shader).  No need to
        reimplement this function in classes that don't use geometry
        shaders.
        """
        return None

    def make_geometry_layout(self):
        """Return the geometry layout for this test (or None if this
        test doesn't require a geometry layout section).  No need to
        reimplement this function in classes that don't use geometry
        shaders.
        """
        return None

    def make_fragment_shader(self):
        """Return the fragment shader for this test (or None if this
        test doesn't require a fragment shader).  No need to
        reimplement this function in classes that don't use fragment
        shaders.
        """
        return None

    def make_compute_shader(self):
        """Return the compute shader for this test (or None if this test
        doesn't require a compute shader).  No need to reimplement
        this function in classes that don't use compute shaders.
        """
        return None

    def make_test_shader(self, additional_declarations, prefix_statements,
                         output_var, suffix_statements):
        """Generate the shader code necessary to test the built-in.
        additional_declarations is a string containing any
        declarations that need to be before the main() function of the
        shader.  prefix_statements is a string containing any
        additional statements than need to be inside the main()
        function of the shader, before the built-in function is
        called.  output_var is the variable that the result of the
        built-in function should be assigned to, after conversion to a
        vec4.  suffix_statements is a string containing any additional
        statements that need to be inside the main() funciton of the
        shader, after the built-in function is called.
        """
        shader = ''
        if self._signature.extension:
            shader += '#extension GL_{0} : require\n'.format(self._signature.extension)
        shader += additional_declarations
        for i in xrange(len(self._signature.argtypes)):
            shader += 'uniform {0} arg{1};\n'.format(
                self._signature.argtypes[i], i)
        shader += self._comparator.make_additional_declarations()
        shader += '\n'
        shader += 'void main()\n'
        shader += '{\n'
        shader += prefix_statements
        invocation = self._signature.template.format(
            *['arg{0}'.format(i)
              for i in xrange(len(self._signature.argtypes))])
        shader += self._comparator.make_result_handler(invocation, output_var)
        shader += suffix_statements
        shader += '}\n'
        return shader

    def make_test_init(self):
        """Generate initialization for the test.
        """
        return ''

    def make_test(self):
        """Make the complete shader_runner test file, and return it as
        a string.
        """
        test = self.make_test_init()
        for test_num, test_vector in enumerate(self._test_vectors):
            for i in xrange(len(test_vector.arguments)):
                test += 'uniform {0} arg{1} {2}\n'.format(
                    shader_runner_type(self._signature.argtypes[i]),
                    i, shader_runner_format(
                        column_major_values(test_vector.arguments[i])))
            # Note: shader_runner uses a 250x250 window so we must
            # ensure that test_num <= 250.
            test += self._comparator.make_result_test(
                test_num % 250, test_vector, self.draw_command())
        return test

    def make_vbo_data(self):
        # Starting with GLSL 1.40/GL 3.1, we need to use VBOs and
        # vertex shader input bindings for our vertex data instead of
        # the piglit drawing utilities and gl_Vertex.
        if self.glsl_version() < 140:
            return ""
        vbo = '[vertex data]\n'
        vbo += 'vertex/float/2\n'
        vbo += '-1.0 -1.0\n'
        vbo += ' 1.0 -1.0\n'
        vbo += ' 1.0  1.0\n'
        vbo += '-1.0  1.0\n'
        vbo += '\n'
        return vbo

    def filename(self):
        argtype_names = '-'.join(
            str(argtype) for argtype in self._signature.argtypes)
        if self._signature.extension:
            subdir = self._signature.extension.lower()
        else:
            subdir = 'glsl-{0:1.2f}'.format(float(self.glsl_version()) / 100)
        return os.path.join(
            'spec', subdir, 'execution', 'built-in-functions',
            '{0}-{1}-{2}{3}.shader_test'.format(
                self.test_prefix(), self._signature.name, argtype_names,
                self._comparator.testname_suffix()))

    def generate_shader_test(self):
        """Generate the test and write it to the output file."""
        shader_test = '[require]\n'
        shader_test += 'GLSL >= {0:1.2f}\n'.format(
            float(self.glsl_version()) / 100)
        shader_test += self.make_additional_requirements()
        shader_test += '\n'
        vs = self.make_vertex_shader()
        if vs:
            shader_test += '[vertex shader]\n'
            shader_test += vs
            shader_test += '\n'
        gs = self.make_geometry_shader()
        if gs:
            shader_test += '[geometry shader]\n'
            shader_test += gs
            shader_test += '\n'
        gl = self.make_geometry_layout()
        if gl:
            shader_test += '[geometry layout]\n'
            shader_test += gl
            shader_test += '\n'
        fs = self.make_fragment_shader()
        if fs:
            shader_test += '[fragment shader]\n'
            shader_test += fs
            shader_test += '\n'
        cs = self.make_compute_shader()
        if cs:
            shader_test += '[compute shader]\n'
            shader_test += cs
            shader_test += '\n'
        if vs:
            shader_test += self.make_vbo_data()
        shader_test += '[test]\n'
        shader_test += self.make_test()
        filename = self.filename()
        dirname = os.path.dirname(filename)
        utils.safe_makedirs(dirname)
        with open(filename, 'w') as f:
            f.write(shader_test)


class VertexShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a vertex
    shader.
    """
    def test_prefix(self):
        return 'vs'

    def make_vertex_shader(self):
        if self.glsl_version() >= 140:
            return self.make_test_shader(
                'in vec4 vertex;\n' +
                'out vec4 color;\n',
                '  gl_Position = vertex;\n',
                'color', '')
        else:
            return self.make_test_shader(
                'varying vec4 color;\n',
                '  gl_Position = gl_Vertex;\n',
                'color', '')

    def make_fragment_shader(self):
        shader = '''varying vec4 color;

void main()
{
  gl_FragColor = color;
}
'''
        return shader


class GeometryShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a
    geometry shader.
    """
    def test_prefix(self):
        return 'gs'

    def glsl_version(self):
        return max(150, ShaderTest.glsl_version(self))

    def make_vertex_shader(self):
        shader = ''
        shader += "in vec4 vertex;\n"
        shader += "out vec4 vertex_to_gs;\n"

        shader += "void main()\n"
        shader += "{\n"
        shader += "     vertex_to_gs = vertex;\n"
        shader += "}\n"

        return shader

    def make_geometry_shader(self):
        additional_declarations = ''
        additional_declarations += 'layout(triangles) in;\n'
        additional_declarations \
            += 'layout(triangle_strip, max_vertices = 3) out;\n'
        additional_declarations += 'in vec4 vertex_to_gs[3];\n'
        additional_declarations += 'out vec4 color;\n'
        return self.make_test_shader(
            additional_declarations,
            '  vec4 tmp_color;\n',
            'tmp_color',
            '  for (int i = 0; i < 3; i++) {\n'
            '    gl_Position = vertex_to_gs[i];\n'
            '    color = tmp_color;\n'
            '    EmitVertex();\n'
            '  }\n')

    def make_fragment_shader(self):
        shader = '''varying vec4 color;

void main()
{
  gl_FragColor = color;
}
'''
        return shader


class FragmentShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a
    fragment shader.
    """
    def test_prefix(self):
        return 'fs'

    def make_vertex_shader(self):
        shader = ""
        if self.glsl_version() >= 140:
            shader += "in vec4 vertex;\n"

        shader += "void main()\n"
        shader += "{\n"
        if self.glsl_version() >= 140:
            shader += "        gl_Position = vertex;\n"
        else:
            shader += "        gl_Position = gl_Vertex;\n"
        shader += "}\n"

        return shader

    def make_fragment_shader(self):
        return self.make_test_shader('', '', 'gl_FragColor', '')


class ComputeShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a
    compute shader.
    """
    def test_prefix(self):
        return 'cs'

    def glsl_version(self):
        return max(430, ShaderTest.glsl_version(self))

    def make_compute_shader(self):
        additional_declarations = 'writeonly uniform image2D tex;\n'
        num_tests = len(self._test_vectors)
        layout_tmpl = 'layout(local_size_x = {0}) in;\n'
        additional_declarations += layout_tmpl.format(num_tests)
        return self.make_test_shader(
            additional_declarations,
            '  vec4 tmp_color;\n',
            'tmp_color',
            '  ivec2 coord = ivec2(gl_GlobalInvocationID.xy);\n'
            '  imageStore(tex, coord, tmp_color);\n')

    def make_test_init(self):
        return '''uniform int tex 0
texture rgbw 0 ({0}, 1)
image texture 0
fb tex 2d 0
'''.format(len(self._test_vectors))


    def draw_command(self):
        return 'compute 1 1 1\n'

def all_tests():
    for use_if in [False, True]:
        for signature, test_vectors in sorted(test_suite.items()):
            if use_if and signature.rettype != glsl_bool:
                continue
            yield VertexShaderTest(signature, test_vectors, use_if)
            yield GeometryShaderTest(signature, test_vectors, use_if)
            yield FragmentShaderTest(signature, test_vectors, use_if)
            yield ComputeShaderTest(signature, test_vectors, use_if)


def main():
    desc = 'Generate shader tests that test built-in functions using uniforms'
    usage = 'usage: %prog [-h] [--names-only]'
    parser = optparse.OptionParser(description=desc, usage=usage)
    parser.add_option(
        '--names-only',
        dest='names_only',
        action='store_true',
        help="Don't output files, just generate a list of filenames to stdout")
    options, args = parser.parse_args()
    for test in all_tests():
        if not options.names_only:
            test.generate_shader_test()
        print(test.filename())


if __name__ == '__main__':
    main()
