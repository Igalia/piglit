# -*- coding: utf-8 -*-
# All tests that come with piglit, using default settings
#
__all__ = ['profile']

import itertools
import os
import platform
import subprocess
import sys

from framework.profile import TestProfile
from framework.exectest import PiglitTest
from framework.gleantest import GleanTest
from framework.glsl_parser_test import GLSLParserTest, add_glsl_parser_test, import_glsl_parser_tests
from framework.shader_test import add_shader_test_dir

# Path to tests dir, correct even when not running from the top directory.
testsDir = os.path.dirname(__file__)
if sys.platform == "cygwin":
    # convert the path to DOS style so it's parsable by shader_runner, etc.
    testsDir = subprocess.check_output(['cygpath', '-d', testsDir]).rstrip()

# Find the generated_tests directory, by looking either in
# $PIGLIT_BUILD_DIR (if that environment variable exists) or in the
# parent directory of the directory containing this file.
generatedTestDir = os.path.normpath(os.path.join(
    os.environ.get('PIGLIT_BUILD_DIR', os.path.join(testsDir, '..')),
    'generated_tests'))

# Quick wrapper for PiglitTest for our usual concurrent args.
def plain_test(args):
    return PiglitTest(args.split() + ['-auto'])

def add_single_param_test_set(group, name, *params):
    for param in params:
        group[name + '-' + param] = plain_test(name + ' ' + param)

def add_plain_test(group, args):
    group[args] = plain_test(args)

def concurrent_test(args):
    test = plain_test(args + ' -fbo')
    test.run_concurrent = True
    return test

def add_concurrent_test(group, args):
    group[args] = concurrent_test(args)

# Generate all possible subsets of the given set, including the empty set.
def power_set(s):
    if len(s) == 0:
        return [[]]
    result = []
    for p in power_set(s[:-1]):
        result.append(p)
        result.append(p + [s[-1]])
    return result

######
# Collecting all tests
profile = TestProfile()

# List of all of the MSAA sample counts we wish to test
MSAA_SAMPLE_COUNTS = (2, 4, 6, 8, 16, 32)

def add_fbo_depthstencil_tests(group, format, num_samples):
    if format == 'default_fb':
        prefix = ''
        create_test = plain_test
    else:
        prefix = 'fbo-'
        create_test = concurrent_test
    if num_samples > 1:
        suffix = ' samples=' + str(num_samples)
        psamples = ' -samples=' + str(num_samples)
    else:
        suffix = ''
        psamples = ''
    group[prefix + 'depthstencil-' + format + '-clear' + suffix] = create_test('fbo-depthstencil clear ' + format + psamples)
    group[prefix + 'depthstencil-' + format + '-readpixels-FLOAT-and-USHORT' + suffix] = create_test('fbo-depthstencil readpixels ' + format + ' FLOAT-and-USHORT' + psamples)
    group[prefix + 'depthstencil-' + format + '-readpixels-24_8' + suffix] = create_test('fbo-depthstencil readpixels ' + format + ' 24_8' + psamples)
    group[prefix + 'depthstencil-' + format + '-readpixels-32F_24_8_REV' + suffix] = create_test('fbo-depthstencil readpixels ' + format + ' 32F_24_8_REV' + psamples)
    group[prefix + 'depthstencil-' + format + '-drawpixels-FLOAT-and-USHORT' + suffix] = create_test('fbo-depthstencil drawpixels ' + format + ' FLOAT-and-USHORT' + psamples)
    group[prefix + 'depthstencil-' + format + '-drawpixels-24_8' + suffix] = create_test('fbo-depthstencil drawpixels ' + format + ' 24_8' + psamples)
    group[prefix + 'depthstencil-' + format + '-drawpixels-32F_24_8_REV' + suffix] = create_test('fbo-depthstencil drawpixels ' + format + ' 32F_24_8_REV' + psamples)
    group[prefix + 'depthstencil-' + format + '-copypixels' + suffix] = create_test('fbo-depthstencil copypixels ' + format + psamples)
    group[prefix + 'depthstencil-' + format + '-blit' + suffix] = create_test('fbo-depthstencil blit ' + format + psamples)

def add_fbo_depthstencil_msaa_visual_tests(group, format):
    add_fbo_depthstencil_tests(group, format, 0)
    for num_samples in MSAA_SAMPLE_COUNTS:
        add_fbo_depthstencil_tests(group, format, num_samples)

def add_depthstencil_render_miplevels_tests(group, test_types):
    # Note: the buffer sizes below have been chosen to exercise
    # many possible combinations of buffer alignments on i965.
    for texture_size in (146, 273, 292, 585, 1024):
        for test_type in test_types:
            test_name = 'depthstencil-render-miplevels {0} {1}'.format(
                    texture_size, test_type)
            add_concurrent_test(group, test_name)

def add_msaa_visual_plain_tests(group, args):
    add_plain_test(group, args)
    for num_samples in MSAA_SAMPLE_COUNTS:
        group[args + ' samples=' + str(num_samples)] = plain_test(args + ' -samples=' + str(num_samples))

glean = {}
glean['basic'] = GleanTest('basic')
glean['api2'] = GleanTest('api2')
glean['makeCurrent'] = GleanTest('makeCurrent')
glean['blendFunc'] = GleanTest('blendFunc')
glean['bufferObject'] = GleanTest('bufferObject')
glean['depthStencil'] = GleanTest('depthStencil')
glean['fbo'] = GleanTest('fbo')
glean['fpexceptions'] = GleanTest('fpexceptions')
glean['getString'] = GleanTest('getString')
glean['logicOp'] = GleanTest('logicOp')
glean['occluquery'] = GleanTest('occluQry')
glean['orthoPosRandTris'] = GleanTest('orthoPosRandTris')
glean['orthoPosRandRects'] = GleanTest('orthoPosRandRects')
glean['orthoPosTinyQuads'] = GleanTest('orthoPosTinyQuads')
glean['orthoPosHLines'] = GleanTest('orthoPosHLines')
glean['orthoPosVLines'] = GleanTest('orthoPosVLines')
glean['orthoPosPoints'] = GleanTest('orthoPosPoints')
glean['paths'] = GleanTest('paths')
glean['pbo'] = GleanTest('pbo')
glean['polygonOffset'] = GleanTest('polygonOffset')
glean['pixelFormats'] = GleanTest('pixelFormats')
glean['pointAtten'] = GleanTest('pointAtten')
glean['pointSprite'] = GleanTest('pointSprite')
# exactRGBA is not included intentionally, because it's too strict and
# the equivalent functionality is covered by other tests
glean['readPixSanity'] = GleanTest('readPixSanity')
glean['shaderAPI'] = GleanTest('shaderAPI')
glean['stencil2'] = GleanTest('stencil2')
glean['texCombine'] = GleanTest('texCombine')
glean['texCube'] = GleanTest('texCube')
glean['texEnv'] = GleanTest('texEnv')
glean['texgen'] = GleanTest('texgen')
glean['texCombine4'] = GleanTest('texCombine4')
glean['texture_srgb'] = GleanTest('texture_srgb')
glean['texUnits'] = GleanTest('texUnits')
glean['vertArrayBGRA'] = GleanTest('vertArrayBGRA')
glean['vertattrib'] = GleanTest('vertattrib')

glean_glsl_tests = [
                    'Primary plus secondary color',
                    'Global vars and initializers',
                    'Global vars and initializers (2)',
                    'Swizzle',
                    'Swizzle (rgba)',
                    'Swizzle (stpq)',
                    'Writemask',
                    'Swizzled writemask',
                    'Swizzled writemask (2)',
                    'Swizzled writemask (rgba)',
                    'Swizzled writemask (stpq)',
                    'Swizzled expression',
                    'Swizzle in-place',
                    'Swizzled swizzle',
                    'Swizzled swizzled swizzle',
                    'gl_FragDepth writing',
                    'chained assignment',
                    'cross() function, in-place',
                    'sequence (comma) operator',
                    '&& operator, short-circuit',
                    '|| operator, short-circuit',
                    'GL state variable reference (gl_FrontMaterial.ambient)',
                    'GL state variable reference (gl_LightSource[0].diffuse)',
                    'GL state variable reference (diffuse product)',
                    'GL state variable reference (point size)',
                    'GL state variable reference (point attenuation)',
                    'linear fog',
                    'built-in constants',
                    'texture2D()',
                    'texture2D(), computed coordinate',
                    'texture2D(), with bias',
                    '2D Texture lookup with explicit lod (Vertex shader)',
                    'texture2DProj()',
                    'texture1D()',
                    'texture3D()',
                    'texture3D(), computed coord',
                    'shadow2D(): 1',
                    'shadow2D(): 2',
                    'shadow2D(): 3',
                    'shadow2D(): 4',
                    'nested function calls (1)',
                    'nested function calls (2)',
                    'nested function calls (3)',
                    'TPPStreamCompiler::assignOperands',
                    'matrix column check (1)',
                    'matrix column check (2)',
                    'matrix, vector multiply (1)',
                    'matrix, vector multiply (2)',
                    'matrix, vector multiply (3)',
                    'uniform matrix',
                    'uniform matrix, transposed',
                    'struct (1)',
                    'struct (2)',
                    'struct (3)',
                    'struct (4)',
                    'Preprocessor test 1 (#if 0)',
                    'Preprocessor test 2 (#if 1)',
                    'Preprocessor test 3 (#if ==)',
                    'Preprocessor test 4 (#if 1, #define macro)',
                    'Preprocessor test 5 (#if 1, #define macro)',
                    'Preprocessor test 6 (#if 0, #define macro)',
                    'Preprocessor test 7 (multi-line #define)',
                    'Preprocessor test 8 (#ifdef)',
                    'Preprocessor test 9 (#ifndef)',
                    'Preprocessor test 10 (#if defined())',
                    'Preprocessor test 11 (#elif)',
                    'Preprocessor test 12 (#elif)',
                    'Preprocessor test 13 (nested #if)',
                    'Preprocessor test 14 (nested #if)',
                    'Preprocessor test 15 (nested #if, #elif)',
                    'Preprocessor test (extension test 1)',
                    'Preprocessor test (extension test 2)',
                    'Preprocessor test (extension test 3)',
                    'Preprocessor test (11)',
                    'undefined variable',
                    'if (boolean-scalar) check',
                    'break with no loop',
                    'continue with no loop',
                    'illegal assignment',
                    'syntax error check (1)',
                    'syntax error check (2)',
                    'syntax error check (3)',
                    'TIntermediate::addUnaryMath',
                    'GLSL 1.30 precision qualifiers',
                    'GLSL 1.20 invariant, centroid qualifiers',
                    'Divide by zero',
                    'gl_Position not written check',
                    'varying var mismatch',
                    'varying read but not written',
                    'texcoord varying']

glean_fp_tests = ['ABS test',
                  'ADD test',
                  'ADD with saturation',
                  'ADD an immediate',
                  'ADD negative immediate',
                  'ADD negative immediate (2)',
                  'CMP test',
                  'COS test',
                  'COS test 2',
                  'DP3 test',
                  'DP3 test (2)',
                  'DP4 test',
                  'DPH test',
                  'DST test',
                  'EX2 test',
                  'FLR test',
                  'FRC test',
                  'LG2 test',
                  'LIT test 1',
                  'LIT test 2 (degenerate case: 0 ^ 0 -> 1)',
                  'LIT test 3 (case x < 0)',
                  'MAD test',
                  'MAX test',
                  'MIN test',
                  'MOV test',
                  'MUL test',
                  'masked MUL test',
                  'POW test (exponentiation)',
                  'RCP test (reciprocal)',
                  'RCP test 2 (reciprocal)',
                  'RSQ test 1 (reciprocal square root)',
                  'RSQ test 2 (reciprocal square root of negative value)',
                  'SCS test',
                  'SGE test',
                  'SIN test',
                  'SIN test 2',
                  'SLT test',
                  'SUB test (with swizzle)',
                  'SUB with saturation',
                  'SWZ test',
                  'swizzled move test',
                  'swizzled add test',
                  'XPD test 1',
                  'Z-write test',
                  'Divide by zero test',
                  'Infinity and nan test',
                  'ARB_fog_linear test',
                  'Computed fog linear test',
                  'ARB_fog_exp test',
                  'Computed fog exp test',
                  'ARB_fog_exp2 test',
                  'Computed fog exp2 test']

glean_vp_tests = ['ABS test',
                  'ADD test',
                  'ARL test',
                  'DP3 test',
                  'DP4 test',
                  'DPH test',
                  'DST test',
                  'EX2 test',
                  'EXP test',
                  'FLR test',
                  'FRC test',
                  'LG2 test',
                  'LIT test 1',
                  'LIT test 2 (degenerate case: 0 ^ 0 -> 1)',
                  'LIT test 3 (case x < 0)',
                  'LOG test',
                  'MAD test',
                  'MAX test',
                  'MIN test',
                  'MOV test (with swizzle)',
                  'MUL test (with swizzle and masking)',
                  'POW test (exponentiation)',
                  'RCP test (reciprocal)',
                  'RSQ test 1 (reciprocal square root)',
                  'RSQ test 2 (reciprocal square root of negative value)',
                  'SGE test',
                  'SLT test',
                  'SUB test (with swizzle)',
                  'SWZ test 1',
                  'SWZ test 2',
                  'SWZ test 3',
                  'SWZ test 4',
                  'SWZ test 5',
                  'XPD test 1',
                  'XPD test 2 (same src and dst arg)',
                  'Position write test (compute position from texcoord)',
                  'Z-write test',
                  'State reference test 1 (material ambient)',
                  'State reference test 2 (light products)',
                  'State reference test 3 (fog params)',
                  'Divide by zero test',
                  'Infinity and nan test']

for pairs in [(['glsl1'], glean_glsl_tests),
              (['fragProg1'], glean_fp_tests),
              (['vertProg1'], glean_vp_tests)]:
    for prefix, name in itertools.product(*pairs):
        groupname = "glean/{0}-{1}".format(prefix, name)
        profile.tests[groupname] = GleanTest(prefix)
        profile.tests[groupname].env['PIGLIT_TEST'] = name

def add_fbo_formats_tests(path, extension, suffix=''):
    profile.tests[path + '/fbo-generatemipmap-formats' + suffix] = concurrent_test('fbo-generatemipmap-formats ' + extension)
    profile.tests[path + '/fbo-clear-formats' + suffix] = concurrent_test('fbo-clear-formats ' + extension)
    profile.tests[path + '/get-renderbuffer-internalformat' + suffix] = concurrent_test('get-renderbuffer-internalformat ' + extension)
    if 'depth' not in extension:
        profile.tests[path + '/fbo-blending-formats' + suffix] = concurrent_test('fbo-blending-formats ' + extension)
        profile.tests[path + '/fbo-alphatest-formats' + suffix] = concurrent_test('fbo-alphatest-formats ' + extension)
        profile.tests[path + '/fbo-colormask-formats' + suffix] = concurrent_test('fbo-colormask-formats ' + extension)

def add_msaa_formats_tests(group, extension):
    for num_samples in MSAA_SAMPLE_COUNTS:
        args = [str(num_samples), extension]
        test_name = ' '.join(['multisample-formats'] + args)
        group[test_name] = concurrent_test(
                'ext_framebuffer_multisample-formats ' + ' '.join(args))

def add_fbo_generatemipmap_extension(group, extension, name):
    group[name] = concurrent_test('fbo-generatemipmap-formats ' + extension)

def add_fbo_clear_extension(group, extension, name):
    group[name] = concurrent_test('fbo-clear-formats ' + extension)

def add_fbo_blending_extension(group, extension, name):
    group[name] = concurrent_test('fbo-blending-formats ' + extension)

def add_fbo_alphatest_extension(group, extension, name):
    group[name] = concurrent_test('fbo-alphatest-formats ' + extension)


def add_fbo_rg(group, format):
    name = "fbo-rg-" + format
    group[name] = concurrent_test('fbo-rg ' + format)

security = {}
add_plain_test(security, 'initialized-texmemory')
add_plain_test(security, 'initialized-fbo')
add_plain_test(security, 'initialized-vbo')


shaders = {}

def add_getactiveuniform_count(group, name, expected):
    path = 'shaders/'
    group['glsl-getactiveuniform-count: ' + name] = concurrent_test(
        'glsl-getactiveuniform-count ' + path + name + '.vert ' + expected)

add_shader_test_dir(shaders,
                    testsDir + '/shaders',
                    recursive=True)
add_concurrent_test(shaders, 'activeprogram-bad-program')
add_concurrent_test(shaders, 'activeprogram-get')
add_concurrent_test(shaders, 'attribute0')
add_concurrent_test(shaders, 'createshaderprogram-bad-type')
add_concurrent_test(shaders, 'createshaderprogram-attached-shaders')
add_concurrent_test(shaders, 'glsl-arb-fragment-coord-conventions')
add_concurrent_test(shaders, 'glsl-arb-fragment-coord-conventions-define')
add_concurrent_test(shaders, 'glsl-bug-22603')
add_concurrent_test(shaders, 'glsl-bindattriblocation')
add_concurrent_test(shaders, 'glsl-dlist-getattriblocation')
add_concurrent_test(shaders, 'glsl-getactiveuniform-array-size')
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-length', '1')
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-ftransform', '2')
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-mvp', '2')
add_concurrent_test(shaders, 'glsl-getactiveuniform-length')
add_concurrent_test(shaders, 'glsl-getattriblocation')
add_concurrent_test(shaders, 'getuniform-01')
add_concurrent_test(shaders, 'getuniform-02')
add_concurrent_test(shaders, 'glsl-invalid-asm-01')
add_concurrent_test(shaders, 'glsl-invalid-asm-02')
add_concurrent_test(shaders, 'glsl-novertexdata')
add_concurrent_test(shaders, 'glsl-preprocessor-comments')
add_concurrent_test(shaders, 'glsl-reload-source')
add_concurrent_test(shaders, 'glsl-uniform-out-of-bounds')
add_concurrent_test(shaders, 'glsl-uniform-out-of-bounds-2')
add_concurrent_test(shaders, 'glsl-uniform-update')
add_concurrent_test(shaders, 'glsl-unused-varying')
add_concurrent_test(shaders, 'glsl-fs-bug25902')
add_concurrent_test(shaders, 'glsl-fs-color-matrix')
add_concurrent_test(shaders, 'glsl-fs-discard-02')
add_concurrent_test(shaders, 'glsl-fs-exp2')
add_concurrent_test(shaders, 'glsl-fs-flat-color')
add_concurrent_test(shaders, 'glsl-fs-fogcolor-statechange')
add_concurrent_test(shaders, 'glsl-fs-fogscale')
add_concurrent_test(shaders, 'glsl-fs-fragcoord')
add_concurrent_test(shaders, 'glsl-fs-fragcoord-zw-ortho')
add_concurrent_test(shaders, 'glsl-fs-fragcoord-zw-perspective')
add_concurrent_test(shaders, 'glsl-fs-loop')
add_concurrent_test(shaders, 'glsl-fs-loop-nested')
add_concurrent_test(shaders, 'glsl-fs-pointcoord')
add_concurrent_test(shaders, 'glsl-fs-raytrace-bug27060')
add_concurrent_test(shaders, 'glsl-fs-sampler-numbering')
add_concurrent_test(shaders, 'glsl-fs-shader-stencil-export')
add_concurrent_test(shaders, 'glsl-fs-sqrt-branch')
add_concurrent_test(shaders, 'glsl-fs-texturecube')
shaders['glsl-fs-texturecube-bias'] = concurrent_test('glsl-fs-texturecube -bias')
add_concurrent_test(shaders, 'glsl-fs-texturecube-2')
shaders['glsl-fs-texturecube-2-bias'] = concurrent_test('glsl-fs-texturecube-2 -bias')
add_concurrent_test(shaders, 'glsl-fs-textureenvcolor-statechange')
add_concurrent_test(shaders, 'glsl-fs-texture2drect');
shaders['glsl-fs-texture2drect-proj3'] = concurrent_test('glsl-fs-texture2drect -proj3')
shaders['glsl-fs-texture2drect-proj4'] = concurrent_test('glsl-fs-texture2drect -proj4')
add_concurrent_test(shaders, 'glsl-fs-user-varying-ff')
add_concurrent_test(shaders, 'glsl-mat-attribute')
shaders['glsl-max-varyings'] = concurrent_test('glsl-max-varyings')
shaders['glsl-max-varyings >MAX_VARYING_COMPONENTS'] = concurrent_test('glsl-max-varyings --exceed-limits')
add_concurrent_test(shaders, 'glsl-orangebook-ch06-bump')
add_concurrent_test(shaders, 'glsl-routing')
add_concurrent_test(shaders, 'glsl-vs-arrays')
add_concurrent_test(shaders, 'glsl-vs-normalscale')
add_concurrent_test(shaders, 'glsl-vs-functions')
add_concurrent_test(shaders, 'glsl-vs-user-varying-ff')
add_concurrent_test(shaders, 'glsl-vs-texturematrix-1')
add_concurrent_test(shaders, 'glsl-vs-texturematrix-2')
add_concurrent_test(shaders, 'glsl-sin')
add_concurrent_test(shaders, 'glsl-cos')
add_concurrent_test(shaders, 'glsl-vs-if-bool')
add_concurrent_test(shaders, 'glsl-vs-loop')
add_concurrent_test(shaders, 'glsl-vs-loop-nested')
add_concurrent_test(shaders, 'glsl-vs-mov-after-deref')
add_concurrent_test(shaders, 'glsl-vs-mvp-statechange')
add_concurrent_test(shaders, 'glsl-vs-raytrace-bug26691')
add_concurrent_test(shaders, 'glsl-vs-statechange-1')
add_concurrent_test(shaders, 'vp-combined-image-units')
add_concurrent_test(shaders, 'glsl-derivs')
add_concurrent_test(shaders, 'glsl-deriv-varyings')
add_concurrent_test(shaders, 'glsl-fwidth')
add_concurrent_test(shaders, 'glsl-lod-bias')
add_concurrent_test(shaders, 'vp-ignore-input')
add_concurrent_test(shaders, 'glsl-empty-vs-no-fs')
add_concurrent_test(shaders, 'glsl-useprogram-displaylist')
add_concurrent_test(shaders, 'glsl-vs-point-size')
add_concurrent_test(shaders, 'glsl-light-model')
add_concurrent_test(shaders, 'glsl-link-bug30552')
add_concurrent_test(shaders, 'glsl-link-bug38015')
add_concurrent_test(shaders, 'glsl-link-empty-prog-01')
add_concurrent_test(shaders, 'glsl-link-empty-prog-02')
shaders['GLSL link single global initializer, 2 shaders'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-01a.vert shaders/glsl-link-initializer-01b.vert pass')
shaders['GLSL link matched global initializer, 2 shaders'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-01c.vert shaders/glsl-link-initializer-01d.vert pass')
shaders['GLSL link mismatched global initializer, 2 shaders'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-01b.vert shaders/glsl-link-initializer-01d.vert fail')
shaders['GLSL link mismatched global initializer, 3 shaders'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-01a.vert shaders/glsl-link-initializer-01b.vert shaders/glsl-link-initializer-01c.vert fail')
shaders['GLSL link mismatched global const initializer'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-02a.vert shaders/glsl-link-initializer-02b.vert fail')
shaders['GLSL link two programs, global initializer'] = concurrent_test('glsl-link-initializer-03')
shaders['GLSL link matched global initializer expression'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-05a.vert shaders/glsl-link-initializer-05b.vert fail')
shaders['GLSL link mismatched global initializer expression'] = concurrent_test('glsl-link-test shaders/glsl-link-initializer-06a.vert shaders/glsl-link-initializer-06b.vert fail')
shaders['GLSL link mismatched invariant'] = concurrent_test('glsl-link-test shaders/glsl-link-invariant-01a.vert shaders/glsl-link-invariant-01b.vert fail')
shaders['GLSL link mismatched centroid'] = concurrent_test('glsl-link-test shaders/glsl-link-centroid-01a.vert shaders/glsl-link-centroid-01b.vert fail')
shaders['GLSL link array-of-struct-of-array'] = concurrent_test('glsl-link-test shaders/glsl-link-struct-array.frag pass')
add_concurrent_test(shaders, 'glsl-max-vertex-attrib')
add_concurrent_test(shaders, 'glsl-kwin-blur-1')
add_concurrent_test(shaders, 'glsl-kwin-blur-2')
add_concurrent_test(shaders, 'gpu_shader4_attribs')
add_concurrent_test(shaders, 'link-unresolved-function')
add_concurrent_test(shaders, 'sso-simple')
add_concurrent_test(shaders, 'sso-uniforms-01')
add_concurrent_test(shaders, 'sso-uniforms-02')
add_concurrent_test(shaders, 'sso-user-varying-01')
add_concurrent_test(shaders, 'sso-user-varying-02')
add_concurrent_test(shaders, 'useprogram-flushverts-1')
add_concurrent_test(shaders, 'useprogram-flushverts-2')
add_concurrent_test(shaders, 'useprogram-inside-begin')
add_concurrent_test(shaders, 'useprogram-refcount-1')
add_concurrent_test(shaders, 'useshaderprogram-bad-type')
add_concurrent_test(shaders, 'useshaderprogram-bad-program')
add_concurrent_test(shaders, 'useshaderprogram-flushverts-1')
for subtest in ('interstage', 'intrastage', 'vs-gs'):
    cmdline = 'version-mixing {0}'.format(subtest)
    shaders[cmdline] = concurrent_test(cmdline)

def add_vpfpgeneric(group, name):
    group[name] = concurrent_test('vpfp-generic ' +
        os.path.join(testsDir, 'shaders', 'generic', name + '.vpfp'))

glx = {}
add_msaa_visual_plain_tests(glx, 'glx-copy-sub-buffer')
add_plain_test(glx, 'glx-destroycontext-1')
add_plain_test(glx, 'glx-destroycontext-2')
add_plain_test(glx, 'glx-dont-care-mask')
add_plain_test(glx, 'glx-close-display')
add_concurrent_test(glx, 'glx-fbconfig-sanity')
add_concurrent_test(glx, 'glx-fbconfig-compliance')
add_plain_test(glx, 'glx-fbo-binding')
add_plain_test(glx, 'glx-multi-context-ib-1')
add_plain_test(glx, 'glx-multithread')
add_plain_test(glx, 'glx-multithread-texture')
add_plain_test(glx, 'glx-multithread-makecurrent-1')
add_plain_test(glx, 'glx-multithread-makecurrent-2')
add_plain_test(glx, 'glx-multithread-makecurrent-3')
add_plain_test(glx, 'glx-multithread-makecurrent-4')
add_concurrent_test(glx, 'glx-multithread-shader-compile')
add_plain_test(glx, 'glx-shader-sharing')
add_plain_test(glx, 'glx-swap-exchange')
glx['glx-swap-event_event'] = plain_test('glx-swap-event --event')
glx['glx-swap-event_async'] = plain_test('glx-swap-event --async')
glx['glx-swap-event_interval'] = plain_test('glx-swap-event --interval')
add_plain_test(glx, 'glx-swap-pixmap')
add_plain_test(glx, 'glx-swap-pixmap-bad')
add_plain_test(glx, 'glx-swap-singlebuffer')
add_plain_test(glx, 'glx-make-current')
add_plain_test(glx, 'glx-make-glxdrawable-current')
add_plain_test(glx, 'glx-buffer-age')
glx['glx-buffer-age vblank_mode=0'] = plain_test('glx-buffer-age')
glx['glx-buffer-age vblank_mode=0'].env['vblank_mode'] = '0'
add_concurrent_test(glx, 'glx-pixmap-life')
add_concurrent_test(glx, 'glx-pixmap13-life')
add_concurrent_test(glx, 'glx-pixmap-multi')
add_plain_test(glx, 'glx-tfp')
add_plain_test(glx, 'glx-visuals-depth')
add_concurrent_test(glx, 'glx-visuals-depth -pixmap')
add_plain_test(glx, 'glx-visuals-stencil')
add_concurrent_test(glx, 'glx-visuals-stencil -pixmap')
add_concurrent_test(glx, 'glx-window-life')
add_concurrent_test(glx, 'glx-pixmap-crosscheck')
glx['glx-query-drawable-GLXWINDOW-GLX_WIDTH'] = plain_test('glx-query-drawable --attr=GLX_WIDTH --type=GLXWINDOW')
glx['glx-query-drawable-GLXWINDOW-GLX_HEIGHT'] = plain_test('glx-query-drawable --attr=GLX_HEIGHT --type=GLXWINDOW')
glx['glx-query-drawable-GLXPIXMAP-GLX_WIDTH'] = plain_test('glx-query-drawable --attr=GLX_WIDTH --type=GLXPIXMAP')
glx['glx-query-drawable-GLXPIXMAP-GLX_HEIGHT'] = plain_test('glx-query-drawable --attr=GLX_HEIGHT --type=GLXPIXMAP')
glx['glx-query-drawable-GLXPBUFFER-GLX_WIDTH'] = plain_test('glx-query-drawable --attr=GLX_WIDTH --type=GLXPBUFFER')
glx['glx-query-drawable-GLXPBUFFER-GLX_HEIGHT'] = plain_test('glx-query-drawable --attr=GLX_HEIGHT --type=GLXPBUFFER')
glx['glx-query-drawable-GLX_WIDTH'] = plain_test('glx-query-drawable --attr=GLX_WIDTH --type=WINDOW')
glx['glx-query-drawable-GLX_HEIGHT'] = plain_test('glx-query-drawable --attr=GLX_HEIGHT --type=WINDOW')
glx['glx-query-drawable-GLX_FBCONFIG_ID-WINDOW'] = plain_test('glx-query-drawable --attr=GLX_FBCONFIG_ID --type=WINDOW')
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXWINDOW'] = plain_test('glx-query-drawable --attr=GLX_FBCONFIG_ID --type=GLXWINDOW')
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXPIXMAP'] = plain_test('glx-query-drawable --attr=GLX_FBCONFIG_ID --type=GLXPIXMAP')
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXPBUFFER'] = plain_test('glx-query-drawable --attr=GLX_FBCONFIG_ID --type=GLXPBUFFER')
glx['glx-query-drawable-GLX_PRESERVED_CONTENTS'] = plain_test('glx-query-drawable --attr=GLX_FBCONFIG_ID --type=GLXPBUFFER')
glx['glx-query-drawable-GLXBadDrawable'] = plain_test('glx-query-drawable --bad-drawable')
glx['extension string sanity'] = concurrent_test('glx-string-sanity')

import_context = {};
glx['GLX_EXT_import_context'] = import_context
import_context['free context'] = PiglitTest(['glx-free-context'])
import_context['get context ID'] = PiglitTest(['glx-get-context-id'])
import_context['get current display'] = PiglitTest(['glx-get-current-display-ext'])
import_context['imported context has same context ID'] = PiglitTest(['glx-import-context-has-same-context-id'])
import_context['import context, multi process'] = PiglitTest(['glx-import-context-multi-process'])
import_context['import context, single process'] = PiglitTest(['glx-import-context-single-process'])
import_context['make current, multi process'] = PiglitTest(['glx-make-current-multi-process'])
import_context['make current, single process'] = PiglitTest(['glx-make-current-single-process'])
import_context['query context info'] = PiglitTest(['glx-query-context-info-ext'])

create_context = {};
glx['GLX_ARB_create_context'] = create_context
create_context['current with no framebuffer'] = concurrent_test('glx-create-context-current-no-framebuffer')
create_context['default major version'] = concurrent_test('glx-create-context-default-major-version')
create_context['default minor version'] = concurrent_test('glx-create-context-default-minor-version')
create_context['invalid attribute'] = concurrent_test('glx-create-context-invalid-attribute')
create_context['invalid flag'] = concurrent_test('glx-create-context-invalid-flag')
create_context['forward-compatible flag with pre-3.0'] = concurrent_test('glx-create-context-invalid-flag-forward-compatible')
create_context['invalid OpenGL version'] = concurrent_test('glx-create-context-invalid-gl-version')
create_context['invalid render type'] = concurrent_test('glx-create-context-invalid-render-type')
create_context['color-index render type with 3.0'] = concurrent_test('glx-create-context-invalid-render-type-color-index')
create_context['empty attribute list'] = concurrent_test('glx-create-context-valid-attribute-empty')
create_context['NULL attribute list'] = concurrent_test('glx-create-context-valid-attribute-null')
create_context['forward-compatible flag with 3.0'] = concurrent_test('glx-create-context-valid-flag-forward-compatible')

create_context_profile = {};
glx['GLX_ARB_create_context_profile'] = create_context_profile
create_context_profile['3.2 core profile required'] = concurrent_test('glx-create-context-core-profile')
create_context_profile['invalid profile'] = concurrent_test('glx-create-context-invalid-profile')
create_context_profile['pre-GL3.2 profile'] = concurrent_test('glx-create-context-pre-GL32-profile')

create_context_robustness = {};
glx['GLX_ARB_create_context_robustness'] = create_context_robustness
create_context_robustness['invalid reset notification strategy'] = concurrent_test('glx-create-context-invalid-reset-strategy')
create_context_robustness['require GL_ARB_robustness'] = concurrent_test('glx-create-context-require-robustness')

create_context_es2_profile = {};
glx['GLX_EXT_create_context_es2_profile'] = create_context_es2_profile
create_context_es2_profile['indirect rendering ES2 profile'] = concurrent_test('glx-create-context-indirect-es2-profile')
create_context_es2_profile['invalid OpenGL ES version'] = concurrent_test('glx-create-context-invalid-es-version')

oml_sync_control = {};
glx['GLX_OML_sync_control'] = oml_sync_control
oml_sync_control['glXGetMscRateOML'] = concurrent_test('glx-oml-sync-control-getmscrate')
oml_sync_control['swapbuffersmsc-divisor-zero'] = concurrent_test('glx-oml-sync-control-swapbuffersmsc-divisor-zero')
oml_sync_control['swapbuffersmsc-return'] = concurrent_test('glx-oml-sync-control-swapbuffersmsc-return')
oml_sync_control['swapbuffersmsc-return swap_interval 0'] = concurrent_test('glx-oml-sync-control-swapbuffersmsc-return 0')
oml_sync_control['swapbuffersmsc-return swap_interval 1'] = concurrent_test('glx-oml-sync-control-swapbuffersmsc-return 1')
oml_sync_control['waitformsc'] = concurrent_test('glx-oml-sync-control-waitformsc')

mesa_query_renderer = {}
glx['GLX_MESA_query_renderer'] = mesa_query_renderer
mesa_query_renderer['coverage'] = concurrent_test('glx-query-renderer-coverage')

def texwrap_test(args):
    return concurrent_test('texwrap ' + ' '.join(args))

def add_texwrap_target_tests(group, target):
    group['texwrap ' + target] = texwrap_test([target, 'GL_RGBA8'])
    group['texwrap ' + target + ' bordercolor'] = texwrap_test([target, 'GL_RGBA8', 'bordercolor'])
    group['texwrap ' + target + ' proj'] = texwrap_test([target, 'GL_RGBA8', 'proj'])
    group['texwrap ' + target + ' proj bordercolor'] = texwrap_test([target, 'GL_RGBA8', 'proj', 'bordercolor'])

def add_texwrap_format_tests(group, ext = '', suffix = ''):
    args = [] if ext == '' else [ext]
    group['texwrap formats' + suffix] = texwrap_test(args)
    group['texwrap formats' + suffix + ' bordercolor'] = texwrap_test(args + ['bordercolor'])
    group['texwrap formats' + suffix + ' bordercolor-swizzled'] = texwrap_test(args + ['bordercolor', 'swizzled'])

def add_fbo_depth_tests(group, format):
    group['fbo-depth-' + format + '-tex1d'] = concurrent_test('fbo-depth-tex1d ' + format)
    group['fbo-depth-' + format + '-clear'] = concurrent_test('fbo-depth clear ' + format)
    group['fbo-depth-' + format + '-readpixels'] = concurrent_test('fbo-depth readpixels ' + format)
    group['fbo-depth-' + format + '-drawpixels'] = concurrent_test('fbo-depth drawpixels ' + format)
    group['fbo-depth-' + format + '-copypixels'] = concurrent_test('fbo-depth copypixels ' + format)
    group['fbo-depth-' + format + '-blit'] = concurrent_test('fbo-depth blit ' + format)

def add_fbo_stencil_tests(group, format):
    group['fbo-stencil-' + format + '-clear'] = concurrent_test('fbo-stencil clear ' + format)
    group['fbo-stencil-' + format + '-readpixels'] = concurrent_test('fbo-stencil readpixels ' + format)
    group['fbo-stencil-' + format + '-drawpixels'] = concurrent_test('fbo-stencil drawpixels ' + format)
    group['fbo-stencil-' + format + '-copypixels'] = concurrent_test('fbo-stencil copypixels ' + format)
    group['fbo-stencil-' + format + '-blit'] = concurrent_test('fbo-stencil blit ' + format)

spec = {}

gl11 = {}
spec['!OpenGL 1.1'] = gl11
add_texwrap_target_tests(gl11, '1D')
add_texwrap_target_tests(gl11, '2D')
add_texwrap_format_tests(gl11)
gl11['copyteximage 1D'] = PiglitTest(['copyteximage', '-auto', '1D'])
gl11['copyteximage 2D'] = PiglitTest(['copyteximage', '-auto', '2D'])
add_plain_test(gl11, 'drawbuffer-modes')
add_plain_test(gl11, 'fdo10370')
add_plain_test(gl11, 'fdo23489')
add_plain_test(gl11, 'fdo23670-depth_test')
add_plain_test(gl11, 'fdo23670-drawpix_stencil')
add_plain_test(gl11, 'r300-readcache')
add_plain_test(gl11, 'tri-tex-crash')
add_plain_test(gl11, 'vbo-buffer-unmap')
add_plain_test(gl11, 'array-stride')
add_plain_test(gl11, 'clear-accum')
add_concurrent_test(gl11, 'clipflat')
add_plain_test(gl11, 'copypixels-draw-sync')
add_plain_test(gl11, 'copypixels-sync')
add_plain_test(gl11, 'degenerate-prims')
add_plain_test(gl11, 'depthfunc')
add_plain_test(gl11, 'depthrange-clear')
add_plain_test(gl11, 'dlist-clear')
add_plain_test(gl11, 'dlist-color-material')
add_plain_test(gl11, 'dlist-fdo3129-01')
add_plain_test(gl11, 'dlist-fdo3129-02')
add_plain_test(gl11, 'dlist-fdo31590')
add_plain_test(gl11, 'draw-arrays-colormaterial')
add_plain_test(gl11, 'draw-copypixels-sync')
add_concurrent_test(gl11, 'draw-pixel-with-texture')
add_msaa_visual_plain_tests(gl11, 'draw-pixels')
add_concurrent_test(gl11, 'drawpix-z')
add_plain_test(gl11, 'fog-modes')
add_plain_test(gl11, 'fragment-center')
add_fbo_depthstencil_msaa_visual_tests(gl11, 'default_fb')
add_plain_test(gl11, 'geterror-invalid-enum')
add_plain_test(gl11, 'geterror-inside-begin')
add_concurrent_test(gl11, 'glinfo')
add_plain_test(gl11, 'hiz')
add_plain_test(gl11, 'infinite-spot-light')
add_plain_test(gl11, 'line-aa-width')
add_concurrent_test(gl11, 'line-flat-clip-color')
add_plain_test(gl11, 'lineloop')
add_plain_test(gl11, 'linestipple')
add_plain_test(gl11, 'longprim')
add_concurrent_test(gl11, 'masked-clear')
add_plain_test(gl11, 'point-line-no-cull')
add_plain_test(gl11, 'polygon-mode')
add_concurrent_test(gl11, 'polygon-mode-offset')
add_plain_test(gl11, 'polygon-offset')
add_concurrent_test(gl11, 'push-pop-texture-state')
add_concurrent_test(gl11, 'quad-invariance')
add_msaa_visual_plain_tests(gl11, 'read-front')
add_msaa_visual_plain_tests(gl11, 'read-front clear-front-first')
add_concurrent_test(gl11, 'readpix-z')
add_plain_test(gl11, 'roundmode-getintegerv')
add_plain_test(gl11, 'roundmode-pixelstore')
add_plain_test(gl11, 'scissor-bitmap')
add_plain_test(gl11, 'scissor-clear')
add_plain_test(gl11, 'scissor-copypixels')
add_plain_test(gl11, 'scissor-depth-clear')
add_plain_test(gl11, 'scissor-many')
add_plain_test(gl11, 'scissor-offscreen')
add_concurrent_test(gl11, 'scissor-polygon')
add_plain_test(gl11, 'scissor-stencil-clear')
gl11['GL_SELECT - no test function'] = PiglitTest(['select', 'gl11'])
gl11['GL_SELECT - depth-test enabled'] = PiglitTest(['select', 'depth'])
gl11['GL_SELECT - stencil-test enabled'] = PiglitTest(['select', 'stencil'])
gl11['GL_SELECT - alpha-test enabled'] = PiglitTest(['select', 'alpha'])
gl11['GL_SELECT - scissor-test enabled'] = PiglitTest(['select', 'scissor'])
add_plain_test(gl11, 'stencil-drawpixels')
add_plain_test(gl11, 'texgen')
add_plain_test(gl11, 'two-sided-lighting')
add_plain_test(gl11, 'user-clip')
add_plain_test(gl11, 'varray-disabled')
add_plain_test(gl11, 'windowoverlap')
add_plain_test(gl11, 'copyteximage-border')
add_plain_test(gl11, 'copyteximage-clipping')
add_plain_test(gl11, 'copytexsubimage')
add_plain_test(gl11, 'getteximage-formats')
add_plain_test(gl11, 'getteximage-luminance')
add_plain_test(gl11, 'getteximage-simple')
gl11['incomplete-texture-fixed'] = concurrent_test('incomplete-texture -auto fixed')
add_plain_test(gl11, 'max-texture-size')
add_concurrent_test(gl11, 'max-texture-size-level')
add_concurrent_test(gl11, 'proxy-texture')
add_concurrent_test(gl11, 'sized-texture-format-channels')
add_plain_test(gl11, 'streaming-texture-leak')
add_plain_test(gl11, 'texredefine')
add_plain_test(gl11, 'texsubimage')
add_plain_test(gl11, 'texture-al')
add_concurrent_test(gl11, 'triangle-guardband-viewport')
add_concurrent_test(gl11, 'getteximage-targets 1D')
add_concurrent_test(gl11, 'getteximage-targets 2D')

gl10 = {}
spec['!OpenGL 1.0'] = gl10
add_concurrent_test(gl10, 'gl-1.0-beginend-coverage')
add_concurrent_test(gl10, 'gl-1.0-dlist-beginend')
add_concurrent_test(gl10, 'gl-1.0-dlist-shademodel')
add_concurrent_test(gl10, 'gl-1.0-edgeflag')
add_concurrent_test(gl10, 'gl-1.0-edgeflag-const')
add_concurrent_test(gl10, 'gl-1.0-edgeflag-quads')
add_concurrent_test(gl10, 'gl-1.0-long-dlist')
add_concurrent_test(gl10, 'gl-1.0-rendermode-feedback')
add_plain_test(gl10, 'gl-1.0-front-invalidate-back')
add_plain_test(gl10, 'gl-1.0-swapbuffers-behavior')

gl12 = {}
spec['!OpenGL 1.2'] = gl12
add_texwrap_target_tests(gl12, '3D')
add_msaa_visual_plain_tests(gl12, 'copyteximage 3D')
add_plain_test(gl12, 'crash-texparameter-before-teximage')
add_plain_test(gl12, 'draw-elements-vs-inputs')
add_plain_test(gl12, 'two-sided-lighting-separate-specular')
add_plain_test(gl12, 'levelclamp')
add_plain_test(gl12, 'lodclamp')
add_plain_test(gl12, 'lodclamp-between')
add_plain_test(gl12, 'lodclamp-between-max')
add_plain_test(gl12, 'mipmap-setup')
add_plain_test(gl12, 'tex-skipped-unit')
add_plain_test(gl12, 'tex3d')
add_plain_test(gl12, 'tex3d-maxsize')
add_plain_test(gl12, 'teximage-errors')
add_plain_test(gl12, 'texture-packed-formats')
add_concurrent_test(gl12, 'getteximage-targets 3D')

gl13 = {}
spec['!OpenGL 1.3'] = gl13
add_plain_test(gl13, 'texunits')
add_plain_test(gl13, 'tex-border-1')
add_concurrent_test(gl13, 'tex3d-depth1')

gl14 = {}
spec['!OpenGL 1.4'] = gl14
add_plain_test(gl14, 'fdo25614-genmipmap')
add_plain_test(gl14, 'tex1d-2dborder')
add_plain_test(gl14, 'blendminmax')
add_plain_test(gl14, 'blendsquare')
add_concurrent_test(gl14, "gl-1.4-dlist-multidrawarrays")
add_msaa_visual_plain_tests(gl14, 'copy-pixels')
add_plain_test(gl14, 'draw-batch')
add_plain_test(gl14, 'stencil-wrap')
add_plain_test(gl14, 'triangle-rasterization')
gl14['triangle-rasterization-fbo'] = PiglitTest(['triangle-rasterization', '-auto', '-use_fbo'])
add_plain_test(gl14, 'triangle-rasterization-overdraw')
gl14['tex-miplevel-selection'] = concurrent_test('tex-miplevel-selection -nobias -nolod')
gl14['tex-miplevel-selection-lod'] = concurrent_test('tex-miplevel-selection -nobias')
gl14['tex-miplevel-selection-lod-bias'] = concurrent_test('tex-miplevel-selection')

gl15 = {}
spec['!OpenGL 1.5'] = gl15
add_plain_test(gl15, 'draw-elements')
gl15['draw-elements-user'] = PiglitTest(['draw-elements', '-auto', 'user'])
add_plain_test(gl15, 'draw-vertices')
gl15['draw-vertices-user'] = PiglitTest(['draw-vertices', '-auto', 'user'])
add_plain_test(gl15, 'isbufferobj')
add_plain_test(gl15, 'depth-tex-compare')
gl15['normal3b3s-invariance-byte'] = PiglitTest(['gl-1.5-normal3b3s-invariance', 'GL_BYTE', '-auto'])
gl15['normal3b3s-invariance-short'] = PiglitTest(['gl-1.5-normal3b3s-invariance', 'GL_SHORT', '-auto'])

gl20 = {}
spec['!OpenGL 2.0'] = gl20
add_concurrent_test(gl20, 'attribs')
add_concurrent_test(gl20, 'gl-2.0-edgeflag')
add_concurrent_test(gl20, 'gl-2.0-edgeflag-immediate')
add_concurrent_test(gl20, 'gl-2.0-vertexattribpointer')
add_plain_test(gl20, 'attrib-assignments')
add_plain_test(gl20, 'getattriblocation-conventional')
add_plain_test(gl20, 'clip-flag-behavior')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front back front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front back front2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front back back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front back')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front front2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front')
add_concurrent_test(gl20, 'vertex-program-two-side enabled back front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled back front2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled back back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled back')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled front2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled back2')
add_concurrent_test(gl20, 'vertex-program-two-side enabled')
add_concurrent_test(gl20, 'vertex-program-two-side front back front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side front back front2')
add_concurrent_test(gl20, 'vertex-program-two-side front back back2')
add_concurrent_test(gl20, 'vertex-program-two-side front back')
add_concurrent_test(gl20, 'vertex-program-two-side front front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side front front2')
add_concurrent_test(gl20, 'vertex-program-two-side front back2')
add_concurrent_test(gl20, 'vertex-program-two-side front')
add_concurrent_test(gl20, 'vertex-program-two-side back front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side back front2')
add_concurrent_test(gl20, 'vertex-program-two-side back back2')
add_concurrent_test(gl20, 'vertex-program-two-side back')
add_concurrent_test(gl20, 'vertex-program-two-side front2 back2')
add_concurrent_test(gl20, 'vertex-program-two-side front2')
add_concurrent_test(gl20, 'vertex-program-two-side back2')
add_concurrent_test(gl20, 'vertex-program-two-side')
add_plain_test(gl20, 'clear-varray-2.0')
add_plain_test(gl20, 'early-z')
add_plain_test(gl20, 'occlusion-query-discard')
add_plain_test(gl20, 'stencil-twoside')
add_plain_test(gl20, 'vs-point_size-zero')
add_plain_test(gl20, 'depth-tex-modes-glsl')
add_plain_test(gl20, 'fragment-and-vertex-texturing')
gl20['incomplete-texture-glsl'] = concurrent_test('incomplete-texture -auto glsl')
add_plain_test(gl20, 'tex3d-npot')
add_concurrent_test(gl20, 'max-samplers')
add_concurrent_test(gl20, 'max-samplers border')
add_concurrent_test(gl20, "gl-2.0-active-sampler-conflict")

gl21 = {}
spec['!OpenGL 2.1'] = gl21
gl21['minmax'] = concurrent_test('gl-2.1-minmax')

gl30 = {}
spec['!OpenGL 3.0'] = gl30
gl30['attribs'] = concurrent_test('attribs GL3')
add_concurrent_test(gl30, 'bindfragdata-invalid-parameters')
add_concurrent_test(gl30, 'bindfragdata-link-error')
add_concurrent_test(gl30, 'bindfragdata-nonexistent-variable')
gl30['bound-resource-limits'] = concurrent_test('gl-3.0-bound-resource-limits')
add_concurrent_test(gl30, 'clearbuffer-depth')
add_concurrent_test(gl30, 'clearbuffer-depth-stencil')
add_plain_test(gl30, 'clearbuffer-display-lists')
add_concurrent_test(gl30, 'clearbuffer-invalid-drawbuffer')
add_concurrent_test(gl30, 'clearbuffer-invalid-buffer')
add_concurrent_test(gl30, 'clearbuffer-mixed-format')
add_concurrent_test(gl30, 'clearbuffer-stencil')
add_concurrent_test(gl30, 'genmipmap-errors')
add_concurrent_test(gl30, 'getfragdatalocation')
add_concurrent_test(gl30, 'integer-errors')
gl30['gl_VertexID used with glMultiDrawArrays'] = concurrent_test('gl-3.0-multidrawarrays-vertexid')
gl30['minmax'] = concurrent_test('gl-3.0-minmax')
gl30['render-integer'] = concurrent_test('gl-3.0-render-integer')
gl30['required-sized-texture-formats'] = concurrent_test('gl-3.0-required-sized-texture-formats 30')
gl30['required-renderbuffer-attachment-formats'] = concurrent_test('gl-3.0-required-renderbuffer-attachment-formats 30')
gl30['required-texture-attachment-formats'] = concurrent_test('gl-3.0-required-texture-attachment-formats 30')
gl30['forward-compatible-bit yes'] = concurrent_test('gl-3.0-forward-compatible-bit yes')
gl30['forward-compatible-bit no'] = concurrent_test('gl-3.0-forward-compatible-bit no')
add_concurrent_test(gl30, 'gl-3.0-texture-integer')
add_concurrent_test(gl30, 'gl-3.0-vertexattribipointer')
add_plain_test(gl30, 'gl30basic')
add_plain_test(gl30, 'array-depth-roundtrip')
add_plain_test(gl30, 'depth-cube-map')
add_plain_test(gl30, 'sampler-cube-shadow')

gl31 = {}
spec['!OpenGL 3.1'] = gl31
gl31['draw-buffers-errors'] = concurrent_test('gl-3.1-draw-buffers-errors')
gl31['genned-names'] = concurrent_test('gl-3.1-genned-names')
gl31['minmax'] = concurrent_test('gl-3.1-minmax')
for subtest in ['generated', 'written', 'flush']:
    cmdline = 'primitive-restart-xfb {0}'.format(subtest)
    gl31[cmdline] = concurrent_test('gl-3.1-' + cmdline)
gl31['required-renderbuffer-attachment-formats'] = concurrent_test('gl-3.0-required-renderbuffer-attachment-formats 31')
gl31['required-sized-texture-formats'] = concurrent_test('gl-3.0-required-sized-texture-formats 31')
gl31['required-texture-attachment-formats'] = concurrent_test('gl-3.0-required-texture-attachment-formats 31')

gl32 = {}
spec['!OpenGL 3.2'] = gl32
add_concurrent_test(gl32, 'glsl-resource-not-bound 1D')
add_concurrent_test(gl32, 'glsl-resource-not-bound 2D')
add_concurrent_test(gl32, 'glsl-resource-not-bound 3D')
add_concurrent_test(gl32, 'glsl-resource-not-bound 2DRect')
add_concurrent_test(gl32, 'glsl-resource-not-bound 1DArray')
add_concurrent_test(gl32, 'glsl-resource-not-bound 2DArray')
add_concurrent_test(gl32, 'glsl-resource-not-bound 2DMS')
add_concurrent_test(gl32, 'glsl-resource-not-bound 2DMSArray')
add_concurrent_test(gl32, 'glsl-resource-not-bound Buffer')
add_concurrent_test(gl32, 'glsl-resource-not-bound Cube')
spec['!OpenGL 3.2/minmax'] = concurrent_test('gl-3.2-minmax')
spec['!OpenGL 3.2/clear-no-buffers'] = concurrent_test('gl-3.2-clear-no-buffers')
spec['!OpenGL 3.2/depth-tex-sampling'] = concurrent_test('gl-3.2-depth-tex-sampling')
spec['!OpenGL 3.2/get-buffer-parameter-i64v'] = concurrent_test('gl-3.2-get-buffer-parameter-i64v')
spec['!OpenGL 3.2/get-integer-64iv'] = concurrent_test('gl-3.2-get-integer-64iv')
spec['!OpenGL 3.2/get-integer-64v'] = concurrent_test('gl-3.2-get-integer-64v')
spec['!OpenGL 3.2/layered-rendering/blit'] = concurrent_test('gl-3.2-layered-rendering-blit')
spec['!OpenGL 3.2/layered-rendering/clear-color'] = concurrent_test('gl-3.2-layered-rendering-clear-color')
for texture_type in ['3d', '2d_array', '2d_multisample_array', '1d_array',
                     'cube_map', 'cube_map_array']:
    for test_type in ['single_level', 'mipmapped']:
        if texture_type == '2d_multisample_array' and test_type == 'mipmapped':
            continue
        cmdline = 'clear-color-all-types {0} {1}'.format(
            texture_type, test_type)
        spec['!OpenGL 3.2/layered-rendering/' + cmdline] = \
            concurrent_test('gl-3.2-layered-rendering-' + cmdline)
spec['!OpenGL 3.2/layered-rendering/clear-color-mismatched-layer-count'] = concurrent_test('gl-3.2-layered-rendering-clear-color-mismatched-layer-count')
spec['!OpenGL 3.2/layered-rendering/clear-depth'] = concurrent_test('gl-3.2-layered-rendering-clear-depth')
spec['!OpenGL 3.2/layered-rendering/framebuffertexture'] = concurrent_test('gl-3.2-layered-rendering-framebuffertexture')
spec['!OpenGL 3.2/layered-rendering/framebuffertexture-buffer-textures'] = concurrent_test('gl-3.2-layered-rendering-framebuffertexture-buffer-textures')
spec['!OpenGL 3.2/layered-rendering/framebuffertexture-defaults'] = concurrent_test('gl-3.2-layered-rendering-framebuffertexture-defaults')
spec['!OpenGL 3.2/layered-rendering/readpixels'] = concurrent_test('gl-3.2-layered-rendering-readpixels')
spec['!OpenGL 3.2/layered-rendering/framebuffer-layer-attachment-mismatch'] = concurrent_test('gl-3.2-layered-rendering-framebuffer-layer-attachment-mismatch')
spec['!OpenGL 3.2/layered-rendering/framebuffer-layer-complete'] = concurrent_test('gl-3.2-layered-rendering-framebuffer-layer-complete')
spec['!OpenGL 3.2/layered-rendering/framebuffer-layer-count-mismatch'] = concurrent_test('gl-3.2-layered-rendering-framebuffer-layer-count-mismatch')
spec['!OpenGL 3.2/layered-rendering/framebuffer-layered-attachments'] = concurrent_test('gl-3.2-layered-rendering-framebuffer-layered-attachments')
spec['!OpenGL 3.2/layered-rendering/gl-layer'] = concurrent_test('gl-3.2-layered-rendering-gl-layer')
spec['!OpenGL 3.2/layered-rendering/gl-layer-cube-map'] = concurrent_test('gl-3.2-layered-rendering-gl-layer-cube-map')
spec['!OpenGL 3.2/layered-rendering/gl-layer-not-layered'] = concurrent_test('gl-3.2-layered-rendering-gl-layer-not-layered')
spec['!OpenGL 3.2/layered-rendering/gl-layer-render'] = concurrent_test('gl-3.2-layered-rendering-gl-layer-render')
spec['!OpenGL/coord-replace-doesnt-eliminate-frag-tex-coords'] = concurrent_test('gl-coord-replace-doesnt-eliminate-frag-tex-coords')
spec['!OpenGL/get-active-attrib-returns-all-inputs'] = concurrent_test('gl-get-active-attrib-returns-all-inputs')
spec['!OpenGL 3.2/texture-border-deprecated'] = concurrent_test('gl-3.2-texture-border-deprecated')

spec['!OpenGL 3.3/minmax'] = concurrent_test('gl-3.3-minmax')
spec['!OpenGL 3.3/required-renderbuffer-attachment-formats'] = concurrent_test('gl-3.0-required-renderbuffer-attachment-formats 33')
spec['!OpenGL 3.3/required-sized-texture-formats'] = concurrent_test('gl-3.0-required-sized-texture-formats 33')
spec['!OpenGL 3.3/required-texture-attachment-formats'] = concurrent_test('gl-3.0-required-texture-attachment-formats 33')

spec['!OpenGL 4.2/required-renderbuffer-attachment-formats'] = concurrent_test('gl-3.0-required-renderbuffer-attachment-formats 42')
spec['!OpenGL 4.2/required-sized-texture-formats'] = concurrent_test('gl-3.0-required-sized-texture-formats 42')
spec['!OpenGL 4.2/required-texture-attachment-formats'] = concurrent_test('gl-3.0-required-texture-attachment-formats 42')

# Group spec/glsl-es-1.00
spec['glsl-es-1.00'] = {}
import_glsl_parser_tests(spec['glsl-es-1.00'],
                         os.path.join(testsDir, 'spec', 'glsl-es-1.00'),
                         ['compiler'])
spec['glsl-es-1.00']['execution'] = {}
add_shader_test_dir(spec['glsl-es-1.00']['execution'],
                    os.path.join(testsDir, 'spec', 'glsl-es-1.00', 'execution'),
                    recursive=True)
spec['glsl-es-1.00']['built-in constants'] = concurrent_test('built-in-constants_gles2 ' + os.path.join(testsDir, 'spec/glsl-es-1.00/minimum-maximums.txt'))

# Group spec/glsl-1.10
spec['glsl-1.10'] = {}
import_glsl_parser_tests(spec['glsl-1.10'],
                         os.path.join(testsDir, 'spec', 'glsl-1.10'),
                         ['preprocessor', 'compiler'])
spec['glsl-1.10']['linker'] = {}
add_shader_test_dir(spec['glsl-1.10']['linker'],
                    os.path.join(testsDir, 'spec', 'glsl-1.10', 'linker'),
                    recursive=True)
spec['glsl-1.10']['execution'] = {}
add_shader_test_dir(spec['glsl-1.10']['execution'],
                    os.path.join(testsDir, 'spec', 'glsl-1.10', 'execution'),
                    recursive=True)
add_concurrent_test(spec['glsl-1.10']['execution'], 'glsl-render-after-bad-attach')
spec['glsl-1.10']['execution']['clipping'] = {}
for mode in ['fixed', 'pos_clipvert', 'clipvert_pos']:
    cmdline = 'clip-plane-transformation ' + mode
    spec['glsl-1.10']['execution']['clipping'][cmdline] = concurrent_test(cmdline)
spec['glsl-1.10']['execution']['varying-packing'] = {}
for type in ['int', 'uint', 'float', 'vec2', 'vec3', 'vec4', 'ivec2', 'ivec3',
             'ivec4', 'uvec2', 'uvec3', 'uvec4', 'mat2', 'mat3', 'mat4',
             'mat2x3', 'mat2x4', 'mat3x2', 'mat3x4', 'mat4x2', 'mat4x3']:
    for arrayspec in ['array', 'separate']:
        cmdline = 'simple {0} {1}'.format(type, arrayspec)
        spec['glsl-1.10']['execution']['varying-packing'][cmdline] = \
            concurrent_test('varying-packing-' + cmdline)
spec['glsl-1.10']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-1.10/minimum-maximums.txt'))

spec['glsl-1.10']['api'] = {}
add_concurrent_test(spec['glsl-1.10']['api'], 'getactiveattrib 110');

# Group spec/glsl-1.20
spec['glsl-1.20'] = {}
import_glsl_parser_tests(spec['glsl-1.20'],
                         os.path.join(testsDir, 'spec', 'glsl-1.20'),
                         ['preprocessor', 'compiler'])
import_glsl_parser_tests(spec['glsl-1.20'],
                         os.path.join(testsDir, 'spec', 'glsl-1.20'),
                         ['compiler'])
spec['glsl-1.20']['execution'] = {}
add_shader_test_dir(spec['glsl-1.20']['execution'],
                    os.path.join(testsDir, 'spec', 'glsl-1.20', 'execution'),
                    recursive=True)
add_shader_test_dir(spec['glsl-1.20']['execution'],
                    os.path.join(generatedTestDir, 'spec', 'glsl-1.20', 'execution'),
                    recursive=True)

def add_recursion_test(group, name):
    # When the recursion tests fail it is usually because the GLSL
    # compiler tries to recursively inline the function until the process
    # runs out of stack or the system runs out of memory.  Run the test
    # with a low rlimit to (hopefully) avoid having the test adversely
    # affect the rest of the system.  This is especially important since
    # there may be other tests running in parallel.
    #
    # This may cause false negatives on systems that map the framebuffer
    # into the processes address space.  This happens on X with DRI1 based
    # drivers, for example.
    group[name] = PiglitTest(['recursion', '-auto', '-rlimit', '268435456', name])

rec = {}
spec['glsl-1.20']['recursion'] = rec
add_recursion_test(rec, 'simple')
add_recursion_test(rec, 'unreachable')
add_recursion_test(rec, 'unreachable-constant-folding')
add_recursion_test(rec, 'indirect')
add_recursion_test(rec, 'indirect-separate')
add_recursion_test(rec, 'indirect-complex')
add_recursion_test(rec, 'indirect-complex-separate')

spec['glsl-1.20']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-1.20/minimum-maximums.txt'))
spec['glsl-1.20']['api'] = {}
add_concurrent_test(spec['glsl-1.20']['api'], 'getactiveattrib 120');

add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() 1D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() 2D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() 3D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() Cube')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() 1DShadow')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture() 2DShadow')

add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) 1D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) 2D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) 3D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) Cube')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:texture(bias) 2DShadow')

add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 1D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 2D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 3D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 1DShadow')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj 2DShadow')

add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 1D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 2D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 3D')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.20']['execution'], 'tex-miplevel-selection GL2:textureProj(bias) 2DShadow')


# Group spec/glsl-1.30
spec['glsl-1.30'] = {}
import_glsl_parser_tests(spec['glsl-1.30'],
                         os.path.join(testsDir, 'spec', 'glsl-1.30'),
                         ['preprocessor', 'compiler'])
spec['glsl-1.30']['execution'] = {}

textureSize_samplers_130 = ['sampler1D', 'sampler2D', 'sampler3D', 'samplerCube', 'sampler1DShadow', 'sampler2DShadow', 'samplerCubeShadow', 'sampler1DArray', 'sampler2DArray', 'sampler1DArrayShadow', 'sampler2DArrayShadow', 'isampler1D', 'isampler2D', 'isampler3D', 'isamplerCube', 'isampler1DArray', 'isampler2DArray', 'usampler1D', 'usampler2D', 'usampler3D', 'usamplerCube', 'usampler1DArray', 'usampler2DArray']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.30'
    # textureSize():
    for sampler in textureSize_samplers_130:
        spec['glsl-{0}/execution/textureSize/{1}-textureSize-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'textureSize {0} {1}'.format(stage, sampler))
    # texelFetch():
    for sampler in ['sampler1D', 'sampler2D', 'sampler3D', 'sampler1DArray', 'sampler2DArray', 'isampler1D', 'isampler2D', 'isampler3D', 'isampler1DArray', 'isampler2DArray', 'usampler1D', 'usampler2D', 'usampler3D', 'usampler1DArray', 'usampler2DArray']:
        spec['glsl-{0}/execution/texelFetch/{1}-texelFetch-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'texelFetch {0} {1}'.format(stage, sampler))
        spec['glsl-{0}/execution/texelFetchOffset/{1}-texelFetch-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'texelFetch offset {0} {1}'.format(stage, sampler))
    # texelFetch() with EXT_texture_swizzle mode "b0r1":
    for type in ['i', 'u', '']:
        spec['glsl-{0}/execution/texelFetch/{1}-texelFetch-{2}sampler2Darray-swizzle'.format(
                version, stage, type)] = concurrent_test(
                'texelFetch {0} {1}sampler2DArray b0r1'.format(
                        stage, type))

add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler1D 1-513')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler1DArray 1x71-501x71')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler1DArray 1x281-501x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler1DArray 71x1-71x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler1DArray 281x1-281x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2D 1x71-501x71')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2D 1x281-501x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2D 71x1-71x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2D 281x1-281x281')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler3D 1x129x9-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler3D 98x1x9-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler3D 98x129x1-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2DArray 1x129x9-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2DArray 98x1x9-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'texelFetch fs sampler2DArray 98x129x1-98x129x9')
add_concurrent_test(spec['glsl-1.30']['execution'], 'fs-texelFetch-2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'fs-texelFetchOffset-2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'fs-textureOffset-2D')
add_shader_test_dir(spec['glsl-1.30']['execution'],
                    os.path.join(testsDir, 'spec', 'glsl-1.30', 'execution'),
                    recursive=True)
spec['glsl-1.30']['linker'] = {}
spec['glsl-1.30']['linker']['clipping'] = {}
add_plain_test(spec['glsl-1.30']['linker']['clipping'], 'mixing-clip-distance-and-clip-vertex-disallowed')
add_plain_test(spec['glsl-1.30']['execution']['clipping'], 'max-clip-distances')
for arg in ['vs_basic', 'vs_xfb', 'vs_fbo', 'fs_basic', 'fs_fbo']:
    test_name = 'isinf-and-isnan ' + arg
    spec['glsl-1.30']['execution'][test_name] = PiglitTest(test_name + ' -auto')
spec['glsl-1.30']['execution']['clipping']['clip-plane-transformation pos'] = \
    concurrent_test('clip-plane-transformation pos')
spec['glsl-1.30']['texel-offset-limits'] = concurrent_test('glsl-1.30-texel-offset-limits')
add_concurrent_test(spec['glsl-1.30']['execution'], 'fs-discard-exit-2')
add_concurrent_test(spec['glsl-1.30']['execution'], 'vertexid-beginend')
add_concurrent_test(spec['glsl-1.30']['execution'], 'vertexid-drawarrays')
add_concurrent_test(spec['glsl-1.30']['execution'], 'vertexid-drawelements')
add_concurrent_test(spec['glsl-1.30']['execution'], 'fs-execution-ordering')

spec['glsl-1.30']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-1.30/minimum-maximums.txt'))
spec['glsl-1.30']['api'] = {}
add_concurrent_test(spec['glsl-1.30']['api'], 'getactiveattrib 130');

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod Cube')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod 1DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLod CubeArray')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) Cube')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) CubeShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) CubeArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture(bias) 1DArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() Cube')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() CubeShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() CubeArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 1DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection texture() CubeArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 1DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset 2DArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureOffset(bias) 1DArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2DRect_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj 2DRectShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProj(bias) 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2DRect_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjOffset(bias) 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureLodOffset 1DArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLod 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjLodOffset 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 3D')
#add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad Cube')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2DShadow')
#add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad CubeShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 1DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad 2DArrayShadow')
#add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGrad CubeArray')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 1DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2DArray')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 1DArrayShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureGradOffset 2DArrayShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2DRect_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGrad 2DShadow')

add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 1D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 1D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2D_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2DRect')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2DRect_ProjVec4')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2DRectShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 3D')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 1DShadow')
add_concurrent_test(spec['glsl-1.30']['execution'], 'tex-miplevel-selection textureProjGradOffset 2DShadow')

# Group spec/glsl-1.40
spec['glsl-1.40'] = {}
import_glsl_parser_tests(spec['glsl-1.40'],
                         os.path.join(testsDir, 'spec', 'glsl-1.40'),
                         ['compiler'])
add_shader_test_dir(spec['glsl-1.40'],
                    os.path.join(testsDir, 'spec', 'glsl-1.40'),
                    recursive=True)
spec['glsl-1.40']['execution']['tf-no-position'] = concurrent_test('glsl-1.40-tf-no-position')
spec['glsl-1.40']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-1.40/minimum-maximums.txt'))

textureSize_samplers_140 = textureSize_samplers_130 + ['sampler2DRect', 'isampler2DRect', 'sampler2DRectShadow', 'samplerBuffer', 'isamplerBuffer', 'usamplerBuffer']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.40'
    # textureSize():
    for sampler in textureSize_samplers_140:
        spec['glsl-{0}/execution/textureSize/{1}-textureSize-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'textureSize 140 {0} {1}'.format(stage, sampler))
    # texelFetch():
    for sampler in ['sampler2DRect', 'usampler2DRect', 'isampler2DRect']:
        spec['glsl-{0}/execution/texelFetch/{1}-texelFetch-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'texelFetch 140 {0} {1}'.format(stage, sampler))
        spec['glsl-{0}/execution/texelFetchOffset/{1}-{2}'.format(
                version, stage, sampler)] = concurrent_test(
                'texelFetch offset 140 {0} {1}'.format(stage, sampler))

spec['glsl-1.50'] = {}
import_glsl_parser_tests(spec['glsl-1.50'],
                         os.path.join(testsDir, 'spec', 'glsl-1.50'),
                         ['compiler'])
add_shader_test_dir(spec['glsl-1.50'],
                    os.path.join(testsDir, 'spec', 'glsl-1.50'),
                    recursive=True)
spec['glsl-1.50']['execution']['interface-blocks-api-access-members'] = concurrent_test('glsl-1.50-interface-blocks-api-access-members')
spec['glsl-1.50']['execution']['get-active-attrib-array'] = concurrent_test('glsl-1.50-get-active-attrib-array')
spec['glsl-1.50']['execution']['vs-input-arrays'] = concurrent_test('glsl-1.50-vs-input-arrays')
spec['glsl-1.50']['execution']['vs-named-block-no-modify'] = concurrent_test('glsl-1.50-vs-named-block-no-modify')
for draw in ['', 'indexed']:
    for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                 'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        add_concurrent_test(spec['glsl-1.50'],
                            ('arb_geometry_shader4-ignore-adjacent-vertices '
                             'core {0} {1}').format(draw, prim))
spec['glsl-1.50']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-1.50/minimum-maximums.txt'))
spec['glsl-1.50']['gs-emits-too-few-verts'] = concurrent_test('glsl-1.50-gs-emits-too-few-verts')
spec['glsl-1.50']['gs-end-primitive-optional-with-points-out'] = concurrent_test('glsl-1.50-geometry-end-primitive-optional-with-points-out')
spec['glsl-1.50']['getshaderiv-may-return-GS'] = concurrent_test('glsl-1.50-getshaderiv-may-return-GS')
spec['glsl-1.50']['query-gs-prim-types'] = concurrent_test('glsl-1.50-query-gs-prim-types')
spec['glsl-1.50']['transform-feedback-type-and-size'] = concurrent_test('glsl-1.50-transform-feedback-type-and-size')
spec['glsl-1.50']['transform-feedback-vertex-id'] = concurrent_test('glsl-1.50-transform-feedback-vertex-id')
for subtest in ['unnamed', 'named', 'array']:
    add_concurrent_test(
        spec['glsl-1.50'],
        'glsl-1.50-interface-block-centroid {0}'.format(subtest))

# max_vertices of 32 and 128 are important transition points for
# mesa/i965 (they are the number of bits in a float and a vec4,
# respectively), so test around there.  Also test 0, which means the
# maximum number of geometry shader output vertices supported by the
# hardware.
for i in [31, 32, 33, 34, 127, 128, 129, 130, 0]:
    cmdline = 'end-primitive {0}'.format(i)
    spec['glsl-1.50']['execution']['geometry'][cmdline] = \
        concurrent_test('glsl-1.50-geometry-' + cmdline)

for prim_type in ['GL_POINTS', 'GL_LINE_LOOP', 'GL_LINE_STRIP', 'GL_LINES',
                  'GL_TRIANGLES', 'GL_TRIANGLE_STRIP', 'GL_TRIANGLE_FAN',
                  'GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                  'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
    cmdline = 'primitive-types {0}'.format(prim_type)
    spec['glsl-1.50']['execution']['geometry'][cmdline] = \
        concurrent_test('glsl-1.50-geometry-' + cmdline)
    for restart_index in ['ffs', 'other']:
        cmdline = 'primitive-id-restart {0} {1}'.format(
            prim_type, restart_index)
        spec['glsl-1.50']['execution']['geometry'][cmdline] = \
            concurrent_test('glsl-1.50-geometry-' + cmdline)

for layout_type in ['points', 'lines', 'lines_adjacency', 'triangles',
                    'triangles_adjacency']:
    add_concurrent_test(spec['glsl-1.50'],
                        'glsl-1.50-gs-mismatch-prim-type {0}'.format(
                            layout_type))

for prim_type in ['GL_TRIANGLE_STRIP', 'GL_TRIANGLE_STRIP_ADJACENCY']:
    for restart_index in ['ffs', 'other']:
        cmdline = 'tri-strip-ordering-with-prim-restart {0} {1}'.format(
            prim_type, restart_index)
        spec['glsl-1.50']['execution']['geometry'][cmdline] = \
            concurrent_test('glsl-1.50-geometry-' + cmdline)

for input_layout in ['points', 'lines', 'lines_adjacency', 'triangles',
                     'triangles_adjacency', 'line_strip', 'triangle_strip']:
    add_concurrent_test(spec['glsl-1.50'],
                        'glsl-1.50-gs-input-layout-qualifiers {0}'.format(
                            input_layout))

for output_layout in ['points', 'lines', 'lines_adjacency', 'triangles',
                      'triangles_adjacency', 'line_strip', 'triangle_strip']:
    add_concurrent_test(spec['glsl-1.50'],
                        'glsl-1.50-gs-output-layout-qualifiers {0}'.format(
                            output_layout))

spec['glsl-3.30'] = {}
spec['glsl-3.30']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/glsl-3.30/minimum-maximums.txt'))

import_glsl_parser_tests(spec['glsl-3.30'],
                         os.path.join(testsDir, 'spec', 'glsl-3.30'),
                         ['compiler'])
add_shader_test_dir(spec['glsl-3.30'],
                    os.path.join(testsDir, 'spec', 'glsl-3.30'),
                    recursive=True)

# Group spec/glsl-es-3.00
spec['glsl-es-3.00'] = {}
import_glsl_parser_tests(spec['glsl-es-3.00'],
                         os.path.join(testsDir, 'spec', 'glsl-es-3.00'),
                         ['compiler'])
add_shader_test_dir(spec['glsl-es-3.00'],
                    os.path.join(testsDir, 'spec', 'glsl-es-3.00'),
                    recursive=True)
add_concurrent_test(spec['glsl-es-3.00']['execution'], 'varying-struct-centroid_gles3')
spec['glsl-es-3.00']['built-in constants'] = concurrent_test('built-in-constants_gles3 ' + os.path.join(testsDir, 'spec/glsl-es-3.00/minimum-maximums.txt'))

# AMD_performance_monitor
profile.test_list['spec/AMD_performance_monitor/api'] = PiglitTest('amd_performance_monitor_api -auto')
profile.test_list['spec/AMD_performance_monitor/measure'] = PiglitTest('amd_performance_monitor_measure -auto')

# Group AMD_conservative_depth
spec['AMD_conservative_depth'] = {}
import_glsl_parser_tests(spec['AMD_conservative_depth'],
                         os.path.join(testsDir, 'spec', 'amd_conservative_depth'),
                         [''])
add_shader_test_dir(spec['AMD_conservative_depth'],
                    os.path.join(testsDir, 'spec', 'amd_conservative_depth'),
                    recursive=True)

# Group ARB_arrays_of_arrays
arb_arrays_of_arrays = {}
spec['ARB_arrays_of_arrays'] = arb_arrays_of_arrays
import_glsl_parser_tests(arb_arrays_of_arrays,
                         os.path.join(testsDir, 'spec', 'arb_arrays_of_arrays'),
                         ['compiler'])

# Group AMD_shader_trinary_minmax
spec['AMD_shader_trinary_minmax'] = {}
import_glsl_parser_tests(spec['AMD_shader_trinary_minmax'],
                         os.path.join(testsDir, 'spec', 'amd_shader_trinary_minmax'),
                         [''])
add_shader_test_dir(spec['AMD_shader_trinary_minmax'],
                    os.path.join(testsDir, 'spec', 'amd_shader_trinary_minmax'),
                    recursive=True)

# Group ARB_point_sprite
arb_point_sprite = {}
spec['ARB_point_sprite'] = arb_point_sprite
add_plain_test(arb_point_sprite, 'point-sprite')

# Group ARB_tessellation_shader
arb_tessellation_shader = {}
spec['ARB_tessellation_shader'] = arb_tessellation_shader
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-get-tcs-params')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-get-tes-params')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-minmax')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-invalid-get-program-params')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-invalid-patch-vertices-range')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-invalid-primitive')
add_concurrent_test(arb_tessellation_shader, 'built-in-constants tests/spec/arb_tessellation_shader/minimum-maximums.txt')
add_concurrent_test(arb_tessellation_shader, 'arb_tessellation_shader-minmax')
import_glsl_parser_tests(arb_tessellation_shader,
                         os.path.join(testsDir, 'spec',
                         'arb_tessellation_shader'), ['compiler'])
add_shader_test_dir(arb_tessellation_shader,
                    os.path.join(testsDir, 'spec', 'arb_tessellation_shader'),
                    recursive=True)

# Group ARB_texture_multisample
samplers_atm = ['sampler2DMS', 'isampler2DMS', 'usampler2DMS',
                'sampler2DMSArray', 'isampler2DMSArray', 'usampler2DMSArray']
arb_texture_multisample = {}
spec['ARB_texture_multisample'] = arb_texture_multisample
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-minmax')
for sample_count in MSAA_SAMPLE_COUNTS:
    # fb-completeness
    spec['ARB_texture_multisample/fb-completeness/%d' % (sample_count,)] = \
        concurrent_test('arb_texture_multisample-fb-completeness %d' % (sample_count,))
    # texel-fetch execution
    for stage in ['vs', 'gs', 'fs']:
        for sampler in samplers_atm:
            spec['ARB_texture_multisample/texelFetch/%d-%s-%s' % (
                sample_count, stage, sampler)] = \
                concurrent_test('texelFetch %s %s %d' % (stage, sampler, sample_count))
    # sample positions
    spec['ARB_texture_multisample/sample-position/%d' % (sample_count,)] = \
        concurrent_test('arb_texture_multisample-sample-position %d' % (sample_count,))
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMS 4 1x71-501x71')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMS 4 1x130-501x130')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMS 4 71x1-71x130')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMS 4 281x1-281x130')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMSArray 4 1x129x9-98x129x9')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMSArray 4 98x1x9-98x129x9')
add_concurrent_test(arb_texture_multisample, 'texelFetch fs sampler2DMSArray 4 98x129x1-98x129x9')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-texstate')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-errors')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-texelfetch 2')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-texelfetch 4')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-texelfetch 8')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-sample-mask')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-sample-mask-value')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-sample-mask-execution')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-sample-mask-execution -tex')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-negative-max-samples')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-teximage-3d-multisample')
add_concurrent_test(arb_texture_multisample, 'arb_texture_multisample-teximage-2d-multisample')

for stage in ['vs', 'gs', 'fs']:
    # textureSize():
    for sampler in samplers_atm:
        spec['ARB_texture_multisample/textureSize/' + stage + '-textureSize-' + sampler] = concurrent_test('textureSize ' + stage + ' ' + sampler)

# Group ARB_texture_gather
arb_texture_gather = {}
spec['ARB_texture_gather'] = arb_texture_gather
for stage in ['vs', 'fs']:
    for comps in ['r', 'rg', 'rgb', 'rgba']:
        for swiz in ['red', 'green', 'blue', 'alpha'][:len(comps)] + ['', 'zero', 'one']:
            for type in ['unorm', 'float', 'int', 'uint']:
                for sampler in ['2D', '2DArray', 'Cube', 'CubeArray']:
                    for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset']:
                        testname = '%s/%s-%s-%s-%s-%s' % (
                            func, stage, comps,
                            swiz if len(swiz) else 'none',
                            type, sampler)
                        cmd = 'textureGather %s %s %s %s %s %s' % (
                            stage,
                            'offset' if func == 'textureGatherOffset' else '',
                            comps, swiz, type, sampler
                            )
                        arb_texture_gather[testname] = concurrent_test(cmd)

# Group AMD_shader_stencil_export
spec['AMD_shader_stencil_export'] = {}
import_glsl_parser_tests(spec['AMD_shader_stencil_export'],
                         os.path.join(testsDir, 'spec', 'amd_shader_stencil_export'),
                         [''])

# Group ARB_shader_stencil_export
spec['ARB_shader_stencil_export'] = {}
import_glsl_parser_tests(spec['ARB_shader_stencil_export'],
                         os.path.join(testsDir, 'spec', 'arb_shader_stencil_export'),
                         [''])

profile.test_list['spec/ARB_stencil_texturing/draw'] = concurrent_test('arb_stencil_texturing-draw')

# Group ARB_sync
arb_sync = {}
spec['ARB_sync'] = arb_sync
arb_sync['ClientWaitSync-errors'] = concurrent_test('arb_sync-client-wait-errors')
arb_sync['DeleteSync'] = concurrent_test('arb_sync-delete')
arb_sync['FenceSync-errors'] = concurrent_test('arb_sync-fence-sync-errors')
arb_sync['GetSynciv-errors'] = concurrent_test('arb_sync-get-sync-errors')
arb_sync['IsSync'] = concurrent_test('arb_sync-is-sync')
arb_sync['repeat-wait'] = concurrent_test('arb_sync-repeat-wait')
arb_sync['sync-initialize'] = concurrent_test('arb_sync-sync-initialize')
arb_sync['timeout-zero'] = concurrent_test('arb_sync-timeout-zero')
arb_sync['WaitSync-errors'] = concurrent_test('arb_sync-WaitSync-errors')
add_plain_test(arb_sync, 'sync_api')

# Group ARB_ES2_compatibility
arb_es2_compatibility = {}
spec['ARB_ES2_compatibility'] = arb_es2_compatibility
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-depthrangef')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-drawbuffers')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-getshaderprecisionformat')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-maxvectors')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-shadercompiler')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-releaseshadercompiler')
add_plain_test(arb_es2_compatibility, 'arb_es2_compatibility-fixed-type')
add_plain_test(arb_es2_compatibility, 'fbo-missing-attachment-clear')
arb_es2_compatibility['FBO blit to missing attachment (ES2 completeness rules)'] = concurrent_test('fbo-missing-attachment-blit es2 to')
arb_es2_compatibility['FBO blit from missing attachment (ES2 completeness rules)'] = concurrent_test('fbo-missing-attachment-blit es2 from')
add_fbo_formats_tests('spec/ARB_ES2_compatibility', 'GL_ARB_ES2_compatibility')
add_texwrap_format_tests(arb_es2_compatibility, 'GL_ARB_ES2_compatibility')
arb_es2_compatibility['NUM_SHADER_BINARY_FORMATS over-run check'] = concurrent_test('arb_get_program_binary-overrun shader')


# Group ARB_get_program_binary
arb_get_program_binary = {}
spec['ARB_get_program_binary'] = arb_get_program_binary
arb_get_program_binary['misc. API error checks'] = concurrent_test('arb_get_program_binary-api-errors')
arb_get_program_binary['NUM_PROGRAM_BINARY_FORMATS over-run check'] = concurrent_test('arb_get_program_binary-overrun program')
arb_get_program_binary['PROGRAM_BINARY_RETRIEVABLE_HINT'] = concurrent_test('arb_get_program_binary-retrievable_hint')

arb_depth_clamp = {}
spec['ARB_depth_clamp'] = arb_depth_clamp
add_plain_test(arb_depth_clamp, 'depth_clamp')
add_plain_test(arb_depth_clamp, 'depth-clamp-range')
add_plain_test(arb_depth_clamp, 'depth-clamp-status')

# Group ARB_draw_elements_base_vertex
arb_draw_elements_base_vertex = {}
spec['ARB_draw_elements_base_vertex'] = arb_draw_elements_base_vertex
arb_draw_elements_base_vertex['dlist'] = concurrent_test('arb_draw_elements_base_vertex-dlist')
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-drawelements')
arb_draw_elements_base_vertex['arb_draw_elements_base_vertex-drawelements-user_varrays'] = PiglitTest(['arb_draw_elements_base_vertex-drawelements', '-auto', 'user_varrays'])
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-negative-index')
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-bounds')
arb_draw_elements_base_vertex['arb_draw_elements_base_vertex-negative-index-user_varrays'] = PiglitTest(['arb_draw_elements_base_vertex-negative-index', '-auto', 'user_varrays'])
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-drawelements-instanced')
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-drawrangeelements')
add_plain_test(arb_draw_elements_base_vertex, 'arb_draw_elements_base_vertex-multidrawelements')

# Group ARB_draw_instanced
arb_draw_instanced = {}
spec['ARB_draw_instanced'] = arb_draw_instanced
import_glsl_parser_tests(arb_draw_instanced,
                        os.path.join(testsDir, 'spec', 'arb_draw_instanced'),
                        [''])

add_shader_test_dir(arb_draw_instanced,
                    os.path.join(testsDir, 'spec', 'arb_draw_instanced', 'execution'),
                    recursive=True)
arb_draw_instanced['dlist'] = concurrent_test('arb_draw_instanced-dlist')
arb_draw_instanced['elements'] = concurrent_test('arb_draw_instanced-elements')
arb_draw_instanced['negative-arrays-first-negative'] = concurrent_test('arb_draw_instanced-negative-arrays-first-negative')
arb_draw_instanced['negative-elements-type'] = concurrent_test('arb_draw_instanced-negative-elements-type')
add_plain_test(arb_draw_instanced, 'arb_draw_instanced-drawarrays')

# Group ARB_draw_indirect
arb_draw_indirect = {}
spec['ARB_draw_indirect'] = arb_draw_indirect
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-api-errors')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-arrays')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-elements')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-arrays-base-instance')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-elements-base-instance')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-elements-prim-restart')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-elements-prim-restart-ugly')
add_concurrent_test(arb_draw_indirect, 'arb_draw_indirect-draw-arrays-instances')

# Group ARB_fragment_program
arb_fragment_program = {}
spec['ARB_fragment_program'] = arb_fragment_program
add_shader_test_dir(spec['ARB_fragment_program'],
                    os.path.join(testsDir, 'spec', 'arb_fragment_program'),
                    recursive=True)
arb_fragment_program['minmax'] = concurrent_test('arb_fragment_program-minmax')
add_vpfpgeneric(arb_fragment_program, 'fdo30337a')
add_vpfpgeneric(arb_fragment_program, 'fdo30337b')
add_vpfpgeneric(arb_fragment_program, 'fdo38145')
add_vpfpgeneric(arb_fragment_program, 'fp-cmp')
add_vpfpgeneric(arb_fragment_program, 'fp-dst-aliasing-1')
add_vpfpgeneric(arb_fragment_program, 'fp-dst-aliasing-2')
add_vpfpgeneric(arb_fragment_program, 'fp-ex2-sat')
add_vpfpgeneric(arb_fragment_program, 'fp-two-constants')
add_plain_test(arb_fragment_program, 'fp-abs-01')
add_plain_test(arb_fragment_program, 'fp-fog')
add_plain_test(arb_fragment_program, 'fp-formats')
add_plain_test(arb_fragment_program, 'fp-fragment-position')
add_plain_test(arb_fragment_program, 'fp-incomplete-tex')
add_plain_test(arb_fragment_program, 'fp-indirections')
add_plain_test(arb_fragment_program, 'fp-indirections2')
add_plain_test(arb_fragment_program, 'fp-kil')
add_plain_test(arb_fragment_program, 'fp-lit-mask')
add_plain_test(arb_fragment_program, 'fp-lit-src-equals-dst')
add_plain_test(arb_fragment_program, 'fp-long-alu')
add_plain_test(arb_fragment_program, 'fp-set-01')
arb_fragment_program['sparse-samplers'] = concurrent_test('arb_fragment_program-sparse-samplers')
add_plain_test(arb_fragment_program, 'trinity-fp1')
arb_fragment_program['incomplete-texture-arb_fp'] = concurrent_test('incomplete-texture -auto arb_fp')

# Group ARB_fragment_program_shadow
arb_fragment_program_shadow = {}
spec['ARB_fragment_program_shadow'] = arb_fragment_program_shadow
add_shader_test_dir(spec['ARB_fragment_program_shadow'],
                    os.path.join(testsDir, 'spec', 'arb_fragment_program_shadow'),
                    recursive=True)

nv_fragment_program_option = {}
spec['NV_fragment_program_option'] = nv_fragment_program_option
add_plain_test(nv_fragment_program_option, 'fp-abs-02')
add_plain_test(nv_fragment_program_option, 'fp-condition_codes-01')
add_plain_test(nv_fragment_program_option, 'fp-rfl')
add_plain_test(nv_fragment_program_option, 'fp-set-02')
add_plain_test(nv_fragment_program_option, 'fp-unpack-01')

arb_fragment_coord_conventions = {}
spec['ARB_fragment_coord_conventions'] = arb_fragment_coord_conventions
add_vpfpgeneric(arb_fragment_coord_conventions, 'fp-arb-fragment-coord-conventions-none')
add_vpfpgeneric(arb_fragment_coord_conventions, 'fp-arb-fragment-coord-conventions-integer')
import_glsl_parser_tests(arb_fragment_coord_conventions,
                         os.path.join(testsDir, 'spec',
                                      'arb_fragment_coord_conventions'),
                         ['compiler'])

arb_fragment_layer_viewport = {}
spec['ARB_fragment_layer_viewport'] = arb_fragment_layer_viewport
add_shader_test_dir(arb_fragment_layer_viewport,
                    os.path.join(testsDir, 'spec', 'arb_fragment_layer_viewport'),
                    recursive=True)

ati_fragment_shader = {}
spec['ATI_fragment_shader'] = ati_fragment_shader
add_plain_test(ati_fragment_shader, 'ati-fs-bad-delete')

# Group ARB_framebuffer_object
arb_framebuffer_object = {}
spec['ARB_framebuffer_object'] = arb_framebuffer_object
add_concurrent_test(arb_framebuffer_object, 'same-attachment-glFramebufferTexture2D-GL_DEPTH_STENCIL_ATTACHMENT')
add_concurrent_test(arb_framebuffer_object, 'same-attachment-glFramebufferRenderbuffer-GL_DEPTH_STENCIL_ATTACHMENT')
add_plain_test(arb_framebuffer_object, 'fdo28551') # must not be concurrent
for format in ('rgba', 'depth', 'stencil'):
    for test_mode in ('draw', 'read'):
        test_name = ' '.join(['framebuffer-blit-levels', test_mode, format])
        arb_framebuffer_object[test_name] = concurrent_test(test_name)
add_concurrent_test(arb_framebuffer_object, 'fbo-alpha')
add_plain_test(arb_framebuffer_object, 'fbo-blit-stretch')
add_concurrent_test(arb_framebuffer_object, 'fbo-blit-scaled-linear')
add_concurrent_test(arb_framebuffer_object, 'fbo-attachments-blit-scaled-linear')
add_concurrent_test(arb_framebuffer_object, 'fbo-deriv')
add_concurrent_test(arb_framebuffer_object, 'fbo-luminance-alpha')
add_concurrent_test(arb_framebuffer_object, 'fbo-getframebufferattachmentparameter-01')
add_concurrent_test(arb_framebuffer_object, 'fbo-gl_pointcoord')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete-invalid-texture')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete-texture-01')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete-texture-02')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete-texture-03')
add_concurrent_test(arb_framebuffer_object, 'fbo-incomplete-texture-04')
add_concurrent_test(arb_framebuffer_object, 'fbo-mipmap-copypix')
add_plain_test(arb_framebuffer_object, 'fbo-viewport') # must not be concurrent
arb_framebuffer_object['FBO blit to missing attachment'] = concurrent_test('fbo-missing-attachment-blit to')
arb_framebuffer_object['FBO blit from missing attachment'] = concurrent_test('fbo-missing-attachment-blit from')
arb_framebuffer_object['fbo-scissor-blit fbo'] = concurrent_test('fbo-scissor-blit fbo')
arb_framebuffer_object['fbo-scissor-blit window'] = plain_test('fbo-scissor-blit window')
arb_framebuffer_object['fbo-tex-rgbx'] = concurrent_test('fbo-tex-rgbx')
arb_framebuffer_object['negative-readpixels-no-rb'] = concurrent_test('arb_framebuffer_object-negative-readpixels-no-rb')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glClear')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glClearBuffer')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none gl_FragColor')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none gl_FragData')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none use_frag_out')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glColorMaskIndexed')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glBlendFunci')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glDrawPixels')
add_concurrent_test(arb_framebuffer_object, 'fbo-drawbuffers-none glBlitFramebuffer')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-cubemap')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-cubemap RGB9_E5')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-cubemap S3TC_DXT1')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-1d')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-1d RGB9_E5')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-3d')
add_concurrent_test(arb_framebuffer_object, 'fbo-generatemipmap-3d RGB9_E5')

# Group ARB_framebuffer_sRGB
arb_framebuffer_srgb = {}
spec['ARB_framebuffer_sRGB'] = arb_framebuffer_srgb
for backing_type in ('texture', 'renderbuffer'):
    for srgb_types in ('linear', 'srgb', 'linear_to_srgb',
                       'srgb_to_linear'):
        for blit_type in ('single_sampled', 'upsample', 'downsample',
                          'msaa', 'scaled'):
            for framebuffer_srgb_setting in ('enabled',
                                             'disabled'):
                test_name = ' '.join(
                        ['blit', backing_type, srgb_types,
                         blit_type, framebuffer_srgb_setting])
                arb_framebuffer_srgb[test_name] = concurrent_test(
                        'arb_framebuffer_srgb-' + test_name)
add_plain_test(arb_framebuffer_srgb, 'framebuffer-srgb') # must not be concurrent

arb_gpu_shader5 = {}
spec['ARB_gpu_shader5'] = arb_gpu_shader5
add_shader_test_dir(arb_gpu_shader5,
                    os.path.join(testsDir, 'spec', 'arb_gpu_shader5'),
                    recursive=True)
import_glsl_parser_tests(arb_gpu_shader5,
                         testsDir + '/spec/arb_gpu_shader5', [''])
for stage in ['vs', 'fs']:
    for type in ['unorm', 'float', 'int', 'uint']:
        for comps in ['r', 'rg', 'rgb', 'rgba']:
            for cs in [0, 1, 2, 3][:len(comps)]:
                for sampler in ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']:
                    for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets' ]:
                        testname = '%s/%s-%s-%s-%s-%s' % (
                                func, stage, comps,
                                cs,
                                type, sampler)
                        address_mode = 'clamp' if sampler == '2DRect' else 'repeat'
                        cmd = 'textureGather %s %s %s %s %s %s %s' % (
                                stage,
                                'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                                comps, cs, type, sampler, address_mode
                                )
                        arb_gpu_shader5[testname] = concurrent_test(cmd)

                        if func == 'textureGatherOffset':
                            # also add a constant offset version.
                            testname = '%s/%s-%s-%s-%s-%s-const' % (
                                    func, stage, comps,
                                    cs,
                                    type, sampler)
                            cmd = 'textureGather %s %s %s %s %s %s %s' % (
                                    stage,
                                    'offset',
                                    comps, cs, type, sampler, address_mode
                                    )
                            arb_gpu_shader5[testname] = concurrent_test(cmd)
    # test shadow samplers
    for sampler in ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']:
        for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets' ]:
            testname = '%s/%s-r-none-shadow-%s' % (func, stage, sampler)
            address_mode = 'clamp' if sampler == '2DRect' else 'repeat'
            cmd = 'textureGather %s shadow r %s %s %s' % (
                    stage,
                    'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                    sampler,
                    address_mode)
            arb_gpu_shader5[testname] = concurrent_test(cmd)
add_concurrent_test(arb_gpu_shader5, 'arb_gpu_shader5-minmax')
add_concurrent_test(arb_gpu_shader5, 'arb_gpu_shader5-invocation-id')
add_concurrent_test(arb_gpu_shader5, 'arb_gpu_shader5-invocations_count_too_large')
add_concurrent_test(arb_gpu_shader5, 'arb_gpu_shader5-xfb-streams')

arb_texture_query_levels = {}
spec['ARB_texture_query_levels'] = arb_texture_query_levels
add_shader_test_dir(arb_texture_query_levels,
                    testsDir + '/spec/arb_texture_query_levels',
                    recursive=True)
import_glsl_parser_tests(arb_texture_query_levels,
                         testsDir + '/spec/arb_texture_query_levels', [''])

arb_occlusion_query = {}
spec['ARB_occlusion_query'] = arb_occlusion_query
add_concurrent_test(arb_occlusion_query, 'occlusion_query')
add_concurrent_test(arb_occlusion_query, 'occlusion_query_lifetime')
add_concurrent_test(arb_occlusion_query, 'occlusion_query_meta_fragments')
add_concurrent_test(arb_occlusion_query, 'occlusion_query_meta_no_fragments')
add_concurrent_test(arb_occlusion_query, 'occlusion_query_order')
add_concurrent_test(arb_occlusion_query, 'gen_delete_while_active')

# Group ARB_separate_shader_objects
arb_separate_shader_objects = {}
spec['ARB_separate_shader_objects'] = arb_separate_shader_objects
arb_separate_shader_objects['ActiveShaderProgram with invalid program'] = concurrent_test('arb_separate_shader_object-ActiveShaderProgram-invalid-program')
arb_separate_shader_objects['GetProgramPipelineiv'] = concurrent_test('arb_separate_shader_object-GetProgramPipelineiv')
arb_separate_shader_objects['IsProgramPipeline'] = concurrent_test('arb_separate_shader_object-IsProgramPipeline')
arb_separate_shader_objects['UseProgramStages - non-separable program'] = concurrent_test('arb_separate_shader_object-UseProgramStages-non-separable')
arb_separate_shader_objects['ProgramUniform coverage'] = concurrent_test('arb_separate_shader_object-ProgramUniform-coverage')
arb_separate_shader_objects['Rendezvous by location'] = plain_test('arb_separate_shader_object-rendezvous_by_location -fbo')
arb_separate_shader_objects['ValidateProgramPipeline'] = concurrent_test('arb_separate_shader_object-ValidateProgramPipeline')
arb_separate_shader_objects['400 combinations by location'] = plain_test('arb_separate_shader_object-400-combinations -fbo --by-location')
arb_separate_shader_objects['400 combinations by name'] = plain_test('arb_separate_shader_object-400-combinations -fbo')
arb_separate_shader_objects['active sampler conflict'] = concurrent_test('arb_separate_shader_object-active-sampler-conflict')

# Group ARB_sampler_objects
arb_sampler_objects = {}
spec['ARB_sampler_objects'] = arb_sampler_objects
arb_sampler_objects['sampler-objects'] = concurrent_test('arb_sampler_objects-sampler-objects')
arb_sampler_objects['sampler-incomplete'] = concurrent_test('arb_sampler_objects-sampler-incomplete')
arb_sampler_objects['GL_EXT_texture_sRGB_decode'] = concurrent_test('arb_sampler_objects-srgb-decode')
arb_sampler_objects['framebufferblit'] = plain_test('arb_sampler_objects-framebufferblit')

# Group ARB_sample_shading
arb_sample_shading = {}
spec['ARB_sample_shading'] = arb_sample_shading
add_plain_test(arb_sample_shading, 'arb_sample_shading-api')

TEST_SAMPLE_COUNTS = (0,) + MSAA_SAMPLE_COUNTS
for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-num-samples {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-id {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-mask {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

for num_samples in (0,2,4,6,8):
    test_name = 'builtin-gl-sample-mask-simple {0}'.format(num_samples)
    executable = 'arb_sample_shading-' + test_name
    arb_sample_shading[test_name] = concurrent_test(executable)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-position {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = 'interpolate-at-sample-position {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = 'ignore-centroid-qualifier {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0} -auto'.format(test_name)
    arb_sample_shading[test_name] = PiglitTest(executable)

import_glsl_parser_tests(spec['ARB_sample_shading'],
                         os.path.join(testsDir, 'spec', 'arb_sample_shading'),
                         ['compiler'])

# Group ARB_debug_output
arb_debug_output = {}
spec['ARB_debug_output'] = arb_debug_output
add_plain_test(arb_debug_output, 'arb_debug_output-api_error')

# Group KHR_debug
khr_debug = {}
spec['KHR_debug'] = khr_debug
khr_debug['object-label_gl'] = concurrent_test('khr_debug-object-label_gl')
khr_debug['object-label_gles2'] = concurrent_test('khr_debug-object-label_gles2')
khr_debug['object-label_gles3'] = concurrent_test('khr_debug-object-label_gles3')
khr_debug['push-pop-group_gl'] = concurrent_test('khr_debug-push-pop-group_gl')
khr_debug['push-pop-group_gles2'] = concurrent_test('khr_debug-push-pop-group_gles2')
khr_debug['push-pop-group_gles3'] = concurrent_test('khr_debug-push-pop-group_gles3')

# Group ARB_occlusion_query2
arb_occlusion_query2 = {}
spec['ARB_occlusion_query2'] = arb_occlusion_query2
arb_occlusion_query2['api'] = concurrent_test('arb_occlusion_query2-api')
arb_occlusion_query2['render'] = concurrent_test('arb_occlusion_query2-render')

arb_pixel_buffer_object = {}
spec['ARB_pixel_buffer_object'] = arb_pixel_buffer_object
add_plain_test(arb_pixel_buffer_object, 'fbo-pbo-readpixels-small')
add_plain_test(arb_pixel_buffer_object, 'pbo-drawpixels')
add_plain_test(arb_pixel_buffer_object, 'pbo-read-argb8888')
add_plain_test(arb_pixel_buffer_object, 'pbo-readpixels-small')
add_plain_test(arb_pixel_buffer_object, 'pbo-teximage')
add_plain_test(arb_pixel_buffer_object, 'pbo-teximage-tiling')
add_plain_test(arb_pixel_buffer_object, 'pbo-teximage-tiling-2')

# Group ARB_provoking_vertex
arb_provoking_vertex = {}
spec['ARB_provoking_vertex'] = arb_provoking_vertex;
add_plain_test(arb_provoking_vertex, 'arb-provoking-vertex-control')
add_plain_test(arb_provoking_vertex, 'arb-provoking-vertex-initial')
add_plain_test(arb_provoking_vertex, 'arb-quads-follow-provoking-vertex')
add_plain_test(arb_provoking_vertex, 'arb-xfb-before-flatshading')

# Group ARB_robustness
arb_robustness = {}
spec['ARB_robustness'] = arb_robustness
add_plain_test(arb_robustness, 'arb_robustness_client-mem-bounds')
# TODO: robust vertex buffer access
#add_plain_test(arb_robustness, 'arb_robustness_draw-vbo-bounds')

# Group ARB_shader_bit_encoding
arb_shader_bit_encoding = {}
spec['ARB_shader_bit_encoding'] = arb_shader_bit_encoding
arb_shader_bit_encoding['execution'] = {}
add_shader_test_dir(arb_shader_bit_encoding['execution'],
                    os.path.join(testsDir, 'spec', 'arb_shader_bit_encoding', 'execution'),
                    recursive=True)

# Group ARB_shader_texture_lod
arb_shader_texture_lod = {}
spec['ARB_shader_texture_lod'] = arb_shader_texture_lod
import_glsl_parser_tests(arb_shader_texture_lod,
                         os.path.join(generatedTestDir, 'spec', 'arb_shader_texture_lod'),
                         ['compiler'])
arb_shader_texture_lod['execution'] = {}
add_shader_test_dir(arb_shader_texture_lod['execution'],
                    os.path.join(testsDir, 'spec', 'arb_shader_texture_lod', 'execution'),
                    recursive=True)
add_plain_test(arb_shader_texture_lod['execution'], 'arb_shader_texture_lod-texgrad')
add_plain_test(arb_shader_texture_lod['execution'], 'arb_shader_texture_lod-texgradcube')

add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod 1D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod 2D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod 3D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod Cube')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod 1DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *Lod 2DShadow')

add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 1D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 1D_ProjVec4')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 2D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 2D_ProjVec4')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 3D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 1DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjLod 2DShadow')

add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 1D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 2D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 3D')
#add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB Cube')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 1DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 2DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 2DRect')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *GradARB 2DRectShadow')

add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 1D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 1D_ProjVec4')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2D_ProjVec4')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 3D')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 1DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2DShadow')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2DRect')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2DRect_ProjVec4')
add_concurrent_test(arb_shader_texture_lod['execution'], 'tex-miplevel-selection *ProjGradARB 2DRectShadow')


# Group ARB_shader_objects
arb_shader_objects = {}
spec['ARB_shader_objects'] = arb_shader_objects
arb_shader_objects['getuniform'] = concurrent_test('arb_shader_objects-getuniform')
arb_shader_objects['bindattriblocation-scratch-name'] = concurrent_test('arb_shader_objects-bindattriblocation-scratch-name')
arb_shader_objects['getactiveuniform-beginend'] = concurrent_test('arb_shader_objects-getactiveuniform-beginend')
arb_shader_objects['getuniformlocation-array-of-struct-of-array'] = concurrent_test('arb_shader_objects-getuniformlocation-array-of-struct-of-array')
arb_shader_objects['clear-with-deleted'] = concurrent_test('arb_shader_objects-clear-with-deleted')
arb_shader_objects['delete-repeat'] = concurrent_test('arb_shader_objects-delete-repeat')

arb_shading_language_420pack = {}
spec['ARB_shading_language_420pack'] = arb_shading_language_420pack
import_glsl_parser_tests(arb_shading_language_420pack,
                         os.path.join(testsDir, 'spec', 'arb_shading_language_420pack'),
                         ['compiler'])
arb_shading_language_420pack['execution'] = {}
add_shader_test_dir(arb_shading_language_420pack['execution'],
                    os.path.join(testsDir, 'spec', 'arb_shading_language_420pack', 'execution'),
                    recursive=True)
spec['ARB_shading_language_420pack']['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/arb_shading_language_420pack/minimum-maximums.txt'))
spec['ARB_shading_language_420pack']['multiple layout qualifiers'] = concurrent_test('arb_shading_language_420pack-multiple-layout-qualifiers')

# Group ARB_explicit_attrib_location
arb_explicit_attrib_location = {}
spec['ARB_explicit_attrib_location'] = arb_explicit_attrib_location
import_glsl_parser_tests(arb_explicit_attrib_location,
                         os.path.join(testsDir,
                         'spec', 'arb_explicit_attrib_location'),
                         [''])
add_plain_test(arb_explicit_attrib_location, 'glsl-explicit-location-01')
add_plain_test(arb_explicit_attrib_location, 'glsl-explicit-location-02')
add_plain_test(arb_explicit_attrib_location, 'glsl-explicit-location-03')
add_plain_test(arb_explicit_attrib_location, 'glsl-explicit-location-04')
add_plain_test(arb_explicit_attrib_location, 'glsl-explicit-location-05')
for test_type in ('shader', 'api'):
    test_name = 'overlapping-locations-input-attribs {0}'.format(test_type)
    executable = '{0} -auto'.format(test_name)
    arb_explicit_attrib_location[test_name] = PiglitTest(executable)

# Group ARB_explicit_uniform_location
arb_explicit_uniform_location = {}
spec['ARB_explicit_uniform_location'] = arb_explicit_uniform_location
import_glsl_parser_tests(arb_explicit_uniform_location,
                         os.path.join(testsDir, 'spec', 'arb_explicit_uniform_location'),
                         [''])
add_shader_test_dir(arb_explicit_uniform_location,
                    os.path.join(testsDir, 'spec', 'arb_explicit_uniform_location'),
                    recursive=True)
add_plain_test(arb_explicit_uniform_location, 'arb_explicit_uniform_location-minmax')
add_plain_test(arb_explicit_uniform_location, 'arb_explicit_uniform_location-boundaries')
add_plain_test(arb_explicit_uniform_location, 'arb_explicit_uniform_location-array-elements')
add_plain_test(arb_explicit_uniform_location, 'arb_explicit_uniform_location-inactive-uniform')

arb_texture_buffer_object = {}
spec['ARB_texture_buffer_object'] = arb_texture_buffer_object
arb_texture_buffer_object['data-sync'] = concurrent_test('arb_texture_buffer_object-data-sync')
arb_texture_buffer_object['dlist'] = concurrent_test('arb_texture_buffer_object-dlist')
arb_texture_buffer_object['formats (FS, 3.1 core)'] = concurrent_test('arb_texture_buffer_object-formats fs core')
arb_texture_buffer_object['formats (VS, 3.1 core)'] = concurrent_test('arb_texture_buffer_object-formats vs core')
arb_texture_buffer_object['formats (FS, ARB)'] = concurrent_test('arb_texture_buffer_object-formats fs arb')
arb_texture_buffer_object['formats (VS, ARB)'] = concurrent_test('arb_texture_buffer_object-formats vs arb')
arb_texture_buffer_object['get'] = concurrent_test('arb_texture_buffer_object-get')
arb_texture_buffer_object['fetch-outside-bounds'] = concurrent_test('arb_texture_buffer_object-fetch-outside-bounds')
arb_texture_buffer_object['minmax'] = concurrent_test('arb_texture_buffer_object-minmax')
arb_texture_buffer_object['negative-bad-bo'] = concurrent_test('arb_texture_buffer_object-negative-bad-bo')
arb_texture_buffer_object['negative-bad-format'] = concurrent_test('arb_texture_buffer_object-negative-bad-format')
arb_texture_buffer_object['negative-bad-target'] = concurrent_test('arb_texture_buffer_object-negative-bad-target')
arb_texture_buffer_object['negative-unsupported'] = concurrent_test('arb_texture_buffer_object-negative-unsupported')
arb_texture_buffer_object['subdata-sync'] = concurrent_test('arb_texture_buffer_object-subdata-sync')
arb_texture_buffer_object['unused-name'] = concurrent_test('arb_texture_buffer_object-unused-name')

arb_texture_buffer_range = {}
spec['ARB_texture_buffer_range'] = arb_texture_buffer_range
arb_texture_buffer_range['dlist'] = concurrent_test('arb_texture_buffer_range-dlist')
arb_texture_buffer_range['errors'] = concurrent_test('arb_texture_buffer_range-errors')
arb_texture_buffer_range['ranges'] = concurrent_test('arb_texture_buffer_range-ranges')

arb_texture_query_lod = {}
spec['ARB_texture_query_lod'] = arb_texture_query_lod
add_shader_test_dir(arb_texture_query_lod,
                    testsDir + '/spec/arb_texture_query_lod',
                    recursive=True)

arb_texture_rectangle = {}
spec['ARB_texture_rectangle'] = arb_texture_rectangle
add_texwrap_target_tests(arb_texture_rectangle, 'RECT')
add_shader_test_dir(arb_texture_rectangle,
                    testsDir + '/spec/arb_texture_rectangle',
                    recursive=True)
add_msaa_visual_plain_tests(arb_texture_rectangle, 'copyteximage RECT')
add_concurrent_test(arb_texture_rectangle, '1-1-linear-texture')
add_plain_test(arb_texture_rectangle, 'texrect-many')
add_concurrent_test(arb_texture_rectangle, 'getteximage-targets RECT')
add_plain_test(arb_texture_rectangle, 'texrect_simple_arb_texrect')
add_plain_test(arb_texture_rectangle, 'arb_texrect-texture-base-level-error')
add_plain_test(arb_texture_rectangle, 'fbo-blit rect')
add_concurrent_test(spec['ARB_texture_rectangle'], 'tex-miplevel-selection GL2:texture() 2DRect')
add_concurrent_test(spec['ARB_texture_rectangle'], 'tex-miplevel-selection GL2:texture() 2DRectShadow')
add_concurrent_test(spec['ARB_texture_rectangle'], 'tex-miplevel-selection GL2:textureProj 2DRect')
add_concurrent_test(spec['ARB_texture_rectangle'], 'tex-miplevel-selection GL2:textureProj 2DRect_ProjVec4')
add_concurrent_test(spec['ARB_texture_rectangle'], 'tex-miplevel-selection GL2:textureProj 2DRectShadow')

arb_texture_storage = {}
spec['ARB_texture_storage'] = arb_texture_storage
arb_texture_storage['texture-storage'] = plain_test('arb_texture_storage-texture-storage')

arb_texture_storage_multisample = {}
spec['ARB_texture_storage_multisample'] = arb_texture_storage_multisample
arb_texture_storage_multisample['tex-storage'] = concurrent_test('arb_texture_storage_multisample-tex-storage')
arb_texture_storage_multisample['tex-param'] = concurrent_test('arb_texture_storage_multisample-tex-param')

arb_texture_view = {}
spec['ARB_texture_view'] = arb_texture_view
arb_texture_view['immutable_levels'] = concurrent_test('arb_texture_view-texture-immutable-levels')
arb_texture_view['params'] = concurrent_test('arb_texture_view-params')
arb_texture_view['formats'] = concurrent_test('arb_texture_view-formats')
arb_texture_view['targets'] = concurrent_test('arb_texture_view-targets')
arb_texture_view['queries'] = concurrent_test('arb_texture_view-queries')
arb_texture_view['rendering-target'] = concurrent_test('arb_texture_view-rendering-target')
arb_texture_view['rendering-levels'] = concurrent_test('arb_texture_view-rendering-levels')
arb_texture_view['rendering-layers'] = concurrent_test('arb_texture_view-rendering-layers')
arb_texture_view['rendering-formats'] = concurrent_test('arb_texture_view-rendering-formats')
arb_texture_view['lifetime-format'] = concurrent_test('arb_texture_view-lifetime-format')
arb_texture_view['getteximage-srgb'] = concurrent_test('arb_texture_view-getteximage-srgb')
arb_texture_view['texsubimage-levels'] = concurrent_test('arb_texture_view-texsubimage-levels')
arb_texture_view['texsubimage-layers'] = concurrent_test('arb_texture_view-texsubimage-layers')
arb_texture_view['clear-into-view-2d'] = concurrent_test('arb_texture_view-clear-into-view-2d')
arb_texture_view['clear-into-view-2d-array'] = concurrent_test('arb_texture_view-clear-into-view-2d-array')
arb_texture_view['clear-into-view-layered'] = concurrent_test('arb_texture_view-clear-into-view-layered')
arb_texture_view['copytexsubimage-layers'] = concurrent_test('arb_texture_view-copytexsubimage-layers')

tdfx_texture_compression_fxt1 = {}
spec['3DFX_texture_compression_FXT1'] = tdfx_texture_compression_fxt1
add_concurrent_test(tdfx_texture_compression_fxt1, 'compressedteximage GL_COMPRESSED_RGB_FXT1_3DFX')
add_concurrent_test(tdfx_texture_compression_fxt1, 'compressedteximage GL_COMPRESSED_RGBA_FXT1_3DFX')
add_fbo_generatemipmap_extension(tdfx_texture_compression_fxt1, 'GL_3DFX_texture_compression_FXT1', 'fbo-generatemipmap-formats')
tdfx_texture_compression_fxt1['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats fxt1')
add_plain_test(tdfx_texture_compression_fxt1, 'fxt1-teximage')

def add_color_buffer_float_test(name, format, p1, p2):
    arb_color_buffer_float[format + '-' + name + ('-' + p1 if len(p1) else '') + ('-' + p2 if len(p2) else '')] = concurrent_test(' '.join(['arb_color_buffer_float-' + name, format, p1, p2]))

arb_color_buffer_float = {}
spec['ARB_color_buffer_float'] = arb_color_buffer_float
add_color_buffer_float_test('mrt', 'mixed', '', '')

add_color_buffer_float_test('getteximage', 'GL_RGBA8', '', '')
add_color_buffer_float_test('queries', 'GL_RGBA8', '', '')
add_color_buffer_float_test('readpixels', 'GL_RGBA8', '', '')
add_color_buffer_float_test('probepixel', 'GL_RGBA8', '', '')
add_color_buffer_float_test('drawpixels', 'GL_RGBA8', '', '')
add_color_buffer_float_test('clear', 'GL_RGBA8', '', '')
add_color_buffer_float_test('render', 'GL_RGBA8', '', '')
add_color_buffer_float_test('render', 'GL_RGBA8', 'fog', '')
add_color_buffer_float_test('render', 'GL_RGBA8', 'sanity', '')
add_color_buffer_float_test('render', 'GL_RGBA8', 'sanity', 'fog')

add_color_buffer_float_test('getteximage', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('queries', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('readpixels', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('probepixel', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('drawpixels', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('clear', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('render', 'GL_RGBA8_SNORM', '', '')
add_color_buffer_float_test('render', 'GL_RGBA8_SNORM', 'fog', '')
add_color_buffer_float_test('render', 'GL_RGBA8_SNORM', 'sanity', '')
add_color_buffer_float_test('render', 'GL_RGBA8_SNORM', 'sanity', 'fog')

add_color_buffer_float_test('getteximage', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('queries', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('readpixels', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('probepixel', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('drawpixels', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('clear', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('render', 'GL_RGBA16F', '', '')
add_color_buffer_float_test('render', 'GL_RGBA16F', 'fog', '')
add_color_buffer_float_test('render', 'GL_RGBA16F', 'sanity', '')
add_color_buffer_float_test('render', 'GL_RGBA16F', 'sanity', 'fog')

add_color_buffer_float_test('getteximage', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('queries', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('readpixels', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('probepixel', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('drawpixels', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('clear', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('render', 'GL_RGBA32F', '', '')
add_color_buffer_float_test('render', 'GL_RGBA32F', 'fog', '')
add_color_buffer_float_test('render', 'GL_RGBA32F', 'sanity', '')
add_color_buffer_float_test('render', 'GL_RGBA32F', 'sanity', 'fog')


arb_depth_texture = {}
spec['ARB_depth_texture'] = arb_depth_texture
add_fbo_formats_tests('spec/ARB_depth_texture', 'GL_ARB_depth_texture')
add_texwrap_format_tests(arb_depth_texture, 'GL_ARB_depth_texture')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT16')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT24')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT32')
add_plain_test(arb_depth_texture, 'depth-level-clamp')
add_plain_test(arb_depth_texture, 'depth-tex-modes')
add_plain_test(arb_depth_texture, 'texdepth')
add_depthstencil_render_miplevels_tests(arb_depth_texture, ('d=z24', 'd=z16'))

arb_depth_buffer_float = {}
spec['ARB_depth_buffer_float'] = arb_depth_buffer_float
add_fbo_depth_tests(arb_depth_buffer_float, 'GL_DEPTH_COMPONENT32F')
add_fbo_depth_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8')
add_fbo_stencil_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8')
add_fbo_depthstencil_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8', 0)
add_fbo_formats_tests('spec/ARB_depth_buffer_float', 'GL_ARB_depth_buffer_float')
add_texwrap_format_tests(arb_depth_buffer_float, 'GL_ARB_depth_buffer_float')
add_depthstencil_render_miplevels_tests(
        arb_depth_buffer_float,
        ('d=z32f_s8', 'd=z32f', 'd=z32f_s8_s=z24_s8', 'd=z32f_s=z24_s8',
         's=z24_s8_d=z32f_s8', 's=z24_s8_d=z32f', 'd=s=z32f_s8', 's=d=z32f_s8',
         'ds=z32f_s8'))
arb_depth_buffer_float['fbo-clear-formats stencil'] = concurrent_test('fbo-clear-formats GL_ARB_depth_buffer_float stencil')

arb_texture_env_crossbar = {}
spec['ARB_texture_env_crossbar'] = arb_texture_env_crossbar
add_plain_test(arb_texture_env_crossbar, 'crossbar')

arb_texture_compression = {}
spec['ARB_texture_compression'] = arb_texture_compression
add_fbo_generatemipmap_extension(arb_texture_compression, 'GL_ARB_texture_compression', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(arb_texture_compression, 'GL_ARB_texture_compression')
arb_texture_compression['GL_TEXTURE_INTERNAL_FORMAT query'] = concurrent_test('arb_texture_compression-internal-format-query')
arb_texture_compression['unknown formats'] = concurrent_test('arb_texture_compression-invalid-formats unknown')

arb_texture_compression_bptc = {}
spec['ARB_texture_compression_bptc'] = arb_texture_compression_bptc
arb_texture_compression_bptc['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats bptc')

ext_vertex_array_bgra = {}
spec['EXT_vertex_array_bgra'] = ext_vertex_array_bgra
add_plain_test(ext_vertex_array_bgra, 'bgra-sec-color-pointer')
add_plain_test(ext_vertex_array_bgra, 'bgra-vert-attrib-pointer')

apple_vertex_array_object = {}
spec['APPLE_vertex_array_object'] = apple_vertex_array_object
add_plain_test(apple_vertex_array_object, 'vao-01')
add_plain_test(apple_vertex_array_object, 'vao-02')
apple_vertex_array_object['isvertexarray'] = concurrent_test('arb_vertex_array-isvertexarray apple')

profile.test_list['spec/ARB_vertex_array_bgra/api-errors'] = PiglitTest('arb_vertex_array_bgra-api-errors -auto')
profile.test_list['spec/ARB_vertex_array_bgra/get'] = PiglitTest('arb_vertex_array_bgra-get -auto')

arb_vertex_array_object = {}
spec['ARB_vertex_array_object'] = arb_vertex_array_object
add_concurrent_test(arb_vertex_array_object, 'vao-element-array-buffer')
arb_vertex_array_object['isvertexarray'] = concurrent_test('arb_vertex_array-isvertexarray')

arb_vertex_buffer_object = {}
spec['ARB_vertex_buffer_object'] = arb_vertex_buffer_object
arb_vertex_buffer_object['elements-negative-offset'] = PiglitTest(['arb_vertex_buffer_object-elements-negative-offset', '-auto'])
arb_vertex_buffer_object['mixed-immediate-and-vbo'] = PiglitTest(['arb_vertex_buffer_object-mixed-immediate-and-vbo', '-auto'])
add_plain_test(arb_vertex_buffer_object, 'fdo14575')
add_plain_test(arb_vertex_buffer_object, 'fdo22540')
add_plain_test(arb_vertex_buffer_object, 'fdo31934')
arb_vertex_buffer_object['ib-data-sync'] = concurrent_test('arb_vertex_buffer_object-ib-data-sync')
arb_vertex_buffer_object['ib-subdata-sync'] = concurrent_test('arb_vertex_buffer_object-ib-subdata-sync')
add_plain_test(arb_vertex_buffer_object, 'pos-array')
add_plain_test(arb_vertex_buffer_object, 'vbo-bufferdata')
add_plain_test(arb_vertex_buffer_object, 'vbo-map-remap')
add_concurrent_test(arb_vertex_buffer_object, 'vbo-map-unsync')
arb_vertex_buffer_object['vbo-subdata-many drawarrays'] = concurrent_test('arb_vertex_buffer_object-vbo-subdata-many drawarrays')
arb_vertex_buffer_object['vbo-subdata-many drawelements'] = concurrent_test('arb_vertex_buffer_object-vbo-subdata-many drawelements')
arb_vertex_buffer_object['vbo-subdata-many drawrangeelements'] = concurrent_test('arb_vertex_buffer_object-vbo-subdata-many drawrangeelements')
add_plain_test(arb_vertex_buffer_object, 'vbo-subdata-sync')
add_plain_test(arb_vertex_buffer_object, 'vbo-subdata-zero')

arb_vertex_program = {}
spec['ARB_vertex_program'] = arb_vertex_program
arb_vertex_program['getenv4d-with-error'] = PiglitTest(['arb_vertex_program-getenv4d-with-error', '-auto'])
arb_vertex_program['getlocal4d-with-error'] = PiglitTest(['arb_vertex_program-getlocal4d-with-error', '-auto'])
arb_vertex_program['getlocal4f-max'] = concurrent_test('arb_vertex_program-getlocal4f-max')
arb_vertex_program['getlocal4-errors'] = concurrent_test('arb_vertex_program-getlocal4-errors')
arb_vertex_program['clip-plane-transformation arb'] = concurrent_test('clip-plane-transformation arb')
arb_vertex_program['minmax'] = concurrent_test('arb_vertex_program-minmax')
add_plain_test(arb_vertex_program, 'fdo24066')
add_vpfpgeneric(arb_vertex_program, 'arl')
add_vpfpgeneric(arb_vertex_program, 'big-param')
add_vpfpgeneric(arb_vertex_program, 'dataflow-bug')
add_vpfpgeneric(arb_vertex_program, 'fogcoord-dp3')
add_vpfpgeneric(arb_vertex_program, 'fogcoord-dph')
add_vpfpgeneric(arb_vertex_program, 'fogcoord-dp4')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge-varying')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge-offset')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge-offset-neg')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge-overwritten')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-huge-relative-offset')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-constant-array-varying')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-env-array')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-local-array')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-neg-array')
add_vpfpgeneric(arb_vertex_program, 'vp-arl-neg-array-2')
add_vpfpgeneric(arb_vertex_program, 'vp-constant-array')
add_vpfpgeneric(arb_vertex_program, 'vp-constant-array-huge')
add_vpfpgeneric(arb_vertex_program, 'vp-constant-negate')
add_vpfpgeneric(arb_vertex_program, 'vp-exp-alias')
add_vpfpgeneric(arb_vertex_program, 'vp-max')
add_vpfpgeneric(arb_vertex_program, 'vp-min')
add_vpfpgeneric(arb_vertex_program, 'vp-sge-alias')
add_vpfpgeneric(arb_vertex_program, 'vp-two-constants')
add_plain_test(arb_vertex_program, 'vp-address-01')
add_plain_test(arb_vertex_program, 'vp-address-02')
add_plain_test(arb_vertex_program, 'vp-address-04')
add_plain_test(arb_vertex_program, 'vp-bad-program')
add_plain_test(arb_vertex_program, 'vp-max-array')

arb_viewport_array = {}
spec['ARB_viewport_array'] = arb_viewport_array
arb_viewport_array['viewport-indices'] = concurrent_test('arb_viewport_array-viewport-indices')
arb_viewport_array['depthrange-indices'] = concurrent_test('arb_viewport_array-depthrange-indices')
arb_viewport_array['scissor-check'] = concurrent_test('arb_viewport_array-scissor-check')
arb_viewport_array['scissor-indices'] = concurrent_test('arb_viewport_array-scissor-indices')
arb_viewport_array['bounds'] = concurrent_test('arb_viewport_array-bounds')
arb_viewport_array['queries'] = concurrent_test('arb_viewport_array-queries')
arb_viewport_array['minmax'] = concurrent_test('arb_viewport_array-minmax')
arb_viewport_array['render-viewport'] = concurrent_test('arb_viewport_array-render-viewport')
arb_viewport_array['render-depthrange'] = concurrent_test('arb_viewport_array-render-depthrange')
arb_viewport_array['render-scissor'] = concurrent_test('arb_viewport_array-render-scissor')
arb_viewport_array['clear'] =  concurrent_test('arb_viewport_array-clear')

nv_vertex_program = {}
spec['NV_vertex_program'] = nv_vertex_program
add_vpfpgeneric(nv_vertex_program, 'nv-mov')
add_vpfpgeneric(nv_vertex_program, 'nv-add')
add_vpfpgeneric(nv_vertex_program, 'nv-arl')
add_vpfpgeneric(nv_vertex_program, 'nv-init-zero-reg')
add_vpfpgeneric(nv_vertex_program, 'nv-init-zero-addr')

nv_vertex_program2_option = {}
spec['NV_vertex_program2_option'] = nv_vertex_program2_option
add_plain_test(nv_vertex_program2_option, 'vp-address-03')
add_plain_test(nv_vertex_program2_option, 'vp-address-05')
add_plain_test(nv_vertex_program2_option, 'vp-address-06')
add_plain_test(nv_vertex_program2_option, 'vp-clipdistance-01')
add_plain_test(nv_vertex_program2_option, 'vp-clipdistance-02')
add_plain_test(nv_vertex_program2_option, 'vp-clipdistance-03')
add_plain_test(nv_vertex_program2_option, 'vp-clipdistance-04')

ext_framebuffer_blit = {}
spec['EXT_framebuffer_blit'] = ext_framebuffer_blit
add_plain_test(ext_framebuffer_blit, 'fbo-blit') # must not be concurrent
add_plain_test(ext_framebuffer_blit, 'fbo-copypix') # must not be concurrent
add_plain_test(ext_framebuffer_blit, 'fbo-readdrawpix') # must not be concurrent
add_concurrent_test(ext_framebuffer_blit, 'fbo-srgb-blit')
add_plain_test(ext_framebuffer_blit, 'fbo-sys-blit') # must not be concurrent
add_plain_test(ext_framebuffer_blit, 'fbo-sys-sub-blit') # must not be concurrent

ext_framebuffer_multisample_blit_scaled = {}
spec['EXT_framebuffer_multisample_blit_scaled'] = ext_framebuffer_multisample_blit_scaled
ext_framebuffer_multisample_blit_scaled['negative-blit-scaled'] = concurrent_test('ext_framebuffer_multisample_blit_scaled-negative-blit-scaled')
for num_samples in MSAA_SAMPLE_COUNTS:
    ext_framebuffer_multisample_blit_scaled['blit-scaled samples=' + str(num_samples)] = concurrent_test('ext_framebuffer_multisample_blit_scaled-blit-scaled ' + str(num_samples))

ext_framebuffer_multisample = {}
spec['EXT_framebuffer_multisample'] = ext_framebuffer_multisample
ext_framebuffer_multisample['blit-mismatched-samples'] = concurrent_test('ext_framebuffer_multisample-blit-mismatched-samples')
ext_framebuffer_multisample['blit-mismatched-sizes'] = concurrent_test('ext_framebuffer_multisample-blit-mismatched-sizes')
ext_framebuffer_multisample['blit-mismatched-formats'] = concurrent_test('ext_framebuffer_multisample-blit-mismatched-formats')
ext_framebuffer_multisample['dlist'] = concurrent_test('ext_framebuffer_multisample-dlist')
ext_framebuffer_multisample['enable-flag'] = concurrent_test('ext_framebuffer_multisample-enable-flag')
ext_framebuffer_multisample['minmax'] = concurrent_test('ext_framebuffer_multisample-minmax')
ext_framebuffer_multisample['negative-copypixels'] = concurrent_test('ext_framebuffer_multisample-negative-copypixels')
ext_framebuffer_multisample['negative-copyteximage'] = concurrent_test('ext_framebuffer_multisample-negative-copyteximage')
ext_framebuffer_multisample['negative-max-samples'] = concurrent_test('ext_framebuffer_multisample-negative-max-samples')
ext_framebuffer_multisample['negative-mismatched-samples'] = concurrent_test('ext_framebuffer_multisample-negative-mismatched-samples')
ext_framebuffer_multisample['negative-readpixels'] = concurrent_test('ext_framebuffer_multisample-negative-readpixels')
ext_framebuffer_multisample['renderbufferstorage-samples'] = concurrent_test('ext_framebuffer_multisample-renderbufferstorage-samples')
ext_framebuffer_multisample['renderbuffer-samples'] = concurrent_test('ext_framebuffer_multisample-renderbuffer-samples')
ext_framebuffer_multisample['samples'] = concurrent_test('ext_framebuffer_multisample-samples')
ext_framebuffer_multisample['alpha-blending'] = concurrent_test('ext_framebuffer_multisample-alpha-blending')
ext_framebuffer_multisample['alpha-blending slow_cc'] = concurrent_test('ext_framebuffer_multisample-alpha-blending slow_cc')

for num_samples in MSAA_SAMPLE_COUNTS:
    if num_samples % 2 != 0:
        continue
    test_name = 'alpha-blending-after-rendering {0}'.format(num_samples)
    ext_framebuffer_multisample[test_name] = concurrent_test(
        'ext_framebuffer_multisample-' + test_name)

for num_samples in MSAA_SAMPLE_COUNTS:
    for test_type in ('color', 'srgb', 'stencil_draw', 'stencil_resolve',
                      'depth_draw', 'depth_resolve'):
        sensible_options = ['small', 'depthstencil']
        if test_type in ('color', 'srgb'):
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' '.join(['accuracy', str(num_samples), test_type]
                                 + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(
                    test_name)
            ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['turn-on-off', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0} -auto'.format(test_name)
    ext_framebuffer_multisample[test_name] = PiglitTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        sensible_options = []
        if buffer_type == 'color':
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' '.join(['upsample', str(num_samples), buffer_type]
                                 + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
            ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        sensible_options = []
        if buffer_type == 'color':
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' ' .join(['multisample-blit', str(num_samples),
                                   buffer_type] + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
            ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        for blit_type in ('msaa', 'upsample', 'downsample'):
            test_name = ' '.join(['unaligned-blit', str(num_samples), buffer_type, blit_type])
            executable = 'ext_framebuffer_multisample-{0}'.format(
                    test_name)
            ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['line-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['point-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['polygon-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['formats', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for test_mode in ('inverted', 'non-inverted'):
        test_name = ' '.join(['sample-coverage', str(num_samples), test_mode])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth'):
        test_name = ' '.join(['sample-alpha-to-coverage', str(num_samples), buffer_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['sample-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['draw-buffers-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['draw-buffers-alpha-to-coverage', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-coverage-no-draw-buffer-zero', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-coverage-dual-src-blend', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-dual-src-blend', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['int-draw-buffers-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['int-draw-buffers-alpha-to-coverage', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-msaa-disabled', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-single-sample-buffer', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['bitmap', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['polygon-stipple', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(
            test_name)
    ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for blit_type in ('msaa', 'upsample', 'downsample', 'normal'):
        test_name = ' '.join(['clip-and-scissor-blit',
                              str(num_samples), blit_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(
                executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for flip_direction in ('x', 'y'):
        test_name = ' '.join(['blit-flipped', str(num_samples),
                              flip_direction])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(
                executable)

# Note: the interpolation tests also check for sensible behaviour with
# non-multisampled framebuffers, so go ahead and test them with
# num_samples==0 as well.
for num_samples in (0,) + MSAA_SAMPLE_COUNTS:
    for test_type in ('non-centroid-disabled', 'centroid-disabled',
                      'centroid-edges', 'non-centroid-deriv',
                      'non-centroid-deriv-disabled', 'centroid-deriv',
                      'centroid-deriv-disabled'):
        test_name = ' '.join(['interpolation', str(num_samples),
                              test_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        test_name = ' '.join(['clear', str(num_samples), buffer_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(
                test_name)
        ext_framebuffer_multisample[test_name] = concurrent_test(
                executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for test_type in ('depth', 'depth-computed', 'stencil'):
        for buffer_config in ('combined', 'separate', 'single'):
            test_name = ' '.join(['no-color', str(num_samples),
                                  test_type, buffer_config])
            executable = 'ext_framebuffer_multisample-{0}'.format(
                    test_name)
            ext_framebuffer_multisample[test_name] = concurrent_test(executable)

ext_framebuffer_object = {}
spec['EXT_framebuffer_object'] = ext_framebuffer_object
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX1')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX4')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX8')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX16')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-noimage')
add_concurrent_test(ext_framebuffer_object, 'fdo20701')
add_concurrent_test(ext_framebuffer_object, 'fbo-1d')
add_concurrent_test(ext_framebuffer_object, 'fbo-3d')
add_concurrent_test(ext_framebuffer_object, 'fbo-alphatest-formats')
add_concurrent_test(ext_framebuffer_object, 'fbo-alphatest-nocolor')
add_concurrent_test(ext_framebuffer_object, 'fbo-alphatest-nocolor-ff')
add_concurrent_test(ext_framebuffer_object, 'fbo-blending-formats')
add_concurrent_test(ext_framebuffer_object, 'fbo-bind-renderbuffer')
add_concurrent_test(ext_framebuffer_object, 'fbo-clearmipmap')
add_concurrent_test(ext_framebuffer_object, 'fbo-clear-formats')
add_concurrent_test(ext_framebuffer_object, 'fbo-copyteximage')
add_concurrent_test(ext_framebuffer_object, 'fbo-copyteximage-simple')
add_concurrent_test(ext_framebuffer_object, 'fbo-cubemap')
add_concurrent_test(ext_framebuffer_object, 'fbo-depthtex')
add_concurrent_test(ext_framebuffer_object, 'fbo-depth-sample-compare')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers masked-clear')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers-arbfp')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers-blend-add')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers-fragcolor')
add_concurrent_test(ext_framebuffer_object, 'fbo-drawbuffers-maxtargets')
add_concurrent_test(ext_framebuffer_object, 'fbo-finish-deleted')
add_concurrent_test(ext_framebuffer_object, 'fbo-flushing')
add_concurrent_test(ext_framebuffer_object, 'fbo-flushing-2')
add_concurrent_test(ext_framebuffer_object, 'fbo-fragcoord')
add_concurrent_test(ext_framebuffer_object, 'fbo-fragcoord2')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-filtering')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-formats')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-scissor')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-swizzle')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-nonsquare')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-npot')
add_concurrent_test(ext_framebuffer_object, 'fbo-generatemipmap-viewport')
add_concurrent_test(ext_framebuffer_object, 'fbo-maxsize')
add_concurrent_test(ext_framebuffer_object, 'fbo-nodepth-test')
add_concurrent_test(ext_framebuffer_object, 'fbo-nostencil-test')
add_concurrent_test(ext_framebuffer_object, 'fbo-readpixels')
add_concurrent_test(ext_framebuffer_object, 'fbo-readpixels-depth-formats')
add_concurrent_test(ext_framebuffer_object, 'fbo-scissor-bitmap')
add_concurrent_test(ext_framebuffer_object, 'fbo-storage-completeness')
add_concurrent_test(ext_framebuffer_object, 'fbo-storage-formats')
add_concurrent_test(ext_framebuffer_object, 'getteximage-formats init-by-rendering')

ext_image_dma_buf_import = {}
spec['EXT_image_dma_buf_import'] = ext_image_dma_buf_import
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-invalid_hints')
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-invalid_attributes')
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-missing_attributes')
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-ownership_transfer')
ext_image_dma_buf_import['ext_image_dma_buf_import-sample_argb8888'] = PiglitTest(['ext_image_dma_buf_import-sample_rgb', '-fmt=AR24', '-auto'])
ext_image_dma_buf_import['ext_image_dma_buf_import-sample_xrgb8888'] = PiglitTest(['ext_image_dma_buf_import-sample_rgb', '-auto', '-fmt=XR24', '-alpha-one'])
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-intel_unsupported_format')
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-intel_external_sampler_only')
add_plain_test(ext_image_dma_buf_import, 'ext_image_dma_buf_import-intel_external_sampler_with_dma_only')

ext_packed_depth_stencil = {}
spec['EXT_packed_depth_stencil'] = ext_packed_depth_stencil
add_fbo_depth_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8')
add_fbo_stencil_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8')
add_fbo_depthstencil_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8', 0)
add_fbo_formats_tests('spec/EXT_packed_depth_stencil', 'GL_EXT_packed_depth_stencil')
add_texwrap_format_tests(ext_packed_depth_stencil, 'GL_EXT_packed_depth_stencil')
ext_packed_depth_stencil['readpixels-24_8'] = PiglitTest(['ext_packed_depth_stencil-readpixels-24_8', '-auto'])
add_plain_test(ext_packed_depth_stencil, 'fbo-blit-d24s8')
add_depthstencil_render_miplevels_tests(
        ext_packed_depth_stencil,
        ('s=z24_s8', 'd=z24_s8', 'd=z24_s8_s=z24_s8', 'd=z24_s=z24_s8',
         's=z24_s8_d=z24_s8', 's=z24_s8_d=z24', 'd=s=z24_s8', 's=d=z24_s8',
         'ds=z24_s8'))
ext_packed_depth_stencil['fbo-clear-formats stencil'] = concurrent_test('fbo-clear-formats GL_EXT_packed_depth_stencil stencil')
ext_packed_depth_stencil['DEPTH_STENCIL texture'] = concurrent_test('ext_packed_depth_stencil-depth-stencil-texture')
ext_packed_depth_stencil['errors'] = concurrent_test('ext_packed_depth_stencil-errors')
ext_packed_depth_stencil['getteximage'] = concurrent_test('ext_packed_depth_stencil-getteximage')
ext_packed_depth_stencil['readdrawpixels'] = concurrent_test('ext_packed_depth_stencil-readdrawpixels')
ext_packed_depth_stencil['texsubimage'] = concurrent_test('ext_packed_depth_stencil-texsubimage')

oes_packed_depth_stencil = {}
spec['OES_packed_depth_stencil'] = oes_packed_depth_stencil
oes_packed_depth_stencil['DEPTH_STENCIL texture GLES2'] = concurrent_test('oes_packed_depth_stencil-depth-stencil-texture_gles2')
oes_packed_depth_stencil['DEPTH_STENCIL texture GLES1'] = concurrent_test('oes_packed_depth_stencil-depth-stencil-texture_gles1')

ext_texture_array = {}
spec['EXT_texture_array'] = ext_texture_array
add_concurrent_test(ext_texture_array, 'fbo-generatemipmap-array')
add_concurrent_test(ext_texture_array, 'fbo-generatemipmap-array RGB9_E5')
add_concurrent_test(ext_texture_array, 'fbo-generatemipmap-array S3TC_DXT1')
spec['EXT_texture_array']['maxlayers'] = concurrent_test('ext_texture_array-maxlayers')
spec['EXT_texture_array']['gen-mipmap'] = concurrent_test('ext_texture_array-gen-mipmap')
add_shader_test_dir(ext_texture_array,
                    testsDir + '/spec/ext_texture_array',
                    recursive=True)
add_msaa_visual_plain_tests(ext_texture_array, 'copyteximage 1D_ARRAY')
add_msaa_visual_plain_tests(ext_texture_array, 'copyteximage 2D_ARRAY')
add_plain_test(ext_texture_array, 'fbo-array')
add_plain_test(ext_texture_array, 'fbo-depth-array')
add_plain_test(ext_texture_array, 'array-texture')
add_concurrent_test(ext_texture_array, 'getteximage-targets 1D_ARRAY')
add_concurrent_test(ext_texture_array, 'getteximage-targets 2D_ARRAY')
for test_mode in ['teximage', 'texsubimage']:
    test_name = 'compressed {0}'.format(test_mode)
    ext_texture_array[test_name] = PiglitTest('ext_texture_array-' + test_name + ' -auto -fbo')

arb_texture_cube_map = {}
spec['ARB_texture_cube_map'] = arb_texture_cube_map
add_msaa_visual_plain_tests(arb_texture_cube_map, 'copyteximage CUBE')
add_plain_test(arb_texture_cube_map, 'crash-cubemap-order')
add_plain_test(arb_texture_cube_map, 'cubemap')
add_concurrent_test(arb_texture_cube_map, 'cubemap-getteximage-pbo')
arb_texture_cube_map['cubemap npot'] = PiglitTest(['cubemap', '-auto', 'npot'])
add_plain_test(arb_texture_cube_map, 'cubemap-shader')
arb_texture_cube_map['cubemap-shader lod'] = PiglitTest(['cubemap-shader', '-auto', 'lod'])
arb_texture_cube_map['cubemap-shader bias'] = PiglitTest(['cubemap-shader', '-auto', 'bias'])
add_concurrent_test(arb_texture_cube_map, 'getteximage-targets CUBE')

arb_texture_cube_map_array = {}
spec['ARB_texture_cube_map_array'] = arb_texture_cube_map_array
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-get')
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-teximage3d-invalid-values')
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-cubemap')
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-cubemap-lod')
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-fbo-cubemap-array')
add_plain_test(arb_texture_cube_map_array, 'arb_texture_cube_map_array-sampler-cube-array-shadow')
add_concurrent_test(arb_texture_cube_map_array, 'getteximage-targets CUBE_ARRAY')
add_concurrent_test(arb_texture_cube_map_array, 'glsl-resource-not-bound CubeArray')
textureSize_samplers_atcma = ['samplerCubeArray', 'isamplerCubeArray', 'usamplerCubeArray', 'samplerCubeArrayShadow' ];
add_concurrent_test(arb_texture_cube_map_array, 'fbo-generatemipmap-cubemap array')
add_concurrent_test(arb_texture_cube_map_array, 'fbo-generatemipmap-cubemap array RGB9_E5')
add_concurrent_test(arb_texture_cube_map_array, 'fbo-generatemipmap-cubemap array S3TC_DXT1')

import_glsl_parser_tests(arb_texture_cube_map_array,
                         os.path.join(testsDir, 'spec', 'arb_texture_cube_map_array'),
                         ['compiler'])
for stage in ['vs', 'gs', 'fs']:
    # textureSize():
    for sampler in textureSize_samplers_atcma:
        spec['ARB_texture_cube_map_array/textureSize/' + stage + '-textureSize-' + sampler] = concurrent_test('textureSize ' + stage + ' ' + sampler)

ext_texture_swizzle = {}
spec['EXT_texture_swizzle'] = ext_texture_swizzle
add_concurrent_test(ext_texture_swizzle, 'ext_texture_swizzle-api')
add_concurrent_test(ext_texture_swizzle, 'ext_texture_swizzle-swizzle')
ext_texture_swizzle['depth_texture_mode_and_swizzle'] = concurrent_test('depth_texture_mode_and_swizzle')

ext_texture_compression_latc = {}
spec['EXT_texture_compression_latc'] = ext_texture_compression_latc
add_fbo_generatemipmap_extension(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc', 'fbo-generatemipmap-formats')
add_fbo_generatemipmap_extension(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc-signed', 'fbo-generatemipmap-formats-signed')
add_texwrap_format_tests(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc')
ext_texture_compression_latc['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats latc')

ext_texture_compression_rgtc = {}
spec['EXT_texture_compression_rgtc'] = ext_texture_compression_rgtc
add_concurrent_test(ext_texture_compression_rgtc, 'compressedteximage GL_COMPRESSED_RED_RGTC1_EXT')
add_concurrent_test(ext_texture_compression_rgtc, 'compressedteximage GL_COMPRESSED_RED_GREEN_RGTC2_EXT')
add_concurrent_test(ext_texture_compression_rgtc, 'compressedteximage GL_COMPRESSED_SIGNED_RED_RGTC1_EXT')
add_concurrent_test(ext_texture_compression_rgtc, 'compressedteximage GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT')
add_fbo_generatemipmap_extension(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc', 'fbo-generatemipmap-formats')
add_fbo_generatemipmap_extension(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc-signed', 'fbo-generatemipmap-formats-signed')
add_texwrap_format_tests(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc')
ext_texture_compression_rgtc['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats rgtc')
add_plain_test(ext_texture_compression_rgtc, 'rgtc-teximage-01')
add_plain_test(ext_texture_compression_rgtc, 'rgtc-teximage-02')

ext_texture_compression_s3tc = {}
spec['EXT_texture_compression_s3tc'] = ext_texture_compression_s3tc
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_RGB_S3TC_DXT1_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_RGBA_S3TC_DXT1_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_RGBA_S3TC_DXT3_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_RGBA_S3TC_DXT5_EXT')
add_fbo_generatemipmap_extension(ext_texture_compression_s3tc, 'GL_EXT_texture_compression_s3tc', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(ext_texture_compression_s3tc, 'GL_EXT_texture_compression_s3tc')
ext_texture_compression_s3tc['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats s3tc')
add_plain_test(ext_texture_compression_s3tc, 'gen-compressed-teximage')
add_concurrent_test(ext_texture_compression_s3tc, 's3tc-errors')
add_plain_test(ext_texture_compression_s3tc, 's3tc-teximage')
add_plain_test(ext_texture_compression_s3tc, 's3tc-texsubimage')
add_concurrent_test(ext_texture_compression_s3tc, 'getteximage-targets S3TC 2D')
add_concurrent_test(ext_texture_compression_s3tc, 'getteximage-targets S3TC 2D_ARRAY')
add_concurrent_test(ext_texture_compression_s3tc, 'getteximage-targets S3TC CUBE')
add_concurrent_test(ext_texture_compression_s3tc, 'getteximage-targets S3TC CUBE_ARRAY')

ati_texture_compression_3dc = {}
spec['ATI_texture_compression_3dc'] = ati_texture_compression_3dc
add_fbo_generatemipmap_extension(ati_texture_compression_3dc, 'GL_ATI_texture_compression_3dc', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(ati_texture_compression_3dc, 'GL_ATI_texture_compression_3dc')
ati_texture_compression_3dc['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats 3dc')

ext_packed_float = {}
spec['EXT_packed_float'] = ext_packed_float
add_fbo_formats_tests('spec/EXT_packed_float', 'GL_EXT_packed_float')
add_texwrap_format_tests(ext_packed_float, 'GL_EXT_packed_float')
ext_packed_float['pack'] = concurrent_test('ext_packed_float-pack')
ext_packed_float['getteximage-invalid-format-for-packed-type'] = concurrent_test('getteximage-invalid-format-for-packed-type')
add_msaa_formats_tests(ext_packed_float, 'GL_EXT_packed_float')

arb_texture_float = {}
spec['ARB_texture_float'] = arb_texture_float
add_fbo_formats_tests('spec/ARB_texture_float', 'GL_ARB_texture_float')
add_texwrap_format_tests(arb_texture_float, 'GL_ARB_texture_float')
add_plain_test(arb_texture_float, 'arb_texture_float-texture-float-formats')
add_msaa_formats_tests(arb_texture_float, 'GL_ARB_texture_float')

ext_texture_integer = {}
spec['EXT_texture_integer'] = ext_texture_integer
# unsupported for int yet
#add_fbo_clear_extension(ext_texture_integer, 'GL_EXT_texture_integer', 'fbo-clear-formats')
ext_texture_integer['api-drawpixels'] = concurrent_test('ext_texture_integer-api-drawpixels')
ext_texture_integer['api-teximage'] = concurrent_test('ext_texture_integer-api-teximage')
ext_texture_integer['api-readpixels'] = concurrent_test('ext_texture_integer-api-readpixels')
ext_texture_integer['fbo-blending'] = concurrent_test('ext_texture_integer-fbo-blending')
ext_texture_integer['fbo-blending GL_ARB_texture_rg'] = concurrent_test('ext_texture_integer-fbo-blending GL_ARB_texture_rg')
ext_texture_integer['fbo_integer_precision_clear'] = plain_test('ext_texture_integer-fbo_integer_precision_clear')
ext_texture_integer['fbo_integer_readpixels_sint_uint'] = plain_test('ext_texture_integer-fbo_integer_readpixels_sint_uint')
ext_texture_integer['getteximage-clamping'] = concurrent_test('ext_texture_integer-getteximage-clamping')
ext_texture_integer['getteximage-clamping GL_ARB_texture_rg'] = concurrent_test('ext_texture_integer-getteximage-clamping GL_ARB_texture_rg')
ext_texture_integer['texture_integer_glsl130'] = concurrent_test('ext_texture_integer-texture_integer_glsl130')
add_msaa_formats_tests(ext_texture_integer, 'GL_EXT_texture_integer')
add_texwrap_format_tests(ext_texture_integer, 'GL_EXT_texture_integer')
add_plain_test(ext_texture_integer, 'fbo-integer')

arb_texture_rg = {}
spec['ARB_texture_rg'] = arb_texture_rg
add_shader_test_dir(arb_texture_rg,
                    testsDir + '/spec/arb_texture_rg/execution',
                    recursive=True)
add_fbo_formats_tests('spec/ARB_texture_rg', 'GL_ARB_texture_rg')
add_fbo_formats_tests('spec/ARB_texture_rg', 'GL_ARB_texture_rg-float', '-float')
# unsupported for int yet
#add_fbo_clear_extension(arb_texture_rg, 'GL_ARB_texture_rg-int', 'fbo-clear-formats-int')
add_msaa_formats_tests(arb_texture_rg, 'GL_ARB_texture_rg')
add_msaa_formats_tests(arb_texture_rg, 'GL_ARB_texture_rg-int')
add_msaa_formats_tests(arb_texture_rg, 'GL_ARB_texture_rg-float')
add_texwrap_format_tests(arb_texture_rg, 'GL_ARB_texture_rg')
add_texwrap_format_tests(arb_texture_rg, 'GL_ARB_texture_rg-float', '-float')
add_texwrap_format_tests(arb_texture_rg, 'GL_ARB_texture_rg-int', '-int')
add_fbo_rg(arb_texture_rg, 'GL_RED')
add_fbo_rg(arb_texture_rg, 'GL_R8')
add_fbo_rg(arb_texture_rg, 'GL_R16')
add_fbo_rg(arb_texture_rg, 'GL_RG')
add_fbo_rg(arb_texture_rg, 'GL_RG8')
add_fbo_rg(arb_texture_rg, 'GL_RG16')
add_plain_test(arb_texture_rg, 'depth-tex-modes-rg')
add_plain_test(arb_texture_rg, 'rg-draw-pixels')
add_plain_test(arb_texture_rg, 'rg-teximage-01')
add_plain_test(arb_texture_rg, 'rg-teximage-02')
add_plain_test(arb_texture_rg, 'texture-rg')

arb_texture_rgb10_a2ui = {}
spec['ARB_texture_rgb10_a2ui'] = arb_texture_rgb10_a2ui
arb_texture_rgb10_a2ui['fbo-blending'] = concurrent_test('ext_texture_integer-fbo-blending GL_ARB_texture_rgb10_a2ui')
add_texwrap_format_tests(arb_texture_rgb10_a2ui, 'GL_ARB_texture_rgb10_a2ui')

ext_texture_shared_exponent = {}
spec['EXT_texture_shared_exponent'] = ext_texture_shared_exponent
ext_texture_shared_exponent['fbo-generatemipmap-formats'] = concurrent_test('fbo-generatemipmap-formats GL_EXT_texture_shared_exponent')
add_texwrap_format_tests(ext_texture_shared_exponent, 'GL_EXT_texture_shared_exponent')

ext_texture_snorm = {}
spec['EXT_texture_snorm'] = ext_texture_snorm
add_fbo_formats_tests('spec/EXT_texture_snorm', 'GL_EXT_texture_snorm')
add_msaa_formats_tests(ext_texture_snorm, 'GL_EXT_texture_snorm')
add_texwrap_format_tests(ext_texture_snorm, 'GL_EXT_texture_snorm')

ext_texture_srgb = {}
spec['EXT_texture_sRGB'] = ext_texture_srgb
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_SRGB_S3TC_DXT1_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT')
add_concurrent_test(ext_texture_compression_s3tc, 'compressedteximage GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT')
add_fbo_generatemipmap_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-generatemipmap-formats')
# TODO: also use GL_ARB_framebuffer_sRGB:
#add_fbo_blending_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-blending-formats')
add_fbo_alphatest_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-alphatest-formats')
add_fbo_generatemipmap_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB-s3tc', 'fbo-generatemipmap-formats-s3tc')
ext_texture_srgb['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats srgb')
add_msaa_formats_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB')
add_texwrap_format_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB')
add_texwrap_format_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB-s3tc', '-s3tc')
add_plain_test(ext_texture_srgb, 'fbo-srgb')
add_plain_test(ext_texture_srgb, 'tex-srgb')

ext_timer_query = {}
spec['EXT_timer_query'] = ext_timer_query
ext_timer_query['time-elapsed'] = concurrent_test('ext_timer_query-time-elapsed')
add_plain_test(ext_timer_query, 'timer_query')

arb_timer_query = {}
spec['ARB_timer_query'] = arb_timer_query
arb_timer_query['query GL_TIMESTAMP'] = concurrent_test('ext_timer_query-time-elapsed timestamp')
arb_timer_query['query-lifetime'] = concurrent_test('ext_timer_query-lifetime')
arb_timer_query['timestamp-get'] = concurrent_test('arb_timer_query-timestamp-get')

ext_transform_feedback = {}
spec['EXT_transform_feedback'] = ext_transform_feedback
for mode in ['interleaved_ok_base', 'interleaved_ok_range',
             'interleaved_ok_offset', 'interleaved_unbound',
             'interleaved_no_varyings', 'separate_ok_1',
             'separate_unbound_0_1', 'separate_ok_2', 'separate_unbound_0_2',
             'separate_unbound_1_2', 'separate_no_varyings', 'no_prog_active',
             'begin_active', 'useprog_active', 'link_current_active',
             'link_other_active', 'bind_base_active', 'bind_range_active',
             'bind_offset_active', 'end_inactive', 'bind_base_max',
             'bind_range_max', 'bind_offset_max', 'bind_range_size_m4',
             'bind_range_size_0', 'bind_range_size_1', 'bind_range_size_2',
             'bind_range_size_3', 'bind_range_size_5', 'bind_range_offset_1',
             'bind_range_offset_2', 'bind_range_offset_3',
             'bind_range_offset_5', 'bind_offset_offset_1',
             'bind_offset_offset_2', 'bind_offset_offset_3',
             'bind_offset_offset_5', 'not_a_program',
             'useprogstage_noactive', 'useprogstage_active',
             'bind_pipeline']:
    test_name = 'api-errors {0}'.format(mode)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))
for varying in ['gl_Color', 'gl_SecondaryColor', 'gl_TexCoord',
                'gl_FogFragCoord', 'gl_Position', 'gl_PointSize',
                'gl_ClipVertex', 'gl_ClipDistance',
                'gl_ClipDistance[1]-no-subscript',
                'gl_ClipDistance[2]-no-subscript',
                'gl_ClipDistance[3]-no-subscript',
                'gl_ClipDistance[4]-no-subscript',
                'gl_ClipDistance[5]-no-subscript',
                'gl_ClipDistance[6]-no-subscript',
                'gl_ClipDistance[7]-no-subscript',
                'gl_ClipDistance[8]-no-subscript']:
    test_name = 'builtin-varyings {0}'.format(varying)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))
ext_transform_feedback['buffer-usage'] = concurrent_test('ext_transform_feedback-buffer-usage')
ext_transform_feedback['discard-api'] = concurrent_test('ext_transform_feedback-discard-api')
ext_transform_feedback['discard-bitmap'] = concurrent_test('ext_transform_feedback-discard-bitmap')
ext_transform_feedback['discard-clear'] = concurrent_test('ext_transform_feedback-discard-clear')
ext_transform_feedback['discard-copypixels'] = concurrent_test('ext_transform_feedback-discard-copypixels')
ext_transform_feedback['discard-drawarrays'] = concurrent_test('ext_transform_feedback-discard-drawarrays')
ext_transform_feedback['discard-drawpixels'] = concurrent_test('ext_transform_feedback-discard-drawpixels')
for mode in ['main_binding', 'indexed_binding', 'buffer_start', 'buffer_size']:
    test_name = 'get-buffer-state {0}'.format(mode)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))
ext_transform_feedback['immediate-reuse'] = concurrent_test('ext_transform_feedback-immediate-reuse')
for mode in ['output', 'prims_generated', 'prims_written']:
    for use_gs in ['', ' use_gs']:
        test_name = 'intervening-read {0}{1}'.format(mode, use_gs)
        ext_transform_feedback[test_name] = concurrent_test(
                'ext_transform_feedback-{0}'.format(test_name))
ext_transform_feedback['max-varyings'] = concurrent_test('ext_transform_feedback-max-varyings')
ext_transform_feedback['nonflat-integral'] = concurrent_test('ext_transform_feedback-nonflat-integral')
ext_transform_feedback['overflow-edge-cases'] = concurrent_test('ext_transform_feedback-overflow-edge-cases')
ext_transform_feedback['overflow-edge-cases use_gs'] = concurrent_test('ext_transform_feedback-overflow-edge-cases use_gs')
ext_transform_feedback['position-readback-bufferbase'] = concurrent_test('ext_transform_feedback-position')
ext_transform_feedback['position-readback-bufferbase-discard'] = concurrent_test('ext_transform_feedback-position discard')
ext_transform_feedback['position-readback-bufferoffset'] = concurrent_test('ext_transform_feedback-position offset')
ext_transform_feedback['position-readback-bufferoffset-discard'] = concurrent_test('ext_transform_feedback-position offset discard')
ext_transform_feedback['position-readback-bufferrange'] = concurrent_test('ext_transform_feedback-position range')
ext_transform_feedback['position-readback-bufferrange-discard'] = concurrent_test('ext_transform_feedback-position range discard')

ext_transform_feedback['negative-prims'] = concurrent_test('ext_transform_feedback-negative-prims')
ext_transform_feedback['primgen-query transform-feedback-disabled'] = concurrent_test('ext_transform_feedback-primgen')

ext_transform_feedback['position-render-bufferbase'] = concurrent_test('ext_transform_feedback-position render')
ext_transform_feedback['position-render-bufferbase-discard'] = concurrent_test('ext_transform_feedback-position render discard')
ext_transform_feedback['position-render-bufferoffset'] = concurrent_test('ext_transform_feedback-position render offset')
ext_transform_feedback['position-render-bufferoffset-discard'] = concurrent_test('ext_transform_feedback-position render offset discard')
ext_transform_feedback['position-render-bufferrange'] = concurrent_test('ext_transform_feedback-position render range')
ext_transform_feedback['position-render-bufferrange-discard'] = concurrent_test('ext_transform_feedback-position render range discard')

ext_transform_feedback['query-primitives_generated-bufferbase'] = concurrent_test('ext_transform_feedback-position primgen')
ext_transform_feedback['query-primitives_generated-bufferbase-discard'] = concurrent_test('ext_transform_feedback-position primgen discard')
ext_transform_feedback['query-primitives_generated-bufferoffset'] = concurrent_test('ext_transform_feedback-position primgen offset')
ext_transform_feedback['query-primitives_generated-bufferoffset-discard'] = concurrent_test('ext_transform_feedback-position primgen offset discard')
ext_transform_feedback['query-primitives_generated-bufferrange'] = concurrent_test('ext_transform_feedback-position primgen range')
ext_transform_feedback['query-primitives_generated-bufferrange-discard'] = concurrent_test('ext_transform_feedback-position primgen range discard')

ext_transform_feedback['query-primitives_written-bufferbase'] = concurrent_test('ext_transform_feedback-position primwritten')
ext_transform_feedback['query-primitives_written-bufferbase-discard'] = concurrent_test('ext_transform_feedback-position primwritten discard')
ext_transform_feedback['query-primitives_written-bufferoffset'] = concurrent_test('ext_transform_feedback-position primwritten offset')
ext_transform_feedback['query-primitives_written-bufferoffset-discard'] = concurrent_test('ext_transform_feedback-position primwritten offset discard')
ext_transform_feedback['query-primitives_written-bufferrange'] = concurrent_test('ext_transform_feedback-position primwritten range')
ext_transform_feedback['query-primitives_written-bufferrange-discard'] = concurrent_test('ext_transform_feedback-position primwritten range discard')

ext_transform_feedback['interleaved-attribs'] = concurrent_test('ext_transform_feedback-interleaved')
ext_transform_feedback['separate-attribs'] = concurrent_test('ext_transform_feedback-separate')
for drawcall in ['arrays', 'elements']:
    for mode in ['triangles', 'lines', 'points']:
        test_name = 'order {0} {1}'.format(drawcall, mode)
        ext_transform_feedback[test_name] = concurrent_test(
                'ext_transform_feedback-{0}'.format(test_name))
for draw_mode in ['points', 'lines', 'line_loop', 'line_strip',
                  'triangles', 'triangle_strip', 'triangle_fan',
                  'quads', 'quad_strip', 'polygon']:
    for shade_mode in ['monochrome', 'smooth', 'flat_first', 'flat_last', 'wireframe']:
        if shade_mode == 'wireframe' and \
                    draw_mode in ['points', 'lines', 'line_loop', 'line_strip']:
            continue
        test_name = 'tessellation {0} {1}'.format(
                draw_mode, shade_mode)
        ext_transform_feedback[test_name] = concurrent_test(
                'ext_transform_feedback-{0}'.format(test_name))
for alignment in [0, 4, 8, 12]:
    test_name = 'alignment {0}'.format(alignment)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))

for output_type in ['float', 'vec2', 'vec3', 'vec4', 'mat2', 'mat2x3',
                    'mat2x4', 'mat3x2', 'mat3', 'mat3x4', 'mat4x2', 'mat4x3',
                    'mat4', 'int', 'ivec2', 'ivec3', 'ivec4', 'uint', 'uvec2',
                    'uvec3', 'uvec4']:
    for suffix in ['', '[2]', '[2]-no-subscript']:
        test_name = 'output-type {0}{1}'.format(output_type, suffix)
        ext_transform_feedback[test_name] = concurrent_test(
                'ext_transform_feedback-{0}'.format(test_name))

for mode in ['discard', 'buffer', 'prims_generated', 'prims_written']:
    test_name = 'generatemipmap {0}'.format(mode)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))

for test_case in ['base-shrink', 'base-grow', 'offset-shrink', 'offset-grow',
                  'range-shrink', 'range-grow']:
    test_name = 'change-size {0}'.format(test_case)
    ext_transform_feedback[test_name] = concurrent_test(
            'ext_transform_feedback-{0}'.format(test_name))
for api_suffix, possible_options in [('', [[], ['interface']]),
                                     ('_gles3', [[]])]:
    for subtest in ['basic-struct', 'struct-whole-array',
                    'struct-array-elem', 'array-struct',
                    'array-struct-whole-array', 'array-struct-array-elem',
                    'struct-struct', 'array-struct-array-struct']:
        for mode in ['error', 'get', 'run', 'run-no-fs']:
            for options in possible_options:
                args = [subtest, mode] + options
                test_name = 'structs{0} {1}'.format(
                        api_suffix, ' '.join(args))
                ext_transform_feedback[test_name] = concurrent_test(
                        'ext_transform_feedback-{0}'.format(test_name))
ext_transform_feedback['geometry-shaders-basic'] = concurrent_test(
        'ext_transform_feedback-geometry-shaders-basic')

arb_transform_feedback2 = {}
spec['ARB_transform_feedback2'] = arb_transform_feedback2
arb_transform_feedback2['Change objects while paused'] = PiglitTest(['arb_transform_feedback2-change-objects-while-paused', '-auto'])
arb_transform_feedback2['Change objects while paused (GLES3)'] = PiglitTest(['arb_transform_feedback2-change-objects-while-paused_gles3', '-auto'])
arb_transform_feedback2['draw-auto'] = PiglitTest(['arb_transform_feedback2-draw-auto', '-auto'])
arb_transform_feedback2['istranformfeedback'] = PiglitTest(['arb_transform_feedback2-istransformfeedback', '-auto'])
arb_transform_feedback2['glGenTransformFeedbacks names only'] = concurrent_test('arb_transform_feedback2-gen-names-only')
arb_transform_feedback2['cannot bind when another object is active'] = concurrent_test('arb_transform_feedback2-cannot-bind-when-active')
arb_transform_feedback2['misc. API queries'] = concurrent_test('arb_transform_feedback2-api-queries')
arb_transform_feedback2['counting with pause'] = concurrent_test('arb_transform_feedback2-pause-counting')

arb_transform_feedback_instanced = {}
spec['ARB_transform_feedback_instanced'] = arb_transform_feedback_instanced
arb_transform_feedback_instanced['draw-auto instanced'] = PiglitTest(['arb_transform_feedback2-draw-auto', '-auto', 'instanced'])

arb_transform_feedback3 = {}
spec['ARB_transform_feedback3'] = arb_transform_feedback3

for param in ['gl_NextBuffer-1', 'gl_NextBuffer-2', 'gl_SkipComponents1-1',
              'gl_SkipComponents1-2', 'gl_SkipComponents1-3', 'gl_SkipComponents2',
              'gl_SkipComponents3', 'gl_SkipComponents4',
              'gl_NextBuffer-gl_SkipComponents1-gl_NextBuffer',
              'gl_NextBuffer-gl_NextBuffer', 'gl_SkipComponents1234']:
    arb_transform_feedback3[param] = concurrent_test(
            'ext_transform_feedback-output-type {0}'.format(param))

arb_transform_feedback3['arb_transform_feedback3-bind_buffer_invalid_index'] = PiglitTest(['arb_transform_feedback3-bind_buffer_invalid_index', '-auto'])
arb_transform_feedback3['arb_transform_feedback3-query_with_invalid_index'] = PiglitTest(['arb_transform_feedback3-query_with_invalid_index', '-auto'])
arb_transform_feedback3['arb_transform_feedback3-end_query_with_name_zero'] = PiglitTest(['arb_transform_feedback3-end_query_with_name_zero', '-auto'])
arb_transform_feedback3['arb_transform_feedback3-draw_using_invalid_stream_index'] = PiglitTest(['arb_transform_feedback3-draw_using_invalid_stream_index', '-auto'])
arb_transform_feedback3['arb_transform_feedback3-set_varyings_with_invalid_args'] = PiglitTest(['arb_transform_feedback3-set_varyings_with_invalid_args', '-auto'])
arb_transform_feedback3['arb_transform_feedback3-set_invalid_varyings'] = PiglitTest(['arb_transform_feedback3-set_invalid_varyings', '-auto'])

arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_vs'] = PiglitTest(['arb_transform_feedback3-ext_interleaved_two_bufs', '-auto', 'vs'])
arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_gs'] = PiglitTest(['arb_transform_feedback3-ext_interleaved_two_bufs', '-auto', 'gs'])
arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_gs_max'] = PiglitTest(['arb_transform_feedback3-ext_interleaved_two_bufs', '-auto', 'gs_max'])

arb_uniform_buffer_object = {}
spec['ARB_uniform_buffer_object'] = arb_uniform_buffer_object
import_glsl_parser_tests(spec['ARB_uniform_buffer_object'],
                         os.path.join(testsDir, 'spec', 'arb_uniform_buffer_object'),
                         [''])
add_shader_test_dir(spec['ARB_uniform_buffer_object'],
                    os.path.join(testsDir, 'spec', 'arb_uniform_buffer_object'),
                    recursive=True)
arb_uniform_buffer_object['bindbuffer-general-point'] = concurrent_test('arb_uniform_buffer_object-bindbuffer-general-point')
arb_uniform_buffer_object['buffer-targets'] = concurrent_test('arb_uniform_buffer_object-buffer-targets')
arb_uniform_buffer_object['bufferstorage'] = concurrent_test('arb_uniform_buffer_object-bufferstorage')
arb_uniform_buffer_object['deletebuffers'] = concurrent_test('arb_uniform_buffer_object-deletebuffers')
arb_uniform_buffer_object['dlist'] = concurrent_test('arb_uniform_buffer_object-dlist')
arb_uniform_buffer_object['getactiveuniformblockiv-uniform-block-data-size'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformblockiv-uniform-block-data-size')
arb_uniform_buffer_object['getactiveuniformblockname'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformblockname')
arb_uniform_buffer_object['getactiveuniformname'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformname')
arb_uniform_buffer_object['getactiveuniformsiv-uniform-array-stride'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformsiv-uniform-array-stride')
arb_uniform_buffer_object['getactiveuniformsiv-uniform-block-index'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformsiv-uniform-block-index')
arb_uniform_buffer_object['getactiveuniformsiv-uniform-matrix-stride'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformsiv-uniform-matrix-stride')
arb_uniform_buffer_object['getactiveuniformsiv-uniform-type'] = concurrent_test('arb_uniform_buffer_object-getactiveuniformsiv-uniform-type')
arb_uniform_buffer_object['getintegeri_v'] = concurrent_test('arb_uniform_buffer_object-getintegeri_v')
arb_uniform_buffer_object['getprogramiv'] = concurrent_test('arb_uniform_buffer_object-getprogramiv')
arb_uniform_buffer_object['getuniformblockindex'] = concurrent_test('arb_uniform_buffer_object-getuniformblockindex')
arb_uniform_buffer_object['getuniformindices'] = concurrent_test('arb_uniform_buffer_object-getuniformindices')
arb_uniform_buffer_object['getuniformlocation'] = concurrent_test('arb_uniform_buffer_object-getuniformlocation')
arb_uniform_buffer_object['layout-std140'] = concurrent_test('arb_uniform_buffer_object-layout-std140')
arb_uniform_buffer_object['layout-std140-base-size-and-alignment'] = concurrent_test('arb_uniform_buffer_object-layout-std140-base-size-and-alignment')
arb_uniform_buffer_object['link-mismatch-blocks'] = concurrent_test('arb_uniform_buffer_object-link-mismatch-blocks')
arb_uniform_buffer_object['maxblocks'] = concurrent_test('arb_uniform_buffer_object-maxblocks')
arb_uniform_buffer_object['maxuniformblocksize/vs'] = concurrent_test('arb_uniform_buffer_object-maxuniformblocksize vs')
arb_uniform_buffer_object['maxuniformblocksize/vsexceed'] = concurrent_test('arb_uniform_buffer_object-maxuniformblocksize vsexceed')
arb_uniform_buffer_object['maxuniformblocksize/fs'] = concurrent_test('arb_uniform_buffer_object-maxuniformblocksize fs')
arb_uniform_buffer_object['maxuniformblocksize/fsexceed'] = concurrent_test('arb_uniform_buffer_object-maxuniformblocksize fsexceed')
arb_uniform_buffer_object['minmax'] = concurrent_test('arb_uniform_buffer_object-minmax')
arb_uniform_buffer_object['negative-bindbuffer-index'] = concurrent_test('arb_uniform_buffer_object-negative-bindbuffer-index')
arb_uniform_buffer_object['negative-bindbuffer-target'] = concurrent_test('arb_uniform_buffer_object-negative-bindbuffer-target')
arb_uniform_buffer_object['negative-bindbufferrange-range'] = concurrent_test('arb_uniform_buffer_object-negative-bindbufferrange-range')
arb_uniform_buffer_object['negative-getactiveuniformblockiv'] = concurrent_test('arb_uniform_buffer_object-negative-getactiveuniformblockiv')
arb_uniform_buffer_object['negative-getactiveuniformsiv'] = concurrent_test('arb_uniform_buffer_object-negative-getactiveuniformsiv')
arb_uniform_buffer_object['referenced-by-shader'] = concurrent_test('arb_uniform_buffer_object-referenced-by-shader')
arb_uniform_buffer_object['rendering'] = concurrent_test('arb_uniform_buffer_object-rendering')
arb_uniform_buffer_object['row-major'] = concurrent_test('arb_uniform_buffer_object-row-major')
arb_uniform_buffer_object['uniformblockbinding'] = concurrent_test('arb_uniform_buffer_object-uniformblockbinding')

ati_draw_buffers = {}
spec['ATI_draw_buffers'] = ati_draw_buffers
add_plain_test(ati_draw_buffers, 'ati_draw_buffers-arbfp')
ati_draw_buffers['arbfp-no-index'] = PiglitTest(['ati_draw_buffers-arbfp-no-index', '-auto'])
ati_draw_buffers['arbfp-no-option'] = PiglitTest(['ati_draw_buffers-arbfp-no-option', '-auto'])

ati_envmap_bumpmap = {}
spec['ATI_envmap_bumpmap'] = ati_envmap_bumpmap
add_plain_test(ati_envmap_bumpmap, 'ati_envmap_bumpmap-bump')

arb_instanced_arrays = {}
spec['ARB_instanced_arrays'] = arb_instanced_arrays
add_plain_test(arb_instanced_arrays, 'arb_instanced_arrays-vertex-attrib-divisor-index-error')
add_plain_test(arb_instanced_arrays, 'arb_instanced_arrays-instanced_arrays')
add_plain_test(arb_instanced_arrays, 'arb_instanced_arrays-drawarrays')
add_single_param_test_set(arb_instanced_arrays, 'arb_instanced_arrays-instanced_arrays', 'vbo')

arb_internalformat_query = {}
spec['ARB_internalformat_query'] = arb_internalformat_query
arb_internalformat_query['misc. API error checks'] = concurrent_test('arb_internalformat_query-api-errors')
arb_internalformat_query['buffer over-run checks'] = concurrent_test('arb_internalformat_query-overrun')
arb_internalformat_query['minmax'] = concurrent_test('arb_internalformat_query-minmax')

arb_map_buffer_range = {}
spec['ARB_map_buffer_range'] = arb_map_buffer_range
add_plain_test(arb_map_buffer_range, 'map_buffer_range_error_check')
add_plain_test(arb_map_buffer_range, 'map_buffer_range_test')
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT offset=0'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_RANGE_BIT offset=0')
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT increment-offset'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_RANGE_BIT increment-offset')
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT decrement-offset'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_RANGE_BIT decrement-offset')
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT offset=0'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_BUFFER_BIT offset=0')
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT increment-offset'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_BUFFER_BIT increment-offset')
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT decrement-offset'] = concurrent_test('map_buffer_range-invalidate MAP_INVALIDATE_BUFFER_BIT decrement-offset')
arb_map_buffer_range['CopyBufferSubData offset=0'] = concurrent_test('map_buffer_range-invalidate CopyBufferSubData offset=0')
arb_map_buffer_range['CopyBufferSubData increment-offset'] = concurrent_test('map_buffer_range-invalidate CopyBufferSubData increment-offset')
arb_map_buffer_range['CopyBufferSubData decrement-offset'] = concurrent_test('map_buffer_range-invalidate CopyBufferSubData decrement-offset')

arb_multisample = {}
spec['ARB_multisample'] = arb_multisample
arb_multisample['beginend'] = concurrent_test('arb_multisample-beginend')
arb_multisample['pushpop'] = concurrent_test('arb_multisample-pushpop')

arb_seamless_cube_map = {}
spec['ARB_seamless_cube_map'] = arb_seamless_cube_map
add_plain_test(arb_seamless_cube_map, 'arb_seamless_cubemap')
add_plain_test(arb_seamless_cube_map, 'arb_seamless_cubemap-initially-disabled')
add_plain_test(arb_seamless_cube_map, 'arb_seamless_cubemap-three-faces-average')

amd_seamless_cubemap_per_texture = {}
spec['AMD_seamless_cubemap_per_texture'] = amd_seamless_cubemap_per_texture
add_plain_test(amd_seamless_cubemap_per_texture, 'amd_seamless_cubemap_per_texture')

amd_vertex_shader_layer = {}
spec['AMD_vertex_shader_layer'] = amd_vertex_shader_layer
add_plain_test(amd_vertex_shader_layer, 'amd_vertex_shader_layer-layered-2d-texture-render')
add_plain_test(amd_vertex_shader_layer, 'amd_vertex_shader_layer-layered-depth-texture-render')

amd_vertex_shader_viewport_index = {}
spec['AMD_vertex_shader_viewport_index'] = amd_vertex_shader_viewport_index
add_concurrent_test(amd_vertex_shader_viewport_index, 'amd_vertex_shader_viewport_index-render')

ext_fog_coord = {}
spec['EXT_fog_coord'] = ext_fog_coord
add_plain_test(ext_fog_coord, 'ext_fog_coord-modes')

ext_shader_integer_mix = {}
spec['EXT_shader_integer_mix'] = ext_shader_integer_mix
add_shader_test_dir(spec['EXT_shader_integer_mix'],
                    os.path.join(testsDir, 'spec', 'ext_shader_integer_mix'),
                    recursive=True)

nv_texture_barrier = {}
spec['NV_texture_barrier'] = nv_texture_barrier
add_plain_test(nv_texture_barrier, 'blending-in-shader')

nv_conditional_render = {}
spec['NV_conditional_render'] = nv_conditional_render
nv_conditional_render['begin-while-active'] = concurrent_test('nv_conditional_render-begin-while-active')
nv_conditional_render['begin-zero'] = concurrent_test('nv_conditional_render-begin-zero')
nv_conditional_render['bitmap'] = PiglitTest(['nv_conditional_render-bitmap', '-auto'])
nv_conditional_render['blitframebuffer'] = PiglitTest(['nv_conditional_render-blitframebuffer', '-auto'])
nv_conditional_render['clear'] = PiglitTest(['nv_conditional_render-clear', '-auto'])
nv_conditional_render['copypixels'] = PiglitTest(['nv_conditional_render-copypixels', '-auto'])
nv_conditional_render['copyteximage'] = PiglitTest(['nv_conditional_render-copyteximage', '-auto'])
nv_conditional_render['copytexsubimage'] = PiglitTest(['nv_conditional_render-copytexsubimage', '-auto'])
nv_conditional_render['dlist'] = PiglitTest(['nv_conditional_render-dlist', '-auto'])
nv_conditional_render['drawpixels'] = PiglitTest(['nv_conditional_render-drawpixels', '-auto'])
nv_conditional_render['generatemipmap'] = PiglitTest(['nv_conditional_render-generatemipmap', '-auto'])
nv_conditional_render['vertex_array'] = PiglitTest(['nv_conditional_render-vertex_array', '-auto'])

oes_compressed_paletted_texture = {}
spec['OES_compressed_paletted_texture'] = oes_compressed_paletted_texture
oes_compressed_paletted_texture['invalid formats'] = concurrent_test('arb_texture_compression-invalid-formats paletted')

oes_matrix_get = {}
spec['OES_matrix_get'] = oes_matrix_get
oes_matrix_get['All queries'] = concurrent_test('oes_matrix_get-api')

oes_fixed_point = {}
spec['OES_fixed_point'] = oes_fixed_point
oes_fixed_point['attribute-arrays'] = concurrent_test('oes_fixed_point-attribute-arrays')

spec['OES_standard_derivatives'] = {}
import_glsl_parser_tests(spec['OES_standard_derivatives'],
                         os.path.join(testsDir, 'spec', 'oes_standard_derivatives'),
                         ['compiler'])

arb_clear_buffer_object = {}
spec['ARB_clear_buffer_object'] = arb_clear_buffer_object
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-formats')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-invalid-internal-format')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-invalid-size')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-mapped')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-no-bound-buffer')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-null-data')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-sub-invalid-size')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-sub-mapped')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-sub-overlap')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-sub-simple')
add_concurrent_test(arb_clear_buffer_object, 'arb_clear_buffer_object-zero-size')

arb_clear_texture = {}
spec['ARB_clear_texture'] = arb_clear_texture
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-simple')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-error')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-3d')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-cube')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-multisample')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-integer')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-base-formats')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-sized-formats')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-float')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-rg')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-depth-stencil')
add_concurrent_test(arb_clear_texture, 'arb_clear_texture-srgb')

arb_copy_buffer = {}
spec['ARB_copy_buffer'] = arb_copy_buffer
add_plain_test(arb_copy_buffer, 'copy_buffer_coherency')
add_plain_test(arb_copy_buffer, 'copybuffersubdata')
arb_copy_buffer['data-sync'] = concurrent_test('arb_copy_buffer-data-sync')
arb_copy_buffer['dlist'] = concurrent_test('arb_copy_buffer-dlist')
arb_copy_buffer['get'] = concurrent_test('arb_copy_buffer-get')
arb_copy_buffer['negative-bound-zero'] = concurrent_test('arb_copy_buffer-negative-bound-zero')
arb_copy_buffer['negative-bounds'] = concurrent_test('arb_copy_buffer-negative-bounds')
arb_copy_buffer['negative-mapped'] = concurrent_test('arb_copy_buffer-negative-mapped')
arb_copy_buffer['overlap'] = concurrent_test('arb_copy_buffer-overlap')
arb_copy_buffer['targets'] = concurrent_test('arb_copy_buffer-targets')
arb_copy_buffer['subdata-sync'] = concurrent_test('arb_copy_buffer-subdata-sync')

arb_half_float_vertex = {}
spec['ARB_half_float_vertex'] = arb_half_float_vertex
add_plain_test(arb_half_float_vertex, 'draw-vertices-half-float')
arb_half_float_vertex['draw-vertices-half-float-user'] = PiglitTest(['draw-vertices-half-float', '-auto', 'user'])

arb_vertex_type_2_10_10_10_rev = {}
spec['ARB_vertex_type_2_10_10_10_rev'] = arb_vertex_type_2_10_10_10_rev
add_plain_test(arb_vertex_type_2_10_10_10_rev, 'draw-vertices-2101010')
arb_vertex_type_2_10_10_10_rev['attribs'] = concurrent_test('attribs GL_ARB_vertex_type_2_10_10_10_rev')

arb_vertex_type_10f_11f_11f_rev = {}
spec['ARB_vertex_type_10f_11f_11f_rev'] = arb_vertex_type_10f_11f_11f_rev
add_plain_test(arb_vertex_type_10f_11f_11f_rev, 'arb_vertex_type_10f_11f_11f_rev-api-errors')
add_concurrent_test(arb_vertex_type_10f_11f_11f_rev, 'arb_vertex_type_10f_11f_11f_rev-draw-vertices')

arb_draw_buffers = {}
spec['ARB_draw_buffers'] = arb_draw_buffers
add_plain_test(arb_draw_buffers, 'arb_draw_buffers-state_change')
add_plain_test(arb_draw_buffers, 'fbo-mrt-alphatest')

ext_draw_buffers2 = {}
spec['EXT_draw_buffers2'] = ext_draw_buffers2
add_plain_test(ext_draw_buffers2, 'fbo-drawbuffers2-blend')
add_plain_test(ext_draw_buffers2, 'fbo-drawbuffers2-colormask')
add_plain_test(ext_draw_buffers2, 'fbo-drawbuffers2-colormask clear')

arb_draw_buffers_blend = {}
spec['ARB_draw_buffers_blend'] = arb_draw_buffers_blend
add_plain_test(arb_draw_buffers_blend, 'fbo-draw-buffers-blend')

arb_blend_func_extended = {}
spec['ARB_blend_func_extended'] = arb_blend_func_extended
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-bindfragdataindexed-invalid-parameters')
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-blend-api')
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-error-at-begin')
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-getfragdataindex')
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-fbo-extended-blend')
add_plain_test(arb_blend_func_extended, 'arb_blend_func_extended-fbo-extended-blend-explicit')

arb_base_instance = {}
spec['ARB_base_instance'] = arb_base_instance
add_plain_test(arb_base_instance, 'arb_base_instance-baseinstance-doesnt-affect-gl-instance-id')
add_concurrent_test(arb_base_instance, 'arb_base_instance-drawarrays')

arb_buffer_storage = {}
spec['ARB_buffer_storage'] = arb_buffer_storage
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent draw')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent draw coherent')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent draw client-storage')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent draw coherent client-storage')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent read')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent read coherent')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent read client-storage')
add_concurrent_test(arb_buffer_storage, 'bufferstorage-persistent read coherent client-storage')

apple_object_purgeable = {}
spec['APPLE_object_purgeable'] = apple_object_purgeable
add_plain_test(apple_object_purgeable, 'object_purgeable-api-pbo')
add_plain_test(apple_object_purgeable, 'object_purgeable-api-texture')
add_plain_test(apple_object_purgeable, 'object_purgeable-api-vbo')

oes_read_format = {}
spec['OES_read_format'] = oes_read_format
add_plain_test(oes_read_format, 'oes-read-format')

nv_primitive_restart = {}
spec['NV_primitive_restart'] = nv_primitive_restart
add_single_param_test_set(
    nv_primitive_restart,
    'primitive-restart',
    "DISABLE_VBO",
    "VBO_VERTEX_ONLY", "VBO_INDEX_ONLY",
    "VBO_SEPARATE_VERTEX_AND_INDEX", "VBO_COMBINED_VERTEX_AND_INDEX")
add_single_param_test_set(
        nv_primitive_restart,
        'primitive-restart-draw-mode',
        'points', 'lines', 'line_loop', 'line_strip', 'triangles',
        'triangle_strip', 'triangle_fan', 'quads', 'quad_strip', 'polygon')

ext_provoking_vertex = {}
spec['EXT_provoking_vertex'] = ext_provoking_vertex
add_plain_test(ext_provoking_vertex, 'provoking-vertex')

ext_texture_lod_bias = {}
spec['EXT_texture_lod_bias'] = ext_texture_lod_bias
add_plain_test(ext_texture_lod_bias, 'lodbias')

sgis_generate_mipmap = {}
spec['SGIS_generate_mipmap'] = sgis_generate_mipmap
add_plain_test(sgis_generate_mipmap, 'gen-nonzero-unit')
add_plain_test(sgis_generate_mipmap, 'gen-teximage')
add_plain_test(sgis_generate_mipmap, 'gen-texsubimage')

arb_map_buffer_alignment = {}
spec['ARB_map_buffer_alignment'] = arb_map_buffer_alignment
add_plain_test(arb_map_buffer_alignment, 'arb_map_buffer_alignment-sanity_test')

arb_geometry_shader4 = {}
for draw in ['', 'indexed']:
    for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY', 'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-ignore-adjacent-vertices {0} {1}'.format(draw, prim))
add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-program-parameter-input-type')
add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-program-parameter-input-type-draw')
add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-program-parameter-output-type')
add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-vertices-in')
for mode in ['1', 'tf 1', 'max', 'tf max']:
    add_concurrent_test(arb_geometry_shader4, 'arb_geometry_shader4-program-parameter-vertices-out {0}'.format(mode))
spec['ARB_geometry_shader4'] = arb_geometry_shader4
add_shader_test_dir(spec['ARB_geometry_shader4'],
                    os.path.join(testsDir, 'spec', 'arb_geometry_shader4'),
                    recursive=True)
import_glsl_parser_tests(spec['ARB_geometry_shader4'],
                         os.path.join(testsDir, 'spec', 'arb_geometry_shader4'),
                         ['compiler'])

arb_compute_shader = {}
spec['ARB_compute_shader'] = arb_compute_shader
arb_compute_shader['api_errors'] = concurrent_test('arb_compute_shader-api_errors')
arb_compute_shader['minmax'] = concurrent_test('arb_compute_shader-minmax')
arb_compute_shader['compiler/work_group_size_too_large'] = \
    concurrent_test('arb_compute_shader-work_group_size_too_large')
add_shader_test_dir(spec['ARB_compute_shader'],
                    os.path.join(testsDir, 'spec', 'arb_compute_shader'),
                    recursive=True)
import_glsl_parser_tests(spec['ARB_compute_shader'],
                         os.path.join(testsDir, 'spec', 'arb_compute_shader'),
                         ['compiler'])
arb_compute_shader['built-in constants'] = concurrent_test('built-in-constants ' + os.path.join(testsDir, 'spec/arb_compute_shader/minimum-maximums.txt'))

# group glslparsertest ------------------------------------------------------
glslparsertest = {}
# Add all shader source files in the directories below.
for filename in os.listdir(testsDir + '/glslparsertest/shaders'):
    ext = filename.rsplit('.')[-1]
    if ext in ['vert', 'geo', 'frag']:
        add_glsl_parser_test(glslparsertest, os.path.join(testsDir, 'glslparsertest/shaders', filename), filename)
for filename in os.listdir(testsDir + '/glslparsertest/glsl2'):
    ext = filename.rsplit('.')[-1]
    if ext in ['vert', 'geo', 'frag']:
        add_glsl_parser_test(glslparsertest, os.path.join(testsDir, 'glslparsertest/glsl2', filename), 'glsl2/' + filename)
# end group glslparsertest ---------------------------------------------------

hiz = {}
add_plain_test(hiz, 'hiz-depth-stencil-test-fbo-d0-s8')
add_plain_test(hiz, 'hiz-depth-stencil-test-fbo-d24-s0')
add_plain_test(hiz, 'hiz-depth-stencil-test-fbo-d24-s8')
add_plain_test(hiz, 'hiz-depth-stencil-test-fbo-d24s8')
add_plain_test(hiz, 'hiz-depth-read-fbo-d24-s0')
add_plain_test(hiz, 'hiz-depth-read-fbo-d24-s8')
add_plain_test(hiz, 'hiz-depth-read-fbo-d24s8')
add_plain_test(hiz, 'hiz-depth-read-window-stencil0')
add_plain_test(hiz, 'hiz-depth-read-window-stencil1')
add_plain_test(hiz, 'hiz-depth-test-fbo-d24-s0')
add_plain_test(hiz, 'hiz-depth-test-fbo-d24-s8')
add_plain_test(hiz, 'hiz-depth-test-fbo-d24s8')
add_plain_test(hiz, 'hiz-depth-test-window-stencil0')
add_plain_test(hiz, 'hiz-depth-test-window-stencil1')
add_plain_test(hiz, 'hiz-stencil-read-fbo-d0-s8')
add_plain_test(hiz, 'hiz-stencil-read-fbo-d24-s8')
add_plain_test(hiz, 'hiz-stencil-read-fbo-d24s8')
add_plain_test(hiz, 'hiz-stencil-read-window-depth0')
add_plain_test(hiz, 'hiz-stencil-read-window-depth1')
add_plain_test(hiz, 'hiz-stencil-test-fbo-d0-s8')
add_plain_test(hiz, 'hiz-stencil-test-fbo-d24-s8')
add_plain_test(hiz, 'hiz-stencil-test-fbo-d24s8')
add_plain_test(hiz, 'hiz-stencil-test-window-depth0')
add_plain_test(hiz, 'hiz-stencil-test-window-depth1')

fast_color_clear = {}
add_shader_test_dir(fast_color_clear, testsDir + '/fast_color_clear',
                    recursive=True)
for subtest in ('sample', 'read_pixels', 'blit', 'copy'):
    for buffer_type in ('rb', 'tex'):
        if subtest == 'sample' and buffer_type == 'rb':
            continue
        test_name = ' '.join(
                ['fcc-read-after-clear', subtest, buffer_type])
        add_concurrent_test(fast_color_clear, test_name)
add_concurrent_test(fast_color_clear, 'fcc-blit-between-clears')
add_plain_test(fast_color_clear, 'fcc-read-to-pbo-after-clear')

asmparsertest = {}
def add_asmparsertest(group, shader):
    asmparsertest[group + '/' + shader] = concurrent_test(
        'asmparsertest ' + group + ' ' +
        os.path.join(testsDir, 'asmparsertest', 'shaders', group, shader))

add_asmparsertest('ARBfp1.0', 'abs-01.txt')
add_asmparsertest('ARBfp1.0', 'abs-02.txt')
add_asmparsertest('ARBfp1.0', 'abs-03.txt')
add_asmparsertest('ARBfp1.0', 'condition_code-01.txt')
add_asmparsertest('ARBfp1.0', 'cos-01.txt')
add_asmparsertest('ARBfp1.0', 'cos-02.txt')
add_asmparsertest('ARBfp1.0', 'cos-03.txt')
add_asmparsertest('ARBfp1.0', 'cos-04.txt')
add_asmparsertest('ARBfp1.0', 'cos-05.txt')
add_asmparsertest('ARBfp1.0', 'ddx-01.txt')
add_asmparsertest('ARBfp1.0', 'ddx-02.txt')
add_asmparsertest('ARBfp1.0', 'ddy-01.txt')
add_asmparsertest('ARBfp1.0', 'ddy-02.txt')
add_asmparsertest('ARBfp1.0', 'depth_range-01.txt')
add_asmparsertest('ARBfp1.0', 'fog-01.txt')
add_asmparsertest('ARBfp1.0', 'fog-02.txt')
add_asmparsertest('ARBfp1.0', 'fog-03.txt')
add_asmparsertest('ARBfp1.0', 'fog-04.txt')
add_asmparsertest('ARBfp1.0', 'option-01.txt')
add_asmparsertest('ARBfp1.0', 'precision_hint-01.txt')
add_asmparsertest('ARBfp1.0', 'precision_hint-02.txt')
add_asmparsertest('ARBfp1.0', 'precision_hint-03.txt')
add_asmparsertest('ARBfp1.0', 'precision_hint-04.txt')
add_asmparsertest('ARBfp1.0', 'precision_hint-05.txt')
add_asmparsertest('ARBfp1.0', 'reserved_words-01.txt')
add_asmparsertest('ARBfp1.0', 'result-01.txt')
add_asmparsertest('ARBfp1.0', 'result-02.txt')
add_asmparsertest('ARBfp1.0', 'result-03.txt')
add_asmparsertest('ARBfp1.0', 'result-04.txt')
add_asmparsertest('ARBfp1.0', 'result-05.txt')
add_asmparsertest('ARBfp1.0', 'result-06.txt')
add_asmparsertest('ARBfp1.0', 'result-07.txt')
add_asmparsertest('ARBfp1.0', 'result-08.txt')
add_asmparsertest('ARBfp1.0', 'result-09.txt')
add_asmparsertest('ARBfp1.0', 'result-10.txt')
add_asmparsertest('ARBfp1.0', 'result-11.txt')
add_asmparsertest('ARBfp1.0', 'shadow-01.txt')
add_asmparsertest('ARBfp1.0', 'shadow-02.txt')
add_asmparsertest('ARBfp1.0', 'shadow-03.txt')
add_asmparsertest('ARBfp1.0', 'sincos-01.txt')
add_asmparsertest('ARBfp1.0', 'sincos-02.txt')
add_asmparsertest('ARBfp1.0', 'sincos-03.txt')
add_asmparsertest('ARBfp1.0', 'sincos-04.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-01.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-02.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-03.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-04.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-05.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-06.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-07.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-08.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-09.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-10.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-11.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-12.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-13.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-14.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-15.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-16.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-17.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-18.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-19.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-20.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-21.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-22.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-23.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-24.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-25.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-26.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-27.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-28.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-29.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-30.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-31.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-32.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-33.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-34.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-35.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-36.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-37.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-38.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-39.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-40.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-41.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-42.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-43.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-44.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-45.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-46.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-47.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-48.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-49.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-50.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-51.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-52.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-53.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-54.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-55.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-56.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-57.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-58.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-59.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-60.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-61.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-62.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-63.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-64.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-65.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-66.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-67.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-68.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-69.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-70.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-71.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-72.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-73.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-74.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-75.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-76.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-77.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-78.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-79.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-80.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-81.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-82.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-83.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-84.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-85.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-86.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-87.txt')
add_asmparsertest('ARBfp1.0', 'size_specifier-88.txt')
add_asmparsertest('ARBfp1.0', 'swz-01.txt')
add_asmparsertest('ARBfp1.0', 'swz-02.txt')
add_asmparsertest('ARBfp1.0', 'swz-03.txt')
add_asmparsertest('ARBfp1.0', 'swz-04.txt')
add_asmparsertest('ARBfp1.0', 'swz-05.txt')
add_asmparsertest('ARBfp1.0', 'swz-06.txt')
add_asmparsertest('ARBfp1.0', 'swz-07.txt')
add_asmparsertest('ARBfp1.0', 'txd-01.txt')
add_asmparsertest('ARBfp1.0', 'txd-02.txt')
add_asmparsertest('ARBfp1.0', 'txd-03.txt')

add_asmparsertest('ARBvp1.0', 'abs-02.txt')
add_asmparsertest('ARBvp1.0', 'abs-03.txt')
add_asmparsertest('ARBvp1.0', 'abs.txt')
add_asmparsertest('ARBvp1.0', 'address-01.txt')
add_asmparsertest('ARBvp1.0', 'address-02.txt')
add_asmparsertest('ARBvp1.0', 'address-03.txt')
add_asmparsertest('ARBvp1.0', 'address-04.txt')
add_asmparsertest('ARBvp1.0', 'address-05.txt')
add_asmparsertest('ARBvp1.0', 'address-07.txt')
add_asmparsertest('ARBvp1.0', 'address-08.txt')
add_asmparsertest('ARBvp1.0', 'address-09.txt')
add_asmparsertest('ARBvp1.0', 'address-10.txt')
add_asmparsertest('ARBvp1.0', 'address-11.txt')
add_asmparsertest('ARBvp1.0', 'add.txt')
add_asmparsertest('ARBvp1.0', 'alias-01.txt')
add_asmparsertest('ARBvp1.0', 'alias-02.txt')
add_asmparsertest('ARBvp1.0', 'all_state-01.txt')
add_asmparsertest('ARBvp1.0', 'all_state-02.txt')
add_asmparsertest('ARBvp1.0', 'all_state-03.txt')
add_asmparsertest('ARBvp1.0', 'all_state-04.txt')
add_asmparsertest('ARBvp1.0', 'all_state-05.txt')
add_asmparsertest('ARBvp1.0', 'all_state-06.txt')
add_asmparsertest('ARBvp1.0', 'all_state-07.txt')
add_asmparsertest('ARBvp1.0', 'all_state-08.txt')
add_asmparsertest('ARBvp1.0', 'all_state-09.txt')
add_asmparsertest('ARBvp1.0', 'all_state-10.txt')
add_asmparsertest('ARBvp1.0', 'all_state-11.txt')
add_asmparsertest('ARBvp1.0', 'all_state-12.txt')
add_asmparsertest('ARBvp1.0', 'all_state-13.txt')
add_asmparsertest('ARBvp1.0', 'all_state-14.txt')
add_asmparsertest('ARBvp1.0', 'all_state-15.txt')
add_asmparsertest('ARBvp1.0', 'all_state-16.txt')
add_asmparsertest('ARBvp1.0', 'all_state-17.txt')
add_asmparsertest('ARBvp1.0', 'all_state-18.txt')
add_asmparsertest('ARBvp1.0', 'ara-01.txt')
add_asmparsertest('ARBvp1.0', 'ara-02.txt')
add_asmparsertest('ARBvp1.0', 'ara-03.txt')
add_asmparsertest('ARBvp1.0', 'ara-04.txt')
add_asmparsertest('ARBvp1.0', 'arbfp.txt')
add_asmparsertest('ARBvp1.0', 'arl-01.txt')
add_asmparsertest('ARBvp1.0', 'arl-02.txt')
add_asmparsertest('ARBvp1.0', 'arl-03.txt')
add_asmparsertest('ARBvp1.0', 'arl-04.txt')
add_asmparsertest('ARBvp1.0', 'arl-05.txt')
add_asmparsertest('ARBvp1.0', 'array_range-01.txt')
add_asmparsertest('ARBvp1.0', 'array_range-02.txt')
add_asmparsertest('ARBvp1.0', 'array_range-03.txt')
add_asmparsertest('ARBvp1.0', 'array_range-04.txt')
add_asmparsertest('ARBvp1.0', 'array_range-05.txt')
add_asmparsertest('ARBvp1.0', 'array_range-06.txt')
add_asmparsertest('ARBvp1.0', 'array_range-07.txt')
add_asmparsertest('ARBvp1.0', 'astack-01.txt')
add_asmparsertest('ARBvp1.0', 'astack-02.txt')
add_asmparsertest('ARBvp1.0', 'astack-03.txt')
add_asmparsertest('ARBvp1.0', 'astack-04.txt')
add_asmparsertest('ARBvp1.0', 'astack-05.txt')
add_asmparsertest('ARBvp1.0', 'astack-06.txt')
add_asmparsertest('ARBvp1.0', 'astack-07.txt')
add_asmparsertest('ARBvp1.0', 'astack-08.txt')
add_asmparsertest('ARBvp1.0', 'astack-09.txt')
add_asmparsertest('ARBvp1.0', 'attrib-01.txt')
add_asmparsertest('ARBvp1.0', 'attrib-02.txt')
add_asmparsertest('ARBvp1.0', 'attrib-03.txt')
add_asmparsertest('ARBvp1.0', 'attrib-04.txt')
add_asmparsertest('ARBvp1.0', 'bra-01.txt')
add_asmparsertest('ARBvp1.0', 'bra-02.txt')
add_asmparsertest('ARBvp1.0', 'bra-03.txt')
add_asmparsertest('ARBvp1.0', 'clipdistance-01.txt')
add_asmparsertest('ARBvp1.0', 'clipdistance-02.txt')
add_asmparsertest('ARBvp1.0', 'clipdistance-03.txt')
add_asmparsertest('ARBvp1.0', 'clipdistance-04.txt')
add_asmparsertest('ARBvp1.0', 'cos-01.txt')
add_asmparsertest('ARBvp1.0', 'cos-02.txt')
add_asmparsertest('ARBvp1.0', 'cos-03.txt')
add_asmparsertest('ARBvp1.0', 'dp3.txt')
add_asmparsertest('ARBvp1.0', 'dp4.txt')
add_asmparsertest('ARBvp1.0', 'dph.txt')
add_asmparsertest('ARBvp1.0', 'dst.txt')
add_asmparsertest('ARBvp1.0', 'ex2.txt')
add_asmparsertest('ARBvp1.0', 'flr.txt')
add_asmparsertest('ARBvp1.0', 'frc.txt')
add_asmparsertest('ARBvp1.0', 'issue-70.txt')
add_asmparsertest('ARBvp1.0', 'issue-74.txt')
add_asmparsertest('ARBvp1.0', 'issue-75.txt')
add_asmparsertest('ARBvp1.0', 'lg2.txt')
add_asmparsertest('ARBvp1.0', 'lit.txt')
add_asmparsertest('ARBvp1.0', 'mad.txt')
add_asmparsertest('ARBvp1.0', 'matrix-01.txt')
add_asmparsertest('ARBvp1.0', 'max.txt')
add_asmparsertest('ARBvp1.0', 'min.txt')
add_asmparsertest('ARBvp1.0', 'mov.txt')
add_asmparsertest('ARBvp1.0', 'mul.txt')
add_asmparsertest('ARBvp1.0', 'numbers-01.txt')
add_asmparsertest('ARBvp1.0', 'numbers-02.txt')
add_asmparsertest('ARBvp1.0', 'numbers-03.txt')
add_asmparsertest('ARBvp1.0', 'numbers-04.txt')
add_asmparsertest('ARBvp1.0', 'numbers-05.txt')
add_asmparsertest('ARBvp1.0', 'numbers-06.txt')
add_asmparsertest('ARBvp1.0', 'numbers-07.txt')
add_asmparsertest('ARBvp1.0', 'option-01.txt')
add_asmparsertest('ARBvp1.0', 'output-01.txt')
add_asmparsertest('ARBvp1.0', 'output-02.txt')
add_asmparsertest('ARBvp1.0', 'param-01.txt')
add_asmparsertest('ARBvp1.0', 'param-02.txt')
add_asmparsertest('ARBvp1.0', 'param-03.txt')
add_asmparsertest('ARBvp1.0', 'param-04.txt')
add_asmparsertest('ARBvp1.0', 'param-05.txt')
add_asmparsertest('ARBvp1.0', 'param-06.txt')
add_asmparsertest('ARBvp1.0', 'param-07.txt')
add_asmparsertest('ARBvp1.0', 'param-08.txt')
add_asmparsertest('ARBvp1.0', 'position_invariant-01.txt')
add_asmparsertest('ARBvp1.0', 'position_invariant-02.txt')
add_asmparsertest('ARBvp1.0', 'pow.txt')
add_asmparsertest('ARBvp1.0', 'rcp-01.txt')
add_asmparsertest('ARBvp1.0', 'rcp-02.txt')
add_asmparsertest('ARBvp1.0', 'rcp-03.txt')
add_asmparsertest('ARBvp1.0', 'rcp-04.txt')
add_asmparsertest('ARBvp1.0', 'rcp-05.txt')
add_asmparsertest('ARBvp1.0', 'rcp-06.txt')
add_asmparsertest('ARBvp1.0', 'rcp-07.txt')
add_asmparsertest('ARBvp1.0', 'reserved_word-01.txt')
add_asmparsertest('ARBvp1.0', 'result-01.txt')
add_asmparsertest('ARBvp1.0', 'result-02.txt')
add_asmparsertest('ARBvp1.0', 'rsq.txt')
add_asmparsertest('ARBvp1.0', 'seq-01.txt')
add_asmparsertest('ARBvp1.0', 'seq-02.txt')
add_asmparsertest('ARBvp1.0', 'sfl-01.txt')
add_asmparsertest('ARBvp1.0', 'sfl-02.txt')
add_asmparsertest('ARBvp1.0', 'sge.txt')
add_asmparsertest('ARBvp1.0', 'sgt-01.txt')
add_asmparsertest('ARBvp1.0', 'sgt-02.txt')
add_asmparsertest('ARBvp1.0', 'sin-01.txt')
add_asmparsertest('ARBvp1.0', 'sin-02.txt')
add_asmparsertest('ARBvp1.0', 'sin-03.txt')
add_asmparsertest('ARBvp1.0', 'sle-01.txt')
add_asmparsertest('ARBvp1.0', 'sle-02.txt')
add_asmparsertest('ARBvp1.0', 'slt.txt')
add_asmparsertest('ARBvp1.0', 'sne-01.txt')
add_asmparsertest('ARBvp1.0', 'sne-02.txt')
add_asmparsertest('ARBvp1.0', 'ssg-01.txt')
add_asmparsertest('ARBvp1.0', 'ssg-02.txt')
add_asmparsertest('ARBvp1.0', 'str-01.txt')
add_asmparsertest('ARBvp1.0', 'str-02.txt')
add_asmparsertest('ARBvp1.0', 'sub.txt')
add_asmparsertest('ARBvp1.0', 'swz-01.txt')
add_asmparsertest('ARBvp1.0', 'swz-02.txt')
add_asmparsertest('ARBvp1.0', 'swz-03.txt')
add_asmparsertest('ARBvp1.0', 'swz-04.txt')
add_asmparsertest('ARBvp1.0', 'swz-05.txt')
add_asmparsertest('ARBvp1.0', 'tex-01.txt')
add_asmparsertest('ARBvp1.0', 'tex-02.txt')
add_asmparsertest('ARBvp1.0', 'tex-03.txt')
add_asmparsertest('ARBvp1.0', 'tex-04.txt')
add_asmparsertest('ARBvp1.0', 'tex-05.txt')
add_asmparsertest('ARBvp1.0', 'tex-06.txt')
add_asmparsertest('ARBvp1.0', 'tex-07.txt')
add_asmparsertest('ARBvp1.0', 'tex-08.txt')
add_asmparsertest('ARBvp1.0', 'tex-09.txt')
add_asmparsertest('ARBvp1.0', 'tex-10.txt')
add_asmparsertest('ARBvp1.0', 'tex-11.txt')
add_asmparsertest('ARBvp1.0', 'tex-12.txt')
add_asmparsertest('ARBvp1.0', 'tex-13.txt')
add_asmparsertest('ARBvp1.0', 'tex-14.txt')
add_asmparsertest('ARBvp1.0', 'tex-15.txt')
add_asmparsertest('ARBvp1.0', 'tex-16.txt')
add_asmparsertest('ARBvp1.0', 'tex-17.txt')
add_asmparsertest('ARBvp1.0', 'tex-18.txt')
add_asmparsertest('ARBvp1.0', 'tex-19.txt')
add_asmparsertest('ARBvp1.0', 'tex-20.txt')
add_asmparsertest('ARBvp1.0', 'txb-01.txt')
add_asmparsertest('ARBvp1.0', 'txb-02.txt')
add_asmparsertest('ARBvp1.0', 'txb-03.txt')
add_asmparsertest('ARBvp1.0', 'txb-04.txt')
add_asmparsertest('ARBvp1.0', 'txb-05.txt')
add_asmparsertest('ARBvp1.0', 'txb-06.txt')
add_asmparsertest('ARBvp1.0', 'txb-07.txt')
add_asmparsertest('ARBvp1.0', 'txb-08.txt')
add_asmparsertest('ARBvp1.0', 'txb-09.txt')
add_asmparsertest('ARBvp1.0', 'txb-10.txt')
add_asmparsertest('ARBvp1.0', 'txb-11.txt')
add_asmparsertest('ARBvp1.0', 'txb-12.txt')
add_asmparsertest('ARBvp1.0', 'txb-13.txt')
add_asmparsertest('ARBvp1.0', 'txb-14.txt')
add_asmparsertest('ARBvp1.0', 'txb-15.txt')
add_asmparsertest('ARBvp1.0', 'txb-16.txt')
add_asmparsertest('ARBvp1.0', 'txb-17.txt')
add_asmparsertest('ARBvp1.0', 'txb-18.txt')
add_asmparsertest('ARBvp1.0', 'txb-19.txt')
add_asmparsertest('ARBvp1.0', 'txb-20.txt')
add_asmparsertest('ARBvp1.0', 'txd-01.txt')
add_asmparsertest('ARBvp1.0', 'txd-02.txt')
add_asmparsertest('ARBvp1.0', 'txd-03.txt')
add_asmparsertest('ARBvp1.0', 'txd-04.txt')
add_asmparsertest('ARBvp1.0', 'txd-05.txt')
add_asmparsertest('ARBvp1.0', 'txd-06.txt')
add_asmparsertest('ARBvp1.0', 'txd-07.txt')
add_asmparsertest('ARBvp1.0', 'txd-08.txt')
add_asmparsertest('ARBvp1.0', 'txd-09.txt')
add_asmparsertest('ARBvp1.0', 'txd-10.txt')
add_asmparsertest('ARBvp1.0', 'txd-11.txt')
add_asmparsertest('ARBvp1.0', 'txd-12.txt')
add_asmparsertest('ARBvp1.0', 'txd-13.txt')
add_asmparsertest('ARBvp1.0', 'txd-14.txt')
add_asmparsertest('ARBvp1.0', 'txd-15.txt')
add_asmparsertest('ARBvp1.0', 'txd-16.txt')
add_asmparsertest('ARBvp1.0', 'txd-17.txt')
add_asmparsertest('ARBvp1.0', 'txd-18.txt')
add_asmparsertest('ARBvp1.0', 'txd-19.txt')
add_asmparsertest('ARBvp1.0', 'txd-20.txt')
add_asmparsertest('ARBvp1.0', 'txf-01.txt')
add_asmparsertest('ARBvp1.0', 'txf-02.txt')
add_asmparsertest('ARBvp1.0', 'txf-03.txt')
add_asmparsertest('ARBvp1.0', 'txf-04.txt')
add_asmparsertest('ARBvp1.0', 'txf-05.txt')
add_asmparsertest('ARBvp1.0', 'txf-06.txt')
add_asmparsertest('ARBvp1.0', 'txf-07.txt')
add_asmparsertest('ARBvp1.0', 'txf-08.txt')
add_asmparsertest('ARBvp1.0', 'txf-09.txt')
add_asmparsertest('ARBvp1.0', 'txf-10.txt')
add_asmparsertest('ARBvp1.0', 'txf-11.txt')
add_asmparsertest('ARBvp1.0', 'txf-12.txt')
add_asmparsertest('ARBvp1.0', 'txf-13.txt')
add_asmparsertest('ARBvp1.0', 'txf-14.txt')
add_asmparsertest('ARBvp1.0', 'txf-15.txt')
add_asmparsertest('ARBvp1.0', 'txf-16.txt')
add_asmparsertest('ARBvp1.0', 'txf-17.txt')
add_asmparsertest('ARBvp1.0', 'txf-18.txt')
add_asmparsertest('ARBvp1.0', 'txf-19.txt')
add_asmparsertest('ARBvp1.0', 'txf-20.txt')
add_asmparsertest('ARBvp1.0', 'txl-01.txt')
add_asmparsertest('ARBvp1.0', 'txl-02.txt')
add_asmparsertest('ARBvp1.0', 'txl-03.txt')
add_asmparsertest('ARBvp1.0', 'txl-04.txt')
add_asmparsertest('ARBvp1.0', 'txl-05.txt')
add_asmparsertest('ARBvp1.0', 'txl-06.txt')
add_asmparsertest('ARBvp1.0', 'txl-07.txt')
add_asmparsertest('ARBvp1.0', 'txl-08.txt')
add_asmparsertest('ARBvp1.0', 'txl-09.txt')
add_asmparsertest('ARBvp1.0', 'txl-10.txt')
add_asmparsertest('ARBvp1.0', 'txl-11.txt')
add_asmparsertest('ARBvp1.0', 'txl-12.txt')
add_asmparsertest('ARBvp1.0', 'txl-13.txt')
add_asmparsertest('ARBvp1.0', 'txl-14.txt')
add_asmparsertest('ARBvp1.0', 'txl-15.txt')
add_asmparsertest('ARBvp1.0', 'txl-16.txt')
add_asmparsertest('ARBvp1.0', 'txl-17.txt')
add_asmparsertest('ARBvp1.0', 'txl-18.txt')
add_asmparsertest('ARBvp1.0', 'txl-19.txt')
add_asmparsertest('ARBvp1.0', 'txl-20.txt')
add_asmparsertest('ARBvp1.0', 'txp-01.txt')
add_asmparsertest('ARBvp1.0', 'txp-02.txt')
add_asmparsertest('ARBvp1.0', 'txp-03.txt')
add_asmparsertest('ARBvp1.0', 'txp-04.txt')
add_asmparsertest('ARBvp1.0', 'txp-05.txt')
add_asmparsertest('ARBvp1.0', 'txp-06.txt')
add_asmparsertest('ARBvp1.0', 'txp-07.txt')
add_asmparsertest('ARBvp1.0', 'txp-08.txt')
add_asmparsertest('ARBvp1.0', 'txp-09.txt')
add_asmparsertest('ARBvp1.0', 'txp-10.txt')
add_asmparsertest('ARBvp1.0', 'txp-11.txt')
add_asmparsertest('ARBvp1.0', 'txp-12.txt')
add_asmparsertest('ARBvp1.0', 'txp-13.txt')
add_asmparsertest('ARBvp1.0', 'txp-14.txt')
add_asmparsertest('ARBvp1.0', 'txp-15.txt')
add_asmparsertest('ARBvp1.0', 'txp-16.txt')
add_asmparsertest('ARBvp1.0', 'txp-17.txt')
add_asmparsertest('ARBvp1.0', 'txp-18.txt')
add_asmparsertest('ARBvp1.0', 'txp-19.txt')
add_asmparsertest('ARBvp1.0', 'txp-20.txt')
add_asmparsertest('ARBvp1.0', 'txq-01.txt')
add_asmparsertest('ARBvp1.0', 'txq-02.txt')
add_asmparsertest('ARBvp1.0', 'txq-03.txt')
add_asmparsertest('ARBvp1.0', 'txq-04.txt')
add_asmparsertest('ARBvp1.0', 'txq-05.txt')
add_asmparsertest('ARBvp1.0', 'txq-06.txt')
add_asmparsertest('ARBvp1.0', 'txq-07.txt')
add_asmparsertest('ARBvp1.0', 'txq-08.txt')
add_asmparsertest('ARBvp1.0', 'txq-09.txt')
add_asmparsertest('ARBvp1.0', 'txq-10.txt')
add_asmparsertest('ARBvp1.0', 'txq-11.txt')
add_asmparsertest('ARBvp1.0', 'txq-12.txt')
add_asmparsertest('ARBvp1.0', 'txq-13.txt')
add_asmparsertest('ARBvp1.0', 'txq-14.txt')
add_asmparsertest('ARBvp1.0', 'txq-15.txt')
add_asmparsertest('ARBvp1.0', 'txq-16.txt')
add_asmparsertest('ARBvp1.0', 'txq-17.txt')
add_asmparsertest('ARBvp1.0', 'txq-18.txt')
add_asmparsertest('ARBvp1.0', 'txq-19.txt')
add_asmparsertest('ARBvp1.0', 'txq-20.txt')
add_asmparsertest('ARBvp1.0', 'xpd.txt')

ext_unpack_subimage = {}
spec['EXT_unpack_subimage'] = ext_unpack_subimage
ext_unpack_subimage['basic'] = concurrent_test('ext_unpack_subimage')

oes_draw_texture = {}
spec['OES_draw_texture'] = oes_draw_texture
oes_draw_texture['oes_draw_texture'] = concurrent_test('oes_draw_texture')

oes_compressed_etc1_rgb8_texture = {}
spec['OES_compressed_ETC1_RGB8_texture'] = oes_compressed_etc1_rgb8_texture
oes_compressed_etc1_rgb8_texture['basic'] = concurrent_test('oes_compressed_etc1_rgb8_texture-basic')
oes_compressed_etc1_rgb8_texture['miptree'] = concurrent_test('oes_compressed_etc1_rgb8_texture-miptree')

oes_compressed_paletted_texture = {}
spec['OES_compressed_paletted_texture'] = oes_compressed_paletted_texture
oes_compressed_paletted_texture['basic API'] = concurrent_test('oes_compressed_paletted_texture-api')

egl14 = {}
spec['EGL 1.4'] = egl14
egl14['eglCreateSurface'] = plain_test('egl-create-surface')
egl14['eglQuerySurface EGL_BAD_ATTRIBUTE'] = plain_test('egl-query-surface --bad-attr')
egl14['eglQuerySurface EGL_BAD_SURFACE'] = plain_test('egl-query-surface --bad-surface')
egl14['eglQuerySurface EGL_HEIGHT'] = plain_test('egl-query-surface --attr=EGL_HEIGHT')
egl14['eglQuerySurface EGL_WIDTH'] = plain_test('egl-query-surface --attr=EGL_WIDTH')
egl14['eglTerminate then unbind context'] = plain_test('egl-terminate-then-unbind-context')

egl_nok_swap_region = {}
spec['EGL_NOK_swap_region'] = egl_nok_swap_region
egl_nok_swap_region['basic']  = plain_test('egl-nok-swap-region')

egl_nok_texture_from_pixmap = {}
spec['EGL_NOK_texture_from_pixmap'] = egl_nok_texture_from_pixmap
egl_nok_texture_from_pixmap['basic'] = plain_test('egl-nok-texture-from-pixmap')

egl_khr_create_context = {};
spec['EGL_KHR_create_context'] = egl_khr_create_context
egl_khr_create_context['default major version GLES'] = plain_test('egl-create-context-default-major-version-gles')
egl_khr_create_context['default major version GL'] = plain_test('egl-create-context-default-major-version-gl')
egl_khr_create_context['default minor version GLES'] = plain_test('egl-create-context-default-minor-version-gles')
egl_khr_create_context['default minor version GL'] = plain_test('egl-create-context-default-minor-version-gl')
egl_khr_create_context['valid attribute empty GLES'] = plain_test('egl-create-context-valid-attribute-empty-gles')
egl_khr_create_context['valid attribute empty GL'] = plain_test('egl-create-context-valid-attribute-empty-gl')
egl_khr_create_context['NULL valid attribute GLES'] = plain_test('egl-create-context-valid-attribute-null-gles')
egl_khr_create_context['NULL valid attribute GL'] = plain_test('egl-create-context-valid-attribute-null-gl')
egl_khr_create_context['invalid OpenGL version'] = plain_test('egl-create-context-invalid-gl-version')
egl_khr_create_context['invalid attribute GLES'] = plain_test('egl-create-context-invalid-attribute-gles')
egl_khr_create_context['invalid attribute GL'] = plain_test('egl-create-context-invalid-attribute-gl')
egl_khr_create_context['invalid flag GLES'] = plain_test('egl-create-context-invalid-flag-gles')
egl_khr_create_context['invalid flag GL'] = plain_test('egl-create-context-invalid-flag-gl')
egl_khr_create_context['valid forward-compatible flag GL'] = plain_test('egl-create-context-valid-flag-forward-compatible-gl')
egl_khr_create_context['invalid profile'] = plain_test('egl-create-context-invalid-profile')
egl_khr_create_context['3.2 core profile required'] = plain_test('egl-create-context-core-profile')
egl_khr_create_context['pre-GL3.2 profile'] = plain_test('egl-create-context-pre-GL32-profile')
egl_khr_create_context['verify GL flavor'] = plain_test('egl-create-context-verify-gl-flavor')
egl_khr_create_context['valid debug flag GL'] = plain_test('egl-create-context-valid-flag-debug-gl gl')
for api in ('gles1', 'gles2', 'gles3'):
    egl_khr_create_context['valid debug flag ' + api] = plain_test('egl-create-context-valid-flag-debug-gles ' + api)

egl_ext_client_extensions = {}
spec['EGL_EXT_client_extensions'] = egl_ext_client_extensions
for i in [1, 2, 3]:
    egl_ext_client_extensions['conformance test {0}'.format(i)] = concurrent_test('egl_ext_client_extensions {0}'.format(i))

egl_khr_fence_sync = {}
spec['EGL_KHR_fence_sync'] = egl_khr_fence_sync
egl_khr_fence_sync['conformance'] = concurrent_test('egl_khr_fence_sync')

egl_chromium_sync_control = {}
spec['EGL_CHROMIUM_sync_control'] = egl_chromium_sync_control
egl_chromium_sync_control['conformance'] = concurrent_test('egl_chromium_sync_control')

gles20 = {}
spec['!OpenGL ES 2.0'] = gles20
gles20['glsl-fs-pointcoord'] = concurrent_test('glsl-fs-pointcoord_gles2')
add_concurrent_test(gles20, 'invalid-es3-queries_gles2')
gles20['link-no-vsfs'] = concurrent_test('link-no-vsfs_gles2')
add_concurrent_test(gles20, 'minmax_gles2')
add_concurrent_test(gles20, 'multiple-shader-objects_gles2')
add_concurrent_test(gles20, 'fbo_discard_gles2')

gles30 = {}
spec['!OpenGL ES 3.0'] = gles30
for tex_format in ('rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11', 'rgb8-punchthrough-alpha1', 'srgb8-punchthrough-alpha1'):
    test_name = ' ' .join(['oes_compressed_etc2_texture-miptree_gles3', tex_format])
    executable = '{0} -auto'.format(test_name)
    gles30[test_name] = concurrent_test(executable)
gles30['minmax'] = concurrent_test('minmax_gles3')
for test_mode in ['teximage', 'texsubimage']:
    test_name = 'ext_texture_array-compressed_gles3 {0}'.format(test_mode)
    gles30[test_name] = PiglitTest(test_name + ' -auto -fbo')
gles30['texture-immutable-levels'] = concurrent_test('texture-immutable-levels_gles3')
gles30['gl_VertexID used with glDrawArrays'] = concurrent_test('gles-3.0-drawarrays-vertexid')

arb_es3_compatibility = {}
spec['ARB_ES3_compatibility'] = arb_es3_compatibility
for tex_format in ('rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11', 'rgb8-punchthrough-alpha1', 'srgb8-punchthrough-alpha1'):
    for context in ('core', 'compat'):
        test_name = ' ' .join(['oes_compressed_etc2_texture-miptree', tex_format, context])
        executable = '{0}'.format(test_name)
        arb_es3_compatibility[test_name] = concurrent_test(executable)

add_shader_test_dir(spec, os.path.join(generatedTestDir, 'spec'),
                    recursive=True)
import_glsl_parser_tests(profile.tests, generatedTestDir, ['spec'])

arb_shader_atomic_counters = {}
spec['ARB_shader_atomic_counters'] = arb_shader_atomic_counters
import_glsl_parser_tests(spec['ARB_shader_atomic_counters'],
                         os.path.join(testsDir, 'spec', 'arb_shader_atomic_counters'),
                         [''])
arb_shader_atomic_counters['active-counters'] = concurrent_test('arb_shader_atomic_counters-active-counters')
arb_shader_atomic_counters['array-indexing'] = concurrent_test('arb_shader_atomic_counters-array-indexing')
arb_shader_atomic_counters['buffer-binding'] = concurrent_test('arb_shader_atomic_counters-buffer-binding')
arb_shader_atomic_counters['default-partition'] = concurrent_test('arb_shader_atomic_counters-default-partition')
arb_shader_atomic_counters['fragment-discard'] = concurrent_test('arb_shader_atomic_counters-fragment-discard')
arb_shader_atomic_counters['function-argument'] = concurrent_test('arb_shader_atomic_counters-function-argument')
arb_shader_atomic_counters['max-counters'] = concurrent_test('arb_shader_atomic_counters-max-counters')
arb_shader_atomic_counters['minmax'] = concurrent_test('arb_shader_atomic_counters-minmax')
arb_shader_atomic_counters['multiple-defs'] = concurrent_test('arb_shader_atomic_counters-multiple-defs')
arb_shader_atomic_counters['semantics'] = concurrent_test('arb_shader_atomic_counters-semantics')
arb_shader_atomic_counters['unique-id'] = concurrent_test('arb_shader_atomic_counters-unique-id')
arb_shader_atomic_counters['unused-result'] = concurrent_test('arb_shader_atomic_counters-unused-result')

profile.tests['hiz'] = hiz
profile.tests['fast_color_clear'] = fast_color_clear
profile.tests['glean'] = glean
profile.tests['glslparsertest'] = glslparsertest
profile.tests['asmparsertest'] = asmparsertest
profile.tests['shaders'] = shaders
profile.tests['security'] = security
profile.tests['spec'] = spec
if platform.system() is not 'Windows':
    profile.tests['glx'] = glx
