#!/usr/bin/env python3
#
# Copyright 2015 Advanced Micro Devices, Inc.
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
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import argparse
import glob
import os
import re
import stat
import subprocess
import sys
import tempfile
import itertools

num_mark_skipped = 0

RE_spirv_shader_groupname = re.compile(r'(.*) shader spirv$')
RE_glsl_shader_groupname = re.compile(r'(.*) shader$')
RE_if_start = re.compile(r'\s*#\s*if\s+([0-9]+)\s*$')
RE_else = re.compile(r'\s*#\s*else\s*$')
RE_endif = re.compile(r'\s*#\s*endif\s*$')

def make_spirv_variable_re(storage_mode):
    return re.compile(r'^\s*%(\w+)\s*=\s*OpVariable\s+%(\w+)\s+' +
                      re.escape(storage_mode) +
                      r'(?: +%\w+)?\s*$',
                      re.MULTILINE)

RE_spirv_output_var = make_spirv_variable_re('Output')
RE_spirv_input_var = make_spirv_variable_re('Input')
RE_spirv_uniform_var = make_spirv_variable_re('UniformConstant')
RE_spirv_ubo_var = make_spirv_variable_re('Uniform')
RE_spirv_location = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpDecorate\s+%(\w+)\s+' +
                               r'Location\s+(\d+)\s*$',
                               re.MULTILINE)
RE_spirv_binding = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpDecorate\s+%(\w+)\s+' +
                              r'Binding\s+(\d+)\s*$',
                              re.MULTILINE)
RE_spirv_array_stride = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpDecorate\s+%(\w+)\s+' +
                                   r'ArrayStride\s+(\d+)\s*$',
                                   re.MULTILINE)
RE_spirv_name = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpName\s+%(\w+)\s+' +
                           r'"([^"]+)"\s*$',
                           re.MULTILINE)
RE_spirv_member_name = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpMemberName\s+' +
                                  r'%(\w+)\s+(\d+)\s+"([^"]+)"\s*$',
                                  re.MULTILINE)
RE_spirv_member_offset = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpMemberDecorate\s+' +
                                    r'%(\w+)\s+(\d+)\s+Offset\s+(\d+)\s*$',
                                    re.MULTILINE)
RE_spirv_matrix_stride = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpMemberDecorate\s+'
                                    r'%(\w+)\s+(\d+)\s+MatrixStride\s+'
                                    r'(\d+)\s*$',
                                    re.MULTILINE)
RE_spirv_matrix_order = re.compile(r'^\s*(?:%\w+\s*=\s*)?OpMemberDecorate\s+'
                                   r'%(\w+)\s+(\d+)\s+(Row|Col)Major\s*$',
                                    re.MULTILINE)
RE_spirv_type = re.compile(r'^\s*%(\w+)\s*=\s*OpType(\w+) *(.*?) *$',
                           re.MULTILINE)
RE_spirv_constant = re.compile(r'^\s*%(\w+)\s*=\s*OpConstant\s+' +
                               r'%\w+\s+(.*?)\s*$',
                               re.MULTILINE)

passthrough_spirv = '''
               OpName %piglit_vertex "piglit_vertex"
               OpDecorate %piglit_vertex Location 0
%piglit_vertex = OpVariable %_ptr_Input_v3float Input
%_ptr_Input_v3float = OpTypePointer Input %v3float
    %v3float = OpTypeVector %float 3
      %float = OpTypeFloat 32
'''

def get_stage_extension(stage):
    d = {
        'vertex': 'vert',
        'fragment': 'frag',
        'tessellation control': 'tesc',
        'tessellation evaluation': 'tese',
        'geometry': 'geom',
        'compute': 'comp',
    }
    return d.get(stage)


def get_stage_order(stage):
    d = [
        'vertex',
        'tessellation control',
        'tessellation evaluation',
        'geometry',
        'fragment',
        'compute',
    ]
    return d.index(stage)


class ShaderSource(object):
    def __init__(self, stage):
        self.stage = stage
        self.__source_chunks = []
        self.__source_cache = None

    def append(self, source):
        self.__source_chunks.append(source)
        self.__source_cache = None

    def source(self):
        if self.__source_cache is None:
            self.__source_cache = ''.join(self.__source_chunks)
            self.__source_chunks = [self.__source_cache]
        return self.__source_cache

    def set_source(self, string):
        self.__source_chunks = [string]
        self.__source_cache = string

class SpirvType:
    def uniform_size(self):
        if self.base_type == 'Array':
            return self.element_type.uniform_size() * self.length
        elif self.base_type == 'Struct':
            return sum(t.uniform_size() for t in self.member_types)
        else:
            return 1

    def attrib_size(self):
        if self.base_type == 'Array' or self.base_type == 'Matrix':
            return self.element_type.attrib_size() * self.length
        elif self.base_type == 'Struct':
            return sum(t.attrib_size() for t in self.member_types)
        else:
            return 1

    def deref(self):
        t = self
        while t.base_type == 'Pointer':
            t = t.deref_type
        return t

class SpirvVariable:
    pass

class SpirvInfo:
    def __init__(self, spirv):
        self.names = {md.group(1): md.group(2)
                      for md in RE_spirv_name.finditer(spirv)}
        self.locations = {md.group(1): int(md.group(2))
                          for md in RE_spirv_location.finditer(spirv)}
        self.bindings = {md.group(1): int(md.group(2))
                         for md in RE_spirv_binding.finditer(spirv)}
        self.array_strides = {md.group(1): int(md.group(2))
                              for md in RE_spirv_array_stride.finditer(spirv)}
        self.type_definitions = {md.group(1): [md.group(2)] +
                                 md.group(3).split()
                                 for md in RE_spirv_type.finditer(spirv)}
        self.constants = {md.group(1): md.group(2)
                          for md in RE_spirv_constant.finditer(spirv)}

        self.types = {}

        def get_member_decorations(regexp, func):
            decorations = {}
            for md in regexp.finditer(spirv):
                if md.group(1) not in decorations:
                    decorations[md.group(1)] = []
                members = decorations[md.group(1)]
                index = int(md.group(2))
                if len(members) <= index:
                    padding = itertools.repeat(None, index - len(members) + 1)
                    members.extend(padding)
                members[index] = func(md.group(3))
            return decorations

        self.member_names = get_member_decorations(RE_spirv_member_name,
                                                   lambda x: x)
        self.member_offsets = get_member_decorations(RE_spirv_member_offset,
                                                     int)
        self.matrix_strides = get_member_decorations(RE_spirv_matrix_stride,
                                                     int)
        self.matrix_orders = get_member_decorations(RE_spirv_matrix_order,
                                                    str.lower)

        self.inputs = self._get_variables(RE_spirv_input_var.finditer(spirv))
        self.outputs = self._get_variables(RE_spirv_output_var.finditer(spirv))
        self.uniforms = self._get_variables(
            RE_spirv_uniform_var.finditer(spirv))
        self.ubos = self._get_variables_list(RE_spirv_ubo_var.finditer(spirv))

    def _get_type(self, name):
        if name in self.types:
            return self.types[name]

        dec = self.type_definitions[name]
        t = SpirvType()
        t.base_type = dec[0]
        t.parts = dec[1:]

        if name in self.names:
            t.name = self.names[name]
        else:
            t.name = None

        if name in self.array_strides:
            t.array_stride = self.array_strides[name]
        else:
            t.array_stride = None

        if t.base_type == 'Array':
            t.element_type = self._get_type(dec[1][1:])
            t.length = int(self.constants[dec[2][1:]])
        elif (t.base_type == 'Vector' or
              t.base_type == 'Matrix'):
            t.element_type = self._get_type(dec[1][1:])
            t.length = int(dec[2])
        elif t.base_type == 'RuntimeArray':
            t.element_type = self._get_type(dec[1][1:])
        elif t.base_type == 'Struct':
            t.member_types = [ self._get_type(mt[1:]) for mt in dec[1:] ]
            t.member_names = self.member_names[name]
            if name in self.member_offsets:
                t.member_offsets = self.member_offsets[name]
            else:
                t.member_offsets = None
            if name in self.matrix_strides:
                t.matrix_strides = self.matrix_strides[name]
            else:
                t.matrix_strides = None
            if name in self.matrix_strides:
                t.matrix_orders = self.matrix_orders[name]
            else:
                t.matrix_orders = None
        elif t.base_type == 'Int':
            t.width = int(dec[1])
            t.signedness = int(dec[2])
        elif t.base_type == 'Float':
            t.width = int(dec[1])
        elif t.base_type == 'Pointer':
            t.storage_class = dec[1]
            t.deref_type = self._get_type(dec[2][1:])

        self.types[name] = t
        return t

    def _get_variable(self, name, typename):
        v = SpirvVariable()
        v.type = self._get_type(typename)
        if name in self.locations:
            v.location = self.locations[name]
        else:
            v.location = None
        if name in self.bindings:
            v.binding = self.bindings[name]
        else:
            v.binding = None
        if name in self.names:
            v.name = self.names[name]
        else:
            v.name = None
        v.result_id = name
        return v

    def _get_variables(self, re_iter):
        return { var.name: var
                 for var in self._get_variables_list(re_iter)
                 if var.name is not None }

    def _get_variables_list(self, re_iter):
        return [ self._get_variable(md.group(1), md.group(2))
                 for md in re_iter ]

class GLSLSource(object):
    def __init__(self, source):
        self.__tokens = self.tokenize(source)

    def source(self):
        return ''.join(self.__tokens)

    def prepend(self, string):
        tokens = self.tokenize(string)
        if len(tokens) % 2 != 0 and self.__tokens:
            self.__tokens[0] = tokens.pop() + self.__tokens[0]
        self.__tokens = tokens + self.__tokens

    def insert_after_versions(self, string):
        idx = 1
        while idx < len(self.__tokens):
            if (not self.__tokens[idx].startswith('#version') and
                not self.__tokens[idx].startswith('#extension')):
                break
            idx += 2
        self.__transform([(idx, idx, string)])

    def transform_tokens(self, fn):
        """
        Each token is passed to fn. If fn returns None, the token is preserved
        unchanged. Otherwise, fn must return a string that is tokenized and
        replaces the original token.
        """
        transformations = []

        for non_whitespace_idx, token in enumerate(self.__tokens[1::2]):
            transformed = fn(token)
            if transformed is not None:
                idx = 2 * non_whitespace_idx + 1
                transformations.append((idx, idx + 1, transformed))

        self.__transform(transformations)

    def __transform(self, transformations):
        src_tokens = self.__tokens
        dst_tokens = []

        def paste(tokens, first_is_whitespace):
            if not tokens:
                return

            if first_is_whitespace and len(dst_tokens) % 2 != 0:
                dst_tokens[-1] += tokens[0]
                dst_tokens.extend(tokens[1:])
                return

            if not first_is_whitespace and len(dst_tokens) % 2 == 0:
                dst_tokens.append('')
            dst_tokens.extend(tokens)

        idx = 0
        for start, end, replacement in transformations:
            paste(src_tokens[idx:start], idx % 2 == 0)

            tokens = self.tokenize(replacement)
            paste(tokens, True)

            idx = end

        paste(src_tokens[idx:], idx % 2 == 0)

        self.__tokens = dst_tokens

    RE_whitespace = re.compile(r'( |\t|\n|//[^\n]*\n|/\*(\*+[^/]|[^*])*\*/)*')
    RE_identifier = re.compile(r'[_a-zA-Z][_a-zA-Z0-9]*')
    RE_numeric_constant = re.compile(r'[+-]?(((\d+(\.\d*)?)(?!x)|\.\d+)|' +
                                     r'(0(x[0-9a-fA-F]+)?|[1-9][0-9]*(.\d+)?))')
    RE_end_of_preprocessor = re.compile(r'[^\\]\n')

    def tokenize(self, source):
        tokens = []

        idx = 0
        while idx < len(source):
            m = self.RE_whitespace.match(source, pos=idx)
            tokens.append(m.group(0))
            idx = m.end()
            if idx >= len(source):
                break

            # Preprocessor lines are kept intact as a whole
            if (source.startswith('#', idx) and
                (idx == 0 or source[idx - 1] == '\n')):
                m = self.RE_end_of_preprocessor.search(source, idx)
                end = m.end()
                tokens.append(source[idx:end])
                idx = end
                continue

            # Identifiers
            m = self.RE_identifier.match(source, pos=idx)
            if m is None:
                m = self.RE_numeric_constant.match(source, pos=idx)
            if m is not None:
                tokens.append(m.group(0))
                idx = m.end()
                continue

            tokens.append(source[idx:idx + 1])
            idx += 1

        return tokens

class CompatReplacement(object):
    def __init__(self, name, declaration):
        self.name = name
        self.declaration = declaration

def fixup_glsl_shaders(shaders, skip_reasons):
    """
    There are many reasons why a set of GLSL shaders may not be suitable for
    compilation to SPIR-V. This function tries to fix many of them, and
    recognizes many that cannot be fixed automatically.

    Fixed automatically:
    - Force compilation with #version 450 (in many cases this is required due
      to glslangValidator defects)
    - Change varying and attribute keywords to in/out
    - Change gl_Vertex and gl_FragColor to generic in/outs
    - Change old-style texture built-ins to core profile built-ins
    - Automatically assign in/out locations
    - Automatically assign uniform locations

    Bail out:
    - Use of compatibility profile-only state uniforms and ftransform
    - Use of gl_TexCoord, gl_Color, gl_FragData and friends
    - Use of old-style shadow sampling built-ins (their behavior depends on
      GL_DEPTH_TEXTURE_MODE)
    """
    RE_compat_texture = re.compile(r'texture([123]D(Array)?|Cube)(Proj)?(Lod)?$')
    RE_compat_shadow = re.compile(r'shadow[12]D(Rect)?(Array)?(Proj)?(Lod)?(Grad)?(ARB)?$')
    RE_extension = re.compile(r'#extension +([A-Za-z0-9_]+) *:')
    compat_replacements = {
        'gl_Vertex': CompatReplacement(
            name='piglit_vertex',
            declaration='in vec4 piglit_vertex;',
        ),
        'gl_FragColor': CompatReplacement(
            name='compat_fragcolor',
            declaration='out vec4 compat_fragcolor;',
        ),
    }
    compat_unsupported = set([
        'ftransform'
    ])
    good_builtins = set([
        # Section 7.1 of the GLSL 4.50 spec
        'gl_NumWorkGroups', 'gl_WorkGroupSize', 'gl_WorkGroupID',
        'gl_LocalInvocationID', 'gl_GlobalInvocationID', 'gl_LocalInvocationIndex',

        'gl_PerVertex', 'gl_VertexID', 'gl_InstanceID', 'gl_Position',
        'gl_PointSize', 'gl_ClipDistance', 'gl_CullDistance',

        'gl_in', 'gl_PrimitiveIDIn', 'gl_InvocationID', 'gl_PrimitiveID',
        'gl_Layer', 'gl_ViewportIndex',

        'gl_MaxPatchVertices', 'gl_PatchVerticesIn', 'gl_TessLevelOuter',
        'gl_TessLevelInner', 'gl_TessCoord', 'gl_out',

        'gl_FragCoord', 'gl_FrontFacing', 'gl_PointCoord', 'gl_SampleID',
        'gl_SamplePosition', 'gl_SampleMaskIn', 'gl_HelperInvocation',
        'gl_FragDepth', 'gl_SampleMask',

        # From Section 7.3 of the GLSL 4.50 spec (incomplete)
        'gl_MaxClipDistances', 'gl_MaxCullDistances',
        'gl_MaxCombinedClipAndCullDistances',
        'gl_MaxGeometryInputComponents',
        'gl_MaxVertexOutputComponents'

        # ARB_shader_ballot
        'gl_SubGroupInvocationARB', 'gl_SubGroupSizeARB',
        'gl_SubGroupLtMaskARB', 'gl_SubGroupGtMaskARB', 'gl_SubGroupEqMaskARB',
        'gl_SubGroupLeMaskARB', 'gl_SubGroupGeMaskARB',
    ])

    # glslang does not support these extensions, but does support the
    # corresponding core functionality.
    glslang_workaround_good_extensions = set([
        'GL_ARB_arrays_of_arrays',
        'GL_ARB_cull_distance',
        'GL_ARB_explicit_uniform_location',
        'GL_ARB_fragment_layer_viewport',
        'GL_ARB_gpu_shader_fp64',
        'GL_ARB_shader_bit_encoding',
        'GL_ARB_shader_precision',
        'GL_ARB_shader_storage_buffer_object',
        'GL_ARB_shading_language_packing',
        'GL_ARB_texture_query_levels',
        'GL_ARB_uniform_buffer_object',
        'GL_ARB_vertex_attrib_64bit',
        'GL_AMD_conservative_depth',
    ])

    def first_transform_tokens(token):
        if token.startswith('#version'):
            return ''

        if token.startswith('#extension'):
            m = RE_extension.match(token)
            if m.group(1) in glslang_workaround_good_extensions:
                return ''
            return None

        if token == 'varying':
            if shader.stage == 'vertex':
                return 'out'
            elif shader.stage == 'fragment':
                return 'in'

        if token == 'attribute' and shader.stage == 'vertex':
            return 'in'

        if token in compat_replacements:
            have_compat.add(token)
            return compat_replacements[token].name

        m = RE_compat_texture.match(token)
        if m is not None:
            return 'texture{}{}'.format(m.group(3) or '', m.group(4) or '')

        m = RE_compat_shadow.match(token)
        if m is not None:
            skip_reasons.add('shadow sampling built-in')
            return None

        return None

    def scan_builtin(token):
        if token.startswith('gl_') and not token in good_builtins:
            skip_reasons.add(token)
        elif token in compat_unsupported:
            skip_reasons.add(token)

    shaders.sort(key=lambda shader: get_stage_order(shader.stage))

    for shader in shaders:
        glsl = GLSLSource(shader.source())

        have_compat = set()
        glsl.transform_tokens(first_transform_tokens)

        glsl.prepend('#version 450\n')

        for compat in have_compat:
            glsl.insert_after_versions(compat_replacements[compat].declaration + '\n')

        glsl.transform_tokens(scan_builtin)

        shader.set_source(glsl.source())

    return skip_reasons


def update_spirv_line(shader_test, skip_reasons):
    with open(shader_test, 'r') as filp:
        lines = filp.readlines()

    require_idx = None
    spirv_idx = None
    for idx, line in enumerate(lines):
        if line.startswith('['):
            if line.startswith('[require]'):
                require_idx = idx
                in_require = True
            elif require_idx is not None:
                break
        elif require_idx is not None and line.startswith('SPIRV'):
            spirv_idx = idx
            break

    if skip_reasons:
        spirv_line = 'SPIRV NO ({})\n'.format(', '.join(skip_reasons))
    else:
        spirv_line = 'SPIRV YES\n'

    if spirv_idx is None:
        if skip_reasons:
            lines.insert(require_idx + 1, spirv_line)
        else:
            return
    else:
        words = spirv_line.strip().split()
        if (len(words) >= 2 and (words[1] == 'ONLY' or (words[1] == 'YES' and not skip_reasons))):
            return

        lines[spirv_idx] = spirv_line

    with open(shader_test, 'w') as filp:
        for line in lines:
            print(line, file=filp, end='')

def supports_uniform_location_override(config):
    proc = subprocess.run([config.glslang, '-h'], stdout=subprocess.PIPE)
    md = re.search(r'^ *-u<name>:<loc>', proc.stdout.decode(), re.MULTILINE)
    return md is not None

def write_glslang_conf(config, filename):
    """
    Override some glslang default settings and write the resulting .conf file
    """
    proc = subprocess.run([config.glslang, '-c'], stdout=subprocess.PIPE)
    settings = dict(
        line.strip().split()
        for line in proc.stdout.decode().split('\n')
        if line.strip()
    )

    settings['MaxAtomicCounterBindings'] = 8

    with open(filename, 'w') as filp:
        for key, value in settings.items():
            print('{} {}'.format(key, value), file=filp)


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--recheck",
                        action="store_true",
                        help="Force a re-check of tests marked SPIRV NO (except SPIRV NO OTHER)")
    parser.add_argument("--mark-skip",
                        action="store_true",
                        help="Update shader_test files in place with required SPIRV NO lines")
    parser.add_argument("-k", "--keep-going",
                        action="store_true",
                        help="Keep going after errors")
    parser.add_argument("-X", "--excludes-from-file",
                        nargs='+',
                        help="Exclude shader_test files with path prefixes from the given file")
    parser.add_argument("-o", "--error-list-file",
                        nargs='+',
                        help="Prints the list of tests that failed to be converted on the given file")
    parser.add_argument("--skip-list-file",
                        nargs='+',
                        help="Prints the list of tests that were skipped or excluded to be converted on the given file")
    parser.add_argument("-n", "--no-transform",
                        action="store_true",
                        help="Don't try to transform GLSL shaders")
    parser.add_argument("-t", "--transformed",
                        action="store_true",
                        help="Print transformed GLSL shaders")
    parser.add_argument("-r", "--replace",
                        action="store_true",
                        help="Replace the original script with the transformed one")
    parser.add_argument("-j", "--jobs",
                        nargs=1,
                        help="Fork the given number of processes")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        help="Print verbose output")
    parser.add_argument("-m", "--mirror",
                        nargs=1,
                        help="Store the SPIR-V generated in a mirror directory, without .spv")
    parser.add_argument("--strip-names",
                        action="store_true",
                        help="Remove all the debug names from the generated SPIR-V")
    parser.add_argument("--no-uniform-remap",
                        action="store_true",
                        help="Don't do a uniform/ubo remap")
    parser.add_argument("shader_tests",
                        nargs='+',
                        help="Path to one or more .shader_test files to process.")
    return parser.parse_args()

def compile_glsl(shader_test, config, shader_group, uniform_map, max_uniform):
    stage = shader_group[0].stage
    extension = get_stage_extension(stage)
    if extension is None:
        print('{}: Bad shader stage: "{}"'.format(shader_test, stage))
        return None

    with tempfile.TemporaryDirectory() as tempdir:
        filenames = []

        for idx, shader in enumerate(shader_group):
            filename = '{}/{}.{}'.format(tempdir, idx, extension)
            filenames.append(filename)
            with open(filename, 'w') as srcf:
                print(shader.source(), file=srcf)

        filename = '{}/glslang.conf'.format(tempdir)
        write_glslang_conf(config, filename)
        filenames.append(filename)

        binary_name = tempdir + '/shader.' + extension + '.spv'

        if config.verbose or config.transformed:
            print('Writing {} shader binary to {}'.
                  format(shader_group[0].stage, binary_name))

        cmdline = [config.glslang, '--aml', '--amb', '-G', '-o', binary_name]

        if config.supports_uniform_location_override:
            cmdline.extend(['--uniform-base', str(max_uniform)])
            for var in uniform_map.values():
                if var.location is not None:
                    cmdline.append("-u{}:{}".format(var.name, var.location))

        cmdline.extend(filenames)
        proc = subprocess.run(cmdline, stdout=subprocess.PIPE)

        if proc.returncode != 0:
            print('{}: Failed to build {} shader'.format(shader_test, stage))
            print('Build command: {}\nOutput:\n{}'.format(
                ' '.join(cmdline),
                proc.stdout.decode(errors='ignore')))
            for idx, shader in enumerate(shader_group):
                print('Transformed GLSL #{}:\n{}'.format(idx, shader.source()))
            return None

        cmdline = [config.spirv_dis, binary_name]
        proc = subprocess.run(cmdline, stdout=subprocess.PIPE)

        if proc.returncode != 0:
            print('{}: spirv-dis failed for {} shader'.format(shader_test, stage))
            return None

        return proc.stdout.decode()

def process_accessors(spirv_type, accessors, size_func):
    offset = 0

    while len(accessors) > 0:
        spirv_type = spirv_type.deref()

        md = re.match(r'\[([0-9]+)\]', accessors)
        if md:
            if (spirv_type.base_type != 'Array' and
                spirv_type.base_type != 'RuntimeArray'):
                return None
            spirv_type = spirv_type.element_type
            offset += int(md.group(1)) * size_func(spirv_type)
            accessors = accessors[len(md.group(0)):]
            continue

        md = re.match(r'\.([a-zA-Z_][a-zA-Z_0-9]*)', accessors)
        if md:
            if spirv_type.base_type != 'Struct':
                return None
            try:
                index = spirv_type.member_names.index(md.group(1))
            except ValueError:
                return None
            for i in range(index):
                offset += size_func(spirv_type.member_types[i])
            spirv_type = spirv_type.member_types[index]
            accessors = accessors[len(md.group(0)):]
            continue

        return None

    return offset

def remap_variable(name, location_map, size_func):
    md = re.match(r'(.+?)([\[\.]|$)', name)
    if md.group(1) not in location_map:
        return None
    var = location_map[md.group(1)]
    if var.location is None:
        return None
    offset = process_accessors(var.type,
                               name[len(md.group(1)):],
                               size_func)
    if offset is None:
        return None
    else:
        return str(var.location + offset)

def process_ubo_accessors(spirv_type, accessors):
    offset = 0
    matrix_stride = None
    matrix_order = None

    while len(accessors) > 0:
        spirv_type = spirv_type.deref()

        md = re.match(r'\[([0-9]+)\]', accessors)
        if md:
            if ((spirv_type.base_type != 'Array' and
                 spirv_type.base_type != 'RuntimeArray') or
                spirv_type.array_stride is None):
                return None

            offset += int(md.group(1)) * spirv_type.array_stride
            spirv_type = spirv_type.element_type
            accessors = accessors[len(md.group(0)):]
            continue

        md = re.match(r'\.([a-zA-Z_][a-zA-Z_0-9]*)', accessors)
        if md:
            if spirv_type.base_type != 'Struct':
                return None
            try:
                index = spirv_type.member_names.index(md.group(1))
            except ValueError:
                return None
            if (spirv_type.matrix_strides is not None and
                index < len(spirv_type.matrix_strides)):
                matrix_stride = spirv_type.matrix_strides[index]
            if (spirv_type.matrix_orders is not None and
                index < len(spirv_type.matrix_orders)):
                matrix_order = spirv_type.matrix_orders[index]
            offset += spirv_type.member_offsets[index]
            spirv_type = spirv_type.member_types[index]
            accessors = accessors[len(md.group(0)):]
            continue

        return None

    return (offset, matrix_stride, matrix_order)

def remap_ubo_member_in_ubo(name, ubo, ubo_type=None):
    if ubo.binding is None:
        return None

    if ubo_type is None:
        ubo_type = ubo.type

    replacement = process_ubo_accessors(ubo_type, "." + name)
    if replacement is None:
        return None

    return (ubo.binding, *replacement)

def remap_ubo_member(name, ubos):
    for ubo in ubos:
        if ubo.name is not None:
            continue
        replacement = remap_ubo_member_in_ubo(name, ubo)
        if replacement:
            return replacement

    md = re.match(r'([^\.]+)\.(.*)', name)
    if md:
        for ubo in ubos:
            ubo_type = ubo.type.deref()
            # The scripts specify UBO arrays by just the name of the
            # block as if it wasn’t an array and then use the
            # out-of-band “ubo array index” command to specify the
            # index
            while ubo_type.base_type == 'Array':
                ubo_type = ubo_type.element_type
            if ubo.name == md.group(1) or ubo_type.name == md.group(1):
                return remap_ubo_member_in_ubo(md.group(2), ubo, ubo_type)

    return None

def remap_uniform(name, uniform_map):
    return remap_variable(name, uniform_map, SpirvType.uniform_size)

def remap_attribute(name, attrib_map):
    return remap_variable(name, attrib_map, SpirvType.attrib_size)

def get_ubo_binding(ubos, name):
    for ubo in ubos:
        t = ubo.type.deref()
        if t.name == name:
            return ubo.binding
    return None

def filter_shader_test(config,
                       fin, fout,
                       replacements,
                       uniform_map,
                       ubos,
                       attrib_map,
                       add_spirv_line,
                       extra_sections):
    skipping = False
    groupname = None
    in_test = False
    in_vertex_data = False
    block_binding = -1
    current_matrix_stride = None
    current_matrix_order = None
    uniform_re = re.compile(r'(\s*uniform\s+\S+\s+)(\S+)(.*)')
    attrib_re = re.compile(r'\b([\[\]a-zA-Z0-9_]+)((?:/[\[\]a-zA-Z_0-9]+){2})')
    block_binding_re = re.compile(r'\s*block\s+binding\s+([0-9]+)')
    program_interface_query_re = re.compile(r'\s*verify\s+program_interface_'
                                            r'query\s+\S+\s+(\S+)')

    def vertex_data_replacement(md):
        replacement = remap_attribute(md.group(1), attrib_map)
        if replacement is None:
            return md.group(0)
        else:
            return replacement + md.group(2)

    for line in fin:
        if line.startswith('['):
            groupname = line[1:line.index(']')]
            skipping = RE_spirv_shader_groupname.match(groupname) is not None
            in_test = groupname == 'test'
            in_vertex_data = groupname == 'vertex data'
            if groupname in replacements:
                replacement = replacements[groupname]
                if len(replacement) > 0:
                    fout.write('[' + groupname + ' spirv]\n')
                    fout.write('; Automatically generated from the GLSL by '
                               'shader_test_spirv.py. DO NOT EDIT\n')
                    if config.strip_names:
                        for spirv_line in replacement.splitlines():
                            if RE_spirv_name.match(spirv_line) is None and RE_spirv_member_name.match(spirv_line) is None:
                                fout.write(spirv_line + '\n')
                    else:
                        fout.write(replacement)
                    fout.write('\n')
                    replacements[groupname] = ''

            if in_test and extra_sections:
                fout.write(extra_sections)
                extra_sections = None

        if in_test:
            md = uniform_re.match(line)
            if md and not config.no_uniform_remap:
                replacement = remap_ubo_member(md.group(2), ubos)
                if replacement:
                    (binding, offset, matrix_stride, matrix_order) = replacement
                    line = "block offset {}\n{}".format(offset, line)
                    if binding != block_binding:
                        block_binding = binding
                        line = "block binding {}\n{}".format(binding, line)
                    if (matrix_stride is not None and
                        matrix_stride != current_matrix_stride):
                        current_matrix_stride = matrix_stride
                        line = "block matrix stride {}\n{}".format(
                            matrix_stride, line)
                    if (matrix_order is not None and
                        matrix_order != current_matrix_order):
                        current_matrix_order = matrix_order
                        line = "block row major {}\n{}".format(
                            1 if matrix_order == 'row' else 0, line)
                else:
                    replacement = remap_uniform(md.group(2), uniform_map)
                    if replacement:
                        line = (md.group(1) +
                                replacement +
                                md.group(3) +
                                '\n')

            md = block_binding_re.match(line)
            if md:
                block_binding = int(md.group(1))

            md = program_interface_query_re.match(line)
            if md:
                binding = get_ubo_binding(ubos, md.group(1))
                if binding is not None and binding != block_binding:
                    line = "block binding {}\n{}".format(binding, line)
                    block_binding = binding

        elif in_vertex_data:
            line = attrib_re.sub(vertex_data_replacement, line)

        if not skipping:
            fout.write(line)

        if add_spirv_line and groupname == 'require':
            fout.write('SPIRV YES\n')
            add_spirv_line = False

# The --aml option of glslangValidator doesn’t know how to make the
# locations match for interstage linking so instead we manually fudge
# the SPIR-V source to use the same locations generated for the
# outputs from the previous stage.
def replace_inputs_with_outputs(spirv, prev_stage, this_stage):
    replacements = {}

    for output in prev_stage.outputs.values():
        if (output.location is None or
            output.name not in this_stage.inputs):
            continue

        replacements[this_stage.inputs[output.name].result_id] = output.location

    def get_replacement(md):
        rid = md.group(1)
        if rid in replacements:
            return spirv[md.start():md.start(2)] + str(replacements[rid])
        else:
            return md.group(0)

    return RE_spirv_location.sub(get_replacement, spirv)


#Returns: 0 for failure, 1 for success, 2 for skip
#  FIXME: better return values
def process_shader_test(shader_test, config, skip_reasons):
    unsupported_gl_extensions = set([
        # Pretty much inherently unsupported
        'GL_ARB_geometry_shader4',
        'GL_ARB_shader_subroutine',
        'GL_ARB_compute_variable_group',
        'GL_ARB_compute_variable_group_size',
        'GL_EXT_shader_framebuffer_fetch',
        'GL_EXT_shader_framebuffer_fetch_non_coherent',
        'GL_NV_shader_atomic_float',

        # glslang limitation
        'GL_MESA_shader_integer_functions',
        'GL_ARB_shader_atomic_counter_ops',
        'GL_EXT_shader_integer_mix',
        'GL_INTEL_shader_atomic_float_minmax'
    ])

    shaders = []

    have_glsl = False
    is_core = False
    has_vertex_shader_passthrough = False
    shader = None

    with open(shader_test, 'r') as filp:

        spirv_line = None
        groupname = ''
        for line in filp:
            if shader is not None:
                if line.startswith('['):
                    shaders.append(shader)
                    shader = None
                else:
                    # Basic preprocessor handling of #if 1 / #else blocks
                    md = RE_if_start.match(line)
                    if md:
                        if_stack.append(int(md.group(1)) != 0)
                        line = '\n'
                    elif len(if_stack) > 0:
                        if RE_else.match(line):
                            if_stack[-1] = not if_stack[-1]
                            line = '\n'
                        elif RE_endif.match(line):
                            if_stack.pop()
                            line = '\n'
                        elif False in if_stack:
                            line = '\n'

                    shader.append(line)
                    continue

            if line.startswith('['):
                if_stack = []
                in_requirements = False

                groupname = line[1:line.index(']')]

                m = RE_glsl_shader_groupname.match(groupname)
                if m is not None:
                    shader = ShaderSource(m.group(1))
                    have_glsl = True
                    continue

                if groupname == 'vertex shader passthrough':
                    has_vertex_shader_passthrough = True

                continue

            if groupname == 'require':
                words = line.strip().split()
                if not words:
                    continue

                if words[0] == 'GL':
                    if words[1] == 'ES':
                        if config.verbose:
                            # Needs no SPIRV line, since shader_runner can also skip automatically
                            print('{}: skip due to GL ES'.format(shader_test))
                        return 2
                    if 'CORE' in words:
                        is_core = True
                elif words[0] == 'SPIRV':
                    assert spirv_line is None
                    spirv_line = words[1:]
                    if len(spirv_line) >= 1:
                        if spirv_line[0] not in ('YES', 'NO', 'ONLY'):
                            print('{}: bad SPIRV line: {}'.format(shader_test, ' '.join(spirv_line)))
                            return 0

                        if (spirv_line[0] == 'NO' and
                            (not config.recheck or
                             (len(spirv_line) >= 2 and spirv_line[1] == 'OTHER'))):
                            if config.verbose:
                                print('{}: skip due to SPIRV NO line'.format(shader_test))
                            return 2
                elif (words[0] in unsupported_gl_extensions or words[0].startswith('GL_OES_')):
                    skip_reasons.add('{} not supported by ARB_gl_spirv'.format(words[0]))
                    if config.verbose:
                        print('{}: skip due to {}'.format(shader_test, words[0]))
                    return 2
                continue

    # This is needed to handle if the shader test doesn't have a
    # [test] section. That could happen if the test is intended to be
    # used by a different program, where the test conditions are
    # handled by them.
    if shader is not None:
        shaders.append(shader)
        shader = None

    if have_glsl and not config.no_transform:
        fixup_glsl_shaders(shaders, skip_reasons)

        if skip_reasons:
            if config.verbose:
                print('{}: skip (reasons: {})'.format(shader_test, ', '.join(skip_reasons)))
            if spirv_line and spirv_line[0] != 'NO':
                print('{}: SPIRV line indicates that skip should not happen'.format(shader_test))
                return 0
            return 2

    shader_groups = []
    for shader in shaders:
        if not shader_groups or shader.stage != shader_groups[-1][0].stage:
            shader_groups.append([shader])
        else:
            shader_groups[-1].append(shader)

    if (len(shader_groups) == 0):
        # If we are here, we are on a case without GLSL code. The options are
        # a vertex/fragment program, or a pure-SPIR-V shader. On the latter
        # we don't want to add a skip_reason, in order to avoid being mark-skipped
        if spirv_line is None:
            skip_reasons.add('There is no GLSL shader to convert (vertex/fragment program perhaps?)')
        if config.verbose:
            print('Skipping {}: There is no GLSL shader to convert (vertex/fragment program, or only SPIR-V perhaps?)'.format(shader_test))
        return 2

    replacements = {}
    uniform_map = {}
    ubos = []
    max_uniform = 0
    prev_stage = None
    vertex_stage = None

    for shader_group in shader_groups:
        spirv = compile_glsl(shader_test, config, shader_group,
                             uniform_map, max_uniform)
        if not spirv:
            return 0

        this_stage = SpirvInfo(spirv)

        for var in this_stage.uniforms.values():
            if var.location is None:
                continue
            end = var.location + var.type.uniform_size()
            if end > max_uniform:
                max_uniform = end

        uniform_map.update(this_stage.uniforms)
        ubos.extend(this_stage.ubos)
        if prev_stage is not None:
            spirv = replace_inputs_with_outputs(spirv, prev_stage, this_stage)

        if shader_group[0].stage == "vertex":
            vertex_stage = this_stage

        replacements[shader_group[0].stage + ' shader'] = spirv

        if config.transformed:
            for shader in shader_group:
                print(shader.source())

        prev_stage = this_stage

    extra_sections = None

    if vertex_stage is None:
        vertex_stage = SpirvInfo(passthrough_spirv)
        # If the test isn’t providing a vertex shader and is assuming
        # a compatibility context, normally this would end up using
        # the legacy fixed vertex processing. However the SPIR-V mode
        # forces a core profile so in that case it will end up doing
        # no vertex processing. Instead we will add a line to add the
        # passthrough vertex shader. We don’t want to do this if the
        # script provides tessellation or geometry shaders because
        # that should fail to link and there are explicit tests for
        # that.
        if not is_core and not has_vertex_shader_passthrough:
            stages = set((s[0].stage for s in shader_groups))
            if len(stages) == 1 and "fragment" in stages:
                extra_sections = '[vertex shader passthrough]\n'

    if config.mirror:
        spv_shader_test_file = config.mirror[0] + '/' + shader_test
        directory_name = os.path.dirname(spv_shader_test_file)

        if not os.path.exists(directory_name):
            os.makedirs(directory_name)
    else:
        spv_shader_test_file = shader_test + '.spv'


    if config.verbose:
        print('Writing transformed shader_test to ' +
              spv_shader_test_file)
    with open(spv_shader_test_file, 'w') as fout:
        with open(shader_test, 'r') as fin:
            filter_shader_test(config,
                               fin, fout,
                               replacements,
                               uniform_map,
                               ubos,
                               vertex_stage.inputs,
                               spirv_line == None,
                               extra_sections)
    if config.replace:
        os.rename(spv_shader_test_file, shader_test)

    return 1


def expand_shader_tests(config):
    """
    Generator that expands the shader_tests list (e.g. recursing into
    directories) and yields all found tests.
    """
    for name in config.shader_tests:
        st = os.stat(name)
        if stat.S_ISDIR(st.st_mode):
            for subname in glob.glob(name + '/**/*.shader_test', recursive=True):
                yield subname
        else:
            yield name


def get_option(env_varname, default=None):
    """Query the given environment variable for the option.

    Return the value of the default argument if opt is None.

    """
    opt = os.environ.get(env_varname, None)
    if opt is not None:
        return opt

    return opt or default

def main():
    config = parse_args()
    success = True

    config.spirv_dis = get_option('PIGLIT_SPIRV_DIS_BINARY', './generated_spv/spirv-dis')
    config.glslang = get_option('PIGLIT_GLSLANG_VALIDATOR_BINARY', './generated_spv/glslangValidator')
    config.supports_uniform_location_override = supports_uniform_location_override(config)

    config.excludes = []

    for exclude_file_name in config.excludes_from_file or []:
        with open(exclude_file_name, 'r') as filp:
            for line in filp:
                line = line.strip()
                if line.startswith('#'):
                    continue
                if line:
                    config.excludes.append(line)

    if config.jobs:
        n_jobs = int(config.jobs[0])
    else:
        n_jobs = 1

    procs = []
    proc_num = 0

    all_tests = list(expand_shader_tests(config))

    for i in range(1, n_jobs):
        pid = os.fork()
        if pid == 0:
            proc_num = i
            procs = []
            break
        procs.append(pid)

    num_excluded = 0
    num_success = 0
    num_fail = 0
    num_total = 0
    num_skipped = 0;

    error_list = None
    if config.error_list_file is not None:
        error_list = open(config.error_list_file[0], 'w')

    skip_list = None
    if config.skip_list_file is not None:
        skip_list = open(config.skip_list_file[0], 'w')

    for shader_test_num in range(proc_num, len(all_tests), n_jobs):
        skip_reasons = set()
        shader_test = all_tests[shader_test_num]
        num_total = num_total + 1

        excluded = False
        for exclude in config.excludes:
            if shader_test.startswith(exclude):
                excluded = True
                break
        if excluded:
            num_excluded = num_excluded + 1
            if skip_list is not None:
                    skip_list.write('{}\n'.format(shader_test))
            if config.mark_skip:
                global num_mark_skipped
                skip_reasons.add('Test included on exclude file')
                update_spirv_line(shader_test, skip_reasons)
                num_mark_skipped = num_mark_skipped + 1
            continue

        try:
            outcome = process_shader_test(shader_test, config, skip_reasons)
            if outcome == 0:
                num_fail = num_fail + 1
                success = False

                if error_list is not None:
                    error_list.write('{}\n'.format(shader_test))

                if not config.keep_going:
                    break
            elif outcome == 1:
                num_success = num_success + 1
            elif outcome == 2:
                num_skipped = num_skipped +1
                if skip_list is not None:
                    skip_list.write('{}\n'.format(shader_test))
            else:
                print("Unknown return type when processing shader {}".format(shader_test))

        except:
            print('Uncaught exception during {}'.format(shader_test))
            raise

        if config.mark_skip:
            if skip_reasons:
                update_spirv_line(shader_test, skip_reasons)
                num_mark_skipped = num_mark_skipped + 1

    for pid in procs:
        pid, status = os.waitpid(pid, 0)
        if (not os.WIFEXITED(status) or
            os.WEXITSTATUS(status) != 0):
            success = False

    if config.mark_skip is False:
        print('[{}] skip: {}, excluded: {}, success: {}, fail: {}'.
              format(num_total, num_skipped, num_excluded, num_success,
                     num_fail))
    else:
        print('[{}] skip: {}, excluded: {}, mark_skipped: {}, success: {}, fail: {}'.
              format(num_total, num_skipped, num_excluded, num_mark_skipped, num_success,
                     num_fail))

    print('Thank you for running shader_test_spirv!')

    if error_list is not None:
        error_list.close()

    if skip_list is not None:
        skip_list.close()

    return success

if __name__ == '__main__':
    if not main():
        sys.exit(1)
