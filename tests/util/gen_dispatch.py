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

# This script generates a C file (and corresponding header) allowing
# Piglit to dispatch calls to OpenGL based on a JSON description of
# the GL API (and extensions).
#
# Invoke this script with 3 command line arguments: the JSON input
# filename, the C output filename, and the header outpit filename.
#
#
# The input looks like this:
#
# {
#   "categories": {
#     <category name>: {
#       "kind": <"GL" or "GLES" for a GL spec API, "extension" for an extension>,
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
#       "categories": <list of categories in which this function appears>,
#       "param_names": <list of param names>,
#       "param_types": <list of param types>,
#       "return_type": <type, or "void" if no return>
#     }, ...
#   },
#   "function_alias_sets": {
#     <list of synonymous function names>, ...
#   },
# }
#
#
# The generated header consists of the following:
#
# - A typedef for each function, of the form that would normally
#   appear in gl.h or glext.h, e.g.:
#
#   typedef GLvoid * (APIENTRY *PFNGLMAPBUFFERPROC)(GLenum, GLenum);
#   typedef GLvoid * (APIENTRY *PFNGLMAPBUFFERARBPROC)(GLenum, GLenum);
#
# - A set of extern declarations for "dispatch function pointers".
#   There is one dispatch function pointer for each set of synonymous
#   functions in the GL API, e.g.:
#
#   extern PFNGLMAPBUFFERPROC piglit_dispatch_glMapBuffer;
#
# - A set of #defines mapping each function name to the corresponding
#   dispatch function pointer, e.g.:
#
#   #define glMapBuffer piglit_dispatch_glMapBuffer
#   #define glMapBufferARB piglit_dispatch_glMapBuffer
#
# - A #define for each enum in the GL API, e.g.:
#
#   #define GL_FRONT 0x0404
#
# - A #define for each extension, e.g.:
#
#   #define GL_ARB_vertex_buffer_object 1
#
# - A #define for each known GL version, e.g.:
#
#   #define GL_VERSION_1_5 1
#
#
# The generated C file consists of the following:
#
# - A resolve function corresponding to each set of synonymous
#   functions in the GL API.  The resolve function determines which of
#   the synonymous names the implementation supports (by consulting
#   the current GL version and/or the extension string), and calls
#   either get_core_proc() or get_ext_proc() to get the function
#   pointer.  It stores the result in the dispatch function pointer,
#   and then returns it as a generic void(void) function pointer.  If
#   the implementation does not support any of the synonymous names,
#   it calls unsupported().  E.g.:
#
#   /* glMapBuffer (GL 1.5) */
#   /* glMapbufferARB (GL_ARB_vertex_buffer_object) */
#   static piglit_dispatch_function_ptr resolve_glMapBuffer()
#   {
#     if (check_version(15))
#       piglit_dispatch_glMapBuffer = (PFNGLMAPBUFFERPROC) get_core_proc("glMapBuffer", 15);
#     else if (check_extension("GL_ARB_vertex_buffer_object"))
#       piglit_dispatch_glMapBuffer = (PFNGLMAPBUFFERARBPROC) get_ext_proc("glMapBufferARB");
#     else
#       unsupported("MapBuffer");
#     return (piglit_dispatch_function_ptr) piglit_dispatch_glMapBuffer;
#   }
#
# - A stub function corresponding to each set of synonymous functions
#   in the GL API.  The stub function first calls
#   check_initialized().  Then it calls the resolve function to
#   ensure that the dispatch function pointer is set.  Finally, it
#   dispatches to the GL function through the dispatch function
#   pointer.  E.g.:
#
#   static GLvoid * APIENTRY stub_glMapBuffer(GLenum target, GLenum access)
#   {
#     check_initialized();
#     resolve_glMapBuffer();
#     return piglit_dispatch_glMapBuffer(target, access);
#   }
#
# - A declaration for each dispatch function pointer, e.g.:
#
#   PFNGLMAPBUFFERPROC piglit_dispatch_glMapBuffer = stub_glMapBuffer;
#
# - An function, reset_dispatch_pointers(), which resets each dispatch
#   pointer to the corresponding stub function.
#
# - A table function_names, containing the name of each function in
#   alphabetical order (including the "gl" prefix).
#
# - A table function_resolvers, containing a pointer to the resolve
#   function corresponding to each entry in function_names.

import collections
import json
import os.path
import sys


# Generate a top-of-file comment cautioning that the file is
# auto-generated and should not be manually edited.
def generated_boilerplate():
    return """\
/**
 * This file was automatically generated by the script {0!r}.
 *
 * DO NOT EDIT!
 *
 * To regenerate, run "make piglit_dispatch_gen" from the toplevel directory.
 *
 * Copyright 2012 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
""".format(os.path.basename(sys.argv[0]))


# Certain param names used in OpenGL are reserved by some compilers.
# Rename them.
PARAM_NAME_FIXUPS = { 'near': 'hither', 'far': 'yon' }
def fixup_param_name(name):
    if name in PARAM_NAME_FIXUPS:
        return PARAM_NAME_FIXUPS[name]
    else:
        return name


# Internal representation of a category.
#
# - For a category representing a GL version, Category.kind is 'GL'
#   and Category.gl_10x_version is 10 times the GL version (e.g. 21
#   for OpenGL version 2.1).
#
# - For a category representing an extension, Category.kind is
#   'extension' and Category.extension_name is the extension name
#   (including the 'GL_' prefix).
class Category(object):
    def __init__(self, json_data):
        self.kind = json_data['kind']
        if 'gl_10x_version' in json_data:
            self.gl_10x_version = json_data['gl_10x_version']
        if 'extension_name' in json_data:
            self.extension_name = json_data['extension_name']

    # Generate a human-readable representation of the category (for
    # use in generated comments).
    def __str__(self):
        if self.kind == 'GL':
            return 'GL {0}.{1}'.format(
                self.gl_10x_version // 10, self.gl_10x_version % 10)
        if self.kind == 'GLES':
            return 'GLES {0}.{1}'.format(
                self.gl_10x_version // 10, self.gl_10x_version % 10)
        elif self.kind == 'extension':
            return self.extension_name
        else:
            raise Exception(
                'Unexpected category kind {0!r}'.format(self.kind))


# Internal representation of a GL function.
#
# - Function.name is the name of the function, without the 'gl'
#   prefix.
#
# - Function.param_names is a list containing the name of each
#   function parameter.
#
# - Function.param_types is a list containing the type of each
#   function parameter.
#
# - Function.return_type is the return type of the function, or 'void'
#   if the function has no return.
#
# - Function.category is a Category object describing the extension or
#   GL version the function is defined in.
class Function(object):
    def __init__(self, name, json_data):
        self.name = name
        self.param_names = [
            fixup_param_name(x) for x in json_data['param_names']]
        self.param_types = json_data['param_types']
        self.return_type = json_data['return_type']
        self.categories = json_data['categories']

    # Name of the function, with the 'gl' prefix.
    @property
    def gl_name(self):
        return 'gl' + self.name

    # Name of the function signature typedef corresponding to this
    # function.  E.g. for the glGetString function, this is
    # 'PFNGLGETSTRINGPROC'.
    @property
    def typedef_name(self):
        return 'pfn{0}proc'.format(self.gl_name).upper()

    # Generate a string representing the function signature in C.
    #
    # - name is inserted before the opening paren--use '' to generate
    #   an anonymous function type signature.
    #
    # - If anonymous_args is True, then the signature contains only
    #   the types of the arguments, not the names.
    def c_form(self, name, anonymous_args):
        if self.param_types:
            if anonymous_args:
                param_decls = ', '.join(self.param_types)
            else:
                param_decls = ', '.join(
                    '{0} {1}'.format(*p)
                    for p in zip(self.param_types, self.param_names))
        else:
            param_decls = 'void'
        return '{rettype} {name}({param_decls})'.format(
            rettype = self.return_type, name = name,
            param_decls = param_decls)


# Internal representation of an enum.
#
# - Enum.value_int is the value of the enum, as a Python integer.
#
# - Enum.value_str is the value of the enum, as a string suitable for
#   emitting as C code.
class Enum(object):
    def __init__(self, json_data):
        self.value_int = json_data['value_int']
        self.value_str = json_data['value_str']


# Data structure keeping track of a set of synonymous functions.  Such
# a set is called a "dispatch set" because it corresponds to a single
# dispatch pointer.
#
# - DispatchSet.cat_fn_pairs is a list of pairs (category, function)
#   for each category this function is defined in.  The list is sorted
#   by category, with categories of kind 'GL' and then 'GLES' appearing first.
class DispatchSet(object):
    # Initialize a dispatch set given a list of synonymous function
    # names.
    #
    # - all_functions is a dict mapping all possible function names to
    #   the Function object describing them.
    #
    # - all_categories is a dict mapping all possible category names
    #   to the Category object describing them.
    def __init__(self, synonym_set, all_functions, all_categories):
        self.cat_fn_pairs = []
        for function_name in synonym_set:
            function = all_functions[function_name]
            for category_name in function.categories:
                category = all_categories[category_name]
                self.cat_fn_pairs.append((category, function))
        # Sort by category, with GL categories preceding extensions.
        self.cat_fn_pairs.sort(key = self.__sort_key)

    # The first Function object in DispatchSet.functions.  This
    # "primary" function is used to name the dispatch pointer, the
    # stub function, and the resolve function.
    @property
    def primary_function(self):
        return self.cat_fn_pairs[0][1]

    # The name of the dispatch pointer that should be generated for
    # this dispatch set.
    @property
    def dispatch_name(self):
        return 'piglit_dispatch_' + self.primary_function.gl_name

    # The name of the stub function that should be generated for this
    # dispatch set.
    @property
    def stub_name(self):
        return 'stub_' + self.primary_function.gl_name

    # The name of the resolve function that should be generated for
    # this dispatch set.
    @property
    def resolve_name(self):
        return 'resolve_' + self.primary_function.gl_name

    @staticmethod
    def __sort_key(cat_fn_pair):
        if cat_fn_pair[0].kind == 'GL':
            return 0, cat_fn_pair[0].gl_10x_version
        elif cat_fn_pair[0].kind == 'GLES':
            return 1, cat_fn_pair[0].gl_10x_version
        elif cat_fn_pair[0].kind == 'extension':
            return 2, cat_fn_pair[0].extension_name
        else:
            raise Exception(
                'Unexpected category kind {0!r}'.format(cat_fn_pair[0].kind))


# Data structure keeping track of all of the known functions and
# enums, and synonym relationships that exist between the functions.
#
# - Api.enums is a dict mapping enum name to an Enum object.
#
# - Api.functions is a dict mapping function name to a Function object.
#
# - Api.function_alias_sets is a list of lists, where each constituent
#   list is a list of function names that are aliases of each other.
#
# - Api.categories is a dict mapping category name to a Category
#   object.
class Api(object):
    def __init__(self, json_data):
        self.enums = dict(
            (key, Enum(value))
            for key, value in json_data['enums'].items())
        self.functions = dict(
            (key, Function(key, value))
            for key, value in json_data['functions'].items())
        self.function_alias_sets = json_data['function_alias_sets']
        self.categories = dict(
            (key, Category(value))
            for key, value in json_data['categories'].items())

    # Generate a list of (name, value) pairs representing all enums in
    # the API.  The resulting list is sorted by enum value.
    def compute_unique_enums(self):
        enums_by_value = [(enum.value_int, (name, enum.value_str))
                          for name, enum in self.enums.items()]
        enums_by_value.sort()
        return [item[1] for item in enums_by_value]

    # A list of all of the extension names declared in the API, as
    # Python strings, sorted alphabetically.
    @property
    def extensions(self):
	return sorted(
	    [category_name
	     for category_name, category in self.categories.items()
	     if category.kind == 'extension'])

    # A list of all of the GL versions declared in the API, as
    # integers (e.g. 13 represents GL version 1.3).
    @property
    def gl_versions(self):
	return sorted(
	    [category.gl_10x_version
	     for category in self.categories.values()
	     if category.kind == 'GL'])

    # Generate a list of DispatchSet objects representing all sets of
    # synonymous functions in the API.  The resulting list is sorted
    # by DispatchSet.stub_name.
    def compute_dispatch_sets(self):
        sets = [DispatchSet(synonym_set, self.functions, self.categories)
                for synonym_set in self.function_alias_sets]
        sets.sort(key = lambda ds: ds.stub_name)

        return sets

    # Generate a list of Function objects representing all functions
    # in the API.  The resulting list is sorted by function name.
    def compute_unique_functions(self):
        return [self.functions[key] for key in sorted(self.functions.keys())]


# Read the given input file and return an Api object containing the
# data in it.
def read_api(filename):
    with open(filename, 'r') as f:
        return Api(json.load(f))


# Generate the resolve function for a given DispatchSet.
def generate_resolve_function(ds):
    f0 = ds.primary_function

    # First figure out all the conditions we want to check in order to
    # figure out which function to dispatch to, and the code we will
    # execute in each case.
    condition_code_pairs = []
    for category, f in ds.cat_fn_pairs:
        if category.kind in ('GL', 'GLES'):
            getter = 'get_core_proc("{0}", {1})'.format(
                f.gl_name, category.gl_10x_version)

            condition = ''
            api_base_version = 0;
            if category.kind == 'GL':
                condition = 'dispatch_api == PIGLIT_DISPATCH_GL'
                api_base_version = 10
            elif category.gl_10x_version >= 20:
                condition = 'dispatch_api == PIGLIT_DISPATCH_ES2'
                api_base_version = 20
            else:
                condition = 'dispatch_api == PIGLIT_DISPATCH_ES'
                api_base_version = 11

            # Only check the version for functions that aren't part of the
            # core for the PIGLIT_DISPATCH api.
            if category.gl_10x_version != api_base_version:
                condition = condition + ' && check_version({0})'.format(category.gl_10x_version)
        elif category.kind == 'extension':
            getter = 'get_ext_proc("{0}")'.format(f.gl_name)
            condition = 'check_extension("{0}")'.format(category.extension_name)
        else:
            raise Exception(
                'Unexpected category type {0!r}'.format(category.kind))

        if f.name == 'TexImage3DEXT':
            # Special case: glTexImage3DEXT has a slightly different
            # type than glTexImage3D (argument 3 is a GLenum rather
            # than a GLint).  This is not a problem, since GLenum and
            # GLint are treated identically by function calling
            # conventions.  So when calling get_proc_address() on
            # glTexImage3DEXT, cast the result to PFNGLTEXIMAGE3DPROC
            # to avoid a warning.
            typedef_name = 'PFNGLTEXIMAGE3DPROC'
        else:
            typedef_name = f.typedef_name

        code = '{0} = ({1}) {2};'.format(
            ds.dispatch_name, typedef_name, getter)

        condition_code_pairs.append((condition, code))

    # XXX: glDraw{Arrays,Elements}InstancedARB are exposed by
    # ARB_instanced_arrays in addition to ARB_draw_instanced, but neither
    # gl.spec nor gl.json can accomodate an extension with two categories, so
    # insert these cases here.
        if f.gl_name in ('glDrawArraysInstancedARB', 'glDrawElementsInstancedARB'):
            condition = 'check_extension("GL_ARB_instanced_arrays")'
            condition_code_pairs.append((condition, code))

    # Finally, if none of the previous conditions were satisfied, then
    # the given dispatch set is not supported by the implementation,
    # so we want to call the unsupported() function.
    condition_code_pairs.append(
        ('true', 'unsupported("{0}");'.format(f0.name)))

    # Start the resolve function
    resolve_fn = 'static piglit_dispatch_function_ptr {0}()\n'.format(
        ds.resolve_name)
    resolve_fn += '{\n'

    # Output code that checks each condition in turn and executes the
    # appropriate case.  To make the generated code more palatable
    # (and to avoid compiler warnings), we convert "if (true) FOO;" to
    # "FOO;" and "else if (true) FOO;" to "else FOO;".
    if condition_code_pairs[0][0] == 'true':
        resolve_fn += '\t{0}\n'.format(condition_code_pairs[0][1])
    else:
        resolve_fn += '\tif ({0})\n\t\t{1}\n'.format(*condition_code_pairs[0])
        for i in xrange(1, len(condition_code_pairs)):
            if condition_code_pairs[i][0] == 'true':
                resolve_fn += '\telse\n\t\t{0}\n'.format(
                    condition_code_pairs[i][1])
                break
            else:
                resolve_fn += '\telse if ({0})\n\t\t{1}\n'.format(
                    *condition_code_pairs[i])

    # Output code to return the dispatch function.
    resolve_fn += '\treturn (piglit_dispatch_function_ptr) {0};\n'.format(
        ds.dispatch_name)
    resolve_fn += '}\n'
    return resolve_fn


# Generate the stub function for a given DispatchSet.
def generate_stub_function(ds):
    f0 = ds.primary_function

    # Start the stub function
    stub_fn = 'static {0}\n'.format(
        f0.c_form('APIENTRY ' + ds.stub_name, anonymous_args = False))
    stub_fn += '{\n'
    stub_fn += '\tcheck_initialized();\n'
    stub_fn += '\t{0}();\n'.format(ds.resolve_name)

    # Output the call to the dispatch function.
    stub_fn += '\t{0}{1}({2});\n'.format(
        'return ' if f0.return_type != 'void' else '',
        ds.dispatch_name, ', '.join(f0.param_names))
    stub_fn += '}\n'
    return stub_fn


# Generate the reset_dispatch_pointers() function, which sets each
# dispatch pointer to point to the corresponding stub function.
def generate_dispatch_pointer_resetter(dispatch_sets):
    result = []
    result.append('static void\n')
    result.append('reset_dispatch_pointers()\n')
    result.append('{\n')
    for ds in dispatch_sets:
        result.append(
            '\t{0} = {1};\n'.format(ds.dispatch_name, ds.stub_name))
    result.append('}\n')
    return ''.join(result)


# Generate the function_names and function_resolvers tables.
def generate_function_names_and_resolvers(dispatch_sets):
    name_resolver_pairs = []
    for ds in dispatch_sets:
        for _, f in ds.cat_fn_pairs:
            name_resolver_pairs.append((f.gl_name, ds.resolve_name))
    name_resolver_pairs.sort()
    result = []
    result.append('static const char * const function_names[] = {\n')
    for name, _ in name_resolver_pairs:
        result.append('\t"{0}",\n'.format(name))
    result.append('};\n')
    result.append('\n')
    result.append('static const piglit_dispatch_resolver_ptr '
                  'function_resolvers[] = {\n')
    for _, resolver in name_resolver_pairs:
        result.append('\t{0},\n'.format(resolver))
    result.append('};\n')
    return ''.join(result)


# Generate the C source and header files for the API.
def generate_code(api):
    c_contents = [generated_boilerplate()]
    h_contents = [generated_boilerplate()]

    unique_functions = api.compute_unique_functions()

    # Emit typedefs for each name
    for f in unique_functions:
        h_contents.append(
            'typedef {0};\n'.format(
                f.c_form('(APIENTRY *{0})'.format(f.typedef_name),
                         anonymous_args = True)))

    dispatch_sets = api.compute_dispatch_sets()

    for ds in dispatch_sets:
        f0 = ds.primary_function

        # Emit comment block
        comments = '\n'
        for cat, f in ds.cat_fn_pairs:
            comments += '/* {0} ({1}) */\n'.format(f.gl_name, cat)
        c_contents.append(comments)
        h_contents.append(comments)

        # Emit extern declaration of dispatch pointer
        h_contents.append(
            'extern {0} {1};\n'.format(f0.typedef_name, ds.dispatch_name))

        # Emit defines aliasing each GL function to the dispatch
        # pointer
        for _, f in ds.cat_fn_pairs:
            h_contents.append(
                '#define {0} {1}\n'.format(f.gl_name, ds.dispatch_name))

        # Emit resolve function
        c_contents.append(generate_resolve_function(ds))

        # Emit stub function
        c_contents.append(generate_stub_function(ds))

        # Emit initializer for dispatch pointer
        c_contents.append(
            '{0} {1} = {2};\n'.format(
                f0.typedef_name, ds.dispatch_name, ds.stub_name))

    # Emit dispatch pointer initialization function
    c_contents.append(generate_dispatch_pointer_resetter(dispatch_sets))

    c_contents.append('\n')

    # Emit function_names and function_resolvers tables.
    c_contents.append(generate_function_names_and_resolvers(dispatch_sets))

    # Emit enum #defines
    for name, value in api.compute_unique_enums():
        h_contents.append('#define GL_{0} {1}\n'.format(name, value))

    # Emit extension #defines
    #
    # While enum.ext lists some old extension names (defined to 1), it
    # doesn't contain the full set that appears in glext.h.
    h_contents.append('\n')
    for ext in api.extensions:
	h_contents.append('#define {0} 1\n'.format(ext))

    # Emit GL version #defines
    #
    # While enum.ext lists GL versions up to 3.2, it didn't continue
    # adding them for later GL versions.
    h_contents.append('\n')
    for ver in api.gl_versions:
	h_contents.append('#define GL_VERSION_{0}_{1} 1\n'.format(
		ver // 10, ver % 10))

    return ''.join(c_contents), ''.join(h_contents)


if __name__ == '__main__':
    file_to_parse = sys.argv[1]
    api = read_api(file_to_parse)

    c_contents, h_contents = generate_code(api)
    with open(sys.argv[2], 'w') as f:
        f.write(c_contents)
    with open(sys.argv[3], 'w') as f:
        f.write(h_contents)
