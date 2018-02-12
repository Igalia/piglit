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

RE_spirv_shader_groupname = re.compile(r'(.*) shader spirv$')
RE_glsl_shader_groupname = re.compile(r'(.*) shader$')


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

def nest_tokens(tokens):
    """
    Given a flat list of tokens, create a nested list structure that reflects
    (), [], and {}.
    """
    matching_parens = {
        '(': ')',
        '[': ']',
        '{': '}'
    }

    stack = [[]]
    for tok in tokens:
        if tok in ['(', '[', '{']:
            stack.append([tok])
            continue

        stack[-1].append(tok)
        if len(stack) > 1 and tok == matching_parens.get(stack[-1][0]):
            nest = stack.pop()
            stack[-1].append(nest)

    assert len(stack) == 1
    return stack[0]

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

    def transform_toplevel_declarations(self, fn):
        """
        Each declaration is passed to fn as a list of tokens without whitespace,
        including the final semicolon. If fn returns None, the declaration is
        preserved unchanged. Otherwise, fn must return a string that is
        tokenized and replaces the original token.
        """
        src_tokens = self.__tokens
        transformations = []

        start = 1
        idx = 1
        scopes = []

        while idx < len(src_tokens):
            token = src_tokens[idx]

            # Skip top-level preprocessor statements
            if token.startswith('#') and start == idx:
                start += 2
                idx += 2
                continue

            # TODO recognize and skip function definitions

            if not scopes and token == ';':
                idx += 2
                transformed = fn(src_tokens[start:idx:2])
                if transformed is not None:
                    transformations.append((start, idx - 1, transformed))
                start = idx
                continue

            if token == '{':
                scopes.append('}')
            elif scopes and token == scopes[-1]:
                scopes.pop()
            idx += 2

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
    RE_numeric_constant = re.compile(r'0(|x[0-9a-fA-F]+)|[1-9][0-9]*')
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


class Declaration(object):
    @staticmethod
    def parse(tokens, stage):
        decl = VariableDeclaration.parse(tokens, stage)
        if decl is not None:
            return decl

        instance_name = None
        end = len(tokens)
        if isinstance(tokens[end - 1], str):
            if tokens[end - 1] in ('in', 'out'):
                # Global layout declaration
                return None

            instance_name = tokens[end - 1]
            end -= 1


class VariableDeclaration(Declaration):
    def __init__(self, tokens, name, array_end, is_block, instance_name, type_end, stage):
        """
        Initialize with nested tokens. The trailing semicolon must have been
        removed.
        """
        self.tokens = tokens
        self.__name = name
        self.__instance_name = instance_name
        self.__array_end = array_end
        self.__type_end = type_end
        self.is_block = is_block

        try:
            self.__layout = tokens.index('layout') + 1
        except ValueError:
            self.__layout = None

        if 'in' in tokens:
            self.mode = 'in'
        elif 'out' in tokens:
            self.mode = 'out'
        elif 'uniform' in tokens:
            self.mode = 'uniform'
        else:
            self.mode = 'other'

        self.is_patch = 'patch' in tokens

        if ((((stage == 'tessellation control' and (self.mode in ('in', 'out'))) or
             (stage == 'tessellation evaluation' and self.mode == 'in')) and
             not self.is_patch) or
            (stage == 'geometry' and self.mode == 'in')):
            assert self.__array_end - 1 > self.__name
            assert type(tokens[self.__array_end - 1]) == list
            assert tokens[self.__array_end - 1][0] == '['
            self.implicit_array = True
        else:
            self.implicit_array = False

    def name(self):
        return self.tokens[self.__name]

    def layout(self):
        if self.__layout is not None:
            return self.tokens[self.__layout]
        return None

    def size(self, skip_reasons):
        """
        Determine the number of locations occupied by this variable.
        """
        aoa_elements = 1
        if not self.is_block:
            for idx in range(self.__name + 1, self.__array_end - self.implicit_array):
                array = self.tokens[idx]
                assert array[0] == '['
                if len(array) != 3:
                    skip_reasons.add('complicated array')
                    return None
                try:
                    elements = int(array[1])
                except ValueError:
                    elements = 0
                if elements <= 0:
                    skip_reasons.add('complicated array')
                    return None

                aoa_elements *= elements

        if (type(self.tokens[self.__type_end - 1]) == list and
            self.tokens[self.__type_end - 1][0] == '{'):

            block = self.tokens[self.__type_end - 1]
            semicolons = [0] + [i for i, tok in enumerate(block) if tok == ';']
            assert semicolons[-1] == len(block) - 2

            members = [
                VariableDeclaration.parse(block[i:j], 'none')
                for i,j in zip(semicolons[:-1], semicolons[1:])
            ]

            base_size = sum(member.size(skip_reasons) or 0 for member in members)
        else:
            base_size = 1

        return base_size * aoa_elements

    @staticmethod
    def parse(tokens, stage):
        try:
            assignment_eq = tokens.index('=')
        except ValueError:
            assignment_eq = None

        array_end = assignment_eq or len(tokens)
        name = array_end
        while name > 0:
            name -= 1
            if isinstance(tokens[name], list):
                if tokens[name][0] == '[':
                    continue
                if tokens[name][0] == '{':
                    # Might be a declaration of a struct or block without instance name
                    type_end = name + 1
                    name = None
                    break
                # Probably a function declaration
                return None
            else:
                assert isinstance(tokens[name], str)
                if tokens[name] in ('in', 'out'):
                    # Global layout declaration
                    return None
                break

        if name is not None:
            type_end = name

        is_block = False
        instance_name = None
        if isinstance(tokens[type_end - 1], list) and tokens[type_end - 1][0] == '{':
            if type_end < 3:
                # Can only be a non-interface variable of anonymous struct type
                return None
            if tokens[type_end - 2] == 'struct':
                pass # Anonymous struct
            elif tokens[type_end - 3] == 'struct':
                pass # Named struct
            else:
                # Block
                is_block = True
                instance_name = name
                name = type_end - 2

        if not is_block and name is None:
            return None # Struct definition

        return VariableDeclaration(tokens, name, array_end, is_block, instance_name,
                                   type_end, stage)


def fixup_glsl_shaders(shaders, vertex_attribs):
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

    def assign_uniform_location(flat_declaration):
        assert flat_declaration[-1] == ';'
        var = VariableDeclaration.parse(nest_tokens(flat_declaration[:-1]), cur_stage)
        if var is None:
            return None

        layout = var.layout()
        loc = None
        size = var.size(skip_reasons)

        if size is None:
            assert skip_reasons
            return None

        if size > 1:
            return None

        if layout == None and var.mode == 'uniform':
            loc = cur_uniform_location[0]
            cur_uniform_location[0] += var.size(skip_reasons)

        if loc is not None:
            return 'layout(location={}) '.format(loc) + ' '.join(flat_declaration)

    def assign_location(flat_declaration):
        assert flat_declaration[-1] == ';'
        var = VariableDeclaration.parse(nest_tokens(flat_declaration[:-1]), cur_stage)
        if var is None:
            return None

        layout = var.layout()
        if layout and 'shared' in layout:
            skip_reasons.add('"shared" as layout qualifier')
            return None

        if var.mode == 'in' and 'invariant' in var.tokens:
            skip_reasons.add('invariant input')
            return None

        if layout and 'location' in layout:
            if var.mode == 'out':
                if cur_out_location[0] > 0:
                    skip_reasons.add('mixed explicit and implicit locations')
                cur_out_location[0] = -1
            return None

        name = var.name()

        loc = prev_stage_out.get(name)
        if loc is None:
            loc = cur_stage_out.get(name)

        if loc is None and var.mode == 'out':
            if name.startswith('gl_'):
                return None

            loc = cur_out_location[0]
            if loc < 0:
                skip_reasons.add('mixed explicit and implicit locations')
                return None

            size = var.size(skip_reasons)
            if size is None:
                assert skip_reasons
                return None

            cur_out_location[0] += size
            cur_stage_out[name] = loc

        if loc is not None:
            return 'layout(location={}) '.format(loc) + ' '.join(flat_declaration)

    def scan_builtin(token):
        if token.startswith('gl_') and not token in good_builtins:
            skip_reasons.add(token)
        elif token in compat_unsupported:
            skip_reasons.add(token)

    shaders.sort(key=lambda shader: get_stage_order(shader.stage))

    cur_stage_out = vertex_attribs
    cur_stage = ''

    skip_reasons = set()

    for shader in shaders:
        if cur_stage != shader.stage:
            prev_stage_out = cur_stage_out
            cur_stage = shader.stage
            cur_stage_out = {}
            cur_out_location = [0]
            cur_uniform_location = [0]


        glsl = GLSLSource(shader.source())

        have_compat = set()
        glsl.transform_tokens(first_transform_tokens)

        glsl.prepend('#version 450\n')

        for compat in have_compat:
            glsl.insert_after_versions(compat_replacements[compat].declaration + '\n')

        glsl.transform_toplevel_declarations(assign_location)
        glsl.transform_toplevel_declarations(assign_uniform_location)

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
    parser.add_argument("-n", "--no-transform",
                        action="store_true",
                        help="Don't try to transform GLSL shaders")
    parser.add_argument("-t", "--transformed",
                        action="store_true",
                        help="Print transformed GLSL shaders")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        help="Print verbose output")
    parser.add_argument("shader_tests",
                        nargs='+',
                        help="Path to one or more .shader_test files to process.")
    return parser.parse_args()

def compile_glsl(shader_test, config, shader_group):
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

        cmdline = [config.glslang, '-G', '-o', binary_name] + filenames
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

def filter_shader_test(fin, fout,
                       replacements,
                       add_spirv_line):
    skipping = False
    groupname = None

    for line in fin:
        if line.startswith('['):
            groupname = line[1:line.index(']')]
            skipping = RE_spirv_shader_groupname.match(groupname) is not None
            if groupname in replacements:
                replacement = replacements[groupname]
                if len(replacement) > 0:
                    fout.write('[' + groupname + ' spirv]\n')
                    fout.write('; Automatically generated from the GLSL by '
                               'shader_test_spirv.py. DO NOT EDIT\n')
                    fout.write(replacement)
                    fout.write('\n')
                    replacements[groupname] = ''


        if not skipping:
            fout.write(line)

        if add_spirv_line and groupname == 'require':
            fout.write('SPIRV YES\n')
            add_spirv_line = False

def process_shader_test(shader_test, config):
    unsupported_gl_extensions = set([
        # Pretty much inherently unsupported
        'GL_ARB_geometry_shader4',
        'GL_ARB_shader_subroutine',

        # glslang limitation
        'GL_MESA_shader_integer_functions',
    ])

    shaders = []

    have_glsl = False

    with open(shader_test, 'r') as filp:
        shader = None

        spirv_line = None
        groupname = ''
        vertex_attribs = None
        for line in filp:
            if shader is not None:
                if line.startswith('['):
                    shaders.append(shader)
                    shader = None
                else:
                    shader.append(line)
                    continue

            if line.startswith('['):
                in_requirements = False

                groupname = line[1:line.index(']')]

                m = RE_glsl_shader_groupname.match(groupname)
                if m is not None:
                    shader = ShaderSource(m.group(1))
                    have_glsl = True
                    continue

                continue

            if groupname == 'require':
                words = line.strip().split()
                if not words:
                    continue

                if words[0] == 'GL' and words[1] == 'ES':
                    if config.verbose:
                        # Needs no SPIRV line, since shader_runner can also skip automatically
                        print('{}: skip due to GL ES'.format(shader_test))
                    return True
                elif words[0] == 'SPIRV':
                    assert spirv_line is None
                    spirv_line = words[1:]
                    if len(spirv_line) >= 1:
                        if spirv_line[0] not in ('YES', 'NO', 'ONLY'):
                            print('{}: bad SPIRV line: {}'.format(shader_test, ' '.join(spirv_line)))
                            return False

                        if (spirv_line[0] == 'NO' and
                            (not config.recheck or
                             (len(spirv_line) >= 2 and spirv_line[1] == 'OTHER'))):
                            if config.verbose:
                                print('{}: skip due to SPIRV NO line'.format(shader_test))
                            return True
                elif (words[0] in unsupported_gl_extensions or words[0].startswith('GL_OES_')):
                    if config.verbose:
                        # Needs no SPIRV line, since shader_runner can also skip automatically
                        print('{}: skip due to {}'.format(shader_test, words[0]))
                    return True
                continue

            if groupname == 'vertex data':
                if vertex_attribs is None:
                    attribs = [attrib.split('/') for attrib in line.strip().split()]
                    vertex_attribs = {}
                    for j, attrib in enumerate(attribs):
                        name = attrib[0]
                        if name.endswith('[0]'):
                            vertex_attribs[name[:-3]] = j
                        else:
                            vertex_attribs[name] = j
                continue

    if vertex_attribs is None:
        vertex_attribs = {'piglit_vertex': 0, 'piglit_texcoord': 1}

    if have_glsl and not config.no_transform:
        skip_reasons = fixup_glsl_shaders(shaders, vertex_attribs)

        if config.mark_skip:
            if skip_reasons or (spirv_line and spirv_line[0] == 'NO'):
                update_spirv_line(shader_test, skip_reasons)

        if skip_reasons:
            print('{}: skip (reasons: {})'.format(shader_test, ', '.join(skip_reasons)))
            if spirv_line and spirv_line[0] != 'NO':
                print('{}: SPIRV line indicates that skip should not happen'.format(shader_test))
                return False
            return True

    shader_groups = []
    for shader in shaders:
        if not shader_groups or shader.stage != shader_groups[-1][0].stage:
            shader_groups.append([shader])
        else:
            shader_groups[-1].append(shader)

    replacements = {}
    for shader_group in shader_groups:
        spirv = compile_glsl(shader_test, config, shader_group)
        if not spirv:
            return False

        replacements[shader_group[0].stage + ' shader'] = spirv

        if config.transformed:
            for shader in shader_group:
                print(shader.source())

    spv_shader_test_file = shader_test + '.spv'
    if config.verbose:
        print('Writing transformed shader_test to ' +
              spv_shader_test_file)
    with open(spv_shader_test_file, 'w') as fout:
        with open(shader_test, 'r') as fin:
            filter_shader_test(fin, fout,
                               replacements,
                               spirv_line == None)

    return True


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

    config.excludes = []

    for exclude_file_name in config.excludes_from_file or []:
        with open(exclude_file_name, 'r') as filp:
            for line in filp:
                line = line.strip()
                if line.startswith('#'):
                    continue
                if line:
                    config.excludes.append(line)

    for shader_test in expand_shader_tests(config):
        excluded = False
        for exclude in config.excludes:
            if shader_test.startswith(exclude):
                excluded = True
                break
        if excluded:
            continue

        try:
            if not process_shader_test(shader_test, config):
                success = False
                if not config.keep_going:
                    break
        except:
            print('Uncaught exception during {}'.format(shader_test))
            raise

    return success


if __name__ == '__main__':
    if not main():
        sys.exit(1)
