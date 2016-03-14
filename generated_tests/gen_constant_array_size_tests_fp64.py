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

# Generate a pair of glsl parser tests for every overloaded version of
# every built-in function, which test that the built-in functions are
# handled properly when applied to constant arguments inside an array
# size declaration.
#
# In each pair of generated tests, one test exercises the built-in
# function in vertex shaders, and the other exercises it in fragment
# shaders.
#
# This program outputs, to stdout, the name of each file it generates.
# With the optional argument --names-only, it only outputs the names
# of the files; it doesn't generate them.

from __future__ import print_function, division, absolute_import
from builtin_function_fp64 import *
import abc
import optparse
import os
import os.path

from six.moves import range

from modules import utils

class ParserTest(object):
    """Class used to build a test of a single built-in.  This is an
    abstract base class--derived types should override test_suffix(),
    output_var(), and other functions if necessary.
    """

    def __init__(self, signature, test_vectors):
        """Prepare to build a test for a single built-in.  signature
        is the signature of the built-in (a key from the
        builtin_function.test_suite dict), and test_vectors is the
        list of test vectors for testing the given builtin (the
        corresponding value from the builtin_function.test_suite
        dict).
        """
        self.__add_exten = ""
        self.__signature = signature
        self.__test_vectors = test_vectors
        if self.__signature.extension:
            self.__add_exten += 'GL_{0}'.format(self.__signature.extension)

    def glsl_version(self):
        if self.__signature.version_introduced < 120:
            # Before version 1.20, built-in function invocations
            # weren't allowed in constant expressions.  So even if
            # this built-in was introduced prior to 1.20, test it in
            # version 1.20.
            return 120
        else:
            return self.__signature.version_introduced

    def version_directive(self):
        return '#version {0}\n'.format(self.glsl_version())

    def additional_declarations(self):
        """Return a string containing any additional declarations that
        should be placed after the version directive.  Returns the
        empty string by default.
        """
        return ''

    def additional_extensions(self):
        """Return a list (or other iterable) containing any additional
        extension requirements that the test has.  Returns the empty
        list by default.
        """
        return self.__add_exten

    @abc.abstractmethod
    def test_suffix(self):
        """Return the suffix that should be used in the test file name
        to identify the type of shader, e.g. "vert" for a vertex
        shader test.
        """

    def make_condition(self, test_vector):
        """Generate a GLSL constant expression that should evaluate to
        true if the GLSL compiler's constant evaluation produces the
        correct result for the given test vector, and false if not.
        """
        invocation = self.__signature.template.format(
            *[glsl_constant(x) for x in test_vector.arguments])
        if self.__signature.rettype.base_type == glsl_double:
            # Test floating-point values within tolerance
            if self.__signature.name == 'distance':
                # Don't use the distance() function to test itself.
                return '{0} <= {1} && {1} <= {2}'.format(
                    test_vector.result - test_vector.tolerance,
                    invocation,
                    test_vector.result + test_vector.tolerance)
            elif self.__signature.rettype.is_matrix:
                # We can't apply distance() to matrices.  So apply it
                # to each column and root-sum-square the results.  It
                # is safe to use pow() here because its behavior is
                # verified in the pow() tests.
                terms = []
                for col in range(self.__signature.rettype.num_cols):
                    terms.append('(distance({0}[{1}], {2}) * distance({0}[{1}], {2}))'.format(
                        invocation, col,
                        glsl_constant(test_vector.result[:, col])))
                rss_distance = ' + '.join(terms)
                sq_tolerance = test_vector.tolerance * test_vector.tolerance
                return '{0} <= {1}'.format(
                    rss_distance, glsl_constant(sq_tolerance))
            else:
                return 'distance({0}, {1}) <= {2}'.format(
                    invocation, glsl_constant(test_vector.result),
                    glsl_constant(test_vector.tolerance))
        else:
            # Test non-floating point values exactly
            assert not self.__signature.rettype.is_matrix
            if self.__signature.name == 'equal':
                # Don't use the equal() function to test itself.
                assert self.__signature.rettype.is_vector
                terms = []
                for row in range(self.__signature.rettype.num_rows):
                    terms.append('{0}[{1}] == {2}'.format(
                        invocation, row,
                        glsl_constant(test_vector.result[row])))
                return ' && '.join(terms)
            elif self.__signature.rettype.is_vector:
                return 'all(equal({0}, {1}))'.format(
                    invocation, glsl_constant(test_vector.result))
            else:
                return '{0} == {1}'.format(
                    invocation, glsl_constant(test_vector.result))

    def make_shader(self):
        """Generate the shader code necessary to test the built-in."""
        shader = self.version_directive()
        if self.__signature.extension:
            shader += '#extension GL_{0} : require\n'.format(self.__signature.extension)
        shader += self.additional_declarations()
        shader += '\n'
        shader += 'void main()\n'
        shader += '{\n'
        lengths = []
        for i, test_vector in enumerate(self.__test_vectors):
            shader += '  double[{0} ? 1 : -1] array{1};\n'.format(
                self.make_condition(test_vector), i)
            lengths.append('array{0}.length()'.format(i))
        shader += '  {0} = dvec4({1});\n'.format(
            self.output_var(), ' + '.join(lengths))
        shader += '}\n'
        return shader

    def filename(self):
        argtype_names = '-'.join(
            str(argtype) for argtype in self.__signature.argtypes)
        if self.__signature.extension:
            subdir = self.__signature.extension.lower()
        else:
            subdir = 'glsl-{0:1.2f}'.format(float(self.glsl_version()) / 100)
        return os.path.join(
            'spec', subdir, 'compiler', 'built-in-functions',
            '{0}-{1}.{2}'.format(
                self.__signature.name, argtype_names, self.test_suffix()))

    def generate_parser_test(self):
        """Generate the test and write it to the output file."""
        parser_test = '/* [config]\n'
        parser_test += ' * expect_result: pass\n'
        parser_test += ' * glsl_version: {0:1.2f}\n'.format(
            float(self.glsl_version()) / 100)
        if self.additional_extensions():
            parser_test += ' * require_extensions: {0}\n'.format(self.additional_extensions())
        parser_test += ' * [end config]\n'
        parser_test += ' *\n'
        parser_test += ' * Check that the following test vectors are constant'\
                       'folded correctly:\n'
        for test_vector in self.__test_vectors:
            parser_test += ' * {0} => {1}\n'.format(
                self.__signature.template.format(
                    *[glsl_constant(arg) for arg in test_vector.arguments]),
                glsl_constant(test_vector.result))
        parser_test += ' */\n'
        parser_test += self.make_shader()
        filename = self.filename()
        dirname = os.path.dirname(filename)
        utils.safe_makedirs(dirname)
        with open(filename, 'w') as f:
            f.write(parser_test)


class VertexParserTest(ParserTest):
    """Derived class for tests that exercise the built-in in a vertex
    shader.
    """
    def test_suffix(self):
        return 'vert'

    def output_var(self):
        return 'gl_Position'


class GeometryParserTest(ParserTest):
    """Derived class for tests that exercise the built-in in a geometry
    shader.
    """
    def glsl_version(self):
        return max(150, ParserTest.glsl_version(self))

    def test_suffix(self):
        return 'geom'

    def output_var(self):
        return 'gl_Position'


class FragmentParserTest(ParserTest):
    """Derived class for tests that exercise the built-in in a fagment
    shader.
    """
    def test_suffix(self):
        return 'frag'

    def output_var(self):
        return 'gl_FragColor'


def all_tests():
    for signature, test_vectors in sorted(test_suite.items()):
        yield VertexParserTest(signature, test_vectors)
        yield GeometryParserTest(signature, test_vectors)
        yield FragmentParserTest(signature, test_vectors)


def main():
    desc = 'Generate shader tests that test built-in functions using constant'\
           'array sizes'
    usage = 'usage: %prog [-h] [--names-only]'
    parser = optparse.OptionParser(description=desc, usage=usage)
    parser.add_option('--names-only',
                      dest='names_only',
                      action='store_true',
                      help="Don't output files, just generate a list of"
                           "filenames to stdout")
    options, args = parser.parse_args()

    for test in all_tests():
        if not options.names_only:
            test.generate_parser_test()
        print(test.filename())


if __name__ == '__main__':
    main()
