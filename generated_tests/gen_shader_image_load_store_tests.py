# coding=utf-8
#
# Copyright (C) 2014 Intel Corporation
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

from __future__ import print_function, division, absolute_import
import os.path
from mako.template import Template
from textwrap import dedent

from modules import utils


def gen_header(status):
    """
    Generate a GLSL program header.

    Generate header code for ARB_shader_image_load_store GLSL parser
    tests that are expected to give status as result.
    """
    return dedent("""\
        /*
         * [config]
         * expect_result: {0}
         * glsl_version: 1.50
         * require_extensions: GL_ARB_shader_image_load_store
         * [end config]
         */
        #version 150
        #extension GL_ARB_shader_image_load_store: require
    """.format(status))


def gen_vector_type(scalar, dim):
    """
    Generate a GLSL vector type name.

    Generate a GLSL vector type based on the given scalar type with the
    specified number of dimensions.
    """
    vector_types = {
        'int': 'ivec',
        'uint': 'uvec',
        'float': 'vec',
    }

    return ('{0}{1}'.format(vector_types[scalar], dim) if dim > 1
            else scalar)


def gen_scalar_args(arity):
    """
    Generate image builtin scalar arguments.

    Returns a generator of well-formed scalar arguments for an image
    built-in of the given arity having t as argument type.
    """
    return lambda t: ', {0}(0)'.format(t) * arity


def gen_vector_args(arity):
    """
    Generate image builtin vector arguments.

    Returns a generator of well-formed arguments for an image built-in
    of the given arity having a 4-component vector of t as argument
    type.
    """
    return lambda t: ', {0}(0)'.format(gen_vector_type(t, 4)) * arity


def gen_address_args(dim):
    """
    Generate image address arguments.

    Returns a generator for the coordinate argument of an image
    built-in expecting an address of the given dimensionality.  If
    fail is True the generated argument will have an arbitrary
    incorrect dimensionality.
    """
    return (lambda fail = False: ', {0}(0)'.format(
        gen_vector_type('int', dim if not fail else dim % 3 + 1)))


def gen_address_args_ms(dim):
    """Generate multisample image address arguments."""
    return (lambda fail = False:
            gen_address_args(dim)() + (', 0' if not fail else ''))


def product(ps, *qss):
    """
    Generate the cartesian product of a number of lists of dictionaries.

    Each generated element will be the union of some combination of
    elements from the iterable arguments.  The resulting value of each
    'name' item will be the concatenation of names of the respective
    element combination separated with dashes.
    """
    for q in (product(*qss) if qss else [{}]):
        for p in ps:
            r = dict(p, **q)
            r['name'] = '-'.join(s['name'] for s in (p, q) if s.get('name'))
            yield r


def gen(name, src, tests):
    """
    Expand a source template for the provided list of test definitions.

    Generate a GLSL parser test for each of the elements of the
    'tests' iterable, each of them should be a dictionary of
    definitions that will be used as environment to render the source
    template.

    The file name of each test will be the concatenation of the 'name'
    argument with the 'name' item from the respective test dictionary.
    """
    template = Template(dedent(src))

    for t in product([{'name': name}], tests):
        filename = os.path.join('spec',
                                'ARB_shader_image_load_store',
                                'compiler',
                                '{0}.{1}'.format(t['name'],
                                                 t['shader_stage']))
        print(filename)

        dirname = os.path.dirname(filename)
        utils.safe_makedirs(dirname)

        with open(filename, 'w') as f:
            f.write(template.render(header = gen_header, **t))


shader_stages = [
    {'shader_stage': 'frag'},
    {'shader_stage': 'vert'}
]


image_atomic_builtins = [
    {
        'name': 'atomic-add',
        'image_builtin': 'imageAtomicAdd',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-min',
        'image_builtin': 'imageAtomicMin',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-max',
        'image_builtin': 'imageAtomicMax',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-and',
        'image_builtin': 'imageAtomicAnd',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-or',
        'image_builtin': 'imageAtomicOr',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-xor',
        'image_builtin': 'imageAtomicXor',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-exchange',
        'image_builtin': 'imageAtomicExchange',
        'image_args': gen_scalar_args(1)
    },
    {
        'name': 'atomic-comp-swap',
        'image_builtin': 'imageAtomicCompSwap',
        'image_args': gen_scalar_args(2)
    }
]


image_load_builtin = [
    {
        'name': 'load',
        'image_builtin': 'imageLoad',
        'image_args': gen_vector_args(0)
    }
]


image_store_builtin = [
    {
        'name': 'store',
        'image_builtin': 'imageStore',
        'image_args': gen_vector_args(1)
    }
]


image_types = [
    {
        'name': '1d',
        'image_type': 'image1D',
        'image_addr': gen_address_args(1)
    },
    {
        'name': '2d',
        'image_type': 'image2D',
        'image_addr': gen_address_args(2)
    },
    {
        'name': '3d',
        'image_type': 'image3D',
        'image_addr': gen_address_args(3)
    },
    {
        'name': '2d-rect',
        'image_type': 'image2DRect',
        'image_addr': gen_address_args(2)
    },
    {
        'name': 'cube',
        'image_type': 'imageCube',
        'image_addr': gen_address_args(3)
    },
    {
        'name': 'buffer',
        'image_type': 'imageBuffer',
        'image_addr': gen_address_args(1)
    },
    {
        'name': '1d-array',
        'image_type': 'image1DArray',
        'image_addr': gen_address_args(2)
    },
    {
        'name': '2d-array',
        'image_type': 'image2DArray',
        'image_addr': gen_address_args(3)
    },
    {
        'name': 'cube-array',
        'image_type': 'imageCubeArray',
        'image_addr': gen_address_args(3)
    },
    {
        'name': '2d-ms',
        'image_type': 'image2DMS',
        'image_addr': gen_address_args_ms(2)
    },
    {
        'name': '2d-ms-array',
        'image_type': 'image2DMSArray',
        'image_addr': gen_address_args_ms(3)
    }
]


#
# Test the preprocessor defines.
#
gen('preprocessor', """\
    ${header('pass')}

    #if !defined GL_ARB_shader_image_load_store
    #  error GL_ARB_shader_image_load_store is not defined
    #elif GL_ARB_shader_image_load_store != 1
    #  error GL_ARB_shader_image_load_store is not equal to 1
    #endif

    void main()
    {
    }
""", shader_stages)

#
# Test the early_fragment_tests layout qualifier.
#
gen('early-fragment-tests', """\
    ${header('pass' if shader_stage == 'frag' and hunk == 'in' else 'fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Fragment shaders also allow the following layout qualifier on 'in'
     *  only (not with variable declarations):
     *
     *  layout-qualifier-id
     *    early_fragment_tests"
     */
    layout(early_fragment_tests) ${hunk};

    void main()
    {
    }
""", product(shader_stages, [
    {'name': 'in', 'hunk': 'in'},
    {'name': 'out', 'hunk': 'out'},
    {'name': 'uniform', 'hunk': 'uniform uint u'},
    {'name': 'in-var', 'hunk': 'in float f'},
    {'name': 'uniform-buffer', 'hunk': 'uniform b { image2D img; }'}
]))

#
# Test image declarations.
#
gen('declaration-allowed', """\
    ${header('pass')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "[Images] can only be declared as function parameters or uniform
     *  variables."
     */
    layout(rgba32f) uniform ${image_type} img;
    layout(rgba32ui) uniform u${image_type} uimg;
    layout(rgba32i) uniform i${image_type} iimg;

    void f(${image_type} arg,
           u${image_type} uarg,
           i${image_type} iarg)
    {
    }

    void main()
    {
    }
""", product(image_types, shader_stages))

gen('declaration-global', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "[Images] can only be declared as function parameters or uniform
     *  variables."
     */
    layout(rgba32f) ${qualifier} image2D img;

    void main()
    {
    }
""", product(shader_stages, [
    {'name': 'const', 'qualifier': 'const'},
    {'name': 'in', 'qualifier': 'in'},
    {'name': 'out', 'qualifier': 'out'}
]))

gen('declaration-local', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "[Images] can only be declared as function parameters or uniform
     *  variables."
     */
    void main()
    {
        layout(rgba32f) image2D img;
    }
""", shader_stages)

gen('declaration-uniform-block', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Sets of uniforms, except for samplers and images, can be grouped
     *  into uniform blocks."
     */
    uniform b {
        layout(rgba32f) image2D img;
    };

    void main()
    {
    }
""", shader_stages)

gen('declaration-argument', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Images cannot be treated as l-values; hence, they cannot be used
     *  as out or inout function parameters, nor can they be assigned
     *  into."
     */
    void f(${qualifier} image2D y)
    {
    }

    void main()
    {
    }
""", product(shader_stages, [
    {'name': 'inout', 'qualifier': 'inout'},
    {'name': 'out', 'qualifier': 'out'}
]))

gen('declaration-initializer', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "[Images] cannot be declared with an initializer in a shader."
     */
    layout(rgba32f) uniform image2D img = 0;

    void main()
    {
    }
""", shader_stages)

gen('declaration-format-qualifier', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "It is an error to declare an image variable where the format
     *  qualifier does not match the image variable type."
     */
    layout(${format}) uniform ${prefix}${image_type} img;

    void main()
    {
    }
""", product(image_types, shader_stages, [
    {'name': 'float',
     'format': 'rgba32f',
     'prefix': 'i'},
    {'name': 'int',
     'format': 'rgba32i',
     'prefix': 'u'},
    {'name': 'uint',
     'format': 'rgba32ui',
     'prefix': ''}
]))

gen('declaration-format-qualifier-duplicate', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Only one format qualifier may be specified for any image variable
     *  declaration."
     */
    layout(rgba32f) layout(rgba32f) uniform image2D img;

    void main()
    {
    }
""", shader_stages)

gen('declaration-format-qualifier-missing', """\
    ${header(status)}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Uniforms not qualified with "writeonly" must have a format
     *  layout qualifier."
     */
    ${qualifier} uniform image2D img;

    void main()
    {
    }
""", product(shader_stages, [
    {'name': 'writeonly',
     'qualifier': 'writeonly',
     'status': 'pass'},
    {'name': 'readonly',
     'qualifier': 'readonly',
     'status': 'fail'},
    {'name': 'readwrite',
     'qualifier': '',
     'status': 'fail'}
]))

gen('declaration-memory-qualifier-sampler', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Only variables declared as image types [...] can be qualified
     *  with a memory qualifier. "
     */
    ${qualifier} uniform sampler2D s;

    void main()
    {
    }
""", product(shader_stages, [
    {'name': 'readonly', 'qualifier': 'readonly'},
    {'name': 'writeonly', 'qualifier': 'writeonly'},
    {'name': 'coherent', 'qualifier': 'coherent'},
    {'name': 'volatile', 'qualifier': 'volatile'},
    {'name': 'restrict', 'qualifier': 'restrict'}
]))

#
# Test expressions involving images.
#
gen('expression-allowed', """\
    ${header('pass')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Except for array indexing, structure field selection, and
     *  parentheses, images are not allowed to be operands in
     *  expressions."
     */
    layout(rgba32f) uniform ${image_type} imgs[2];
    uniform vec4 y;

    out vec4 color;

    void main()
    {
            color = y + imageLoad((imgs[1]) ${image_addr()});
    }
""", product(image_types[:1], shader_stages))

gen('expression', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Except for array indexing, structure field selection, and
     *  parentheses, images are not allowed to be operands in
     *  expressions."
     */
    layout(rgba32f) uniform image2D img;

    void main()
    {
            ${expression};
    }
""", product(shader_stages, [
    {'name': 'arithmetic-1', 'expression': '-img'},
    {'name': 'arithmetic-2', 'expression': 'img + img'},
    {'name': 'arithmetic-3', 'expression': 'img - img'},
    {'name': 'arithmetic-4', 'expression': 'img * img'},
    {'name': 'arithmetic-5', 'expression': 'img / img'},
    {'name': 'arithmetic-6', 'expression': '++img'},
    {'name': 'arithmetic-7', 'expression': '--img'},
    {'name': 'arithmetic-8', 'expression': 'img++'},
    {'name': 'arithmetic-9', 'expression': 'img--'},
    {'name': 'assignment-1', 'expression': 'img ^= img'},
    {'name': 'assignment-2', 'expression': 'img |= img'},
    {'name': 'assignment-3', 'expression': 'img = img'},
    {'name': 'assignment-4', 'expression': 'img += img'},
    {'name': 'assignment-5', 'expression': 'img -= img'},
    {'name': 'assignment-6', 'expression': 'img *= img'},
    {'name': 'assignment-7', 'expression': 'img /= img'},
    {'name': 'assignment-8', 'expression': 'img %= img'},
    {'name': 'assignment-9', 'expression': 'img <<= img'},
    {'name': 'assignment-10', 'expression': 'img >>= img'},
    {'name': 'assignment-11', 'expression': 'img &= img'},
    {'name': 'binary-1', 'expression': '~img'},
    {'name': 'binary-2', 'expression': 'img << img'},
    {'name': 'binary-3', 'expression': 'img >> img'},
    {'name': 'binary-4', 'expression': 'img & img'},
    {'name': 'binary-5', 'expression': 'img | img'},
    {'name': 'binary-6', 'expression': 'img ^ img'},
    {'name': 'conversion-1', 'expression': 'float(img)'},
    {'name': 'conversion-2', 'expression': 'uint(img)'},
    {'name': 'conversion-3', 'expression': 'bool(img)'},
    {'name': 'conversion-4', 'expression': 'image1D(img)'},
    {'name': 'field-selection', 'expression': 'img.x'},
    {'name': 'function-call', 'expression': 'img()'},
    {'name': 'logical-1', 'expression': '!img'},
    {'name': 'logical-2', 'expression': 'img && img'},
    {'name': 'logical-3', 'expression': 'img || img'},
    {'name': 'logical-4', 'expression': 'img ^^ img'},
    {'name': 'relational-1', 'expression': 'img == img'},
    {'name': 'relational-2', 'expression': 'img != img'},
    {'name': 'relational-3', 'expression': 'img < img'},
    {'name': 'relational-4', 'expression': 'img > img'},
    {'name': 'relational-5', 'expression': 'img <= img'},
    {'name': 'relational-6', 'expression': 'img >= img'},
    {'name': 'selection', 'expression': 'true ? img : img'},
    {'name': 'subscript', 'expression': 'img[0]'}
]))

#
# Test passing of image variables in function calls.
#
gen('call-argument-qualifiers', """\
    ${header(status)}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "The values of image variables qualified with 'coherent',
     *  'volatile', 'restrict', 'readonly', or 'writeonly' may not be
     *  passed to functions whose formal parameters lack such
     *  qualifiers. [...] It is legal to have additional qualifiers on a
     *  formal parameter, but not to have fewer."
     */
    layout(rgba32f) ${actual_qualifier} uniform image2D x;

    void f(${formal_qualifier} image2D y)
    {
    }

    void main()
    {
        f(x);
    }
""", product(shader_stages, [
    {'name': 'allowed-volatile',
     'actual_qualifier': 'volatile',
     'formal_qualifier': 'volatile coherent',
     'status': 'pass'},
    {'name': 'allowed-coherent',
     'actual_qualifier': 'coherent',
     'formal_qualifier': 'volatile coherent',
     'status': 'pass'},
    {'name': 'allowed-restrict',
     'actual_qualifier': 'restrict',
     'formal_qualifier': 'volatile restrict',
     'status': 'pass'},
    {'name': 'allowed-readonly',
     'actual_qualifier': 'readonly',
     'formal_qualifier': 'restrict readonly',
     'status': 'pass'},
    {'name': 'allowed-writeonly',
     'actual_qualifier': 'writeonly',
     'formal_qualifier': 'volatile writeonly',
     'status': 'pass'},
    {'name': 'disallowed-volatile',
     'actual_qualifier': 'volatile',
     'formal_qualifier': '',
     'status': 'fail'},
    {'name': 'disallowed-coherent',
     'actual_qualifier': 'coherent',
     'formal_qualifier': 'restrict',
     'status': 'fail'},
    {'name': 'disallowed-restrict',
     'actual_qualifier': 'restrict',
     'formal_qualifier': '',
     'status': 'fail'},
    {'name': 'disallowed-readonly',
     'actual_qualifier': 'readonly',
     'formal_qualifier': 'restrict writeonly',
     'status': 'fail'},
    {'name': 'disallowed-writeonly',
     'actual_qualifier': 'writeonly',
     'formal_qualifier': 'volatile readonly',
     'status': 'fail'}
]))

gen('call-argument-type', """\
    ${header('pass' if image_type == 'image2D' else 'fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "As function parameters, images may only be passed to [arguments]
     *  of matching type."
     */
    layout(rgba32f) uniform image2D x;

    void f(${image_type} y)
    {
    }

    void main()
    {
        f(x);
    }
""", product(image_types, shader_stages))

#
# Test the language built-in constants and functions.
#
gen('builtin-constants', """\
    ${header('pass')}
    /*
     * Check that the builtin constants defined by the extension
     * are present.
     */
    out ivec4 color;

    void main()
    {
        color.x = gl_MaxImageUnits +
                gl_MaxCombinedImageUnitsAndFragmentOutputs +
                gl_MaxImageSamples +
                gl_MaxVertexImageUniforms +
                gl_MaxTessControlImageUniforms +
                gl_MaxTessEvaluationImageUniforms +
                gl_MaxGeometryImageUniforms +
                gl_MaxFragmentImageUniforms +
                gl_MaxCombinedImageUniforms;
    }
""", shader_stages)

gen('builtin-image-argument-mismatch', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "Atomic memory operations are supported on only [...]: an image
     *  variable with signed integer components (iimage*) format qualifier
     *  of 'r32i', or an image variable with unsigned integer components
     *  (uimage*) and format qualifier of 'r32ui'."
     *
     * Call an atomic built-in with a floating point image data type.
     */
    layout(r32f) uniform ${image_type} img;

    void main()
    {
        ${image_builtin}(img ${image_addr()} ${image_args('float')});
    }
""", product(image_atomic_builtins, image_types[:1], shader_stages))

gen('builtin-data-argument-mismatch', """\
    ${header('fail')}
    /*
     * Call a signed integer atomic built-in with a mismatching
     * argument data type.
     */
    layout(r32i) uniform i${image_type} img;

    void main()
    {
        ${image_builtin}(img ${image_addr()} ${image_args('uint')});
    }
""", product(image_store_builtin + image_atomic_builtins,
             image_types[:1], shader_stages))

gen('builtin-address-argument-mismatch', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "The 'IMAGE_INFO' placeholder is replaced by one of the following
     *  parameter lists:
     *       gimage1D image, int coord
     *       gimage2D image, ivec2 coord
     *       gimage3D image, ivec3 coord
     *       gimage2DRect image, ivec2 coord
     *       gimageCube image, ivec3 coord
     *       gimageBuffer image, int coord
     *       gimage1DArray image, ivec2 coord
     *       gimage2DArray image, ivec3 coord
     *       gimageCubeArray image, ivec3 coord
     *       gimage2DMS image, ivec2 coord, int sample
     *       gimage2DMSArray image, ivec3 coord, int sample"
     *
     * Pass an argument as address coordinate that doesn't match the
     * dimensionality of the specified image.
     */
    layout(r32i) uniform i${image_type} img;

    void main()
    {
        ${image_builtin}(img ${image_addr(fail = True)} ${image_args('int')});
    }
""", product(image_load_builtin + image_store_builtin + image_atomic_builtins,
             image_types, shader_stages))

gen('builtin-qualifier-mismatch-readonly', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "It is an error to pass an image variable qualified with 'readonly'
     *  to imageStore() or other built-in functions that modify image
     *  memory."
     *
     * Call a built-in function on a readonly qualified image.
     */
    layout(r32i) readonly uniform i${image_type} img;

    void main()
    {
        ${image_builtin}(img ${image_addr()} ${image_args('int')});
    }
""", product(image_store_builtin + image_atomic_builtins,
             image_types[:1], shader_stages))

gen('builtin-qualifier-mismatch-writeonly', """\
    ${header('fail')}
    /*
     * From the ARB_shader_image_load_store spec:
     *
     * "It is an error to pass an image variable qualified with 'writeonly'
     *  to imageLoad() or other built-in functions that read image
     *  memory."
     *
     * Call a built-in function on a writeonly qualified image.
     */
    layout(r32i) writeonly uniform i${image_type} img;

    void main()
    {
        ${image_builtin}(img ${image_addr()} ${image_args('int')});
    }
""", product(image_load_builtin + image_atomic_builtins,
             image_types[:1], shader_stages))
