# coding=utf-8
#
# Copyright © 2011, 2018 Intel Corporation
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
# builtin_function.py. The sources are generated directly as SPIR-V
# assembly.
#
# In each set of generated tests, one test exercises the built-in
# function in each type of shader (vertex, geometry, and fragment). In
# all cases, the inputs to the built-in function come from uniforms,
# so that the effectiveness of the test won't be circumvented by
# constant folding in the GLSL compiler. Note that all but the
# fragment shader tests are currently commented out on the asumption
# that there won’t be much difference between the codepaths taken by
# the driver for the various stages that aren’t already tested in the
# GLSL versions of the tests.
#
# For built-in functions whose result type is a matrix, the test
# checks one column at a time.
#
# This program outputs, to stdout, the name of each file it generates.
# With the optional argument --names-only, it only outputs the names
# of the files; it doesn't generate them.

from __future__ import print_function, division, absolute_import
from builtin_function import *
import abc
import numpy
import optparse
import os
import os.path
import itertools

from six.moves import range

from modules import utils


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


def matrix_stride(glsl_type):
    """Calculates the matrix stride for a glsl type assuming std140 rules."""
    component_size = 4

    if glsl_type.num_rows == 3:
        base_alignment = component_size * 4
    else:
        base_alignment = component_size * glsl_type.num_rows

    # Align to a vec4
    return (base_alignment + 15) & ~15


def set_uniform(api, glsl_type, location, value):
    if api == 'vulkan':
        command = 'uniform ubo 0'
    else:
        command = 'uniform'
    return "{} {} {} {}\n".format(
        command,
        shader_runner_type(glsl_type),
        location,
        shader_runner_format(value))


class ShaderSource:
    """An object with an array of strings for each section of a SPIR-V
    shader. These can be combined together to generate the final
    source in the right order. This is done because the various parts
    of the SPIR-V source need to be in a specific order but we want to
    add them when it’s convenient.
    """

    Uniform = collections.namedtuple('Uniform', ('name', 'type', 'location'))

    def __init__(self, stage, api):
        self.stage = stage
        self.api = api
        self.capabilities = []
        self.extensions = []
        self.instruction_imports = ["%glsl450 = "
                                    "OpExtInstImport \"GLSL.std.450\""]
        self.interfaces = []
        self.annotations = []
        self.source = []
        self.globals = ["%void = OpTypeVoid",
                        "%void_function = OpTypeFunction %void"]
        self.types = set()
        self.execution_modes = ["OriginLowerLeft"]
        self.next_uniform_location = 0
        self.uniforms = []

        if stage in ('Vertex', 'Fragment', 'GLCompute'):
            self.add_capability('Shader')
        elif stage == 'Geometry':
            self.add_capability('Geometry')
        elif stage in ('TessellationControl', 'TessellationEvaluation'):
            self.add_capability('Tessellation')

    def get_next_uniform_location(self, type):
        location = self.next_uniform_location

        if self.api == 'gl':
            self.next_uniform_location += 1
            return location

        # Align the location to the component size
        base_type = type.base_type
        if base_type in (glsl_int64_t, glsl_uint64_t):
            comp_size = 8
        else:
            comp_size = 4

        location = (location + comp_size - 1) & ~(comp_size - 1)

        if type.num_cols > 1:
            self.next_uniform_location = (location + matrix_stride(type) *
                                          type.num_cols)
        else:
            self.next_uniform_location = location + comp_size * type.num_rows

        return location

    def get_source(self):
        if self.api == 'vulkan' and len(self.uniforms) > 0:
            globals = self.globals + ["%uniforms_type = OpTypeStruct " +
                                      " ".join(x.type for x in self.uniforms),
                                      "%uniforms_ptr_type = OpTypePointer "
                                      "Uniform %uniforms_type",
                                      "%uniforms = OpVariable "
                                      "%uniforms_ptr_type Uniform"]
            annotations = self.annotations + ["OpDecorate %uniforms Binding 0",
                                              "OpDecorate %uniforms "
                                              "DescriptorSet 0",
                                              "OpDecorate %uniforms_type Block"]
        else:
            globals = self.globals
            annotations = self.annotations

        return ("\n".join("OpCapability {}".format(x)
                          for x in self.capabilities) + "\n" +
                "\n".join("OpExtension \"{}\"".format(x)
                          for x in self.extensions) + "\n" +
                "\n".join(self.instruction_imports) + "\n" +
                "OpMemoryModel Logical GLSL450\n"
                "OpEntryPoint " + self.stage + " %main \"main\" " +
                " ".join(self.interfaces) + "\n" +
                "\n".join("OpExecutionMode %main {}".format(x)
                          for x in self.execution_modes) + "\n" +
                "\n".join(annotations) + "\n" +
                "\n".join(globals) + "\n" +
                "%main = OpFunction %void None %void_function\n" +
                "%main_label = OpLabel\n" +
                "\n".join(self.source) + "\n"
                "OpReturn\n" +
                "OpFunctionEnd\n")

    def add_uniform(self, name, type):
        location = self.get_next_uniform_location(type)
        type_name = self.get_glsl_type(type)
        self.uniforms.append(ShaderSource.Uniform(name, type_name, location))

        if self.api == 'gl':
            self.add_global("%uniform{} = OpVariable {} UniformConstant".format(
                location, type_name))
            self.add_annotation("OpDecorate %uniform{} Location {}".format(
                location, location))
            self.add_source("{} = OpLoad {} %uniform{}".format(
                name, type_name, location))
        elif self.api == 'vulkan':
            type_ptr = self.get_pointer_type(type_name, 'Uniform')
            uniform_num = len(self.uniforms) - 1
            self.add_global("%uniforms_index{} = OpConstant {} {}".format(
                uniform_num, self.get_int_type(32, False), uniform_num))
            self.add_source("%uniform{}_ptr = OpAccessChain "
                            "{} %uniforms %uniforms_index{}".format(
                                uniform_num, type_ptr, uniform_num))
            self.add_source("{} = OpLoad {} %uniform{}_ptr".format(
                name, type_name, uniform_num))
            self.add_annotation("OpMemberDecorate %uniforms_type "
                                "{} Offset {}".format(uniform_num, location))
            if type.num_cols > 1:
                self.add_annotation("OpMemberDecorate %uniforms_type "
                                    "{} MatrixStride {}".format(
                                        uniform_num, matrix_stride(type)))

        return location

    def add_annotation(self, line):
        self.annotations.append(line)

    def add_global(self, line):
        self.globals.append(line)

    def add_interface(self, interface):
        self.interfaces.append(interface)

    def add_instruction_import(self, line):
        self.instruction_imports.append(line)

    def add_capability(self, line):
        self.capabilities.append(line)

    def add_extension(self, line):
        self.extensions.append(line)

    def add_execution_mode(self, line):
        self.execution_modes.append(line)

    def get_int_type(self, bits, signed):
        name = "%{}int_t_{}".format("" if signed else "u", bits)
        if name not in self.types:
            self.add_global("{} = OpTypeInt {} {}".format(
                name, bits, int(signed)))
            self.types.add(name)

        return name

    def get_float_type(self, bits):
        name = "%float_t_{}".format(bits)
        if name not in self.types:
            self.add_global("{} = OpTypeFloat {}".format(name, bits))
            self.types.add(name)

        return name

    def get_bool_type(self):
        name = "%bool_t"
        if name not in self.types:
            self.add_global("{} = OpTypeBool".format(name))
            self.types.add(name)

        return name

    def get_vec_type(self, base_type, size):
        name = "{}_vec{}".format(base_type, size)
        if name not in self.types:
            self.add_global("{} = OpTypeVector {} {}".format(
                name, base_type, size))
            self.types.add(name)

        return name

    def get_matrix_type(self, column_type, columns):
        name = "{}_mat{}".format(column_type, columns)
        if name not in self.types:
            self.add_global("{} = OpTypeMatrix {} {}".format(
                name, column_type, columns))
            self.types.add(name)

        return name

    def get_pointer_type(self, base_type, storage_class):
        name = "{}_ptr_{}".format(base_type, storage_class)
        if name not in self.types:
            self.add_global("{} = OpTypePointer {} {}".format(
                name, storage_class, base_type))
            self.types.add(name)

        return name

    def get_glsl_type(self, glsl_type):
        glsl_base_type = glsl_type.base_type

        if glsl_base_type == glsl_bool:
            type = self.get_bool_type()
        elif glsl_base_type == glsl_int:
            type = self.get_int_type(32, True)
        elif glsl_base_type == glsl_uint:
            type = self.get_int_type(32, False)
        elif glsl_base_type == glsl_int64_t:
            type = self.get_int_type(64, True)
        elif glsl_base_type == glsl_uint64_t:
            type = self.get_int_type(64, False)
        elif glsl_base_type == glsl_float:
            type = self.get_float_type(32)

        if glsl_type.num_rows > 1:
            type = self.get_vec_type(type, glsl_type.num_rows)

            if glsl_type.num_cols > 1:
                type = self.get_matrix_type(type, glsl_type.num_cols)

        return type

    def add_source(self, line):
        self.source.append(line)


class Comparator(object):
    """Base class which abstracts how we compare expected and actual
    values.
    """
    __metaclass__ = abc.ABCMeta

    @abc.abstractmethod
    def add_result_handler(self, shader, invocation, output_var):
        """Add the code to calculate the result to shader. Returns the
        ResultId of the result.
        """

    @abc.abstractmethod
    def draw_test(self, test_vector, draw_command):
        """Return the shader_runner test code that is needed to run a
        single test vector.
        """

    @abc.abstractmethod
    def result_vector(self, test_vector):
        """Return the expected result color as a list of floats."""

    def testname_suffix(self):
        """Return a string to be used as a suffix on the test name to
        distinguish it from tests using other comparators."""
        return ''

    def add_result_start(self, shader, rettype):
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        result_type = shader.get_glsl_type(rettype)

        self.expected_location = shader.add_uniform("%expected", rettype)
        shader.add_global("%one = OpConstant {} 1.0".format(float_type))
        shader.add_global("%zero = OpConstant {} 0.0".format(float_type))
        shader.add_global("%red = OpConstantComposite {} "
                          "%one %zero %zero %one".format(vec4_type))
        shader.add_global("%green = OpConstantComposite {} "
                          "%zero %one %zero %one".format(vec4_type))

    def add_result_end(self, shader):
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        bool_type = shader.get_bool_type()
        bvec4_type = shader.get_vec_type(bool_type, 4)
        shader.add_source("%resv = OpCompositeConstruct {} "
                          "%res %res %res %res".format(bvec4_type))
        shader.add_source("%result = OpSelect {} %resv %green %red".format(
            vec4_type))

        return "%result"


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

    def add_result_handler(self, shader, invocation):
        bool_type = shader.get_bool_type()
        bvec4_type = shader.get_vec_type(bool_type, 4)
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        num_rows = self.__signature.rettype.num_rows
        shader.add_global("%false = OpConstantFalse {}".format(bool_type))

        if num_rows > 1:
            for i in range(num_rows):
                shader.add_source("%part{} = OpCompositeExtract "
                                  "{} {} {}".format(
                                      i, bool_type, invocation, i))
            parts = " ".join("%part{}".format(i) for i in range(num_rows))
            parts += " " + " ".join(itertools.repeat("%false", self.__padding))
            shader.add_source("%parts = OpCompositeConstruct {} {}".format(
                bvec4_type, parts))
        else:
            shader.add_source("%parts = OpCompositeConstruct {} {} "
                              "%false %false %false".format(
                                  bvec4_type, invocation))

        shader.add_global("%one = OpConstant {} 1.0".format(float_type))
        shader.add_global("%zero = OpConstant {} 0.0".format(float_type))
        shader.add_global("%ones = OpConstantComposite {} "
                          "%one %one %one %one".format(vec4_type))
        shader.add_global("%zeros = OpConstantComposite {} "
                          "%zero %zero %zero %zero".format(vec4_type))
        shader.add_source("%result = OpSelect {} %parts %ones %zeros".format(
            vec4_type))

        return "%result"

    def draw_test(self, test_vector, draw_command):
        test = draw_command
        return test

    def convert_to_float(self, value):
        """Convert the given vector or scalar value to a list of
        floats representing the expected color produced by the test.
        """
        value = value*1.0  # convert bools to floats
        value = column_major_values(value)
        value += [0.0] * self.__padding
        return value

    def draw_test(self, test_vector, draw_command):
        return draw_command

    def result_vector(self, test_vector):
        return self.convert_to_float(test_vector.result)


class IntComparator(Comparator):
    """Comparator that tests functions returning ints or ivecs using a
    strict equality test.

    This comparator causes code to be generated in the following form:

        rettype result = func(args);
        output_var = result == expected ? vec4(0.0, 1.0, 0.0, 1.0)
                                        : vec4(1.0, 0.0, 0.0, 1.0);
    """
    def __init__(self, signature, api):
        self.__signature = signature
        self.__api = api

    def add_result_handler(self, shader, invocation):
        self.add_result_start(shader, self.__signature.rettype)

        bool_type = shader.get_bool_type()
        num_rows = self.__signature.rettype.num_rows

        if num_rows == 1:
            shader.add_source("%res = OpIEqual {} "
                              "%expected {}".format(bool_type, invocation))
        else:
            bvec_type = shader.get_vec_type(bool_type, num_rows)
            shader.add_source("%res_parts = OpIEqual {} "
                              "%expected {}".format(bvec_type, invocation))
            shader.add_source("%res = OpAll {} %res_parts".format(bool_type))

        self.add_result_end(shader)

        return "%result"

    def draw_test(self, test_vector, draw_command):
        test = set_uniform(self.__api,
                           self.__signature.rettype,
                           self.expected_location,
                           column_major_values(test_vector.result))
        test += draw_command
        return test

    def result_vector(self, test_vector):
        return [0.0, 1.0, 0.0, 1.0]


class FloatComparator(Comparator):
    """Comparator that tests functions returning floats or vecs using a
    strict equality test.

    This comparator causes code to be generated in the following form:

        rettype result = func(args);
        output_var = distance(result, expected) <= tolerance
                     ? vec4(0.0, 1.0, 0.0, 1.0) : vec4(1.0, 0.0, 0.0, 1.0);
    """
    def __init__(self, signature, api):
        self.__signature = signature
        self.__api = api

    def add_result_handler(self, shader, invocation):
        self.add_result_start(shader, self.__signature.rettype)

        bool_type = shader.get_bool_type()
        bvec4_type = shader.get_vec_type(bool_type, 4)
        float_type = shader.get_float_type(32)

        self.tolerance_location = shader.add_uniform("%tolerance", glsl_float)

        if self.__signature.rettype.num_cols == 1:
            shader.add_source("%dist = OpExtInst {} %glsl450 Distance {} "
                              "%expected".format(float_type, invocation))
            shader.add_source("%res = OpFOrdLessThanEqual {} "
                              "%dist %tolerance".format(bool_type))
        else:
            base_type = shader.get_glsl_type(self.__signature.rettype.base_type)
            num_rows = self.__signature.rettype.num_rows
            index = 0

            for col in range(self.__signature.rettype.num_cols):
                for row in range(num_rows):
                    shader.add_source("%ic{} = OpCompositeExtract "
                                      "{} {} {} {}".format(
                                          index, base_type, invocation,
                                          col, row))
                    shader.add_source("%ec{} = OpCompositeExtract "
                                      "{} %expected {} {}".format(
                                          index, base_type, col, row))
                    shader.add_source("%residual{} = OpFSub "
                                      "{} %ic{} %ec{}".format(
                                          index, base_type, index, index))
                    shader.add_source("%residual_sq{} = OpFMul "
                                      "{} %residual{} %residual{}".format(
                                          index, base_type, index, index))
                    if index == 1:
                        shader.add_source("%sum1 = OpFAdd "
                                          "{} %residual_sq0 "
                                          "%residual_sq1".format(base_type))
                    elif index > 1:
                        shader.add_source("%sum{} = OpFAdd "
                                          "{} %sum{} %residual_sq{}".format(
                                              index, base_type,
                                              index - 1, index))
                    index += 1
            shader.add_source("%tolerance_sq = OpFMul {} "
                              "%tolerance %tolerance".format(base_type))
            shader.add_source("%res = OpFOrdLessThanEqual {} "
                              "%sum{} %tolerance_sq".format(
                                  bool_type, index - 1))

        self.add_result_end(shader)

        return "%result"

    def draw_test(self, test_vector, draw_command):
        test = set_uniform(self.__api,
                           self.__signature.rettype,
                           self.expected_location,
                           column_major_values(test_vector.result))
        test += set_uniform(self.__api,
                            glsl_float,
                            self.tolerance_location,
                            [test_vector.tolerance])
        test += draw_command
        return test

    def result_vector(self, test_vector):
        return [0.0, 1.0, 0.0, 1.0]


class ShaderTest(object):
    """Class used to build a test of a single built-in.  This is an
    abstract base class--derived types should override test_prefix(),
    make_vertex_shader(), make_fragment_shader(), and other functions
    if necessary.
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, signature, test_vectors, api):
        """Prepare to build a test for a single built-in.  signature
        is the signature of the built-in (a key from the
        builtin_function.test_suite dict), and test_vectors is the
        list of test vectors for testing the given builtin (the
        corresponding value from the builtin_function.test_suite
        dict).
        """
        self._signature = signature
        self._test_vectors = test_vectors
        self._api = api

        # Size of the rectangles drawn by the test.
        self.rect_width = 4
        self.rect_height = 4
        # shader_runner currently defaults to a 250x250 window.  We
        # could reduce window size to cut test time, but there are
        # platform-dependent limits we haven't really characterized
        # (smaller on Linux than Windows, but still present in some
        # window managers).
        self.win_width = 250
        self.win_height = 250
        self.tests_per_row = (self.win_width // self.rect_width)
        self.test_rows = (self.win_height // self.rect_height)

        if signature.rettype.base_type == glsl_bool:
            self._comparator = BoolComparator(signature)
        elif signature.rettype.base_type in (glsl_int, glsl_uint,
                                             glsl_int64_t, glsl_uint64_t):
            self._comparator = IntComparator(signature, self._api)
        else:
            self._comparator = FloatComparator(signature, self._api)

    def glsl_version(self):
        return self._signature.version_introduced

    def draw_command(self, test_num):
        x = (test_num % self.tests_per_row) * self.rect_width
        y = (test_num // self.tests_per_row) * self.rect_height
        assert(y < self.test_rows)
        return 'draw rect ortho {0} {1} {2} {3}\n'.format(x, y,
                                                          self.rect_width,
                                                          self.rect_height)

    def probe_command(self, test_num, probe_vector):
        return 'probe rect rgba ({0}, {1}, {2}, {3}) ({4}, {5}, {6}, {7})\n'.format(
            (test_num % self.tests_per_row) * self.rect_width,
            (test_num // self.tests_per_row) * self.rect_height,
            self.rect_width,
            self.rect_height,
            probe_vector[0], probe_vector[1], probe_vector[2], probe_vector[3])

    def extensions(self):
        ext = []
        if self._signature.extension:
            ext.append(self._signature.extension)
        return ext

    def make_additional_requirements(self):
        """Return a string that should be included in the test's
        [require] section.
        """
        return ''

    def make_additional_vulkan_requirements(self):
        """Return a string that should be included in the test's
        [require] section when running on Vulkan.
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

    def make_tess_ctrl_shader(self):
        """Return the tessellation control shader for this test
        (or None if this test doesn't require a geometry shader).
        No need to reimplement this function in classes that don't
        use tessellation shaders.
        """
        return None

    def make_tess_eval_shader(self):
        """Return the tessellation evaluation shader for this test
        (or None if this test doesn't require a geometry shader).
        No need to reimplement this function in classes that don't
        use tessellation shaders.
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

    def add_vertex_shader_passthrough(self, shader, builtin=True):
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_inptr_type = shader.get_pointer_type(vec4_type, "Input")
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")

        shader.add_global("%pos_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%pos_out")
        if builtin:
            shader.add_annotation("OpDecorate %pos_out BuiltIn Position")
        else:
            shader.add_annotation("OpDecorate %pos_out Location 0")

        shader.add_global("%pos_in = OpVariable {} Input".format(
            vec4_inptr_type))
        shader.add_interface("%pos_in")
        shader.add_annotation("OpDecorate %pos_in Location 0")
        shader.add_source("%pos_in_v = OpLoad {} %pos_in".format(vec4_type))
        shader.add_source("OpStore %pos_out %pos_in_v")

    def add_fragment_shader_passthrough(self, shader):
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_inptr_type = shader.get_pointer_type(vec4_type, "Input")
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")
        shader.add_global("%color_in = OpVariable {} Input".format(
            vec4_inptr_type))
        shader.add_interface("%color_in")
        shader.add_annotation("OpDecorate %color_in Location 0")
        shader.add_global("%color_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color_out")
        shader.add_annotation("OpDecorate %color_out Location 0")
        shader.add_source("%result = OpLoad {} %color_in".format(
            vec4_type))
        shader.add_source("OpStore %color_out %result")

    def needs_probe_per_draw(self):
        """Returns whether the test needs the probe to be immediately after each

        draw call.
        """
        return self._api == 'vulkan'

    def make_test_shader(self, stage):
        """Generate the shader code necessary to test the built-in.
        additional_declarations is a string containing any
        declarations that need to be before the main() function of the
        shader.
        """
        shader = ShaderSource(stage, self._api)

        if self._signature.extension == 'ARB_gpu_shader_int64':
            shader.add_capability('Int64')

        if self._signature.extension in ["AMD_shader_trinary_minmax"]:
            spv_extension = "SPV_" + self._signature.extension
            shader.add_instruction_import('%ext = OpExtInstImport "{}"'.format(
                spv_extension))
            shader.add_extension(spv_extension)

        self.arg_locations = []

        for i, arg in enumerate(self._signature.argtypes):
            t = shader.get_glsl_type(arg)
            arg_loc = shader.add_uniform("%arg{}".format(i), arg)
            self.arg_locations.append(arg_loc)

        shader.add_source("%invocation = {}".format(
            self._signature.template_spirv.format(
                shader.get_glsl_type(self._signature.rettype),
                *['%arg{}'.format(i)
                  for i in range(len(self._signature.argtypes))])))

        self._comparator.add_result_handler(shader, "%invocation")

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
            for i in range(len(test_vector.arguments)):
                test += set_uniform(self._api,
                                    self._signature.argtypes[i],
                                    self.arg_locations[i],
                                    column_major_values(
                                        test_vector.arguments[i]))
            test += self._comparator.draw_test(test_vector,
                                               self.draw_command(test_num))
            if self.needs_probe_per_draw():
                result_color = self._comparator.result_vector(test_vector)
                test += self.probe_command(test_num, result_color)

        if not self.needs_probe_per_draw():
            for test_num, test_vector in enumerate(self._test_vectors):
                result_color = self._comparator.result_vector(test_vector)
                test += self.probe_command(test_num, result_color)
        return test

    def filename(self):
        argtype_names = '-'.join(
            str(argtype) for argtype in self._signature.argtypes)
        if self.extensions():
            subdir = self.extensions()[0].lower()
        else:
            subdir = 'glsl-{0:1.2f}'.format(float(self.glsl_version()) / 100)
        if self._api == 'gl':
            api_dir = ['spec', 'arb_gl_spirv']
            ext = 'shader_test'
        elif self._api == 'vulkan':
            api_dir = ['vulkan']
            ext = 'vk_shader_test'
        return os.path.join(
            *api_dir, subdir, 'execution', 'built-in-functions',
            '{0}-{1}-{2}{3}.{4}'.format(
                self.test_prefix(), self._signature.name, argtype_names,
                self._comparator.testname_suffix(), ext))

    def map_vulkan_extension(self, ext):
        vulkan_extensions = {
            'ARB_tessellation_shader': 'tessellationShader',
            'ARB_gpu_shader_int64': 'shaderInt64',
            'AMD_shader_trinary_minmax': 'VK_AMD_shader_trinary_minmax'
        }
        if ext in vulkan_extensions:
            return vulkan_extensions[ext]
        else:
            return None

    def generate_shader_test(self):
        """Generate the test and write it to the output file."""
        shader_test = '[require]\n'
        if self._api == 'gl':
            shader_test += ('SPIRV ONLY\n'
                            'GLSL >= 4.50\n')
            for extension in self.extensions():
                shader_test += 'GL_{}\n'.format(extension)
            shader_test += self.make_additional_requirements()
        elif self._api == 'vulkan':
            for extension in self.extensions():
                vulkan_ext = self.map_vulkan_extension(extension)
                if vulkan_ext:
                    shader_test += '{}\n'.format(vulkan_ext)
            extension = self.make_additional_requirements()
            vulkan_ext = self.map_vulkan_extension(extension)
            if vulkan_ext:
                shader_test += '{}\n'.format(vulkan_ext)
            shader_test += self.make_additional_vulkan_requirements()
        shader_test += '\n'
        vs = self.make_vertex_shader()
        if vs:
            shader_test += '[vertex shader spirv]\n'
            shader_test += vs.get_source()
            shader_test += '\n'
        tcs = self.make_tess_ctrl_shader()
        if tcs:
            shader_test += '[tessellation control shader spirv]\n'
            shader_test += tcs.get_source()
            shader_test += '\n'
        tes = self.make_tess_eval_shader()
        if tes:
            shader_test += '[tessellation evaluation shader spirv]\n'
            shader_test += tes.get_source()
            shader_test += '\n'
        gs = self.make_geometry_shader()
        if gs:
            shader_test += '[geometry shader spirv]\n'
            shader_test += gs.get_source()
            shader_test += '\n'
        gl = self.make_geometry_layout()
        if gl:
            shader_test += '[geometry layout]\n'
            shader_test += gl.get_source()
            shader_test += '\n'
        fs = self.make_fragment_shader()
        if fs:
            shader_test += '[fragment shader spirv]\n'
            shader_test += fs.get_source()
            shader_test += '\n'
        cs = self.make_compute_shader()
        if cs:
            shader_test += '[compute shader spirv]\n'
            shader_test += cs.get_source()
            shader_test += '\n'
        shader_test += '[test]\n'
        shader_test += 'clear color 0.0 0.0 1.0 0.0\n'
        shader_test += 'clear\n'
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
        shader = self.make_test_shader('Vertex')
        self.add_vertex_shader_passthrough(shader)
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")

        shader.add_global("%color_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color_out")
        shader.add_annotation("OpDecorate %color_out Location 0")
        shader.add_source("OpStore %color_out %result")

        return shader

    def make_fragment_shader(self):
        shader = ShaderSource('Fragment', self._api)
        self.add_fragment_shader_passthrough(shader)
        return shader


class TessellationShaderTest(ShaderTest):
    """Abstract class for tests that exercise the built-in in
    tessellation shaders.
    """
    def test_prefix(self):
        return 'tcs'

    def make_additional_requirements(self):
        return 'GL_ARB_tessellation_shader'

    def extensions(self):
        ext = []
        if self._signature.extension:
            ext.append(self._signature.extension)
        ext.append("ARB_tessellation_shader")
        return ext

    def draw_command(self, test_num):
        x = (test_num % self.tests_per_row) * self.rect_width
        y = (test_num // self.tests_per_row) * self.rect_height
        assert(y < self.test_rows)
        return 'draw rect ortho patch {0} {1} {2} {3}\n'.format(x, y,
                                                                self.rect_width,
                                                                self.rect_height)

    def make_vertex_shader(self):
        shader = ShaderSource('Vertex', self._api)
        self.add_vertex_shader_passthrough(shader, builtin=False)
        return shader

    def make_tess_ctrl_shader(self):
        shader = self.make_test_shader('TessellationControl')

        shader.add_execution_mode("OutputVertices 4")

        int_type = shader.get_int_type(32, False)
        int_inptr_type = shader.get_pointer_type(int_type, 'Input')
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_inptr_type = shader.get_pointer_type(vec4_type, "Input")
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")
        shader.add_global("%four = OpConstant {} 4".format(int_type))
        shader.add_global("%vec4_arr = OpTypeArray {} %four".format(vec4_type))
        vec4_arr_inptr_type = shader.get_pointer_type("%vec4_arr", "Input")
        vec4_arr_outptr_type = shader.get_pointer_type("%vec4_arr", "Output")
        shader.add_global("%two = OpConstant {} 2".format(int_type))
        shader.add_global("%float_arr2 = OpTypeArray {} %two".format(
            float_type))
        float_arr2_outptr_type = shader.get_pointer_type("%float_arr2",
                                                         "Output")
        shader.add_global("%float_arr4 = OpTypeArray {} %four".format(
            float_type))
        float_arr4_outptr_type = shader.get_pointer_type("%float_arr4",
                                                         "Output")

        shader.add_annotation("OpDecorate %invocation_id BuiltIn InvocationId")
        shader.add_interface("%invocation_id")
        shader.add_global("%invocation_id = OpVariable {} Input".format(
            int_inptr_type))
        shader.add_source("%invocation_id_v = OpLoad {} %invocation_id".format(
            int_type))

        shader.add_global("%pos_in_a = OpVariable {} Input".format(
            vec4_arr_inptr_type))
        shader.add_interface("%pos_in_a")
        shader.add_annotation("OpDecorate %pos_in_a Location 0")

        shader.add_global("%pos_out_a = OpVariable {} Output".format(
            vec4_arr_outptr_type))
        shader.add_interface("%pos_out_a")
        shader.add_annotation("OpDecorate %pos_out_a Location 0")

        shader.add_source("%pos_out = OpAccessChain {} "
                          "%pos_out_a %invocation_id_v".format(
                              vec4_outptr_type))
        shader.add_source("%pos_in = OpAccessChain {} "
                          "%pos_in_a %invocation_id_v".format(
                              vec4_inptr_type))
        shader.add_source("%pos_in_v = OpLoad {} %pos_in".format(vec4_type))
        shader.add_source("OpStore %pos_out %pos_in_v")

        shader.add_global("%outer = OpVariable {} Output".format(
            float_arr4_outptr_type))
        shader.add_interface("%outer")
        shader.add_annotation("OpDecorate %outer Patch")
        shader.add_annotation("OpDecorate %outer BuiltIn TessLevelOuter")
        shader.add_global("%outer_value = OpConstantComposite %float_arr4 "
                          "%one %one %one %one")
        shader.add_source("OpStore %outer %outer_value")
        shader.add_global("%inner = OpVariable {} Output".format(
            float_arr2_outptr_type))
        shader.add_interface("%inner")
        shader.add_annotation("OpDecorate %inner Patch")
        shader.add_annotation("OpDecorate %inner BuiltIn TessLevelInner")
        shader.add_global("%inner_value = OpConstantComposite %float_arr2 "
                          "%one %one")
        shader.add_source("OpStore %inner %inner_value")

        shader.add_global("%color_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color_out")
        shader.add_annotation("OpDecorate %color_out Location 1")
        shader.add_annotation("OpDecorate %color_out Patch")
        shader.add_source("OpStore %color_out %result")

        return shader

    def make_tess_eval_shader(self):
        shader = ShaderSource('TessellationEvaluation', self._api)

        shader.add_execution_mode("Quads")
        shader.add_execution_mode("SpacingEqual")
        shader.add_execution_mode("VertexOrderCcw")

        int_type = shader.get_int_type(32, False)
        float_type = shader.get_float_type(32)
        float_inptr_type = shader.get_pointer_type(float_type, "Input")
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_inptr_type = shader.get_pointer_type(vec4_type, "Input")
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")
        shader.add_global("%four = OpConstant {} 4".format(int_type))
        shader.add_global("%vec4_arr = OpTypeArray {} %four".format(vec4_type))
        vec4_arr_inptr_type = shader.get_pointer_type("%vec4_arr", "Input")
        vec3_type = shader.get_vec_type(float_type, 3)
        vec3_inptr_type = shader.get_pointer_type(vec3_type, "Input")

        shader.add_global("%pos_in_a = OpVariable {} Input".format(
            vec4_arr_inptr_type))
        shader.add_interface("%pos_in_a")
        shader.add_annotation("OpDecorate %pos_in_a Location 0")

        shader.add_global("%color_in = OpVariable {} Input".format(
            vec4_inptr_type))
        shader.add_interface("%color_in")
        shader.add_annotation("OpDecorate %color_in Location 1")
        shader.add_annotation("OpDecorate %color_in Patch")
        shader.add_source("%color_in_v = OpLoad {} %color_in".format(vec4_type))

        shader.add_global("%color_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color_out")
        shader.add_annotation("OpDecorate %color_out Location 0")
        shader.add_source("OpStore %color_out %color_in_v")

        shader.add_global("%tess_coord = OpVariable {} Input".format(
            vec3_inptr_type))
        shader.add_annotation("OpDecorate %tess_coord BuiltIn TessCoord")
        shader.add_interface("%tess_coord")

        for i in range(4):
            shader.add_global("%index{} = OpConstant {} {}".format(
                i, int_type, i))

        shader.add_source("%coord = OpLoad {} %tess_coord".format(vec3_type))
        shader.add_source("%coord_x_vec = OpVectorShuffle {} "
                          "%coord %coord 0 0 0 0".format(vec4_type))
        shader.add_source("%coord_y_vec = OpVectorShuffle {} "
                          "%coord %coord 1 1 1 1".format(vec4_type))

        for i in range(2):
            for j in range(2):
                shader.add_source("%pos{}_p = OpAccessChain {} "
                                  "%pos_in_a %index{}".format(
                                      i * 2 + j, vec4_inptr_type, i * 2 + j))
                shader.add_source("%pos{} = OpLoad {} %pos{}_p".format(
                    i * 2 + j, vec4_type, i * 2 + j))
            shader.add_source("%mix{} = OpExtInst {} %glsl450 "
                              "FMix %pos{} %pos{} %coord_x_vec".format(
                                  i, vec4_type, i * 2, i * 2 + 1))
        shader.add_source("%mix2 = OpExtInst {} %glsl450 "
                          "FMix %mix0 %mix1 %coord_y_vec".format(
                              vec4_type, i * 2, i * 2 + 1))

        shader.add_global("%pos_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%pos_out")
        shader.add_annotation("OpDecorate %pos_out BuiltIn Position")

        shader.add_source("OpStore %pos_out %mix2")

        return shader

    def make_fragment_shader(self):
        shader = ShaderSource('Fragment', self._api)
        self.add_fragment_shader_passthrough(shader)
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
        shader = ShaderSource('Vertex', self._api)
        self.add_vertex_shader_passthrough(shader, builtin=False)
        return shader

    def make_geometry_shader(self):
        shader = self.make_test_shader('Geometry')
        shader.add_execution_mode("Triangles")
        shader.add_execution_mode("Invocations 1")
        shader.add_execution_mode("OutputTriangleStrip")
        shader.add_execution_mode("OutputVertices 3")

        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_inptr_type = shader.get_pointer_type(vec4_type, "Input")
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")
        int_type = shader.get_int_type(32, False)
        shader.add_global("%three = OpConstant {} 3".format(int_type))
        shader.add_global("%vec4_arr_type = OpTypeArray {} %three".format(
            vec4_type))
        vec4_arr_inptr_type = shader.get_pointer_type("%vec4_arr_type", "Input")

        shader.add_global("%pos_in = OpVariable {} Input".format(
            vec4_arr_inptr_type))
        shader.add_interface("%pos_in")
        shader.add_annotation("OpDecorate %pos_in Location 0")

        shader.add_global("%pos_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%pos_out")
        shader.add_annotation("OpDecorate %pos_out BuiltIn Position")

        shader.add_global("%color = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color")
        shader.add_annotation("OpDecorate %color Location 0")

        for i in range(3):
            shader.add_global("%index{} = OpConstant {} {}".format(
                i, int_type, i))
            shader.add_source("%pos_in_ac{} = OpAccessChain "
                              "{} %pos_in %index{}".format(
                                  i, vec4_inptr_type, i))
            shader.add_source("%pos_in{} = OpLoad {} %pos_in_ac{}".format(
                i, vec4_type, i))
            shader.add_source("OpStore %pos_out %pos_in{}".format(i))
            shader.add_source("OpStore %color %result")
            shader.add_source("OpEmitVertex")

        return shader

    def make_fragment_shader(self):
        shader = ShaderSource('Fragment', self._api)
        self.add_fragment_shader_passthrough(shader)
        return shader

    def make_additional_vulkan_requirements(self):
        return 'geometryShader'

class FragmentShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a fragment
    shader.
    """
    def test_prefix(self):
        return 'fs'

    def make_vertex_shader(self):
        shader = ShaderSource('Vertex', self._api)
        self.add_vertex_shader_passthrough(shader)
        return shader

    def make_fragment_shader(self):
        shader = self.make_test_shader('Fragment')
        float_type = shader.get_float_type(32)
        vec4_type = shader.get_vec_type(float_type, 4)
        vec4_outptr_type = shader.get_pointer_type(vec4_type, "Output")
        shader.add_global("%color_out = OpVariable {} Output".format(
            vec4_outptr_type))
        shader.add_interface("%color_out")
        shader.add_annotation("OpDecorate %color_out Location 0")
        shader.add_source("OpStore %color_out %result")

        return shader


class ComputeShaderTest(ShaderTest):
    """Derived class for tests that exercise the built-in in a
    compute shader.
    """
    def test_prefix(self):
        return 'cs'

    def glsl_version(self):
        return max(430, ShaderTest.glsl_version(self))

    def make_compute_shader(self):
        shader = self.make_test_shader('GLCompute')
        shader.add_capability('StorageImageWriteWithoutFormat')

        float_type = shader.get_float_type(32)
        uint_type = shader.get_int_type(32, False)
        uvec2_type = shader.get_vec_type(uint_type, 2)
        uvec3_type = shader.get_vec_type(uint_type, 3)
        uvec3_inptr_type = shader.get_pointer_type(uvec3_type, 'Input')
        int_type = shader.get_int_type(32, True)
        ivec2_type = shader.get_vec_type(int_type, 2)

        num_tests = len(self._test_vectors)
        shader.add_execution_mode("LocalSize {} 1 1".format(num_tests))
        shader.add_global("%image_type = OpTypeImage {} "
                          "2D 0 0 0 2 Unknown".format(float_type))
        shader.add_global("%image_ptr_type = OpTypePointer "
                          "UniformConstant %image_type")
        shader.add_global("%tex = OpVariable %image_ptr_type UniformConstant")
        shader.add_annotation("OpDecorate %tex Location 6")
        shader.add_annotation("OpDecorate %tex Binding 0")
        shader.add_annotation("OpDecorate %tex NonReadable")

        shader.add_global("%num_tests = OpConstant {} {}".format(
            uint_type, num_tests))
        shader.add_global("%ione = OpConstant {} 1".format(uint_type))
        shader.add_global("%workgroup_size = OpConstantComposite {} "
                          "%num_tests %ione %ione".format(uvec3_type))
        shader.add_annotation("OpDecorate %workgroup_size "
                              "BuiltIn WorkgroupSize")

        shader.add_annotation("OpDecorate %invocation_id BuiltIn "
                              "GlobalInvocationId")
        shader.add_interface("%invocation_id")
        shader.add_global("%invocation_id = OpVariable {} Input".format(
            uvec3_inptr_type))
        shader.add_source("%invocation_id_v = OpLoad {} %invocation_id".format(
            uvec3_type))
        shader.add_source("%ucoord = OpVectorShuffle {} "
                          "%invocation_id_v %invocation_id_v 0 1".format(
                              uvec2_type))
        shader.add_source("%coord = OpBitcast {} %ucoord".format(ivec2_type))

        shader.add_source("%tex_v = OpLoad %image_type %tex")
        shader.add_source("OpImageWrite %tex_v %coord %result")

        return shader

    def make_test_init(self):
        return '''texture rgbw 0 ({0}, 1) GL_RGBA8
image texture 0 GL_RGBA8
fb tex 2d 0
clear
'''.format(len(self._test_vectors))

    def draw_command(self, test_num):
        return 'compute 1 1 1\n'

    def probe_command(self, test_num, probe_vector):
        # Note: shader_runner uses a 250x250 window so we must
        # ensure that test_num <= 250.
        return 'probe rgb {0} 0 {1} {2} {3} {4}\n'.format(test_num % 250,
                                                          probe_vector[0],
                                                          probe_vector[1],
                                                          probe_vector[2],
                                                          probe_vector[3])
    def needs_probe_per_draw(self):
        return True


def all_tests():
    for api in ('gl', 'vulkan'):
        for signature, test_vectors in sorted(test_suite.items()):
            if signature.template_spirv is None:
                continue
            yield VertexShaderTest(signature, test_vectors, api)
            yield TessellationShaderTest(signature, test_vectors, api)
            yield GeometryShaderTest(signature, test_vectors, api)
            yield FragmentShaderTest(signature, test_vectors, api)
            #yield ComputeShaderTest(signature, test_vectors, api)


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
