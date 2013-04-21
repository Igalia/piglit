# Copyright 2012 Intel Corporation
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# This script generates a JSON description of the GL API based on
# source files published by www.opengl.org.
#
# The source file "gl.tm" is a CSV file mapping from abstract type
# names (in column 0) to C types (in column 3).  All other columns are
# ignored.
#
#
# The source file "gl.spec" consists of a record for each API
# function, which looks like this:
#
# # Function name (excluding the "gl" prefix), followed by the names
# # of all function parameters:
# GetVertexAttribdvARB(index, pname, params)
#
#         # Property/value pairs follow.  Order is irrelevant.
#
#         # "return" specifies the return type (as a reference to
#         # column 0 of gl.tm).
#         return          void
#
#         # "param" specifies the type of a single function parameter
#         # (as a reference to column 0 of gl.tm).  In addition, it
#         # specifies whether the parameter is passed as an input
#         # value, as an input or output array, or an input or output
#         # reference.  Input arrays and references get translated
#         # into const pointers, and Output arrays and references get
#         # translated into non-const pointers.  Note that for arrays,
#         # the size of the array appears in brackets after the word
#         # "array".  This value is ignored.
#         param           index           UInt32 in value
#         param           pname           VertexAttribPropertyARB in value
#         param           params          Float64 out array [4]
#
#         # "category" specifies which extension (or GL version) this
#         # function was introduced in.  For extensions, the category
#         # name is the extension name (without the "GL_" prefix).
#         # For GL versions, the category name looks like
#         # e.g. "VERSION_1_0" or "VERSION_1_0_DEPRECATED" (for
#         # deprecated features).
#         category        ARB_vertex_program
#
#         # "alias" specifies the name of a function that is
#         # behaviorally indistinguishable from this function (if
#         # any).
#         alias           GetVertexAttribdv
#
#         # Other property/value pairs are ignored.
#
# # Lines in any other format are ignored.
#
#
# The source file "enumext.spec" consists of lines of the form <enum
# name> = <value>, e.g.:
#
#         FRONT                                      = 0x0404
#
# The enum name is the name without the "GL_" prefix.
#
# It is also permissible for the value to be a reference to an enum
# that appeared earlier, e.g.:
#
#         DRAW_FRAMEBUFFER_BINDING                   = GL_FRAMEBUFFER_BINDING
#
# Note that when this happens the value *does* include the "GL_"
# prefix.
#
#
# The JSON output format is as follows:
# {
#   "categories": {
#     <category name>: {
#       "kind": <"GL" for a GL version, "extension" for an extension>,
#       "gl_10x_version": <For a GL version, version number times 10>,
#       "extension_name" <For an extension, name of the extension>
#     }, ...
#   },
#   "enums": {
#     <enum name, without "GL_" prefix>: {
#       "value_int": <value integer>
#       "value_str": <value string>
#     }, ...
#   },
#   "functions": {
#     <function name, without "gl" prefix>: {
#       "category": <category in which this function appears>,
#       "param_names": <list of param names>,
#       "param_types": <list of param types>,
#       "return_type": <type, or "void" if no return>
#     }, ...
#   },
#   "function_alias_sets": {
#     <list of synonymous function names>, ...
#   },
# }



import collections
import csv
import json
import re
import sys


GLSPEC_HEADER_REGEXP = re.compile(r'^(\w+)\((.*)\)$')
GLSPEC_ATTRIBUTE_REGEXP = re.compile(r'^\s+(\w+)\s+(.*)$')
GL_VERSION_REGEXP = re.compile('^VERSION_([0-9])_([0-9])(_DEPRECATED)?$')
ENUM_REGEXP = re.compile(r'^\s+(\w+)\s+=\s+(\w+)$')


# Convert a type into a canonical form that is consistent about its
# use of spaces.
#
# Example input: 'const       void**'
# Example output: 'const void * *'
def normalize_type(typ):
    tokens = [token for token in typ.replace('*', ' * ').split(' ')
              if token != '']
    return ' '.join(tokens)


# Interpret an enumerated value, which may be in hex or decimal, and
# may include a type suffix such as "ull".
#
# Example input: '0xFFFFFFFFul'
# Example output: 4294967295
def decode_enum_value(value_str):
    for suffix in ('u', 'ul', 'ull'):
        if value_str.endswith(suffix):
            value_str = value_str[:-len(suffix)]
            break
    return int(value_str, 0)


# Convert an object to a form that can be serialized to JSON.  Python
# "set" objects are converted to lists.
def jsonize(obj):
    if type(obj) in (set, frozenset):
        return sorted(obj)
    else:
        raise Exception('jsonize failed for {0}'.format(type(obj)))


# Iterate through the lines of a file, discarding end-of-line comments
# delimited by "#".  Blank lines are discarded, as well as any
# whitespace at the end of a line.
def filter_comments(f):
    for line in f:
        if '#' in line:
            line = line[:line.find('#')]
        line = line.rstrip()
        if line != '':
            yield line.rstrip()


# Convert a category name from the form used in the gl.spec file to
# the form we want to output in JSON.  E.g.:
#
# - "2.1" is converted into { 'kind': 'GL', 'gl_10x_version': 21 }
#
# - "FOO" is converted into { 'kind': 'extension', 'extension_name': 'GL_FOO' }
def translate_category(category_name):
    m = GL_VERSION_REGEXP.match(category_name)
    if m:
        ones = int(m.group(1))
        tenths = int(m.group(2))
        return '{0}.{1}'.format(ones, tenths), {
            'kind': 'GL',
            'gl_10x_version': 10 * ones + tenths
            }
    else:
        extension_name = 'GL_' + category_name
        return extension_name, {
            'kind': 'extension',
            'extension_name': extension_name
            }


# Data structure keeping track of which function names are known, and
# which names are synonymous with which other names.
class SynonymMap(object):
    def __init__(self):
        # __name_to_synonyms maps from a function name to the set of
        # all names that are synonymous with it (including itself).
        self.__name_to_synonyms = {}

    # Add a single function name which is not (yet) known to be
    # synonymous with any other name.  No effect if the function name
    # is already known.
    def add_singleton(self, name):
        if name not in self.__name_to_synonyms:
            self.__name_to_synonyms[name] = frozenset([name])
        return self.__name_to_synonyms[name]

    # Add a pair of function names, and note that they are synonymous.
    # Synonymity is transitive, so if either of the two function names
    # previously had known synonyms, all synonyms are combined into a
    # single set.
    def add_alias(self, name, alias):
        name_ss = self.add_singleton(name)
        alias_ss = self.add_singleton(alias)
        combined_set = name_ss | alias_ss
        for n in combined_set:
            self.__name_to_synonyms[n] = combined_set

    # Get a set of sets of synonymous functions.
    def get_synonym_sets(self):
        return frozenset(self.__name_to_synonyms.values())


# In-memory representation of the GL API.
class Api(object):
    def __init__(self):
        # Api.type_translation is a dict mapping abstract type names
        # to C types.  It is based on the data in the gl.tm file.  For
        # example, the dict entry for String is:
        #
        # 'String': 'const GLubyte *'
        self.type_translation = {}

        # Api.enums is a dict mapping enum names (without the 'GL_'
        # prefix) to a dict containing (a) the enum value expressed as
        # an integer, and (b) the enum value expressed as a C literal.
        # It is based on the data in the gl.spec file.  For example,
        # the dict entry for GL_CLIENT_ALL_ATTRIB_BITS is:
        #
        # 'CLIENT_ALL_ATTRIB_BITS': { 'value_int': 4294967295,
        #                             'value_str': "0xFFFFFFFF" }
        self.enums = {}

        # Api.functions is a dict mapping function names (without the
        # 'gl' prefix) to a dict containing (a) the name of the
        # category the function is in, (b) the function call parameter
        # names, (c) the function call parameter types, and (d) the
        # function return type.  It is based on the data in the
        # gl.spec file, cross-referenced against the type translations
        # from the gl.tm file.  For example, the dict entry for
        # glAreTexturesResident is:
        #
        # 'AreTexturesResident': {
        #    'category': '1.1',
        #    'param_names': ['n', 'textures', 'residences'],
        #    'param_types': ['GLsizei', 'const GLuint *', 'GLboolean *'],
        #    'return_type': ['GLboolean'] }
        self.functions = {}

        # Api.synonyms is a SynonymMap object which records which
        # function names are aliases of each other.  It is based on
        # the "alias" declarations from the gl.spec file.
        self.synonyms = SynonymMap()

        # Api.categories is a dict mapping category names to a dict
        # describing the category.  For categories representing a GL
        # version, the dict entry looks like this:
        #
        # '2.1': { 'kind': 'GL', 'gl_10x_version': 21 }
        #
        # For categories representing an extension, the dict entry
        # looks like this:
        #
        # 'GL_ARB_sync': { 'kind': 'extension',
        #                  'extension_name': 'GL_ARB_sync' }
        self.categories = {}

    # Convert each line in the gl.tm file into a key/value pair in
    # self.type_translation, mapping an abstract type name to a C
    # type.
    def read_gl_tm(self, f):
        for line in csv.reader(filter_comments(f)):
            name = line[0].strip()
            typ = line[3].strip()
            if typ == '*':
                # gl.tm uses "*" to represent void (for void used as a
                # return value).
                typ = 'void'
            self.type_translation[name] = normalize_type(typ)

    # Group the lines in the gl.spec file into triples (function_name,
    # param_names, attributes).  For example, the following gl.spec
    # input:
    #
    # Foo(bar, baz):
    #     x  value1
    #     y  value2 other_info
    #     y  value3 more_info
    #
    # Produces this output triple:
    #
    # ('Foo', ['bar', 'baz'],
    #  {'x': ['value1'], 'y': ['value2 other_info', 'value3 more_info']})
    @staticmethod
    def group_gl_spec_functions(f):
        function_name = None
        param_names = None
        attributes = None
        for line in filter_comments(f):
            m = GLSPEC_HEADER_REGEXP.match(line)
            if m:
                if function_name:
                    yield function_name, param_names, attributes
                function_name = m.group(1)
                if m.group(2) == '':
                    param_names = []
                else:
                    param_names = [n.strip() for n in m.group(2).split(',')]
                attributes = collections.defaultdict(list)
                continue
            m = GLSPEC_ATTRIBUTE_REGEXP.match(line)
            if m:
                attribute_type, attribute_data = m.groups()
                attributes[attribute_type].append(attribute_data)
                continue
            continue
        if function_name:
            yield function_name, param_names, attributes

    # Process the data in gl.spec, and populate self.functions,
    # self.synonyms, and self.categories based on it.
    def read_gl_spec(self, f):
        for name, param_names, attributes in self.group_gl_spec_functions(f):
            if name in self.functions:
                raise Exception(
                    'Function {0!r} appears more than once'.format(name))
            param_name_to_index = dict(
                (param_name, index)
                for index, param_name in enumerate(param_names))
            param_types = [None] * len(param_names)
            if len(attributes['param']) != len(param_names):
                raise Exception(
                    'Function {0!r} has a different number of parameters and '
                    'param declarations'.format(name))
            for param_datum in attributes['param']:
                param_datum_tokens = param_datum.split()
                param_name = param_datum_tokens[0]
                param_index = param_name_to_index[param_name]
                param_base_type = self.type_translation[param_datum_tokens[1]]
                param_dir = param_datum_tokens[2]
                param_multiplicity = param_datum_tokens[3]
                if param_types[param_index] is not None:
                    raise Exception(
                        'Function {0!r} contains more than one param '
                        'declaration for parameter {1!r}'.format(
                            name, param_name))
                if param_multiplicity == 'value':
                    assert param_dir == 'in'
                    param_type = param_base_type
                elif param_multiplicity in ('array', 'reference'):
                    if param_dir == 'in':
                        # Note: technically this is not correct if
                        # param_base_type is a pointer type (e.g. to
                        # make an "in array" of "void *" we should
                        # produce "void * const *", not "const void *
                        # *").  However, the scripts used by the GL
                        # consortium to produce glext.h from gl.spec
                        # produce "const void * *", and fortunately
                        # the only ill effect of this is that clients
                        # have to do a little more typecasting than
                        # they should.  So to avoid confusing people,
                        # we're going to make the same mistake, so
                        # that the resulting function signatures match
                        # those in glext.h.
                        param_type = normalize_type(
                            'const {0} *'.format(param_base_type))
                    elif param_dir == 'out':
                        param_type = normalize_type(
                            '{0} *'.format(param_base_type))
                    else:
                        raise Exception(
                            'Function {0!r} parameter {1!r} uses unrecognized '
                            'direction {2!r}'.format(
                                name, param_name, param_dir))
                else:
                    raise Exception(
                        'Function {0!r} parameter {1!r} uses unrecognized '
                        'multiplicity {2!r}'.format(
                            name, param_name, param_multiplicity))
                param_types[param_index] = param_type
            if len(attributes['return']) != 1:
                raise Exception(
                    'Function {0!r} contains {1} return attributes'.format(
                        name, len(attributes['return'])))
            if len(attributes['category']) != 1:
                raise Exception(
                    'Function {0!r} contains {1} category attributes'.format(
                        name, len(attributes['category'])))
            category, additional_data = translate_category(
                attributes['category'][0])
            if category not in self.categories:
                self.categories[category] = additional_data
            self.functions[name] = {
                'return_type': self.type_translation[attributes['return'][0]],
                'param_names': param_names,
                'param_types': param_types,
                'category': category,
                }
            self.synonyms.add_singleton(name)
            for alias in attributes['alias']:
                self.synonyms.add_alias(name, alias)

    # Convert each line in the enumext.spec file into a key/value pair
    # in self.enums, mapping an enum name to a dict.  For example, the
    # following enumext.spec input:
    #
    # CLIENT_ALL_ATTRIB_BITS = 0xFFFFFFFF # ClientAttribMask
    #
    # Produces the dict entry:
    #
    # 'CLIENT_ALL_ATTRIB_BITS': { 'value_int': 4294967295,
    #                             'value_str': "0xFFFFFFFF" }
    def read_enumext_spec(self, f):
        for line in filter_comments(f):
            m = ENUM_REGEXP.match(line)
            if m:
                name, value = m.groups()
                if value.startswith('GL_'):
                    value_rhs = value[3:]
                    value_int = self.enums[value_rhs]['value_int']
                else:
                    value_int = decode_enum_value(value)
                self.enums[name] = {
                    'value_str': value,
                    'value_int': value_int
                    }

    # Convert the stored API into JSON.  To make diffing easier, all
    # dictionaries are sorted by key, and all sets are sorted by set
    # element.
    def to_json(self):
        return json.dumps({
                'categories': self.categories,
                'enums': self.enums,
                'functions': self.functions,
                'function_alias_sets':
                    self.synonyms.get_synonym_sets(),
                }, indent = 2, sort_keys = True, default = jsonize)


if __name__ == '__main__':
    api = Api()
    with open(sys.argv[1]) as f:
        api.read_gl_tm(f)
    with open(sys.argv[2]) as f:
        api.read_gl_spec(f)
    with open(sys.argv[3]) as f:
        api.read_enumext_spec(f)
    with open(sys.argv[4], 'w') as f:
        f.write(api.to_json())
