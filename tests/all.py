# -*- coding: utf-8 -*-
# All tests that come with piglit, using default settings

__all__ = ['profile']

import itertools
import os
import platform

from framework import grouptools
from framework.profile import TestProfile
from framework.test import (PiglitGLTest, GleanTest, ShaderTest,
                            GLSLParserTest, GLSLParserNoConfigError)
from py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR


def add_single_param_test_set(group, name, *params):
    for param in params:
        group['{}-{}'.format(name, param)] = PiglitGLTest([name, param])

def add_plain_test(group, args, **kwargs):
    for a in args:
        if isinstance(a, basestring):
            assert '/' not in a, args

    gname = ' '.join(args)
    assert not gname.startswith('/')

    group[gname] = PiglitGLTest(args, **kwargs)

def add_concurrent_test(group, args):
    add_plain_test(group, args, run_concurrent=True)

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

# Find and add all shader tests.
for basedir in [TESTS_DIR, GENERATED_TESTS_DIR]:
    for dirpath, _, filenames in os.walk(basedir):
        for filename in filenames:
            testname, ext = os.path.splitext(filename)
            if ext == '.shader_test':
                test = ShaderTest(os.path.join(dirpath, filename))
            elif ext in ['.vert', '.tesc', '.tese', '.geom', '.frag', '.comp']:
                try:
                    test = GLSLParserTest(os.path.join(dirpath, filename))
                except GLSLParserNoConfigError:
                    # In the event that there is no config assume that it is a
                    # legacy test, and continue
                    continue

                # For glslparser tests you can have multiple tests with the
                # same name, but a different stage, so keep the extension.
                testname = filename
            else:
                continue

            group = grouptools.join(
                grouptools.from_path(os.path.relpath(dirpath, basedir)),
                testname)
            assert group not in profile.test_list, group

            profile.test_list[group] = test



# List of all of the MSAA sample counts we wish to test
MSAA_SAMPLE_COUNTS = (2, 4, 6, 8, 16, 32)

def add_fbo_depthstencil_tests(group, format, num_samples):
    assert format, 'add_fbo_depthstencil_tests argument "format" cannot be empty'

    if format == 'default_fb':
        prefix = ''
        create_test = PiglitGLTest
    else:
        prefix = 'fbo-'
        create_test = lambda a: PiglitGLTest(a, run_concurrent=True)

    if num_samples > 1:
        suffix = ' samples=' + str(num_samples)
        psamples = '-samples=' + str(num_samples)
    else:
        suffix = ''
        psamples = ''

    group['{}depthstencil-{}-clear{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'clear', format, psamples])
    group['{}depthstencil-{}-readpixels-FLOAT-and-USHORT{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'readpixels', format, 'FLOAT-and-USHORT', psamples])
    group['{}depthstencil-{}-readpixels-24_8{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'readpixels', format, '24_8', psamples])
    group['{}depthstencil-{}-readpixels-32F_24_8_REV{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'readpixels', format, '32F_24_8_REV', psamples])
    group['{}depthstencil-{}-drawpixels-FLOAT-and-USHORT{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'drawpixels', format, 'FLOAT-and-USHORT', psamples])
    group['{}depthstencil-{}-drawpixels-24_8{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'drawpixels', format, '24_8', psamples])
    group['{}depthstencil-{}-drawpixels-32F_24_8_REV{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'drawpixels', format, '32F_24_8_REV', psamples])
    group['{}depthstencil-{}-copypixels{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'copypixels', format, psamples])
    group['{}depthstencil-{}-blit{}'.format(prefix, format, suffix)] = \
        create_test(['fbo-depthstencil', 'blit', format, psamples])

def add_fbo_depthstencil_msaa_visual_tests(group, format):
    add_fbo_depthstencil_tests(group, format, 0)
    for num_samples in MSAA_SAMPLE_COUNTS:
        add_fbo_depthstencil_tests(group, format, num_samples)

def add_depthstencil_render_miplevels_tests(group, test_types):
    # Note: the buffer sizes below have been chosen to exercise
    # many possible combinations of buffer alignments on i965.
    for texture_size in ['146', '273', '292', '585', '1024']:
        for test_type in test_types:
            add_concurrent_test(
                group,
                ['depthstencil-render-miplevels', texture_size, test_type])

def add_msaa_visual_plain_tests(group, args, **kwargs):
    assert isinstance(args, list)

    group[' '.join(args)] = PiglitGLTest(args, **kwargs)
    for num_samples in MSAA_SAMPLE_COUNTS:
        group[' '.join(args + ['samples={}'.format(num_samples)])] = \
            PiglitGLTest(args + ['-samples={}'.format(num_samples)], **kwargs)

glean = profile.tests['glean']
glean['basic'] = GleanTest('basic')
glean['api2'] = GleanTest('api2')
glean['makeCurrent'] = GleanTest('makeCurrent')
glean['bufferObject'] = GleanTest('bufferObject')
glean['depthStencil'] = GleanTest('depthStencil')
glean['fbo'] = GleanTest('fbo')
glean['getString'] = GleanTest('getString')
glean['occluquery'] = GleanTest('occluQry')
glean['paths'] = GleanTest('paths')
glean['pixelFormats'] = GleanTest('pixelFormats')
glean['pointAtten'] = GleanTest('pointAtten')
glean['pointSprite'] = GleanTest('pointSprite')
# exactRGBA is not included intentionally, because it's too strict and
# the equivalent functionality is covered by other tests
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
        groupname = grouptools.join('glean', '{0}-{1}'.format(prefix, name))
        profile.tests[groupname] = GleanTest(prefix)
        profile.tests[groupname].env['PIGLIT_TEST'] = name

def add_fbo_formats_tests(path, extension, suffix=''):
    path = grouptools.from_path(path)
    profile.tests[grouptools.join(path, 'fbo-generatemipmap-formats' + suffix)] = \
        PiglitGLTest(['fbo-generatemipmap-formats', extension], run_concurrent=True)
    profile.tests[grouptools.join(path, 'fbo-clear-formats' + suffix)] = \
        PiglitGLTest(['fbo-clear-formats', extension], run_concurrent=True)
    profile.tests[grouptools.join(path, 'get-renderbuffer-internalformat' + suffix)] = \
        PiglitGLTest(['get-renderbuffer-internalformat', extension], run_concurrent=True)
    if 'depth' not in extension:
        profile.tests[grouptools.join(path, 'fbo-blending-formats' + suffix)] = \
            PiglitGLTest(['fbo-blending-formats', extension], run_concurrent=True)
        profile.tests[grouptools.join(path, 'fbo-alphatest-formats' + suffix)] = \
            PiglitGLTest(['fbo-alphatest-formats', extension], run_concurrent=True)
        profile.tests[grouptools.join(path, 'fbo-colormask-formats' + suffix)] = \
            PiglitGLTest(['fbo-colormask-formats', extension], run_concurrent=True)

def add_msaa_formats_tests(group, extension):
    for num_samples in MSAA_SAMPLE_COUNTS:
        args = [str(num_samples), extension]
        test_name = ' '.join(['multisample-formats'] + args)
        group[test_name] = PiglitGLTest(
                ['ext_framebuffer_multisample-formats'] + args,
                run_concurrent=True)

def add_fbo_generatemipmap_extension(group, extension, name):
    group[name] = PiglitGLTest(['fbo-generatemipmap-formats', extension],
                               run_concurrent=True)

def add_fbo_clear_extension(group, extension, name):
    group[name] = PiglitGLTest(['fbo-clear-formats', extension],
                               run_concurrent=True)

def add_fbo_blending_extension(group, extension, name):
    group[name] = PiglitGLTest(['fbo-blending-formats', extension],
                               run_concurrent=True)

def add_fbo_alphatest_extension(group, extension, name):
    group[name] = PiglitGLTest(['fbo-alphatest-formats', extension],
                               run_concurrent=True)


def add_fbo_rg(group, format):
    name = "fbo-rg-" + format
    group[name] = PiglitGLTest(['fbo-rg', format], run_concurrent=True)

security = profile.tests['security']
add_plain_test(security, ['initialized-texmemory'])
add_plain_test(security, ['initialized-fbo'])
add_plain_test(security, ['initialized-vbo'])


def add_getactiveuniform_count(group, name, expected):
    group['glsl-getactiveuniform-count: ' + name] = PiglitGLTest(
        ['glsl-getactiveuniform-count',
         os.path.join('shaders',  name + '.vert'), expected],
        run_concurrent=True)

shaders = profile.tests['shaders']
add_concurrent_test(shaders, ['activeprogram-bad-program'])
add_concurrent_test(shaders, ['activeprogram-get'])
add_concurrent_test(shaders, ['attribute0'])
add_concurrent_test(shaders, ['createshaderprogram-bad-type'])
add_concurrent_test(shaders, ['createshaderprogram-attached-shaders'])
add_concurrent_test(shaders, ['glsl-arb-fragment-coord-conventions'])
add_concurrent_test(shaders, ['glsl-arb-fragment-coord-conventions-define'])
add_concurrent_test(shaders, ['glsl-bug-22603'])
add_concurrent_test(shaders, ['glsl-bindattriblocation'])
add_concurrent_test(shaders, ['glsl-dlist-getattriblocation'])
add_concurrent_test(shaders, ['glsl-getactiveuniform-array-size'])
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-length', '1')
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-ftransform', '2')
add_getactiveuniform_count(shaders, 'glsl-getactiveuniform-mvp', '2')
add_concurrent_test(shaders, ['glsl-getactiveuniform-length'])
add_concurrent_test(shaders, ['glsl-getattriblocation'])
add_concurrent_test(shaders, ['getuniform-01'])
add_concurrent_test(shaders, ['getuniform-02'])
add_concurrent_test(shaders, ['glsl-invalid-asm-01'])
add_concurrent_test(shaders, ['glsl-invalid-asm-02'])
add_concurrent_test(shaders, ['glsl-novertexdata'])
add_concurrent_test(shaders, ['glsl-preprocessor-comments'])
add_concurrent_test(shaders, ['glsl-reload-source'])
add_concurrent_test(shaders, ['glsl-uniform-out-of-bounds'])
add_concurrent_test(shaders, ['glsl-uniform-out-of-bounds-2'])
add_concurrent_test(shaders, ['glsl-uniform-update'])
add_concurrent_test(shaders, ['glsl-unused-varying'])
add_concurrent_test(shaders, ['glsl-fs-bug25902'])
add_concurrent_test(shaders, ['glsl-fs-color-matrix'])
add_concurrent_test(shaders, ['glsl-fs-discard-02'])
add_concurrent_test(shaders, ['glsl-fs-exp2'])
add_concurrent_test(shaders, ['glsl-fs-flat-color'])
add_concurrent_test(shaders, ['glsl-fs-fogcolor-statechange'])
add_concurrent_test(shaders, ['glsl-fs-fogscale'])
add_concurrent_test(shaders, ['glsl-fs-fragcoord'])
add_concurrent_test(shaders, ['glsl-fs-fragcoord-zw-ortho'])
add_concurrent_test(shaders, ['glsl-fs-fragcoord-zw-perspective'])
add_concurrent_test(shaders, ['glsl-fs-loop'])
add_concurrent_test(shaders, ['glsl-fs-loop-nested'])
add_concurrent_test(shaders, ['glsl-fs-pointcoord'])
add_concurrent_test(shaders, ['glsl-fs-raytrace-bug27060'])
add_concurrent_test(shaders, ['glsl-fs-sampler-numbering'])
add_concurrent_test(shaders, ['glsl-fs-shader-stencil-export'])
add_concurrent_test(shaders, ['glsl-fs-sqrt-branch'])
add_concurrent_test(shaders, ['glsl-fs-texturecube'])
shaders['glsl-fs-texturecube-bias'] = PiglitGLTest(['glsl-fs-texturecube', '-bias'],
                                                   run_concurrent=True)
add_concurrent_test(shaders, ['glsl-fs-texturecube-2'])
shaders['glsl-fs-texturecube-2-bias'] = PiglitGLTest(['glsl-fs-texturecube-2', '-bias'],
                                                     run_concurrent=True)
add_concurrent_test(shaders, ['glsl-fs-textureenvcolor-statechange'])
add_concurrent_test(shaders, ['glsl-fs-texture2drect'])
shaders['glsl-fs-texture2drect-proj3'] = PiglitGLTest(['glsl-fs-texture2drect', '-proj3'], run_concurrent=True)
shaders['glsl-fs-texture2drect-proj4'] = PiglitGLTest(['glsl-fs-texture2drect', '-proj4'], run_concurrent=True)
add_concurrent_test(shaders, ['glsl-fs-user-varying-ff'])
add_concurrent_test(shaders, ['glsl-mat-attribute'])
shaders['glsl-max-varyings'] = PiglitGLTest(['glsl-max-varyings'], run_concurrent=True)
shaders['glsl-max-varyings >MAX_VARYING_COMPONENTS'] = PiglitGLTest(['glsl-max-varyings', '--exceed-limits'], run_concurrent=True)
add_concurrent_test(shaders, ['glsl-orangebook-ch06-bump'])
add_concurrent_test(shaders, ['glsl-routing'])
add_concurrent_test(shaders, ['glsl-vs-arrays'])
add_concurrent_test(shaders, ['glsl-vs-normalscale'])
add_concurrent_test(shaders, ['glsl-vs-functions'])
add_concurrent_test(shaders, ['glsl-vs-user-varying-ff'])
add_concurrent_test(shaders, ['glsl-vs-texturematrix-1'])
add_concurrent_test(shaders, ['glsl-vs-texturematrix-2'])
add_concurrent_test(shaders, ['glsl-sin'])
add_concurrent_test(shaders, ['glsl-cos'])
add_concurrent_test(shaders, ['glsl-vs-if-bool'])
add_concurrent_test(shaders, ['glsl-vs-loop'])
add_concurrent_test(shaders, ['glsl-vs-loop-nested'])
add_concurrent_test(shaders, ['glsl-vs-mov-after-deref'])
add_concurrent_test(shaders, ['glsl-vs-mvp-statechange'])
add_concurrent_test(shaders, ['glsl-vs-raytrace-bug26691'])
add_concurrent_test(shaders, ['glsl-vs-statechange-1'])
add_concurrent_test(shaders, ['vp-combined-image-units'])
add_concurrent_test(shaders, ['glsl-derivs'])
add_concurrent_test(shaders, ['glsl-deriv-varyings'])
add_concurrent_test(shaders, ['glsl-fwidth'])
add_concurrent_test(shaders, ['glsl-lod-bias'])
add_concurrent_test(shaders, ['vp-ignore-input'])
add_concurrent_test(shaders, ['glsl-empty-vs-no-fs'])
add_concurrent_test(shaders, ['glsl-useprogram-displaylist'])
add_concurrent_test(shaders, ['glsl-vs-point-size'])
add_concurrent_test(shaders, ['glsl-light-model'])
add_concurrent_test(shaders, ['glsl-link-bug30552'])
add_concurrent_test(shaders, ['glsl-link-bug38015'])
add_concurrent_test(shaders, ['glsl-link-empty-prog-01'])
add_concurrent_test(shaders, ['glsl-link-empty-prog-02'])
shaders['GLSL link single global initializer, 2 shaders'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-01a.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-01b.vert'),
                  'pass'], run_concurrent=True)
shaders['GLSL link matched global initializer, 2 shaders'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-01c.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-01d.vert'),
                  'pass'], run_concurrent=True)
shaders['GLSL link mismatched global initializer, 2 shaders'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-01b.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-01d.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link mismatched global initializer, 3 shaders'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-01a.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-01b.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-01c.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link mismatched global const initializer'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-02a.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-02b.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link two programs, global initializer'] = PiglitGLTest(['glsl-link-initializer-03'], run_concurrent=True)
shaders['GLSL link matched global initializer expression'] = \
    PiglitGLTest(['glsl-link-test',
                 os.path.join('shaders', 'glsl-link-initializer-05a.vert'),
                 os.path.join('shaders', 'glsl-link-initializer-05b.vert'),
                 'fail'], run_concurrent=True)
shaders['GLSL link mismatched global initializer expression'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-initializer-06a.vert'),
                  os.path.join('shaders', 'glsl-link-initializer-06b.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link mismatched invariant'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-invariant-01a.vert'),
                  os.path.join('shaders', 'glsl-link-invariant-01b.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link mismatched centroid'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-centroid-01a.vert'),
                  os.path.join('shaders', 'glsl-link-centroid-01b.vert'),
                  'fail'], run_concurrent=True)
shaders['GLSL link array-of-struct-of-array'] = \
    PiglitGLTest(['glsl-link-test',
                  os.path.join('shaders', 'glsl-link-struct-array.frag'),
                  'pass'], run_concurrent=True)
add_concurrent_test(shaders, ['glsl-max-vertex-attrib'])
add_concurrent_test(shaders, ['glsl-kwin-blur-1'])
add_concurrent_test(shaders, ['glsl-kwin-blur-2'])
add_concurrent_test(shaders, ['gpu_shader4_attribs'])
add_concurrent_test(shaders, ['link-unresolved-function'])
add_concurrent_test(shaders, ['sso-simple'])
add_concurrent_test(shaders, ['sso-uniforms-01'])
add_concurrent_test(shaders, ['sso-uniforms-02'])
add_concurrent_test(shaders, ['sso-user-varying-01'])
add_concurrent_test(shaders, ['sso-user-varying-02'])
add_concurrent_test(shaders, ['useprogram-flushverts-1'])
add_concurrent_test(shaders, ['useprogram-flushverts-2'])
add_concurrent_test(shaders, ['useprogram-inside-begin'])
add_concurrent_test(shaders, ['useprogram-refcount-1'])
add_concurrent_test(shaders, ['useshaderprogram-bad-type'])
add_concurrent_test(shaders, ['useshaderprogram-bad-program'])
add_concurrent_test(shaders, ['useshaderprogram-flushverts-1'])
add_concurrent_test(shaders, ['point-vertex-id'])
add_concurrent_test(shaders, ['glsl-vs-int-attrib'])
for subtest in ('interstage', 'intrastage', 'vs-gs'):
    cmdline = 'version-mixing {0}'.format(subtest)
    shaders[cmdline] = PiglitGLTest(cmdline.split(), run_concurrent=True)

def add_vpfpgeneric(group, name):
    group[name] = PiglitGLTest(['vpfp-generic',
        os.path.join(TESTS_DIR, 'shaders', 'generic', name + '.vpfp')],
        run_concurrent=True)

glx = profile.tests['glx']
add_msaa_visual_plain_tests(glx, ['glx-copy-sub-buffer'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-destroycontext-1'] = PiglitGLTest(['glx-destroycontext-1'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-destroycontext-2'] = PiglitGLTest(['glx-destroycontext-2'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-dont-care-mask'] = PiglitGLTest(['glx-dont-care-mask'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-close-display'] = PiglitGLTest(['glx-close-display'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-fbconfig-sanity'] = PiglitGLTest(['glx-fbconfig-sanity'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-fbconfig-compliance'] = PiglitGLTest(['glx-fbconfig-compliance'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-fbo-binding'] = PiglitGLTest(['glx-fbo-binding'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multi-context-ib-1'] = PiglitGLTest(['glx-multi-context-ib-1'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread'] = PiglitGLTest(['glx-multithread'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-texture'] = PiglitGLTest(['glx-multithread-texture'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-makecurrent-1'] = PiglitGLTest(['glx-multithread-makecurrent-1'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-makecurrent-2'] = PiglitGLTest(['glx-multithread-makecurrent-2'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-makecurrent-3'] = PiglitGLTest(['glx-multithread-makecurrent-3'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-makecurrent-4'] = PiglitGLTest(['glx-multithread-makecurrent-4'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-multithread-shader-compile'] = PiglitGLTest(['glx-multithread-shader-compile'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-shader-sharing'] = PiglitGLTest(['glx-shader-sharing'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-exchange'] = PiglitGLTest(['glx-swap-exchange'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-event_event'] = PiglitGLTest(['glx-swap-event', '--event'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-event_async'] = PiglitGLTest(['glx-swap-event', '--async'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-event_interval'] = PiglitGLTest(['glx-swap-event', '--interval'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-pixmap'] = PiglitGLTest(['glx-swap-pixmap'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-pixmap-bad'] = PiglitGLTest(['glx-swap-pixmap-bad'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-swap-singlebuffer'] = PiglitGLTest(['glx-swap-singlebuffer'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-make-current'] = PiglitGLTest(['glx-make-current'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-make-glxdrawable-current'] = PiglitGLTest(['glx-make-glxdrawable-current'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-context-flush-control'] = PiglitGLTest(['glx-context-flush-control'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-buffer-age'] = PiglitGLTest(['glx-buffer-age'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-buffer-age vblank_mode=0'] = PiglitGLTest(['glx-buffer-age'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-buffer-age vblank_mode=0'].env['vblank_mode'] = '0'
glx['glx-pixmap-life'] = PiglitGLTest(['glx-pixmap-life'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-pixmap13-life'] = PiglitGLTest(['glx-pixmap13-life'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-pixmap-multi'] = PiglitGLTest(['glx-pixmap-multi'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-tfp'] = PiglitGLTest(['glx-tfp'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-visuals-depth'] = PiglitGLTest(['glx-visuals-depth'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-visuals-depth -pixmap'] = PiglitGLTest(['glx-visuals-depth'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-visuals-stencil'] = PiglitGLTest(['glx-visuals-stencil'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-visuals-stencil -pixmap'] = PiglitGLTest(['glx-visuals-stencil'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-window-life'] = PiglitGLTest(['glx-window-life'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-pixmap-crosscheck'] = PiglitGLTest(['glx-pixmap-crosscheck'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXWINDOW-GLX_WIDTH'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXWINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXWINDOW-GLX_HEIGHT'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXWINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXPIXMAP-GLX_WIDTH'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXPIXMAP'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXPIXMAP-GLX_HEIGHT'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXPIXMAP'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXPBUFFER-GLX_WIDTH'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXPBUFFER'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXPBUFFER-GLX_HEIGHT'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXPBUFFER'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_WIDTH'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=WINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_HEIGHT'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=WINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_FBCONFIG_ID-WINDOW'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=WINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXWINDOW'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXWINDOW'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXPIXMAP'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPIXMAP'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_FBCONFIG_ID-GLXPBUFFER'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPBUFFER'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLX_PRESERVED_CONTENTS'] = PiglitGLTest(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPBUFFER'], require_platforms=['glx', 'mixed_glx_egl'])
glx['glx-query-drawable-GLXBadDrawable'] = PiglitGLTest(['glx-query-drawable', '--bad-drawable'], require_platforms=['glx', 'mixed_glx_egl'])
glx['extension string sanity'] = PiglitGLTest(['glx-string-sanity'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

import_context = glx['GLX_EXT_import_context']
import_context['free context'] = PiglitGLTest(['glx-free-context'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['get context ID'] = PiglitGLTest(['glx-get-context-id'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['get current display'] = PiglitGLTest(['glx-get-current-display-ext'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['imported context has same context ID'] = PiglitGLTest(['glx-import-context-has-same-context-id'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['import context, multi process'] = PiglitGLTest(['glx-import-context-multi-process'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['import context, single process'] = PiglitGLTest(['glx-import-context-single-process'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['make current, multi process'] = PiglitGLTest(['glx-make-current-multi-process'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['make current, single process'] = PiglitGLTest(['glx-make-current-single-process'], require_platforms=['glx', 'mixed_glx_egl'])
import_context['query context info'] = PiglitGLTest(['glx-query-context-info-ext'], require_platforms=['glx', 'mixed_glx_egl'])

create_context = glx['GLX_ARB_create_context']
create_context['current with no framebuffer'] = PiglitGLTest(['glx-create-context-current-no-framebuffer'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['default major version'] = PiglitGLTest(['glx-create-context-default-major-version'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['default minor version'] = PiglitGLTest(['glx-create-context-default-minor-version'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['invalid attribute'] = PiglitGLTest(['glx-create-context-invalid-attribute'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['invalid flag'] = PiglitGLTest(['glx-create-context-invalid-flag'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['forward-compatible flag with pre-3.0'] = PiglitGLTest(['glx-create-context-invalid-flag-forward-compatible'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['invalid OpenGL version'] = PiglitGLTest(['glx-create-context-invalid-gl-version'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['invalid render type'] = PiglitGLTest(['glx-create-context-invalid-render-type'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['color-index render type with 3.0'] = PiglitGLTest(['glx-create-context-invalid-render-type-color-index'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['empty attribute list'] = PiglitGLTest(['glx-create-context-valid-attribute-empty'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['NULL attribute list'] = PiglitGLTest(['glx-create-context-valid-attribute-null'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context['forward-compatible flag with 3.0'] = PiglitGLTest(['glx-create-context-valid-flag-forward-compatible'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

create_context_profile = glx['GLX_ARB_create_context_profile']
create_context_profile['3.2 core profile required'] = PiglitGLTest(['glx-create-context-core-profile'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context_profile['invalid profile'] = PiglitGLTest(['glx-create-context-invalid-profile'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context_profile['pre-GL3.2 profile'] = PiglitGLTest(['glx-create-context-pre-GL32-profile'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

create_context_robustness = glx['GLX_ARB_create_context_robustness']
create_context_robustness['invalid reset notification strategy'] = PiglitGLTest(['glx-create-context-invalid-reset-strategy'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context_robustness['require GL_ARB_robustness'] = PiglitGLTest(['glx-create-context-require-robustness'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

create_context_es2_profile = glx['GLX_EXT_create_context_es2_profile']
create_context_es2_profile['indirect rendering ES2 profile'] = PiglitGLTest(['glx-create-context-indirect-es2-profile'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
create_context_es2_profile['invalid OpenGL ES version'] = PiglitGLTest(['glx-create-context-invalid-es-version'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

oml_sync_control = glx['GLX_OML_sync_control']
oml_sync_control['glXGetMscRateOML'] = PiglitGLTest(['glx-oml-sync-control-getmscrate'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
oml_sync_control['swapbuffersmsc-divisor-zero'] = PiglitGLTest(['glx-oml-sync-control-swapbuffersmsc-divisor-zero'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
oml_sync_control['swapbuffersmsc-return'] = PiglitGLTest(['glx-oml-sync-control-swapbuffersmsc-return'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
oml_sync_control['swapbuffersmsc-return swap_interval 0'] = PiglitGLTest(['glx-oml-sync-control-swapbuffersmsc-return', '0'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
oml_sync_control['swapbuffersmsc-return swap_interval 1'] = PiglitGLTest(['glx-oml-sync-control-swapbuffersmsc-return', '1'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])
oml_sync_control['waitformsc'] = PiglitGLTest(['glx-oml-sync-control-waitformsc'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

oml_sync_control_nonzeros = [
    mode + [kind, period]
    for mode in [[], ['-fullscreen'], ['-waitformsc']]
    for kind in ['-divisor', '-msc-delta']
    for period in ['1', '2']
]
for arg in oml_sync_control_nonzeros:
    oml_sync_control[' '.join(['timing'] + arg)] = PiglitGLTest(['glx-oml-sync-control-timing'] + arg, require_platforms=['glx', 'mixed_glx_egl'])

mesa_query_renderer = glx['GLX_MESA_query_renderer']
mesa_query_renderer['coverage'] = PiglitGLTest(['glx-query-renderer-coverage'], run_concurrent=True, require_platforms=['glx', 'mixed_glx_egl'])

def texwrap_test(args):
    return PiglitGLTest(['texwrap'] + args, run_concurrent=True)

def add_texwrap_target_tests(group, target):
    group['texwrap {}'.format(target)] = texwrap_test([target, 'GL_RGBA8'])
    group['texwrap {} bordercolor'.format(target)] = texwrap_test([target, 'GL_RGBA8', 'bordercolor'])
    group['texwrap {} proj'.format(target)] = texwrap_test([target, 'GL_RGBA8', 'proj'])
    group['texwrap {} proj bordercolor'.format(target)] = texwrap_test([target, 'GL_RGBA8', 'proj', 'bordercolor'])

def add_texwrap_format_tests(group, ext = '', suffix = ''):
    args = [] if ext == '' else [ext]
    group['texwrap formats' + suffix] = texwrap_test(args)
    group['texwrap formats{} bordercolor'.format(suffix)] = texwrap_test(args + ['bordercolor'])
    group['texwrap formats{} bordercolor-swizzled'.format(suffix)] = texwrap_test(args + ['bordercolor', 'swizzled'])

def add_fbo_depth_tests(group, format):
    group['fbo-depth-{}-tex1d'.format(format)] = PiglitGLTest(['fbo-depth-tex1d', format], run_concurrent=True)
    group['fbo-depth-{}-clear'.format(format)] = PiglitGLTest(['fbo-depth', 'clear', format], run_concurrent=True)
    group['fbo-depth-{}-readpixels'.format(format)] = PiglitGLTest(['fbo-depth', 'readpixels', format], run_concurrent=True)
    group['fbo-depth-{}-drawpixels'.format(format)] = PiglitGLTest(['fbo-depth', 'drawpixels', format], run_concurrent=True)
    group['fbo-depth-{}-copypixels'.format(format)] = PiglitGLTest(['fbo-depth', 'copypixels', format], run_concurrent=True)
    group['fbo-depth-{}-blit'.format(format)] = PiglitGLTest(['fbo-depth', 'blit', format], run_concurrent=True)

def add_fbo_stencil_tests(group, format):
    group['fbo-stencil-{}-clear'.format(format)] = PiglitGLTest(['fbo-stencil', 'clear', format], run_concurrent=True)
    group['fbo-stencil-{}-readpixels'.format(format)] = PiglitGLTest(['fbo-stencil', 'readpixels', format], run_concurrent=True)
    group['fbo-stencil-{}-drawpixels'.format(format)] = PiglitGLTest(['fbo-stencil', 'drawpixels', format], run_concurrent=True)
    group['fbo-stencil-{}-copypixels'.format(format)] = PiglitGLTest(['fbo-stencil', 'copypixels', format], run_concurrent=True)
    group['fbo-stencil-{}-blit'.format(format)] = PiglitGLTest(['fbo-stencil', 'blit', format], run_concurrent=True)

spec = profile.tests['spec']

gl11 = spec['!OpenGL 1.1']
add_texwrap_target_tests(gl11, '1D')
add_texwrap_target_tests(gl11, '2D')
add_texwrap_format_tests(gl11)
gl11['copyteximage 1D'] = PiglitGLTest(['copyteximage', '1D'])
gl11['copyteximage 2D'] = PiglitGLTest(['copyteximage', '2D'])
add_plain_test(gl11, ['drawbuffer-modes'])
add_plain_test(gl11, ['fdo10370'])
add_plain_test(gl11, ['fdo23489'])
add_plain_test(gl11, ['fdo23670-depth_test'])
add_plain_test(gl11, ['fdo23670-drawpix_stencil'])
add_plain_test(gl11, ['r300-readcache'])
add_plain_test(gl11, ['tri-tex-crash'])
add_plain_test(gl11, ['vbo-buffer-unmap'])
add_plain_test(gl11, ['array-stride'])
add_plain_test(gl11, ['clear-accum'])
add_concurrent_test(gl11, ['clipflat'])
add_plain_test(gl11, ['copypixels-draw-sync'])
add_plain_test(gl11, ['copypixels-sync'])
add_plain_test(gl11, ['degenerate-prims'])
add_plain_test(gl11, ['depthfunc'])
add_plain_test(gl11, ['depthrange-clear'])
add_plain_test(gl11, ['dlist-clear'])
add_plain_test(gl11, ['dlist-color-material'])
add_plain_test(gl11, ['dlist-fdo3129-01'])
add_plain_test(gl11, ['dlist-fdo3129-02'])
add_plain_test(gl11, ['dlist-fdo31590'])
add_plain_test(gl11, ['draw-arrays-colormaterial'])
add_plain_test(gl11, ['draw-copypixels-sync'])
add_concurrent_test(gl11, ['draw-pixel-with-texture'])
add_msaa_visual_plain_tests(gl11, ['draw-pixels'])
add_concurrent_test(gl11, ['drawpix-z'])
add_plain_test(gl11, ['fog-modes'])
add_plain_test(gl11, ['fragment-center'])
add_fbo_depthstencil_msaa_visual_tests(gl11, 'default_fb')
add_plain_test(gl11, ['geterror-invalid-enum'])
add_plain_test(gl11, ['geterror-inside-begin'])
add_concurrent_test(gl11, ['glinfo'])
add_plain_test(gl11, ['hiz'])
add_plain_test(gl11, ['infinite-spot-light'])
add_plain_test(gl11, ['line-aa-width'])
add_concurrent_test(gl11, ['line-flat-clip-color'])
add_plain_test(gl11, ['lineloop'])
add_plain_test(gl11, ['linestipple'])
add_plain_test(gl11, ['longprim'])
add_concurrent_test(gl11, ['masked-clear'])
add_plain_test(gl11, ['point-line-no-cull'])
add_plain_test(gl11, ['polygon-mode'])
add_concurrent_test(gl11, ['polygon-mode-offset'])
add_plain_test(gl11, ['polygon-offset'])
add_concurrent_test(gl11, ['push-pop-texture-state'])
add_concurrent_test(gl11, ['quad-invariance'])
add_msaa_visual_plain_tests(gl11, ['read-front'])
add_msaa_visual_plain_tests(gl11, ['read-front', 'clear-front-first'])
add_concurrent_test(gl11, ['readpix-z'])
add_plain_test(gl11, ['roundmode-getintegerv'])
add_plain_test(gl11, ['roundmode-pixelstore'])
add_plain_test(gl11, ['scissor-bitmap'])
add_plain_test(gl11, ['scissor-clear'])
add_plain_test(gl11, ['scissor-copypixels'])
add_plain_test(gl11, ['scissor-depth-clear'])
add_plain_test(gl11, ['scissor-many'])
add_plain_test(gl11, ['scissor-offscreen'])
add_concurrent_test(gl11, ['scissor-polygon'])
add_plain_test(gl11, ['scissor-stencil-clear'])
gl11['GL_SELECT - no test function'] = PiglitGLTest(['select', 'gl11'])
gl11['GL_SELECT - depth-test enabled'] = PiglitGLTest(['select', 'depth'])
gl11['GL_SELECT - stencil-test enabled'] = PiglitGLTest(['select', 'stencil'])
gl11['GL_SELECT - alpha-test enabled'] = PiglitGLTest(['select', 'alpha'])
gl11['GL_SELECT - scissor-test enabled'] = PiglitGLTest(['select', 'scissor'])
add_plain_test(gl11, ['stencil-drawpixels'])
add_plain_test(gl11, ['texgen'])
add_plain_test(gl11, ['two-sided-lighting'])
add_plain_test(gl11, ['user-clip'])
add_plain_test(gl11, ['varray-disabled'])
add_plain_test(gl11, ['windowoverlap'])
add_plain_test(gl11, ['copyteximage-border'])
add_plain_test(gl11, ['copyteximage-clipping'])
add_plain_test(gl11, ['copytexsubimage'])
add_plain_test(gl11, ['getteximage-formats'])
add_plain_test(gl11, ['getteximage-luminance'])
add_plain_test(gl11, ['getteximage-simple'])
gl11['incomplete-texture-fixed'] = PiglitGLTest(['incomplete-texture', 'fixed'], run_concurrent=True)
add_plain_test(gl11, ['max-texture-size'])
add_concurrent_test(gl11, ['max-texture-size-level'])
add_concurrent_test(gl11, ['proxy-texture'])
add_concurrent_test(gl11, ['sized-texture-format-channels'])
add_plain_test(gl11, ['streaming-texture-leak'])
add_plain_test(gl11, ['texredefine'])
add_plain_test(gl11, ['texsubimage'])
add_plain_test(gl11, ['texsubimage-depth-formats'])
add_plain_test(gl11, ['texture-al'])
add_concurrent_test(gl11, ['triangle-guardband-viewport'])
add_concurrent_test(gl11, ['getteximage-targets', '1D'])
add_concurrent_test(gl11, ['getteximage-targets', '2D'])
add_concurrent_test(gl11, ['teximage-scale-bias'])

color_formats = [
    'GL_RED', 'GL_R8', 'GL_R8_SNORM', 'GL_R16', 'GL_R16_SNORM',
    'GL_R16F', 'GL_R32F',

    'GL_RG', 'GL_RG8', 'GL_RG8_SNORM', 'GL_RG16', 'GL_RG16_SNORM',
    'GL_RG16F', 'GL_RG32F',

    'GL_RGB', 'GL_R3_G3_B2', 'GL_RGB4', 'GL_RGB5', 'GL_RGB8',
    'GL_RGB8_SNORM', 'GL_SRGB8', 'GL_RGB10', 'GL_R11F_G11F_B10F',
    'GL_RGB12', 'GL_RGB9_E5', 'GL_RGB16', 'GL_RGB16F',
    'GL_RGB16_SNORM', 'GL_RGB32F',

    'GL_RGBA', 'GL_RGBA2', 'GL_RGBA4', 'GL_RGB5_A1', 'GL_RGBA8',
    'GL_RGB10_A2', 'GL_RGBA8_SNORM', 'GL_SRGB8_ALPHA8', 'GL_RGBA12',
    'GL_RGBA16', 'GL_RGBA16_SNORM', 'GL_RGBA32F',

    'GL_ALPHA', 'GL_ALPHA4', 'GL_ALPHA8', 'GL_ALPHA12', 'GL_ALPHA16',

    'GL_LUMINANCE', 'GL_LUMINANCE4', 'GL_LUMINANCE8', 'GL_SLUMINANCE8',
    'GL_LUMINANCE12', 'GL_LUMINANCE16',

    'GL_LUMINANCE_ALPHA', 'GL_LUMINANCE4_ALPHA4',
    'GL_LUMINANCE6_ALPHA2', 'GL_LUMINANCE8_ALPHA8',
    'GL_SLUMINANCE8_ALPHA8', 'GL_LUMINANCE12_ALPHA4',
    'GL_LUMINANCE12_ALPHA12', 'GL_LUMINANCE16_ALPHA16',
]
for format in color_formats:
    add_concurrent_test(gl11, ['teximage-colors', format])

gl10 = spec['!OpenGL 1.0']
add_concurrent_test(gl10, ['gl-1.0-beginend-coverage'])
add_concurrent_test(gl10, ['gl-1.0-dlist-beginend'])
add_concurrent_test(gl10, ['gl-1.0-dlist-shademodel'])
add_concurrent_test(gl10, ['gl-1.0-edgeflag'])
add_concurrent_test(gl10, ['gl-1.0-edgeflag-const'])
add_concurrent_test(gl10, ['gl-1.0-edgeflag-quads'])
add_concurrent_test(gl10, ['gl-1.0-long-dlist'])
add_concurrent_test(gl10, ['gl-1.0-rendermode-feedback'])
add_plain_test(gl10, ['gl-1.0-front-invalidate-back'])
add_plain_test(gl10, ['gl-1.0-swapbuffers-behavior'])
add_concurrent_test(gl10, ['gl-1.0-polygon-line-aa'])
add_concurrent_test(gl10, ['gl-1.0-blend-func'])
add_concurrent_test(gl10, ['gl-1.0-fpexceptions'])
add_concurrent_test(gl10, ['gl-1.0-ortho-pos'])
add_concurrent_test(gl10, ['gl-1.0-readpixsanity'])
add_concurrent_test(gl10, ['gl-1.0-logicop'])

gl12 = spec['!OpenGL 1.2']
add_texwrap_target_tests(gl12, '3D')
add_msaa_visual_plain_tests(gl12, ['copyteximage', '3D'])
add_plain_test(gl12, ['crash-texparameter-before-teximage'])
add_plain_test(gl12, ['draw-elements-vs-inputs'])
add_plain_test(gl12, ['two-sided-lighting-separate-specular'])
add_plain_test(gl12, ['levelclamp'])
add_plain_test(gl12, ['lodclamp'])
add_plain_test(gl12, ['lodclamp-between'])
add_plain_test(gl12, ['lodclamp-between-max'])
add_plain_test(gl12, ['mipmap-setup'])
add_plain_test(gl12, ['tex-skipped-unit'])
add_plain_test(gl12, ['tex3d'])
add_plain_test(gl12, ['tex3d-maxsize'])
add_plain_test(gl12, ['teximage-errors'])
add_plain_test(gl12, ['texture-packed-formats'])
add_concurrent_test(gl12, ['getteximage-targets', '3D'])

gl13 = spec['!OpenGL 1.3']
add_plain_test(gl13, ['texunits'])
add_plain_test(gl13, ['tex-border-1'])
add_concurrent_test(gl13, ['tex3d-depth1'])

gl14 = spec['!OpenGL 1.4']
add_plain_test(gl14, ['fdo25614-genmipmap'])
add_plain_test(gl14, ['tex1d-2dborder'])
add_plain_test(gl14, ['blendminmax'])
add_plain_test(gl14, ['blendsquare'])
add_concurrent_test(gl14, ['gl-1.4-dlist-multidrawarrays'])
add_concurrent_test(gl14, ['gl-1.4-polygon-offset'])
add_msaa_visual_plain_tests(gl14, ['copy-pixels'])
add_plain_test(gl14, ['draw-batch'])
add_plain_test(gl14, ['stencil-wrap'])
add_plain_test(gl14, ['triangle-rasterization'])
gl14['triangle-rasterization-fbo'] = PiglitGLTest(['triangle-rasterization', '-use_fbo'])
add_plain_test(gl14, ['triangle-rasterization-overdraw'])
gl14['tex-miplevel-selection'] = PiglitGLTest(['tex-miplevel-selection', '-nobias', '-nolod'], run_concurrent=True)
gl14['tex-miplevel-selection-lod'] = PiglitGLTest(['tex-miplevel-selection', '-nobias'], run_concurrent=True)
gl14['tex-miplevel-selection-lod-bias'] = PiglitGLTest(['tex-miplevel-selection'], run_concurrent=True)

gl15 = spec['!OpenGL 1.5']
add_plain_test(gl15, ['draw-elements'])
gl15['draw-elements-user'] = PiglitGLTest(['draw-elements', 'user'])
add_plain_test(gl15, ['draw-vertices'])
gl15['draw-vertices-user'] = PiglitGLTest(['draw-vertices', 'user'])
add_plain_test(gl15, ['isbufferobj'])
add_plain_test(gl15, ['depth-tex-compare'])
gl15['normal3b3s-invariance-byte'] = PiglitGLTest(['gl-1.5-normal3b3s-invariance', 'GL_BYTE'])
gl15['normal3b3s-invariance-short'] = PiglitGLTest(['gl-1.5-normal3b3s-invariance', 'GL_SHORT'])

gl20 = spec['!OpenGL 2.0']
add_concurrent_test(gl20, ['attribs'])
add_concurrent_test(gl20, ['gl-2.0-edgeflag'])
add_concurrent_test(gl20, ['gl-2.0-edgeflag-immediate'])
add_concurrent_test(gl20, ['gl-2.0-vertexattribpointer'])
add_plain_test(gl20, ['attrib-assignments'])
add_plain_test(gl20, ['getattriblocation-conventional'])
add_plain_test(gl20, ['clip-flag-behavior'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'back', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'back', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'back', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'back'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'back', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'back', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'back', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'back'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'enabled'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'back', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'back', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'back', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'back'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'back', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'back', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'back', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'back'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front2', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'front2'])
add_concurrent_test(gl20, ['vertex-program-two-side', 'back2'])
add_concurrent_test(gl20, ['vertex-program-two-side'])
add_plain_test(gl20, ['clear-varray-2.0'])
add_plain_test(gl20, ['early-z'])
add_plain_test(gl20, ['occlusion-query-discard'])
add_plain_test(gl20, ['stencil-twoside'])
add_plain_test(gl20, ['vs-point_size-zero'])
add_plain_test(gl20, ['depth-tex-modes-glsl'])
add_plain_test(gl20, ['fragment-and-vertex-texturing'])
gl20['incomplete-texture-glsl'] = PiglitGLTest(['incomplete-texture', 'glsl'], run_concurrent=True)
add_plain_test(gl20, ['tex3d-npot'])
add_concurrent_test(gl20, ['max-samplers'])
add_concurrent_test(gl20, ['max-samplers', 'border'])
add_concurrent_test(gl20, ['gl-2.0-active-sampler-conflict'])

gl21 = spec['!OpenGL 2.1']
gl21['minmax'] = PiglitGLTest(['gl-2.1-minmax'], run_concurrent=True)
gl21['pbo'] = PiglitGLTest(['gl-2.1-pbo'], run_concurrent=True)

gl30 = spec['!OpenGL 3.0']
gl30['attribs'] = PiglitGLTest(['attribs', 'GL3'], run_concurrent=True)
add_concurrent_test(gl30, ['bindfragdata-invalid-parameters'])
add_concurrent_test(gl30, ['bindfragdata-link-error'])
add_concurrent_test(gl30, ['bindfragdata-nonexistent-variable'])
gl30['bound-resource-limits'] = PiglitGLTest(['gl-3.0-bound-resource-limits'], run_concurrent=True)
add_concurrent_test(gl30, ['clearbuffer-depth'])
add_concurrent_test(gl30, ['clearbuffer-depth-stencil'])
add_plain_test(gl30, ['clearbuffer-display-lists'])
add_concurrent_test(gl30, ['clearbuffer-invalid-drawbuffer'])
add_concurrent_test(gl30, ['clearbuffer-invalid-buffer'])
add_concurrent_test(gl30, ['clearbuffer-mixed-format'])
add_concurrent_test(gl30, ['clearbuffer-stencil'])
add_concurrent_test(gl30, ['genmipmap-errors'])
add_concurrent_test(gl30, ['getfragdatalocation'])
add_concurrent_test(gl30, ['integer-errors'])
gl30['gl_VertexID used with glMultiDrawArrays'] = PiglitGLTest(['gl-3.0-multidrawarrays-vertexid'], run_concurrent=True)
gl30['minmax'] = PiglitGLTest(['gl-3.0-minmax'], run_concurrent=True)
gl30['render-integer'] = PiglitGLTest(['gl-3.0-render-integer'], run_concurrent=True)
gl30['required-sized-texture-formats'] = PiglitGLTest(['gl-3.0-required-sized-texture-formats', '30'], run_concurrent=True)
gl30['required-renderbuffer-attachment-formats'] = PiglitGLTest(['gl-3.0-required-renderbuffer-attachment-formats', '30'], run_concurrent=True)
gl30['required-texture-attachment-formats'] = PiglitGLTest(['gl-3.0-required-texture-attachment-formats', '30'], run_concurrent=True)
gl30['forward-compatible-bit yes'] = PiglitGLTest(['gl-3.0-forward-compatible-bit', 'yes'], run_concurrent=True)
gl30['forward-compatible-bit no'] = PiglitGLTest(['gl-3.0-forward-compatible-bit', 'no'], run_concurrent=True)
add_concurrent_test(gl30, ['gl-3.0-texture-integer'])
add_concurrent_test(gl30, ['gl-3.0-vertexattribipointer'])
add_plain_test(gl30, ['gl30basic'])
add_plain_test(gl30, ['array-depth-roundtrip'])
add_plain_test(gl30, ['depth-cube-map'])
add_plain_test(gl30, ['sampler-cube-shadow'])

gl31 = spec['!OpenGL 3.1']
gl31['draw-buffers-errors'] = PiglitGLTest(['gl-3.1-draw-buffers-errors'], run_concurrent=True)
gl31['genned-names'] = PiglitGLTest(['gl-3.1-genned-names'], run_concurrent=True)
gl31['minmax'] = PiglitGLTest(['gl-3.1-minmax'], run_concurrent=True)
gl31['vao-broken-attrib'] = PiglitGLTest(['gl-3.1-vao-broken-attrib'], run_concurrent=True)
for subtest in ['generated', 'written', 'flush']:
    cmdline = 'primitive-restart-xfb {0}'.format(subtest)
    gl31[cmdline] = PiglitGLTest(['gl-3.1-primitive-restart-xfb', subtest], run_concurrent=True)
gl31['required-renderbuffer-attachment-formats'] = PiglitGLTest(['gl-3.0-required-renderbuffer-attachment-formats', '31'], run_concurrent=True)
gl31['required-sized-texture-formats'] = PiglitGLTest(['gl-3.0-required-sized-texture-formats', '31'], run_concurrent=True)
gl31['required-texture-attachment-formats'] = PiglitGLTest(['gl-3.0-required-texture-attachment-formats', '31'], run_concurrent=True)

gl32 = spec['!OpenGL 3.2']
add_concurrent_test(gl32, ['glsl-resource-not-bound', '1D'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '2D'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '3D'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '2DRect'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '1DArray'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '2DArray'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '2DMS'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', '2DMSArray'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', 'Buffer'])
add_concurrent_test(gl32, ['glsl-resource-not-bound', 'Cube'])
spec[grouptools.join('!OpenGL 3.2', 'gl_VertexID used with glMultiDrawElementsBaseVertex')] = PiglitGLTest(['gl-3.2-basevertex-vertexid'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'minmax')] = PiglitGLTest(['gl-3.2-minmax'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'clear-no-buffers')] = PiglitGLTest(['gl-3.2-clear-no-buffers'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'depth-tex-sampling')] = PiglitGLTest(['gl-3.2-depth-tex-sampling'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'get-buffer-parameter-i64v')] = PiglitGLTest(['gl-3.2-get-buffer-parameter-i64v'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'get-integer-64iv')] = PiglitGLTest(['gl-3.2-get-integer-64iv'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'get-integer-64v')] = PiglitGLTest(['gl-3.2-get-integer-64v'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'blit')] = PiglitGLTest(['gl-3.2-layered-rendering-blit'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'clear-color')] = PiglitGLTest(['gl-3.2-layered-rendering-clear-color'], run_concurrent=True)
for texture_type in ['3d', '2d_array', '2d_multisample_array', '1d_array',
                     'cube_map', 'cube_map_array']:
    for test_type in ['single_level', 'mipmapped']:
        if texture_type == '2d_multisample_array' and test_type == 'mipmapped':
            continue
        spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'clear-color-all-types {} {}'.format(texture_type, test_type))] = \
            PiglitGLTest(['gl-3.2-layered-rendering-clear-color-all-types', texture_type, test_type], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'clear-color-mismatched-layer-count')] = PiglitGLTest(['gl-3.2-layered-rendering-clear-color-mismatched-layer-count'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'clear-depth')] = PiglitGLTest(['gl-3.2-layered-rendering-clear-depth'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffertexture')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffertexture'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffertexture-buffer-textures')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffertexture-buffer-textures'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffertexture-defaults')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffertexture-defaults'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'readpixels')] = PiglitGLTest(['gl-3.2-layered-rendering-readpixels'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffer-layer-attachment-mismatch')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffer-layer-attachment-mismatch'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffer-layer-complete')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffer-layer-complete'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffer-layer-count-mismatch')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffer-layer-count-mismatch'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'framebuffer-layered-attachments')] = PiglitGLTest(['gl-3.2-layered-rendering-framebuffer-layered-attachments'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'gl-layer')] = PiglitGLTest(['gl-3.2-layered-rendering-gl-layer'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'gl-layer-cube-map')] = PiglitGLTest(['gl-3.2-layered-rendering-gl-layer-cube-map'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'gl-layer-not-layered')] = PiglitGLTest(['gl-3.2-layered-rendering-gl-layer-not-layered'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'layered-rendering', 'gl-layer-render')] = PiglitGLTest(['gl-3.2-layered-rendering-gl-layer-render'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'pointsprite-coord')] = PiglitGLTest(['gl-3.2-pointsprite-coord'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'pointsprite-origin')] = PiglitGLTest(['gl-3.2-pointsprite-origin'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'coord-replace-doesnt-eliminate-frag-tex-coords')] = PiglitGLTest(['gl-coord-replace-doesnt-eliminate-frag-tex-coords'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'get-active-attrib-returns-all-inputs')] = PiglitGLTest(['gl-get-active-attrib-returns-all-inputs'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.2', 'texture-border-deprecated')] = PiglitGLTest(['gl-3.2-texture-border-deprecated'], run_concurrent=True)

spec[grouptools.join('!OpenGL 3.3', 'minmax')] = PiglitGLTest(['gl-3.3-minmax'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.3', 'required-renderbuffer-attachment-formats')] = PiglitGLTest(['gl-3.0-required-renderbuffer-attachment-formats', '33'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.3', 'required-sized-texture-formats')] = PiglitGLTest(['gl-3.0-required-sized-texture-formats', '33'], run_concurrent=True)
spec[grouptools.join('!OpenGL 3.3', 'required-texture-attachment-formats')] = PiglitGLTest(['gl-3.0-required-texture-attachment-formats', '33'], run_concurrent=True)

spec[grouptools.join('!OpenGL 4.2', 'required-renderbuffer-attachment-formats')] = PiglitGLTest(['gl-3.0-required-renderbuffer-attachment-formats', '42'], run_concurrent=True)
spec[grouptools.join('!OpenGL 4.2', 'required-sized-texture-formats')] = PiglitGLTest(['gl-3.0-required-sized-texture-formats', '42'], run_concurrent=True)
spec[grouptools.join('!OpenGL 4.2', 'required-texture-attachment-formats')] = PiglitGLTest(['gl-3.0-required-texture-attachment-formats', '42'], run_concurrent=True)
spec[grouptools.join('!OpenGL 4.4', 'gl-max-vertex-attrib-stride')] = PiglitGLTest(['gl-4.4-max_vertex_attrib_stride'], run_concurrent=True)
spec[grouptools.join('!OpenGL 4.4', 'tex-errors')] = PiglitGLTest(['tex-errors'], run_concurrent=True)

# Group spec/glsl-es-1.00
spec['glsl-es-1.00']['built-in constants'] = PiglitGLTest(
    ['built-in-constants_gles2',
     os.path.join(TESTS_DIR, 'spec', 'glsl-es-1.00', 'minimum-maximums.txt')],
    run_concurrent=True)

# Group spec/glsl-1.10
add_concurrent_test(spec['glsl-1.10']['execution'], ['glsl-render-after-bad-attach'])
add_concurrent_test(spec['glsl-1.10']['execution'], ['glsl-1.10-fragdepth'])
for mode in ['fixed', 'pos_clipvert', 'clipvert_pos']:
    cmdline = 'clip-plane-transformation ' + mode
    spec['glsl-1.10']['execution']['clipping'][cmdline] = PiglitGLTest(cmdline.split(), run_concurrent=True)
for type in ['int', 'uint', 'float', 'vec2', 'vec3', 'vec4', 'ivec2', 'ivec3',
             'ivec4', 'uvec2', 'uvec3', 'uvec4', 'mat2', 'mat3', 'mat4',
             'mat2x3', 'mat2x4', 'mat3x2', 'mat3x4', 'mat4x2', 'mat4x3']:
    for arrayspec in ['array', 'separate']:
        cmdline = 'simple {0} {1}'.format(type, arrayspec)
        spec['glsl-1.10']['execution']['varying-packing'][cmdline] = \
            PiglitGLTest(['varying-packing-simple', type, arrayspec], run_concurrent=True)
spec['glsl-1.10']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-1.10', 'minimum-maximums.txt')],
    run_concurrent=True)

add_concurrent_test(spec['glsl-1.10']['api'], ['getactiveattrib', '110'])

# Group spec/glsl-1.20
add_concurrent_test(spec['glsl-1.20'], ['glsl-1.20-getactiveuniform-constant'])

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
    group[name] = PiglitGLTest(['recursion', '-rlimit', '268435456', name])

rec = spec['glsl-1.20']['recursion']
add_recursion_test(rec, 'simple')
add_recursion_test(rec, 'unreachable')
add_recursion_test(rec, 'unreachable-constant-folding')
add_recursion_test(rec, 'indirect')
add_recursion_test(rec, 'indirect-separate')
add_recursion_test(rec, 'indirect-complex')
add_recursion_test(rec, 'indirect-complex-separate')

spec['glsl-1.20']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-1.20', 'minimum-maximums.txt')], run_concurrent=True)
add_concurrent_test(spec['glsl-1.20']['api'], ['getactiveattrib', '120'])

add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', '1D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', '2D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', '3D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', 'Cube'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', '1DShadow'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture()', '2DShadow'])

add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', '1D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', '2D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', '3D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', 'Cube'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:texture(bias)', '2DShadow'])

add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '1D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '2D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '3D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '1DShadow'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj', '2DShadow'])

add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '1D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '2D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '3D'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.20']['execution'], ['tex-miplevel-selection', 'GL2:textureProj(bias)', '2DShadow'])


# Group spec/glsl-1.30
textureSize_samplers_130 = ['sampler1D', 'sampler2D', 'sampler3D', 'samplerCube', 'sampler1DShadow', 'sampler2DShadow', 'samplerCubeShadow', 'sampler1DArray', 'sampler2DArray', 'sampler1DArrayShadow', 'sampler2DArrayShadow', 'isampler1D', 'isampler2D', 'isampler3D', 'isamplerCube', 'isampler1DArray', 'isampler2DArray', 'usampler1D', 'usampler2D', 'usampler3D', 'usamplerCube', 'usampler1DArray', 'usampler2DArray']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.30'
    # textureSize():
    for sampler in textureSize_samplers_130:
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'textureSize',
            '{}-textureSize-{}'.format(stage, sampler))] = PiglitGLTest(
                ['textureSize', stage, sampler],
                run_concurrent=True)
    # texelFetch():
    for sampler in ['sampler1D', 'sampler2D', 'sampler3D', 'sampler1DArray', 'sampler2DArray', 'isampler1D', 'isampler2D', 'isampler3D', 'isampler1DArray', 'isampler2DArray', 'usampler1D', 'usampler2D', 'usampler3D', 'usampler1DArray', 'usampler2DArray']:
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', stage, sampler],
                run_concurrent=True)
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'texelFetchOffset',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', 'offset', stage, sampler],
                run_concurrent=True)
    # texelFetch() with EXT_texture_swizzle mode "b0r1":
    for type in ['i', 'u', '']:
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}sampler2Darray-swizzle'.format(stage, type))] = PiglitGLTest(
                ['texelFetch', stage, '{}sampler2DArray'.format(type), 'b0r1'],
                run_concurrent=True)

add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler1D', '1-513'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler1DArray', '1x71-501x71'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler1DArray', '1x281-501x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler1DArray', '71x1-71x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler1DArray', '281x1-281x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2D', '1x71-501x71'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2D', '1x281-501x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2D', '71x1-71x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2D', '281x1-281x281'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler3D', '1x129x9-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler3D', '98x1x9-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler3D', '98x129x1-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2DArray', '1x129x9-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2DArray', '98x1x9-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['texelFetch', 'fs', 'sampler2DArray', '98x129x1-98x129x9'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['fs-texelFetch-2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['fs-texelFetchOffset-2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['fs-textureOffset-2D'])
add_plain_test(spec['glsl-1.30']['linker']['clipping'], ['mixing-clip-distance-and-clip-vertex-disallowed'])
add_plain_test(spec['glsl-1.30']['execution']['clipping'], ['max-clip-distances'])
for arg in ['vs_basic', 'vs_xfb', 'vs_fbo', 'fs_basic', 'fs_fbo']:
    test_name = ['isinf-and-isnan', arg]
    spec['glsl-1.30']['execution'][' '.join(test_name)] = PiglitGLTest(test_name)
spec['glsl-1.30']['execution']['clipping']['clip-plane-transformation pos'] = \
    PiglitGLTest(['clip-plane-transformation', 'pos'], run_concurrent=True)
spec['glsl-1.30']['texel-offset-limits'] = PiglitGLTest(['glsl-1.30-texel-offset-limits'], run_concurrent=True)
add_concurrent_test(spec['glsl-1.30']['execution'], ['fs-discard-exit-2'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['vertexid-beginend'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['vertexid-drawarrays'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['vertexid-drawelements'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['fs-execution-ordering'])

spec['glsl-1.30']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-1.30', 'minimum-maximums.txt')],
    run_concurrent=True)
add_concurrent_test(spec['glsl-1.30']['api'], ['getactiveattrib', '130'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', 'Cube'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', '1DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLod', 'CubeArray'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', 'Cube'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', 'CubeShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', 'CubeArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture(bias)', '1DArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', 'Cube'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', 'CubeShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', 'CubeArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '1DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'texture()', 'CubeArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '1DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset', '2DArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureOffset(bias)', '1DArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2DRect_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj', '2DRectShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProj(bias)', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2DRect_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjOffset(bias)', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureLodOffset', '1DArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLod', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjLodOffset', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', 'Cube'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', 'CubeShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '1DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', '2DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGrad', 'CubeArray'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '1DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2DArray'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '1DArrayShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureGradOffset', '2DArrayShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2DRect_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGrad', '2DShadow'])

add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '1D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '1D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2D_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2DRect'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2DRect_ProjVec4'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2DRectShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '3D'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '1DShadow'])
add_concurrent_test(spec['glsl-1.30']['execution'], ['tex-miplevel-selection', 'textureProjGradOffset', '2DShadow'])

# Group spec/glsl-1.40
spec['glsl-1.40']['execution']['tf-no-position'] = PiglitGLTest(['glsl-1.40-tf-no-position'], run_concurrent=True)
spec['glsl-1.40']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-1.40', 'minimum-maximums.txt')],
    run_concurrent=True)

textureSize_samplers_140 = textureSize_samplers_130 + ['sampler2DRect', 'isampler2DRect', 'sampler2DRectShadow', 'samplerBuffer', 'isamplerBuffer', 'usamplerBuffer']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.40'
    # textureSize():
    for sampler in textureSize_samplers_140:
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'textureSize',
            '{}-textureSize-{}'.format(stage, sampler))] = PiglitGLTest(
                ['textureSize', '140', stage, sampler],
                run_concurrent=True)
    # texelFetch():
    for sampler in ['sampler2DRect', 'usampler2DRect', 'isampler2DRect']:
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', '140', stage, sampler],
                run_concurrent=True)
        spec[grouptools.join(
            'glsl-{}'.format(version), 'execution', 'texelFetchOffset',
            '{}-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', 'offset', '140', stage, sampler],
                run_concurrent=True)

spec['glsl-1.50']['execution']['interface-blocks-api-access-members'] = PiglitGLTest(['glsl-1.50-interface-blocks-api-access-members'], run_concurrent=True)
spec['glsl-1.50']['execution']['get-active-attrib-array'] = PiglitGLTest(['glsl-1.50-get-active-attrib-array'], run_concurrent=True)
spec['glsl-1.50']['execution']['vs-input-arrays'] = PiglitGLTest(['glsl-1.50-vs-input-arrays'], run_concurrent=True)
spec['glsl-1.50']['execution']['vs-named-block-no-modify'] = PiglitGLTest(['glsl-1.50-vs-named-block-no-modify'], run_concurrent=True)
for draw in ['', 'indexed']:
    for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                 'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        add_concurrent_test(spec['glsl-1.50'],
                            ['arb_geometry_shader4-ignore-adjacent-vertices',
                             'core', draw, prim])
spec['glsl-1.50']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-1.50', 'minimum-maximums.txt')],
    run_concurrent=True)
spec['glsl-1.50']['gs-emits-too-few-verts'] = PiglitGLTest(['glsl-1.50-gs-emits-too-few-verts'], run_concurrent=True)
spec['glsl-1.50']['gs-end-primitive-optional-with-points-out'] = PiglitGLTest(['glsl-1.50-geometry-end-primitive-optional-with-points-out'], run_concurrent=True)
spec['glsl-1.50']['getshaderiv-may-return-GS'] = PiglitGLTest(['glsl-1.50-getshaderiv-may-return-GS'], run_concurrent=True)
spec['glsl-1.50']['query-gs-prim-types'] = PiglitGLTest(['glsl-1.50-query-gs-prim-types'], run_concurrent=True)
spec['glsl-1.50']['transform-feedback-type-and-size'] = PiglitGLTest(['glsl-1.50-transform-feedback-type-and-size'], run_concurrent=True)
spec['glsl-1.50']['transform-feedback-vertex-id'] = PiglitGLTest(['glsl-1.50-transform-feedback-vertex-id'], run_concurrent=True)
spec['glsl-1.50']['transform-feedback-builtins'] = PiglitGLTest(['glsl-1.50-transform-feedback-builtins'], run_concurrent=True)
for subtest in ['unnamed', 'named', 'array']:
    add_concurrent_test(
        spec['glsl-1.50'],
        ['glsl-1.50-interface-block-centroid', subtest])

# max_vertices of 32 and 128 are important transition points for
# mesa/i965 (they are the number of bits in a float and a vec4,
# respectively), so test around there.  Also test 0, which means the
# maximum number of geometry shader output vertices supported by the
# hardware.
for i in [31, 32, 33, 34, 127, 128, 129, 130, 0]:
    spec['glsl-1.50']['execution']['geometry']['end-primitive {0}'.format(i)] = \
        PiglitGLTest(['glsl-1.50-geometry-end-primitive', str(i)], run_concurrent=True)

for prim_type in ['GL_POINTS', 'GL_LINE_LOOP', 'GL_LINE_STRIP', 'GL_LINES',
                  'GL_TRIANGLES', 'GL_TRIANGLE_STRIP', 'GL_TRIANGLE_FAN',
                  'GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                  'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
    spec['glsl-1.50']['execution']['geometry']['primitive-types {0}'.format(prim_type)] = \
        PiglitGLTest(['glsl-1.50-geometry-primitive-types', prim_type], run_concurrent=True)
    for restart_index in ['ffs', 'other']:
        cmdline = 'primitive-id-restart {0} {1}'.format(prim_type, restart_index)
        spec['glsl-1.50']['execution']['geometry'][cmdline] = \
            PiglitGLTest(['glsl-1.50-geometry-primitive-id-restart', prim_type, restart_index], run_concurrent=True)

for layout_type in ['points', 'lines', 'lines_adjacency', 'triangles',
                    'triangles_adjacency']:
    add_concurrent_test(spec['glsl-1.50'],
                        ['glsl-1.50-gs-mismatch-prim-type', layout_type])

for prim_type in ['GL_TRIANGLE_STRIP', 'GL_TRIANGLE_STRIP_ADJACENCY']:
    for restart_index in ['ffs', 'other']:
        cmdline = 'tri-strip-ordering-with-prim-restart {0} {1}'.format(
            prim_type, restart_index)
        spec['glsl-1.50']['execution']['geometry'][cmdline] = \
            PiglitGLTest(['glsl-1.50-geometry-tri-strip-ordering-with-prim-restart', prim_type, restart_index], run_concurrent=True)

for input_layout in ['points', 'lines', 'lines_adjacency', 'triangles',
                     'triangles_adjacency', 'line_strip', 'triangle_strip']:
    add_concurrent_test(spec['glsl-1.50'],
                        ['glsl-1.50-gs-input-layout-qualifiers', input_layout])

for output_layout in ['points', 'lines', 'lines_adjacency', 'triangles',
                      'triangles_adjacency', 'line_strip', 'triangle_strip']:
    add_concurrent_test(spec['glsl-1.50'],
                        ['glsl-1.50-gs-output-layout-qualifiers', output_layout])

spec['glsl-3.30']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'glsl-3.30', 'minimum-maximums.txt')], run_concurrent=True)

# Group spec/glsl-es-3.00
add_concurrent_test(spec['glsl-es-3.00']['execution'], ['varying-struct-centroid_gles3'])
spec['glsl-es-3.00']['built-in constants'] = PiglitGLTest(
    ['built-in-constants_gles3',
     os.path.join(TESTS_DIR, 'spec', 'glsl-es-3.00', 'minimum-maximums.txt')], run_concurrent=True)

# AMD_performance_monitor
profile.test_list[grouptools.join('spec', 'AMD_performance_monitor', 'api')] = PiglitGLTest(['amd_performance_monitor_api'])
profile.test_list[grouptools.join('spec', 'AMD_performance_monitor', 'measure')] = PiglitGLTest(['amd_performance_monitor_measure'])

# Group ARB_point_sprite
arb_point_sprite = spec['ARB_point_sprite']
add_plain_test(arb_point_sprite, ['point-sprite'])

# Group ARB_tessellation_shader
arb_tessellation_shader = spec['ARB_tessellation_shader']
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-get-tcs-params'])
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-get-tes-params'])
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-minmax'])
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-invalid-get-program-params'])
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-invalid-patch-vertices-range'])
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-invalid-primitive'])
arb_tessellation_shader['built-in-constants'] = PiglitGLTest(
    ['built-in-constants', os.path.join(TESTS_DIR, 'spec', 'arb_tessellation_shader', 'minimum-maximums.txt')],
    run_concurrent=True)
add_concurrent_test(arb_tessellation_shader, ['arb_tessellation_shader-minmax'])

# Group ARB_texture_multisample
samplers_atm = ['sampler2DMS', 'isampler2DMS', 'usampler2DMS',
                'sampler2DMSArray', 'isampler2DMSArray', 'usampler2DMSArray']
arb_texture_multisample = spec['ARB_texture_multisample']
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-minmax'])
for sample_count in MSAA_SAMPLE_COUNTS:
    sample_count = str(sample_count)
    # fb-completeness
    spec[grouptools.join('ARB_texture_multisample', 'fb-completeness', sample_count)] = \
        PiglitGLTest(['arb_texture_multisample-fb-completeness', sample_count], run_concurrent=True)
    # texel-fetch execution
    for stage in ['vs', 'gs', 'fs']:
        for sampler in samplers_atm:
            spec[grouptools.join(
                'ARB_texture_multisample', 'texelFetch',
                '{}-{}-{}'.format(sample_count, stage, sampler))] = \
                    PiglitGLTest(['texelFetch', stage, sampler, sample_count], run_concurrent=True)
    # sample positions
    spec[grouptools.join('ARB_texture_multisample', 'sample-position', sample_count)] = \
        PiglitGLTest(['arb_texture_multisample-sample-position', sample_count], run_concurrent=True)
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMS', '4', '1x71-501x71'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMS', '4', '1x130-501x130'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMS', '4', '71x1-71x130'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMS', '4', '281x1-281x130'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMSArray', '4', '1x129x9-98x129x9'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMSArray', '4', '98x1x9-98x129x9'])
add_concurrent_test(arb_texture_multisample, ['texelFetch', 'fs', 'sampler2DMSArray', '4', '98x129x1-98x129x9'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-texstate'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-errors'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-texelfetch', '2'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-texelfetch', '4'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-texelfetch', '8'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-sample-mask'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-sample-mask-value'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-sample-mask-execution'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-sample-mask-execution', '-tex'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-negative-max-samples'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-teximage-3d-multisample'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-teximage-2d-multisample'])
add_concurrent_test(arb_texture_multisample, ['arb_texture_multisample-sample-depth'])

for stage in ['vs', 'gs', 'fs']:
    # textureSize():
    for sampler in samplers_atm:
        spec[grouptools.join('ARB_texture_multisample', 'textureSize', '{}-textureSize-{}'.format(stage, sampler))] = \
            PiglitGLTest(['textureSize', stage, sampler], run_concurrent=True)

# Group ARB_texture_gather
arb_texture_gather = spec['ARB_texture_gather']
for stage in ['vs', 'fs']:
    for comps in ['r', 'rg', 'rgb', 'rgba']:
        for swiz in ['red', 'green', 'blue', 'alpha'][:len(comps)] + ['', 'zero', 'one']:
            for type in ['unorm', 'float', 'int', 'uint']:
                for sampler in ['2D', '2DArray', 'Cube', 'CubeArray']:
                    for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset']:
                        testname = grouptools.join(
                            func,
                            '{}-{}-{}-{}-{}'.format(
                                stage, comps,
                                swiz if len(swiz) else 'none',
                                type, sampler))
                        cmd = ['textureGather', stage,
                            'offset' if func == 'textureGatherOffset' else '',
                            comps, swiz, type, sampler]
                        arb_texture_gather[testname] = PiglitGLTest(cmd, run_concurrent=True)

profile.test_list[grouptools.join('spec', 'ARB_stencil_texturing', 'draw')] = PiglitGLTest(['arb_stencil_texturing-draw'], run_concurrent=True)

# Group ARB_sync
arb_sync = spec['ARB_sync']
arb_sync['ClientWaitSync-errors'] = PiglitGLTest(['arb_sync-client-wait-errors'], run_concurrent=True)
arb_sync['DeleteSync'] = PiglitGLTest(['arb_sync-delete'], run_concurrent=True)
arb_sync['FenceSync-errors'] = PiglitGLTest(['arb_sync-fence-sync-errors'], run_concurrent=True)
arb_sync['GetSynciv-errors'] = PiglitGLTest(['arb_sync-get-sync-errors'], run_concurrent=True)
arb_sync['IsSync'] = PiglitGLTest(['arb_sync-is-sync'], run_concurrent=True)
arb_sync['repeat-wait'] = PiglitGLTest(['arb_sync-repeat-wait'], run_concurrent=True)
arb_sync['sync-initialize'] = PiglitGLTest(['arb_sync-sync-initialize'], run_concurrent=True)
arb_sync['timeout-zero'] = PiglitGLTest(['arb_sync-timeout-zero'], run_concurrent=True)
arb_sync['WaitSync-errors'] = PiglitGLTest(['arb_sync-WaitSync-errors'], run_concurrent=True)
add_plain_test(arb_sync, ['sync_api'])

# Group ARB_ES2_compatibility
arb_es2_compatibility = spec['ARB_ES2_compatibility']
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-depthrangef'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-drawbuffers'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-getshaderprecisionformat'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-maxvectors'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-shadercompiler'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-releaseshadercompiler'])
add_plain_test(arb_es2_compatibility, ['arb_es2_compatibility-fixed-type'])
add_plain_test(arb_es2_compatibility, ['fbo-missing-attachment-clear'])
arb_es2_compatibility['FBO blit to missing attachment (ES2 completeness rules)'] = PiglitGLTest(['fbo-missing-attachment-blit', 'es2', 'to'], run_concurrent=True)
arb_es2_compatibility['FBO blit from missing attachment (ES2 completeness rules)'] = PiglitGLTest(['fbo-missing-attachment-blit', 'es2', 'from'], run_concurrent=True)
add_fbo_formats_tests(os.path.join('spec', 'ARB_ES2_compatibility'), 'GL_ARB_ES2_compatibility')
add_texwrap_format_tests(arb_es2_compatibility, 'GL_ARB_ES2_compatibility')
arb_es2_compatibility['NUM_SHADER_BINARY_FORMATS over-run check'] = PiglitGLTest(['arb_get_program_binary-overrun', 'shader'], run_concurrent=True)


# Group ARB_get_program_binary
arb_get_program_binary = spec['ARB_get_program_binary']
arb_get_program_binary['misc. API error checks'] = PiglitGLTest(['arb_get_program_binary-api-errors'], run_concurrent=True)
arb_get_program_binary['NUM_PROGRAM_BINARY_FORMATS over-run check'] = PiglitGLTest(['arb_get_program_binary-overrun', 'program'], run_concurrent=True)
arb_get_program_binary['PROGRAM_BINARY_RETRIEVABLE_HINT'] = PiglitGLTest(['arb_get_program_binary-retrievable_hint'], run_concurrent=True)

arb_depth_clamp = spec['ARB_depth_clamp']
add_plain_test(arb_depth_clamp, ['depth_clamp'])
add_plain_test(arb_depth_clamp, ['depth-clamp-range'])
add_plain_test(arb_depth_clamp, ['depth-clamp-status'])

# Group ARB_draw_elements_base_vertex
arb_draw_elements_base_vertex = spec['ARB_draw_elements_base_vertex']
arb_draw_elements_base_vertex['dlist'] = PiglitGLTest(['arb_draw_elements_base_vertex-dlist'], run_concurrent=True)
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-drawelements'])
arb_draw_elements_base_vertex['arb_draw_elements_base_vertex-drawelements-user_varrays'] = PiglitGLTest(['arb_draw_elements_base_vertex-drawelements', 'user_varrays'])
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-negative-index'])
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-bounds'])
arb_draw_elements_base_vertex['arb_draw_elements_base_vertex-negative-index-user_varrays'] = PiglitGLTest(['arb_draw_elements_base_vertex-negative-index', 'user_varrays'])
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-drawelements-instanced'])
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-drawrangeelements'])
add_plain_test(arb_draw_elements_base_vertex, ['arb_draw_elements_base_vertex-multidrawelements'])

# Group ARB_draw_instanced
arb_draw_instanced = spec['ARB_draw_instanced']
arb_draw_instanced['dlist'] = PiglitGLTest(['arb_draw_instanced-dlist'], run_concurrent=True)
arb_draw_instanced['elements'] = PiglitGLTest(['arb_draw_instanced-elements'], run_concurrent=True)
arb_draw_instanced['negative-arrays-first-negative'] = PiglitGLTest(['arb_draw_instanced-negative-arrays-first-negative'], run_concurrent=True)
arb_draw_instanced['negative-elements-type'] = PiglitGLTest(['arb_draw_instanced-negative-elements-type'], run_concurrent=True)
add_plain_test(arb_draw_instanced, ['arb_draw_instanced-drawarrays'])

# Group ARB_draw_indirect
arb_draw_indirect = spec['ARB_draw_indirect']
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-api-errors'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-arrays'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-arrays-prim-restart'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-elements'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-arrays-base-instance'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-elements-base-instance'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-elements-prim-restart'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-elements-prim-restart-ugly'])
add_concurrent_test(arb_draw_indirect, ['arb_draw_indirect-draw-arrays-instances'])
arb_draw_indirect['gl_VertexID used with glDrawArraysIndirect'] = PiglitGLTest(['arb_draw_indirect-vertexid'], run_concurrent=True)
arb_draw_indirect['gl_VertexID used with glDrawElementsIndirect'] = PiglitGLTest(['arb_draw_indirect-vertexid', 'elements'], run_concurrent=True)

# Group ARB_fragment_program
arb_fragment_program = spec['ARB_fragment_program']
arb_fragment_program['minmax'] = PiglitGLTest(['arb_fragment_program-minmax'], run_concurrent=True)
add_vpfpgeneric(arb_fragment_program, 'fdo30337a')
add_vpfpgeneric(arb_fragment_program, 'fdo30337b')
add_vpfpgeneric(arb_fragment_program, 'fdo38145')
add_vpfpgeneric(arb_fragment_program, 'fp-cmp')
add_vpfpgeneric(arb_fragment_program, 'fp-dst-aliasing-1')
add_vpfpgeneric(arb_fragment_program, 'fp-dst-aliasing-2')
add_vpfpgeneric(arb_fragment_program, 'fp-ex2-sat')
add_vpfpgeneric(arb_fragment_program, 'fp-two-constants')
add_plain_test(arb_fragment_program, ['fp-abs-01'])
add_plain_test(arb_fragment_program, ['fp-fog'])
add_plain_test(arb_fragment_program, ['fp-formats'])
add_plain_test(arb_fragment_program, ['fp-fragment-position'])
add_plain_test(arb_fragment_program, ['fp-incomplete-tex'])
add_plain_test(arb_fragment_program, ['fp-indirections'])
add_plain_test(arb_fragment_program, ['fp-indirections2'])
add_plain_test(arb_fragment_program, ['fp-kil'])
add_plain_test(arb_fragment_program, ['fp-lit-mask'])
add_plain_test(arb_fragment_program, ['fp-lit-src-equals-dst'])
add_plain_test(arb_fragment_program, ['fp-long-alu'])
add_plain_test(arb_fragment_program, ['fp-set-01'])
arb_fragment_program['sparse-samplers'] = PiglitGLTest(['arb_fragment_program-sparse-samplers'], run_concurrent=True)
add_plain_test(arb_fragment_program, ['trinity-fp1'])
arb_fragment_program['incomplete-texture-arb_fp'] = PiglitGLTest(['incomplete-texture', 'arb_fp'], run_concurrent=True)

# Group ARB_fragment_program_shadow

nv_fragment_program_option = spec['NV_fragment_program_option']
add_plain_test(nv_fragment_program_option, ['fp-abs-02'])
add_plain_test(nv_fragment_program_option, ['fp-condition_codes-01'])
add_plain_test(nv_fragment_program_option, ['fp-rfl'])
add_plain_test(nv_fragment_program_option, ['fp-set-02'])
add_plain_test(nv_fragment_program_option, ['fp-unpack-01'])

arb_fragment_coord_conventions = spec['ARB_fragment_coord_conventions']
add_vpfpgeneric(arb_fragment_coord_conventions, 'fp-arb-fragment-coord-conventions-none')
add_vpfpgeneric(arb_fragment_coord_conventions, 'fp-arb-fragment-coord-conventions-integer')

ati_fragment_shader = spec['ATI_fragment_shader']
add_plain_test(ati_fragment_shader, ['ati-fs-bad-delete'])

# Group ARB_framebuffer_object
arb_framebuffer_object = spec['ARB_framebuffer_object']
add_concurrent_test(arb_framebuffer_object, ['same-attachment-glFramebufferTexture2D-GL_DEPTH_STENCIL_ATTACHMENT'])
add_concurrent_test(arb_framebuffer_object, ['same-attachment-glFramebufferRenderbuffer-GL_DEPTH_STENCIL_ATTACHMENT'])
add_plain_test(arb_framebuffer_object, ['fdo28551']) # must not be concurrent
for format in ('rgba', 'depth', 'stencil'):
    for test_mode in ('draw', 'read'):
        test_name = ['framebuffer-blit-levels', test_mode, format]
        arb_framebuffer_object[' '.join(test_name)] = PiglitGLTest(test_name, run_concurrent=True)
add_concurrent_test(arb_framebuffer_object, ['fbo-alpha'])
add_plain_test(arb_framebuffer_object, ['fbo-blit-stretch'])
add_concurrent_test(arb_framebuffer_object, ['fbo-blit-scaled-linear'])
add_concurrent_test(arb_framebuffer_object, ['fbo-attachments-blit-scaled-linear'])
add_concurrent_test(arb_framebuffer_object, ['fbo-deriv'])
add_concurrent_test(arb_framebuffer_object, ['fbo-luminance-alpha'])
add_concurrent_test(arb_framebuffer_object, ['fbo-getframebufferattachmentparameter-01'])
add_concurrent_test(arb_framebuffer_object, ['fbo-gl_pointcoord'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete-invalid-texture'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete-texture-01'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete-texture-02'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete-texture-03'])
add_concurrent_test(arb_framebuffer_object, ['fbo-incomplete-texture-04'])
add_concurrent_test(arb_framebuffer_object, ['fbo-mipmap-copypix'])
add_plain_test(arb_framebuffer_object, ['fbo-viewport']) # must not be concurrent
arb_framebuffer_object['FBO blit to missing attachment'] = PiglitGLTest(['fbo-missing-attachment-blit', 'to'], run_concurrent=True)
arb_framebuffer_object['FBO blit from missing attachment'] = PiglitGLTest(['fbo-missing-attachment-blit', 'from'], run_concurrent=True)
arb_framebuffer_object['fbo-scissor-blit fbo'] = PiglitGLTest(['fbo-scissor-blit', 'fbo'], run_concurrent=True)
arb_framebuffer_object['fbo-scissor-blit window'] = PiglitGLTest(['fbo-scissor-blit', 'window'])
arb_framebuffer_object['fbo-tex-rgbx'] = PiglitGLTest(['fbo-tex-rgbx'], run_concurrent=True)
arb_framebuffer_object['negative-readpixels-no-rb'] = PiglitGLTest(['arb_framebuffer_object-negative-readpixels-no-rb'], run_concurrent=True)
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glClear'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glClearBuffer'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'gl_FragColor'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'gl_FragData'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'use_frag_out'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glColorMaskIndexed'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glBlendFunci'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glDrawPixels'])
add_concurrent_test(arb_framebuffer_object, ['fbo-drawbuffers-none', 'glBlitFramebuffer'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-cubemap'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-cubemap', 'RGB9_E5'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-cubemap', 'S3TC_DXT1'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-1d'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-1d', 'RGB9_E5'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-3d'])
add_concurrent_test(arb_framebuffer_object, ['fbo-generatemipmap-3d', 'RGB9_E5'])

# Group ARB_framebuffer_sRGB
arb_framebuffer_srgb = spec['ARB_framebuffer_sRGB']
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
                arb_framebuffer_srgb[test_name] = PiglitGLTest(
                        ['arb_framebuffer_srgb-blit', backing_type, srgb_types,
                         blit_type, framebuffer_srgb_setting],
                        run_concurrent=True)
add_plain_test(arb_framebuffer_srgb, ['framebuffer-srgb']) # must not be concurrent
add_concurrent_test(arb_framebuffer_srgb, ['arb_framebuffer_srgb-clear'])

arb_gpu_shader5 = spec['ARB_gpu_shader5']
for stage in ['vs', 'fs']:
    for type in ['unorm', 'float', 'int', 'uint']:
        for comps in ['r', 'rg', 'rgb', 'rgba']:
            for cs in [0, 1, 2, 3][:len(comps)]:
                for sampler in ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']:
                    for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets' ]:
                        testname = grouptools.join(
                            func, '{}-{}-{}-{}-{}'.format(
                                stage, comps, cs, type, sampler))
                        address_mode = 'clamp' if sampler == '2DRect' else 'repeat'
                        cmd = ['textureGather', stage,
                                'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                                comps, str(cs), type, sampler, address_mode]
                        arb_gpu_shader5[testname] = PiglitGLTest(cmd, run_concurrent=True)

                        if func == 'textureGatherOffset':
                            # also add a constant offset version.
                            testname = grouptools.join(
                                func, '{}-{}-{}-{}-{}-const'.format(
                                    stage, comps, cs, type, sampler))
                            cmd = ['textureGather', stage, 'offset',
                                    comps, str(cs), type, sampler, address_mode]
                            arb_gpu_shader5[testname] = PiglitGLTest(cmd, run_concurrent=True)
    # test shadow samplers
    for sampler in ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']:
        for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets' ]:
            testname = grouptools.join(func, '{}-r-none-shadow-{}'.format(
                stage, sampler))
            address_mode = 'clamp' if sampler == '2DRect' else 'repeat'
            cmd = ['textureGather', stage, 'shadow', 'r',
                    'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                    sampler, address_mode]
            arb_gpu_shader5[testname] = PiglitGLTest(cmd, run_concurrent=True)
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-minmax'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-invocation-id'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-invocations_count_too_large'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-xfb-streams'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-stream_value_too_large'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-emitstreamvertex_stream_too_large'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-tf-wrong-stream-value'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-xfb-streams-without-invocations'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-emitstreamvertex_nodraw'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtCentroid'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtCentroid-packing'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtCentroid-flat'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtCentroid-centroid'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtCentroid-noperspective'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtSample'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtSample-nonconst'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtOffset'])
add_concurrent_test(arb_gpu_shader5, ['arb_gpu_shader5-interpolateAtOffset-nonconst'])

arb_shader_subroutine = spec['ARB_shader_subroutine']
add_concurrent_test(arb_shader_subroutine, ['arb_shader_subroutine-minmax'])

for type in ['double', 'dvec2', 'dvec3', 'dvec4', 'dmat2', 'dmat3', 'dmat4',
             'dmat2x3', 'dmat2x4', 'dmat3x2', 'dmat3x4', 'dmat4x2', 'dmat4x3']:
    for arrayspec in ['array', 'separate']:
        cmdline = 'simple {0} {1}'.format(type, arrayspec)
        spec['ARB_gpu_shader_fp64']['varying-packing'][cmdline] = \
            PiglitGLTest(['varying-packing-simple', type, arrayspec], run_concurrent=True)

arb_occlusion_query = spec['ARB_occlusion_query']
add_concurrent_test(arb_occlusion_query, ['occlusion_query'])
add_concurrent_test(arb_occlusion_query, ['occlusion_query_lifetime'])
add_concurrent_test(arb_occlusion_query, ['occlusion_query_meta_fragments'])
add_concurrent_test(arb_occlusion_query, ['occlusion_query_meta_no_fragments'])
add_concurrent_test(arb_occlusion_query, ['occlusion_query_meta_save'])
add_concurrent_test(arb_occlusion_query, ['occlusion_query_order'])
add_concurrent_test(arb_occlusion_query, ['gen_delete_while_active'])

# Group ARB_separate_shader_objects
arb_separate_shader_objects = spec['ARB_separate_shader_objects']
arb_separate_shader_objects['ActiveShaderProgram with invalid program'] = PiglitGLTest(['arb_separate_shader_object-ActiveShaderProgram-invalid-program'], run_concurrent=True)
arb_separate_shader_objects['GetProgramPipelineiv'] = PiglitGLTest(['arb_separate_shader_object-GetProgramPipelineiv'], run_concurrent=True)
arb_separate_shader_objects['IsProgramPipeline'] = PiglitGLTest(['arb_separate_shader_object-IsProgramPipeline'], run_concurrent=True)
arb_separate_shader_objects['UseProgramStages - non-separable program'] = PiglitGLTest(['arb_separate_shader_object-UseProgramStages-non-separable'], run_concurrent=True)
arb_separate_shader_objects['ProgramUniform coverage'] = PiglitGLTest(['arb_separate_shader_object-ProgramUniform-coverage'], run_concurrent=True)
arb_separate_shader_objects['Rendezvous by location'] = PiglitGLTest(['arb_separate_shader_object-rendezvous_by_location', '-fbo'])
arb_separate_shader_objects['ValidateProgramPipeline'] = PiglitGLTest(['arb_separate_shader_object-ValidateProgramPipeline'], run_concurrent=True)
arb_separate_shader_objects['400 combinations by location'] = PiglitGLTest(['arb_separate_shader_object-400-combinations', '-fbo', '--by-location'])
arb_separate_shader_objects['400 combinations by name'] = PiglitGLTest(['arb_separate_shader_object-400-combinations', '-fbo'])
arb_separate_shader_objects['active sampler conflict'] = PiglitGLTest(['arb_separate_shader_object-active-sampler-conflict'], run_concurrent=True)

# Group ARB_sampler_objects
arb_sampler_objects = spec['ARB_sampler_objects']
arb_sampler_objects['sampler-objects'] = PiglitGLTest(['arb_sampler_objects-sampler-objects'], run_concurrent=True)
arb_sampler_objects['sampler-incomplete'] = PiglitGLTest(['arb_sampler_objects-sampler-incomplete'], run_concurrent=True)
arb_sampler_objects['GL_EXT_texture_sRGB_decode'] = PiglitGLTest(['arb_sampler_objects-srgb-decode'], run_concurrent=True)
arb_sampler_objects['framebufferblit'] = PiglitGLTest(['arb_sampler_objects-framebufferblit'])

# Group ARB_sample_shading
arb_sample_shading = spec['ARB_sample_shading']
add_plain_test(arb_sample_shading, ['arb_sample_shading-api'])

TEST_SAMPLE_COUNTS = (0,) + MSAA_SAMPLE_COUNTS
for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-num-samples {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-id {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-mask {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

for num_samples in (0,2,4,6,8):
    test_name = 'builtin-gl-sample-mask-simple {0}'.format(num_samples)
    executable = 'arb_sample_shading-{}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = 'builtin-gl-sample-position {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = 'interpolate-at-sample-position {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = 'ignore-centroid-qualifier {0}'.format(num_samples)
    executable = 'arb_sample_shading-{0}'.format(test_name).split()
    arb_sample_shading[test_name] = PiglitGLTest(executable)

# Group ARB_debug_output
arb_debug_output = spec['ARB_debug_output']
add_plain_test(arb_debug_output, ['arb_debug_output-api_error'])

# Group KHR_debug
khr_debug = spec['KHR_debug']
khr_debug['object-label_gl'] = PiglitGLTest(['khr_debug-object-label_gl'], run_concurrent=True)
khr_debug['object-label_gles2'] = PiglitGLTest(['khr_debug-object-label_gles2'], run_concurrent=True)
khr_debug['object-label_gles3'] = PiglitGLTest(['khr_debug-object-label_gles3'], run_concurrent=True)
khr_debug['push-pop-group_gl'] = PiglitGLTest(['khr_debug-push-pop-group_gl'], run_concurrent=True)
khr_debug['push-pop-group_gles2'] = PiglitGLTest(['khr_debug-push-pop-group_gles2'], run_concurrent=True)
khr_debug['push-pop-group_gles3'] = PiglitGLTest(['khr_debug-push-pop-group_gles3'], run_concurrent=True)

# Group ARB_occlusion_query2
arb_occlusion_query2 = spec['ARB_occlusion_query2']
arb_occlusion_query2['api'] = PiglitGLTest(['arb_occlusion_query2-api'], run_concurrent=True)
arb_occlusion_query2['render'] = PiglitGLTest(['arb_occlusion_query2-render'], run_concurrent=True)

arb_pixel_buffer_object = spec['ARB_pixel_buffer_object']
add_plain_test(arb_pixel_buffer_object, ['fbo-pbo-readpixels-small'])
add_plain_test(arb_pixel_buffer_object, ['pbo-drawpixels'])
add_plain_test(arb_pixel_buffer_object, ['pbo-read-argb8888'])
add_plain_test(arb_pixel_buffer_object, ['pbo-readpixels-small'])
add_plain_test(arb_pixel_buffer_object, ['pbo-teximage'])
add_plain_test(arb_pixel_buffer_object, ['pbo-teximage-tiling'])
add_plain_test(arb_pixel_buffer_object, ['pbo-teximage-tiling-2'])
add_concurrent_test(arb_pixel_buffer_object, ['texsubimage', 'pbo'])
add_concurrent_test(arb_pixel_buffer_object, ['texsubimage', 'array', 'pbo'])
add_concurrent_test(arb_pixel_buffer_object, ['texsubimage', 'cube_map_array', 'pbo'])

# Group ARB_provoking_vertex
arb_provoking_vertex = spec['ARB_provoking_vertex']
add_plain_test(arb_provoking_vertex, ['arb-provoking-vertex-control'])
add_plain_test(arb_provoking_vertex, ['arb-provoking-vertex-initial'])
add_plain_test(arb_provoking_vertex, ['arb-provoking-vertex-render'])
add_plain_test(arb_provoking_vertex, ['arb-quads-follow-provoking-vertex'])
add_plain_test(arb_provoking_vertex, ['arb-xfb-before-flatshading'])

# Group ARB_robustness
arb_robustness = spec['ARB_robustness']
add_plain_test(arb_robustness, ['arb_robustness_client-mem-bounds'])
# TODO: robust vertex buffer access
#add_plain_test(arb_robustness, ['arb_robustness_draw-vbo-bounds'])

# Group ARB_shader_bit_encoding

# Group ARB_shader_texture_lod
arb_shader_texture_lod = spec['ARB_shader_texture_lod']
add_plain_test(arb_shader_texture_lod['execution'], ['arb_shader_texture_lod-texgrad'])
add_plain_test(arb_shader_texture_lod['execution'], ['arb_shader_texture_lod-texgradcube'])

add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', '1D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', '2D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', '3D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', 'Cube'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', '1DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*Lod', '2DShadow'])

add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '1D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '1D_ProjVec4'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '2D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '2D_ProjVec4'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '3D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '1DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjLod', '2DShadow'])

add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '1D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '2D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '3D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', 'Cube'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '1DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '2DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '2DRect'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*GradARB', '2DRectShadow'])

add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '1D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '1D_ProjVec4'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2D_ProjVec4'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '3D'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '1DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2DShadow'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2DRect'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2DRect_ProjVec4'])
add_concurrent_test(arb_shader_texture_lod['execution'], ['tex-miplevel-selection', '*ProjGradARB', '2DRectShadow'])


# Group ARB_shader_objects
arb_shader_objects = spec['ARB_shader_objects']
arb_shader_objects['getuniform'] = PiglitGLTest(['arb_shader_objects-getuniform'], run_concurrent=True)
arb_shader_objects['bindattriblocation-scratch-name'] = PiglitGLTest(['arb_shader_objects-bindattriblocation-scratch-name'], run_concurrent=True)
arb_shader_objects['getactiveuniform-beginend'] = PiglitGLTest(['arb_shader_objects-getactiveuniform-beginend'], run_concurrent=True)
arb_shader_objects['getuniformlocation-array-of-struct-of-array'] = PiglitGLTest(['arb_shader_objects-getuniformlocation-array-of-struct-of-array'], run_concurrent=True)
arb_shader_objects['clear-with-deleted'] = PiglitGLTest(['arb_shader_objects-clear-with-deleted'], run_concurrent=True)
arb_shader_objects['delete-repeat'] = PiglitGLTest(['arb_shader_objects-delete-repeat'], run_concurrent=True)

arb_shading_language_420pack = spec['ARB_shading_language_420pack']
spec['ARB_shading_language_420pack']['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'arb_shading_language_420pack', 'minimum-maximums.txt')],
    run_concurrent=True)
spec['ARB_shading_language_420pack']['multiple layout qualifiers'] = PiglitGLTest(['arb_shading_language_420pack-multiple-layout-qualifiers'], run_concurrent=True)
spec['ARB_shading_language_420pack']['binding layout'] = PiglitGLTest(['arb_shading_language_420pack-binding-layout'], run_concurrent=True)

# Group ARB_explicit_attrib_location
arb_explicit_attrib_location = spec['ARB_explicit_attrib_location']
add_plain_test(arb_explicit_attrib_location, ['glsl-explicit-location-01'])
add_plain_test(arb_explicit_attrib_location, ['glsl-explicit-location-02'])
add_plain_test(arb_explicit_attrib_location, ['glsl-explicit-location-03'])
add_plain_test(arb_explicit_attrib_location, ['glsl-explicit-location-04'])
add_plain_test(arb_explicit_attrib_location, ['glsl-explicit-location-05'])
for test_type in ('shader', 'api'):
    test_name = ['overlapping-locations-input-attribs', test_type]
    arb_explicit_attrib_location[' '.join(test_name)] = PiglitGLTest(test_name)

# Group ARB_explicit_uniform_location
arb_explicit_uniform_location = spec['ARB_explicit_uniform_location']
add_plain_test(arb_explicit_uniform_location, ['arb_explicit_uniform_location-minmax'])
add_plain_test(arb_explicit_uniform_location, ['arb_explicit_uniform_location-boundaries'])
add_plain_test(arb_explicit_uniform_location, ['arb_explicit_uniform_location-array-elements'])
add_plain_test(arb_explicit_uniform_location, ['arb_explicit_uniform_location-inactive-uniform'])
add_plain_test(arb_explicit_uniform_location, ['arb_explicit_uniform_location-use-of-unused-loc'])

arb_texture_buffer_object = spec['ARB_texture_buffer_object']
arb_texture_buffer_object['data-sync'] = PiglitGLTest(['arb_texture_buffer_object-data-sync'], run_concurrent=True)
arb_texture_buffer_object['dlist'] = PiglitGLTest(['arb_texture_buffer_object-dlist'], run_concurrent=True)
arb_texture_buffer_object['formats (FS, 3.1 core)'] = PiglitGLTest(['arb_texture_buffer_object-formats', 'fs', 'core'], run_concurrent=True)
arb_texture_buffer_object['formats (VS, 3.1 core)'] = PiglitGLTest(['arb_texture_buffer_object-formats', 'vs', 'core'], run_concurrent=True)
arb_texture_buffer_object['formats (FS, ARB)'] = PiglitGLTest(['arb_texture_buffer_object-formats', 'fs', 'arb'], run_concurrent=True)
arb_texture_buffer_object['formats (VS, ARB)'] = PiglitGLTest(['arb_texture_buffer_object-formats', 'vs', 'arb'], run_concurrent=True)
arb_texture_buffer_object['get'] = PiglitGLTest(['arb_texture_buffer_object-get'], run_concurrent=True)
arb_texture_buffer_object['fetch-outside-bounds'] = PiglitGLTest(['arb_texture_buffer_object-fetch-outside-bounds'], run_concurrent=True)
arb_texture_buffer_object['minmax'] = PiglitGLTest(['arb_texture_buffer_object-minmax'], run_concurrent=True)
arb_texture_buffer_object['negative-bad-bo'] = PiglitGLTest(['arb_texture_buffer_object-negative-bad-bo'], run_concurrent=True)
arb_texture_buffer_object['negative-bad-format'] = PiglitGLTest(['arb_texture_buffer_object-negative-bad-format'], run_concurrent=True)
arb_texture_buffer_object['negative-bad-target'] = PiglitGLTest(['arb_texture_buffer_object-negative-bad-target'], run_concurrent=True)
arb_texture_buffer_object['negative-unsupported'] = PiglitGLTest(['arb_texture_buffer_object-negative-unsupported'], run_concurrent=True)
arb_texture_buffer_object['subdata-sync'] = PiglitGLTest(['arb_texture_buffer_object-subdata-sync'], run_concurrent=True)
arb_texture_buffer_object['unused-name'] = PiglitGLTest(['arb_texture_buffer_object-unused-name'], run_concurrent=True)

arb_texture_buffer_range = spec['ARB_texture_buffer_range']
arb_texture_buffer_range['dlist'] = PiglitGLTest(['arb_texture_buffer_range-dlist'], run_concurrent=True)
arb_texture_buffer_range['errors'] = PiglitGLTest(['arb_texture_buffer_range-errors'], run_concurrent=True)
arb_texture_buffer_range['ranges'] = PiglitGLTest(['arb_texture_buffer_range-ranges'], run_concurrent=True)
arb_texture_buffer_range['ranges-2'] = PiglitGLTest(['arb_texture_buffer_range-ranges-2'], run_concurrent=True)


arb_texture_rectangle = spec['ARB_texture_rectangle']
add_texwrap_target_tests(arb_texture_rectangle, 'RECT')
add_msaa_visual_plain_tests(arb_texture_rectangle, ['copyteximage', 'RECT'])
add_concurrent_test(arb_texture_rectangle, ['1-1-linear-texture'])
add_plain_test(arb_texture_rectangle, ['texrect-many'])
add_concurrent_test(arb_texture_rectangle, ['getteximage-targets', 'RECT'])
add_plain_test(arb_texture_rectangle, ['texrect_simple_arb_texrect'])
add_plain_test(arb_texture_rectangle, ['arb_texrect-texture-base-level-error'])
add_plain_test(arb_texture_rectangle, ['fbo-blit', 'rect'])
add_concurrent_test(spec['ARB_texture_rectangle'], ['tex-miplevel-selection', 'GL2:texture()', '2DRect'])
add_concurrent_test(spec['ARB_texture_rectangle'], ['tex-miplevel-selection', 'GL2:texture()', '2DRectShadow'])
add_concurrent_test(spec['ARB_texture_rectangle'], ['tex-miplevel-selection', 'GL2:textureProj', '2DRect'])
add_concurrent_test(spec['ARB_texture_rectangle'], ['tex-miplevel-selection', 'GL2:textureProj', '2DRect_ProjVec4'])
add_concurrent_test(spec['ARB_texture_rectangle'], ['tex-miplevel-selection', 'GL2:textureProj', '2DRectShadow'])

arb_texture_storage = spec['ARB_texture_storage']
arb_texture_storage['texture-storage'] = PiglitGLTest(['arb_texture_storage-texture-storage'])

arb_texture_storage_multisample = spec['ARB_texture_storage_multisample']
arb_texture_storage_multisample['tex-storage'] = PiglitGLTest(['arb_texture_storage_multisample-tex-storage'], run_concurrent=True)
arb_texture_storage_multisample['tex-param'] = PiglitGLTest(['arb_texture_storage_multisample-tex-param'], run_concurrent=True)

arb_texture_view = spec['ARB_texture_view']
arb_texture_view['cubemap-view'] = PiglitGLTest(['arb_texture_view-cubemap-view'], run_concurrent=True)
arb_texture_view['immutable_levels'] = PiglitGLTest(['arb_texture_view-texture-immutable-levels'], run_concurrent=True)
arb_texture_view['max-level'] = PiglitGLTest(['arb_texture_view-max-level'], run_concurrent=True)
arb_texture_view['params'] = PiglitGLTest(['arb_texture_view-params'], run_concurrent=True)
arb_texture_view['formats'] = PiglitGLTest(['arb_texture_view-formats'], run_concurrent=True)
arb_texture_view['targets'] = PiglitGLTest(['arb_texture_view-targets'], run_concurrent=True)
arb_texture_view['queries'] = PiglitGLTest(['arb_texture_view-queries'], run_concurrent=True)
arb_texture_view['rendering-target'] = PiglitGLTest(['arb_texture_view-rendering-target'], run_concurrent=True)
arb_texture_view['rendering-levels'] = PiglitGLTest(['arb_texture_view-rendering-levels'], run_concurrent=True)
arb_texture_view['rendering-layers'] = PiglitGLTest(['arb_texture_view-rendering-layers'], run_concurrent=True)
arb_texture_view['rendering-formats'] = PiglitGLTest(['arb_texture_view-rendering-formats'], run_concurrent=True)
arb_texture_view['lifetime-format'] = PiglitGLTest(['arb_texture_view-lifetime-format'], run_concurrent=True)
arb_texture_view['getteximage-srgb'] = PiglitGLTest(['arb_texture_view-getteximage-srgb'], run_concurrent=True)
arb_texture_view['texsubimage-levels'] = PiglitGLTest(['arb_texture_view-texsubimage-levels'], run_concurrent=True)
arb_texture_view['texsubimage-layers'] = PiglitGLTest(['arb_texture_view-texsubimage-layers'], run_concurrent=True)
arb_texture_view['clear-into-view-2d'] = PiglitGLTest(['arb_texture_view-clear-into-view-2d'], run_concurrent=True)
arb_texture_view['clear-into-view-2d-array'] = PiglitGLTest(['arb_texture_view-clear-into-view-2d-array'], run_concurrent=True)
arb_texture_view['clear-into-view-layered'] = PiglitGLTest(['arb_texture_view-clear-into-view-layered'], run_concurrent=True)
arb_texture_view['copytexsubimage-layers'] = PiglitGLTest(['arb_texture_view-copytexsubimage-layers'], run_concurrent=True)
arb_texture_view['sampling-2d-array-as-cubemap'] = PiglitGLTest(['arb_texture_view-sampling-2d-array-as-cubemap'], run_concurrent=True)
arb_texture_view['sampling-2d-array-as-cubemap-array'] = PiglitGLTest(['arb_texture_view-sampling-2d-array-as-cubemap-array'], run_concurrent=True)

tdfx_texture_compression_fxt1 = spec['3DFX_texture_compression_FXT1']
add_concurrent_test(tdfx_texture_compression_fxt1, ['compressedteximage', 'GL_COMPRESSED_RGB_FXT1_3DFX'])
add_concurrent_test(tdfx_texture_compression_fxt1, ['compressedteximage', 'GL_COMPRESSED_RGBA_FXT1_3DFX'])
add_fbo_generatemipmap_extension(tdfx_texture_compression_fxt1, 'GL_3DFX_texture_compression_FXT1', 'fbo-generatemipmap-formats')
tdfx_texture_compression_fxt1['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'fxt1'], run_concurrent=True)
add_plain_test(tdfx_texture_compression_fxt1, ['fxt1-teximage'])

def add_color_buffer_float_test(name, format, p1, p2):
    group = '{}-{}{}{}'.format(
        format, name,
        '-{}'.format(p1) if p1 else '',
        '-{}'.format(p2) if p2 else '')
    arb_color_buffer_float[group] = PiglitGLTest(
        ['arb_color_buffer_float-' + name, format, p1, p2],
        run_concurrent=True)

arb_color_buffer_float = spec['ARB_color_buffer_float']
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


arb_depth_texture = spec['ARB_depth_texture']
add_fbo_formats_tests(grouptools.join('spec', 'ARB_depth_texture'), 'GL_ARB_depth_texture')
add_texwrap_format_tests(arb_depth_texture, 'GL_ARB_depth_texture')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT16')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT24')
add_fbo_depth_tests(arb_depth_texture, 'GL_DEPTH_COMPONENT32')
add_plain_test(arb_depth_texture, ['depth-level-clamp'])
add_plain_test(arb_depth_texture, ['depth-tex-modes'])
add_plain_test(arb_depth_texture, ['texdepth'])
add_depthstencil_render_miplevels_tests(arb_depth_texture, ('d=z24', 'd=z16'))

arb_depth_buffer_float = spec['ARB_depth_buffer_float']
add_fbo_depth_tests(arb_depth_buffer_float, 'GL_DEPTH_COMPONENT32F')
add_fbo_depth_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8')
add_fbo_stencil_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8')
add_fbo_depthstencil_tests(arb_depth_buffer_float, 'GL_DEPTH32F_STENCIL8', 0)
add_fbo_formats_tests(grouptools.join('spec', 'ARB_depth_buffer_float'), 'GL_ARB_depth_buffer_float')
add_texwrap_format_tests(arb_depth_buffer_float, 'GL_ARB_depth_buffer_float')
add_depthstencil_render_miplevels_tests(
        arb_depth_buffer_float,
        ('d=z32f_s8', 'd=z32f', 'd=z32f_s8_s=z24_s8', 'd=z32f_s=z24_s8',
         's=z24_s8_d=z32f_s8', 's=z24_s8_d=z32f', 'd=s=z32f_s8', 's=d=z32f_s8',
         'ds=z32f_s8'))
arb_depth_buffer_float['fbo-clear-formats stencil'] = PiglitGLTest(['fbo-clear-formats', 'GL_ARB_depth_buffer_float', 'stencil'], run_concurrent=True)

arb_texture_env_crossbar = spec['ARB_texture_env_crossbar']
add_plain_test(arb_texture_env_crossbar, ['crossbar'])

arb_texture_compression = spec['ARB_texture_compression']
add_fbo_generatemipmap_extension(arb_texture_compression, 'GL_ARB_texture_compression', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(arb_texture_compression, 'GL_ARB_texture_compression')
arb_texture_compression['GL_TEXTURE_INTERNAL_FORMAT query'] = PiglitGLTest(['arb_texture_compression-internal-format-query'], run_concurrent=True)
arb_texture_compression['unknown formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'unknown'], run_concurrent=True)

arb_texture_compression_bptc = spec['ARB_texture_compression_bptc']
arb_texture_compression_bptc['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'bptc'], run_concurrent=True)
add_concurrent_test(arb_texture_compression_bptc, ['bptc-modes'])
add_concurrent_test(arb_texture_compression_bptc, ['bptc-float-modes'])
add_fbo_generatemipmap_extension(arb_texture_compression_bptc, 'GL_ARB_texture_compression_bptc-unorm', 'fbo-generatemipmap-formats')
add_fbo_generatemipmap_extension(arb_texture_compression_bptc, 'GL_ARB_texture_compression_bptc-float', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(arb_texture_compression_bptc, 'GL_ARB_texture_compression_bptc')
add_concurrent_test(arb_texture_compression_bptc, ['compressedteximage', 'GL_COMPRESSED_RGBA_BPTC_UNORM'])
add_concurrent_test(arb_texture_compression_bptc, ['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM'])
add_concurrent_test(arb_texture_compression_bptc, ['compressedteximage', 'GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT'])
add_concurrent_test(arb_texture_compression_bptc, ['compressedteximage', 'GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT'])

ext_vertex_array_bgra = spec['EXT_vertex_array_bgra']
add_plain_test(ext_vertex_array_bgra, ['bgra-sec-color-pointer'])
add_plain_test(ext_vertex_array_bgra, ['bgra-vert-attrib-pointer'])

apple_vertex_array_object = spec['APPLE_vertex_array_object']
add_plain_test(apple_vertex_array_object, ['vao-01'])
add_plain_test(apple_vertex_array_object, ['vao-02'])
apple_vertex_array_object['isvertexarray'] = PiglitGLTest(['arb_vertex_array-isvertexarray', 'apple'], run_concurrent=True)

profile.test_list[grouptools.join('spec', 'ARB_vertex_array_bgra', 'api-errors')] = PiglitGLTest(['arb_vertex_array_bgra-api-errors'])
profile.test_list[grouptools.join('spec', 'ARB_vertex_array_bgra', 'get')] = PiglitGLTest(['arb_vertex_array_bgra-get'])

arb_vertex_array_object = spec['ARB_vertex_array_object']
add_concurrent_test(arb_vertex_array_object, ['vao-element-array-buffer'])
arb_vertex_array_object['isvertexarray'] = PiglitGLTest(['arb_vertex_array-isvertexarray'], run_concurrent=True)

arb_vertex_buffer_object = spec['ARB_vertex_buffer_object']
arb_vertex_buffer_object['elements-negative-offset'] = PiglitGLTest(['arb_vertex_buffer_object-elements-negative-offset'])
arb_vertex_buffer_object['mixed-immediate-and-vbo'] = PiglitGLTest(['arb_vertex_buffer_object-mixed-immediate-and-vbo'])
add_plain_test(arb_vertex_buffer_object, ['fdo14575'])
add_plain_test(arb_vertex_buffer_object, ['fdo22540'])
add_plain_test(arb_vertex_buffer_object, ['fdo31934'])
arb_vertex_buffer_object['ib-data-sync'] = PiglitGLTest(['arb_vertex_buffer_object-ib-data-sync'], run_concurrent=True)
arb_vertex_buffer_object['ib-subdata-sync'] = PiglitGLTest(['arb_vertex_buffer_object-ib-subdata-sync'], run_concurrent=True)
add_plain_test(arb_vertex_buffer_object, ['pos-array'])
add_plain_test(arb_vertex_buffer_object, ['vbo-bufferdata'])
add_plain_test(arb_vertex_buffer_object, ['vbo-map-remap'])
add_concurrent_test(arb_vertex_buffer_object, ['vbo-map-unsync'])
arb_vertex_buffer_object['vbo-subdata-many drawarrays'] = PiglitGLTest(['arb_vertex_buffer_object-vbo-subdata-many', 'drawarrays'], run_concurrent=True)
arb_vertex_buffer_object['vbo-subdata-many drawelements'] = PiglitGLTest(['arb_vertex_buffer_object-vbo-subdata-many', 'drawelements'], run_concurrent=True)
arb_vertex_buffer_object['vbo-subdata-many drawrangeelements'] = PiglitGLTest(['arb_vertex_buffer_object-vbo-subdata-many', 'drawrangeelements'], run_concurrent=True)
add_plain_test(arb_vertex_buffer_object, ['vbo-subdata-sync'])
add_plain_test(arb_vertex_buffer_object, ['vbo-subdata-zero'])

arb_vertex_program = spec['ARB_vertex_program']
arb_vertex_program['getenv4d-with-error'] = PiglitGLTest(['arb_vertex_program-getenv4d-with-error'])
arb_vertex_program['getlocal4d-with-error'] = PiglitGLTest(['arb_vertex_program-getlocal4d-with-error'])
arb_vertex_program['getlocal4f-max'] = PiglitGLTest(['arb_vertex_program-getlocal4f-max'], run_concurrent=True)
arb_vertex_program['getlocal4-errors'] = PiglitGLTest(['arb_vertex_program-getlocal4-errors'], run_concurrent=True)
arb_vertex_program['clip-plane-transformation arb'] = PiglitGLTest(['clip-plane-transformation', 'arb'], run_concurrent=True)
arb_vertex_program['minmax'] = PiglitGLTest(['arb_vertex_program-minmax'], run_concurrent=True)
add_plain_test(arb_vertex_program, ['fdo24066'])
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
add_plain_test(arb_vertex_program, ['vp-address-01'])
add_plain_test(arb_vertex_program, ['vp-address-02'])
add_plain_test(arb_vertex_program, ['vp-address-04'])
add_plain_test(arb_vertex_program, ['vp-bad-program'])
add_plain_test(arb_vertex_program, ['vp-max-array'])

arb_viewport_array = spec['ARB_viewport_array']
arb_viewport_array['viewport-indices'] = PiglitGLTest(['arb_viewport_array-viewport-indices'], run_concurrent=True)
arb_viewport_array['depthrange-indices'] = PiglitGLTest(['arb_viewport_array-depthrange-indices'], run_concurrent=True)
arb_viewport_array['scissor-check'] = PiglitGLTest(['arb_viewport_array-scissor-check'], run_concurrent=True)
arb_viewport_array['scissor-indices'] = PiglitGLTest(['arb_viewport_array-scissor-indices'], run_concurrent=True)
arb_viewport_array['bounds'] = PiglitGLTest(['arb_viewport_array-bounds'], run_concurrent=True)
arb_viewport_array['queries'] = PiglitGLTest(['arb_viewport_array-queries'], run_concurrent=True)
arb_viewport_array['minmax'] = PiglitGLTest(['arb_viewport_array-minmax'], run_concurrent=True)
arb_viewport_array['render-viewport'] = PiglitGLTest(['arb_viewport_array-render-viewport'], run_concurrent=True)
arb_viewport_array['render-depthrange'] = PiglitGLTest(['arb_viewport_array-render-depthrange'], run_concurrent=True)
arb_viewport_array['render-scissor'] = PiglitGLTest(['arb_viewport_array-render-scissor'], run_concurrent=True)
arb_viewport_array['clear'] =  PiglitGLTest(['arb_viewport_array-clear'], run_concurrent=True)

nv_vertex_program = spec['NV_vertex_program']
add_vpfpgeneric(nv_vertex_program, 'nv-mov')
add_vpfpgeneric(nv_vertex_program, 'nv-add')
add_vpfpgeneric(nv_vertex_program, 'nv-arl')
add_vpfpgeneric(nv_vertex_program, 'nv-init-zero-reg')
add_vpfpgeneric(nv_vertex_program, 'nv-init-zero-addr')

nv_vertex_program2_option = spec['NV_vertex_program2_option']
add_plain_test(nv_vertex_program2_option, ['vp-address-03'])
add_plain_test(nv_vertex_program2_option, ['vp-address-05'])
add_plain_test(nv_vertex_program2_option, ['vp-address-06'])
add_plain_test(nv_vertex_program2_option, ['vp-clipdistance-01'])
add_plain_test(nv_vertex_program2_option, ['vp-clipdistance-02'])
add_plain_test(nv_vertex_program2_option, ['vp-clipdistance-03'])
add_plain_test(nv_vertex_program2_option, ['vp-clipdistance-04'])

ext_framebuffer_blit = spec['EXT_framebuffer_blit']
add_plain_test(ext_framebuffer_blit, ['fbo-blit']) # must not be concurrent
add_plain_test(ext_framebuffer_blit, ['fbo-copypix']) # must not be concurrent
add_plain_test(ext_framebuffer_blit, ['fbo-readdrawpix']) # must not be concurrent
add_concurrent_test(ext_framebuffer_blit, ['fbo-srgb-blit'])
add_plain_test(ext_framebuffer_blit, ['fbo-sys-blit']) # must not be concurrent
add_plain_test(ext_framebuffer_blit, ['fbo-sys-sub-blit']) # must not be concurrent

ext_framebuffer_multisample_blit_scaled = spec['EXT_framebuffer_multisample_blit_scaled']
ext_framebuffer_multisample_blit_scaled['negative-blit-scaled'] = PiglitGLTest(['ext_framebuffer_multisample_blit_scaled-negative-blit-scaled'], run_concurrent=True)
for num_samples in MSAA_SAMPLE_COUNTS:
    ext_framebuffer_multisample_blit_scaled['blit-scaled samples=' + str(num_samples)] = \
        PiglitGLTest(['ext_framebuffer_multisample_blit_scaled-blit-scaled', str(num_samples)], run_concurrent=True)

ext_framebuffer_multisample = spec['EXT_framebuffer_multisample']
ext_framebuffer_multisample['blit-mismatched-samples'] = PiglitGLTest(['ext_framebuffer_multisample-blit-mismatched-samples'], run_concurrent=True)
ext_framebuffer_multisample['blit-mismatched-sizes'] = PiglitGLTest(['ext_framebuffer_multisample-blit-mismatched-sizes'], run_concurrent=True)
ext_framebuffer_multisample['blit-mismatched-formats'] = PiglitGLTest(['ext_framebuffer_multisample-blit-mismatched-formats'], run_concurrent=True)
ext_framebuffer_multisample['dlist'] = PiglitGLTest(['ext_framebuffer_multisample-dlist'], run_concurrent=True)
ext_framebuffer_multisample['enable-flag'] = PiglitGLTest(['ext_framebuffer_multisample-enable-flag'], run_concurrent=True)
ext_framebuffer_multisample['minmax'] = PiglitGLTest(['ext_framebuffer_multisample-minmax'], run_concurrent=True)
ext_framebuffer_multisample['negative-copypixels'] = PiglitGLTest(['ext_framebuffer_multisample-negative-copypixels'], run_concurrent=True)
ext_framebuffer_multisample['negative-copyteximage'] = PiglitGLTest(['ext_framebuffer_multisample-negative-copyteximage'], run_concurrent=True)
ext_framebuffer_multisample['negative-max-samples'] = PiglitGLTest(['ext_framebuffer_multisample-negative-max-samples'], run_concurrent=True)
ext_framebuffer_multisample['negative-mismatched-samples'] = PiglitGLTest(['ext_framebuffer_multisample-negative-mismatched-samples'], run_concurrent=True)
ext_framebuffer_multisample['negative-readpixels'] = PiglitGLTest(['ext_framebuffer_multisample-negative-readpixels'], run_concurrent=True)
ext_framebuffer_multisample['renderbufferstorage-samples'] = PiglitGLTest(['ext_framebuffer_multisample-renderbufferstorage-samples'], run_concurrent=True)
ext_framebuffer_multisample['renderbuffer-samples'] = PiglitGLTest(['ext_framebuffer_multisample-renderbuffer-samples'], run_concurrent=True)
ext_framebuffer_multisample['samples'] = PiglitGLTest(['ext_framebuffer_multisample-samples'], run_concurrent=True)
ext_framebuffer_multisample['alpha-blending'] = PiglitGLTest(['ext_framebuffer_multisample-alpha-blending'], run_concurrent=True)
ext_framebuffer_multisample['alpha-blending slow_cc'] = PiglitGLTest(['ext_framebuffer_multisample-alpha-blending', 'slow_cc'], run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    if num_samples % 2 != 0:
        continue
    test_name = 'alpha-blending-after-rendering'
    ext_framebuffer_multisample['{} {}'.format(test_name, str(num_samples))] = PiglitGLTest(
        ['ext_framebuffer_multisample-' + test_name, str(num_samples)], run_concurrent=True)

for num_samples in ('all_samples', ) + MSAA_SAMPLE_COUNTS:
    for test_type in ('color', 'srgb', 'stencil_draw', 'stencil_resolve',
                      'depth_draw', 'depth_resolve'):
        sensible_options = ['small', 'depthstencil']
        if test_type in ('color', 'srgb'):
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' '.join(['accuracy', str(num_samples), test_type]
                                 + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
            ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['turn-on-off', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        sensible_options = []
        if buffer_type == 'color':
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' '.join(['upsample', str(num_samples), buffer_type]
                                 + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
            ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        sensible_options = []
        if buffer_type == 'color':
            sensible_options.append('linear')
        for options in power_set(sensible_options):
            test_name = ' ' .join(['multisample-blit', str(num_samples),
                                   buffer_type] + options)
            executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
            ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        for blit_type in ('msaa', 'upsample', 'downsample'):
            test_name = ' '.join(['unaligned-blit', str(num_samples), buffer_type, blit_type])
            executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
            ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['line-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['point-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' ' .join(['polygon-smooth', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in ('all_samples', ) + MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['formats', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for test_mode in ('inverted', 'non-inverted'):
        test_name = ' '.join(['sample-coverage', str(num_samples), test_mode])
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth'):
        test_name = ' '.join(['sample-alpha-to-coverage', str(num_samples), buffer_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['sample-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['draw-buffers-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['draw-buffers-alpha-to-coverage', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-coverage-no-draw-buffer-zero', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-coverage-dual-src-blend', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-dual-src-blend', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['int-draw-buffers-alpha-to-one', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['int-draw-buffers-alpha-to-coverage', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-msaa-disabled', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['alpha-to-one-single-sample-buffer', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['bitmap', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    test_name = ' '.join(['polygon-stipple', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for blit_type in ('msaa', 'upsample', 'downsample', 'normal'):
        test_name = ' '.join(['clip-and-scissor-blit',
                              str(num_samples), blit_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(
            executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for flip_direction in ('x', 'y'):
        test_name = ' '.join(['blit-flipped', str(num_samples),
                              flip_direction])
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(
            executable, run_concurrent=True)

for num_samples in TEST_SAMPLE_COUNTS:
    test_name = ' '.join(['blit-multiple-render-targets', str(num_samples)])
    executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
    ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

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
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for buffer_type in ('color', 'depth', 'stencil'):
        test_name = ' '.join(['clear', str(num_samples), buffer_type])
        executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
        ext_framebuffer_multisample[test_name] = PiglitGLTest(
            executable, run_concurrent=True)

for num_samples in MSAA_SAMPLE_COUNTS:
    for test_type in ('depth', 'depth-computed', 'stencil'):
        for buffer_config in ('combined', 'separate', 'single'):
            test_name = ' '.join(['no-color', str(num_samples),
                                  test_type, buffer_config])
            executable = 'ext_framebuffer_multisample-{0}'.format(test_name).split()
            ext_framebuffer_multisample[test_name] = PiglitGLTest(executable, run_concurrent=True)

ext_framebuffer_object = spec['EXT_framebuffer_object']
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX1')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX4')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX8')
add_fbo_stencil_tests(ext_framebuffer_object, 'GL_STENCIL_INDEX16')
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-noimage'])
add_concurrent_test(ext_framebuffer_object, ['fdo20701'])
add_concurrent_test(ext_framebuffer_object, ['fbo-1d'])
add_concurrent_test(ext_framebuffer_object, ['fbo-3d'])
add_concurrent_test(ext_framebuffer_object, ['fbo-alphatest-formats'])
add_concurrent_test(ext_framebuffer_object, ['fbo-alphatest-nocolor'])
add_concurrent_test(ext_framebuffer_object, ['fbo-alphatest-nocolor-ff'])
add_concurrent_test(ext_framebuffer_object, ['fbo-blending-formats'])
add_concurrent_test(ext_framebuffer_object, ['fbo-bind-renderbuffer'])
add_concurrent_test(ext_framebuffer_object, ['fbo-clearmipmap'])
add_concurrent_test(ext_framebuffer_object, ['fbo-clear-formats'])
add_concurrent_test(ext_framebuffer_object, ['fbo-copyteximage'])
add_concurrent_test(ext_framebuffer_object, ['fbo-copyteximage-simple'])
add_concurrent_test(ext_framebuffer_object, ['fbo-cubemap'])
add_concurrent_test(ext_framebuffer_object, ['fbo-depthtex'])
add_concurrent_test(ext_framebuffer_object, ['fbo-depth-sample-compare'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers', 'masked-clear'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers-arbfp'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers-blend-add'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers-fragcolor'])
add_concurrent_test(ext_framebuffer_object, ['fbo-drawbuffers-maxtargets'])
add_concurrent_test(ext_framebuffer_object, ['fbo-finish-deleted'])
add_concurrent_test(ext_framebuffer_object, ['fbo-flushing'])
add_concurrent_test(ext_framebuffer_object, ['fbo-flushing-2'])
add_concurrent_test(ext_framebuffer_object, ['fbo-fragcoord'])
add_concurrent_test(ext_framebuffer_object, ['fbo-fragcoord2'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-filtering'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-formats'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-scissor'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-swizzle'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-nonsquare'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-npot'])
add_concurrent_test(ext_framebuffer_object, ['fbo-generatemipmap-viewport'])
add_concurrent_test(ext_framebuffer_object, ['fbo-maxsize'])
add_concurrent_test(ext_framebuffer_object, ['fbo-nodepth-test'])
add_concurrent_test(ext_framebuffer_object, ['fbo-nostencil-test'])
add_concurrent_test(ext_framebuffer_object, ['fbo-readpixels'])
add_concurrent_test(ext_framebuffer_object, ['fbo-readpixels-depth-formats'])
add_concurrent_test(ext_framebuffer_object, ['fbo-scissor-bitmap'])
add_concurrent_test(ext_framebuffer_object, ['fbo-storage-completeness'])
add_concurrent_test(ext_framebuffer_object, ['fbo-storage-formats'])
add_concurrent_test(ext_framebuffer_object, ['getteximage-formats', 'init-by-rendering'])

ext_image_dma_buf_import = spec['EXT_image_dma_buf_import']
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-invalid_hints'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-invalid_attributes'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-missing_attributes'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-ownership_transfer'])
ext_image_dma_buf_import['ext_image_dma_buf_import-sample_argb8888'] = PiglitGLTest(['ext_image_dma_buf_import-sample_rgb', '-fmt=AR24'])
ext_image_dma_buf_import['ext_image_dma_buf_import-sample_xrgb8888'] = PiglitGLTest(['ext_image_dma_buf_import-sample_rgb', '-fmt=XR24', '-alpha-one'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-intel_unsupported_format'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-intel_external_sampler_only'])
add_plain_test(ext_image_dma_buf_import, ['ext_image_dma_buf_import-intel_external_sampler_with_dma_only'])

ext_packed_depth_stencil = spec['EXT_packed_depth_stencil']
add_fbo_depth_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8')
add_fbo_stencil_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8')
add_fbo_depthstencil_tests(ext_packed_depth_stencil, 'GL_DEPTH24_STENCIL8', 0)
add_fbo_formats_tests(grouptools.join('spec', 'EXT_packed_depth_stencil'), 'GL_EXT_packed_depth_stencil')
add_texwrap_format_tests(ext_packed_depth_stencil, 'GL_EXT_packed_depth_stencil')
ext_packed_depth_stencil['readpixels-24_8'] = PiglitGLTest(['ext_packed_depth_stencil-readpixels-24_8'])
add_plain_test(ext_packed_depth_stencil, ['fbo-blit-d24s8'])
add_depthstencil_render_miplevels_tests(
        ext_packed_depth_stencil,
        ('s=z24_s8', 'd=z24_s8', 'd=z24_s8_s=z24_s8', 'd=z24_s=z24_s8',
         's=z24_s8_d=z24_s8', 's=z24_s8_d=z24', 'd=s=z24_s8', 's=d=z24_s8',
         'ds=z24_s8'))
ext_packed_depth_stencil['fbo-clear-formats stencil'] = PiglitGLTest(['fbo-clear-formats', 'GL_EXT_packed_depth_stencil', 'stencil'], run_concurrent=True)
ext_packed_depth_stencil['DEPTH_STENCIL texture'] = PiglitGLTest(['ext_packed_depth_stencil-depth-stencil-texture'], run_concurrent=True)
ext_packed_depth_stencil['errors'] = PiglitGLTest(['ext_packed_depth_stencil-errors'], run_concurrent=True)
ext_packed_depth_stencil['getteximage'] = PiglitGLTest(['ext_packed_depth_stencil-getteximage'], run_concurrent=True)
ext_packed_depth_stencil['readdrawpixels'] = PiglitGLTest(['ext_packed_depth_stencil-readdrawpixels'], run_concurrent=True)
ext_packed_depth_stencil['texsubimage'] = PiglitGLTest(['ext_packed_depth_stencil-texsubimage'], run_concurrent=True)

oes_packed_depth_stencil = spec['OES_packed_depth_stencil']
oes_packed_depth_stencil['DEPTH_STENCIL texture GLES2'] = PiglitGLTest(['oes_packed_depth_stencil-depth-stencil-texture_gles2'], run_concurrent=True)
oes_packed_depth_stencil['DEPTH_STENCIL texture GLES1'] = PiglitGLTest(['oes_packed_depth_stencil-depth-stencil-texture_gles1'], run_concurrent=True)

ext_texture_array = spec['EXT_texture_array']
add_concurrent_test(ext_texture_array, ['fbo-generatemipmap-array'])
add_concurrent_test(ext_texture_array, ['fbo-generatemipmap-array', 'RGB9_E5'])
add_concurrent_test(ext_texture_array, ['fbo-generatemipmap-array', 'S3TC_DXT1'])
spec['EXT_texture_array']['maxlayers'] = PiglitGLTest(['ext_texture_array-maxlayers'], run_concurrent=True)
spec['EXT_texture_array']['gen-mipmap'] = PiglitGLTest(['ext_texture_array-gen-mipmap'], run_concurrent=True)
add_msaa_visual_plain_tests(ext_texture_array, ['copyteximage', '1D_ARRAY'])
add_msaa_visual_plain_tests(ext_texture_array, ['copyteximage', '2D_ARRAY'])
add_plain_test(ext_texture_array, ['fbo-array'])
for test in ('depth-clear', 'depth-layered-clear', 'depth-draw', 'fs-writes-depth',
             'stencil-clear', 'stencil-layered-clear', 'stencil-draw', 'fs-writes-stencil'):
    add_concurrent_test(ext_texture_array, ['fbo-depth-array', test])
add_plain_test(ext_texture_array, ['array-texture'])
add_concurrent_test(ext_texture_array, ['ext_texture_array-errors'])
add_concurrent_test(ext_texture_array, ['getteximage-targets', '1D_ARRAY'])
add_concurrent_test(ext_texture_array, ['getteximage-targets', '2D_ARRAY'])
for test_mode in ['teximage', 'texsubimage']:
    test_name = 'compressed {0}'.format(test_mode)
    ext_texture_array[test_name] = PiglitGLTest(['ext_texture_array-{}'.format(test_name), '-fbo'])
add_concurrent_test(ext_texture_array, ['texsubimage', 'array'])

arb_texture_cube_map = spec['ARB_texture_cube_map']
add_msaa_visual_plain_tests(arb_texture_cube_map, ['copyteximage', 'CUBE'])
add_plain_test(arb_texture_cube_map, ['crash-cubemap-order'])
add_plain_test(arb_texture_cube_map, ['cubemap'])
add_concurrent_test(arb_texture_cube_map, ['cubemap-getteximage-pbo'])
add_plain_test(arb_texture_cube_map, ['cubemap-mismatch'])
arb_texture_cube_map['cubemap npot'] = PiglitGLTest(['cubemap', 'npot'])
add_plain_test(arb_texture_cube_map, ['cubemap-shader'])
arb_texture_cube_map['cubemap-shader lod'] = PiglitGLTest(['cubemap-shader', 'lod'])
arb_texture_cube_map['cubemap-shader bias'] = PiglitGLTest(['cubemap-shader', 'bias'])
add_concurrent_test(arb_texture_cube_map, ['getteximage-targets', 'CUBE'])

arb_texture_cube_map_array = spec['ARB_texture_cube_map_array']
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-get'])
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-teximage3d-invalid-values'])
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-cubemap'])
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-cubemap-lod'])
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-fbo-cubemap-array'])
add_plain_test(arb_texture_cube_map_array, ['arb_texture_cube_map_array-sampler-cube-array-shadow'])
add_concurrent_test(arb_texture_cube_map_array, ['getteximage-targets', 'CUBE_ARRAY'])
add_concurrent_test(arb_texture_cube_map_array, ['glsl-resource-not-bound', 'CubeArray'])
textureSize_samplers_atcma = ['samplerCubeArray', 'isamplerCubeArray', 'usamplerCubeArray', 'samplerCubeArrayShadow' ]
add_concurrent_test(arb_texture_cube_map_array, ['fbo-generatemipmap-cubemap', 'array'])
add_concurrent_test(arb_texture_cube_map_array, ['fbo-generatemipmap-cubemap', 'array', 'RGB9_E5'])
add_concurrent_test(arb_texture_cube_map_array, ['fbo-generatemipmap-cubemap', 'array', 'S3TC_DXT1'])
add_concurrent_test(arb_texture_cube_map_array, ['texsubimage', 'cube_map_array'])

for stage in ['vs', 'gs', 'fs']:
    # textureSize():
    for sampler in textureSize_samplers_atcma:
        spec[grouptools.join('ARB_texture_cube_map_array',
                             'textureSize',
                             '{}-textureSize-{}'.format(stage, sampler))] = \
            PiglitGLTest(['textureSize', stage, sampler], run_concurrent=True)

ext_texture_swizzle = spec['EXT_texture_swizzle']
add_concurrent_test(ext_texture_swizzle, ['ext_texture_swizzle-api'])
add_concurrent_test(ext_texture_swizzle, ['ext_texture_swizzle-swizzle'])
ext_texture_swizzle['depth_texture_mode_and_swizzle'] = PiglitGLTest(['depth_texture_mode_and_swizzle'], run_concurrent=True)

ext_texture_compression_latc = spec['EXT_texture_compression_latc']
add_fbo_generatemipmap_extension(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc', 'fbo-generatemipmap-formats')
add_fbo_generatemipmap_extension(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc-signed', 'fbo-generatemipmap-formats-signed')
add_texwrap_format_tests(ext_texture_compression_latc, 'GL_EXT_texture_compression_latc')
ext_texture_compression_latc['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'latc'], run_concurrent=True)

ext_texture_compression_rgtc = spec['EXT_texture_compression_rgtc']
add_concurrent_test(ext_texture_compression_rgtc, ['compressedteximage', 'GL_COMPRESSED_RED_RGTC1_EXT'])
add_concurrent_test(ext_texture_compression_rgtc, ['compressedteximage', 'GL_COMPRESSED_RED_GREEN_RGTC2_EXT'])
add_concurrent_test(ext_texture_compression_rgtc, ['compressedteximage', 'GL_COMPRESSED_SIGNED_RED_RGTC1_EXT'])
add_concurrent_test(ext_texture_compression_rgtc, ['compressedteximage', 'GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT'])
add_fbo_generatemipmap_extension(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc', 'fbo-generatemipmap-formats')
add_fbo_generatemipmap_extension(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc-signed', 'fbo-generatemipmap-formats-signed')
add_texwrap_format_tests(ext_texture_compression_rgtc, 'GL_EXT_texture_compression_rgtc')
ext_texture_compression_rgtc['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'rgtc'], run_concurrent=True)
add_plain_test(ext_texture_compression_rgtc, ['rgtc-teximage-01'])
add_plain_test(ext_texture_compression_rgtc, ['rgtc-teximage-02'])

ext_texture_compression_s3tc = spec['EXT_texture_compression_s3tc']
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_RGB_S3TC_DXT1_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT1_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT3_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT5_EXT'])
add_fbo_generatemipmap_extension(ext_texture_compression_s3tc, 'GL_EXT_texture_compression_s3tc', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(ext_texture_compression_s3tc, 'GL_EXT_texture_compression_s3tc')
ext_texture_compression_s3tc['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 's3tc'], run_concurrent=True)
add_plain_test(ext_texture_compression_s3tc, ['gen-compressed-teximage'])
add_concurrent_test(ext_texture_compression_s3tc, ['s3tc-errors'])
add_plain_test(ext_texture_compression_s3tc, ['s3tc-teximage'])
add_plain_test(ext_texture_compression_s3tc, ['s3tc-texsubimage'])
add_concurrent_test(ext_texture_compression_s3tc, ['getteximage-targets', 'S3TC', '2D'])
add_concurrent_test(ext_texture_compression_s3tc, ['getteximage-targets', 'S3TC', '2D_ARRAY'])
add_concurrent_test(ext_texture_compression_s3tc, ['getteximage-targets', 'S3TC', 'CUBE'])
add_concurrent_test(ext_texture_compression_s3tc, ['getteximage-targets', 'S3TC', 'CUBE_ARRAY'])

ati_texture_compression_3dc = spec['ATI_texture_compression_3dc']
add_fbo_generatemipmap_extension(ati_texture_compression_3dc, 'GL_ATI_texture_compression_3dc', 'fbo-generatemipmap-formats')
add_texwrap_format_tests(ati_texture_compression_3dc, 'GL_ATI_texture_compression_3dc')
ati_texture_compression_3dc['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', '3dc'], run_concurrent=True)

ext_packed_float = spec['EXT_packed_float']
add_fbo_formats_tests(grouptools.join('spec', 'EXT_packed_float'), 'GL_EXT_packed_float')
add_texwrap_format_tests(ext_packed_float, 'GL_EXT_packed_float')
ext_packed_float['pack'] = PiglitGLTest(['ext_packed_float-pack'], run_concurrent=True)
ext_packed_float['getteximage-invalid-format-for-packed-type'] = PiglitGLTest(['getteximage-invalid-format-for-packed-type'], run_concurrent=True)
add_msaa_formats_tests(ext_packed_float, 'GL_EXT_packed_float')

arb_texture_float = spec['ARB_texture_float']
add_fbo_formats_tests(grouptools.join('spec', 'ARB_texture_float'), 'GL_ARB_texture_float')
add_texwrap_format_tests(arb_texture_float, 'GL_ARB_texture_float')
add_plain_test(arb_texture_float, ['arb_texture_float-texture-float-formats'])
add_msaa_formats_tests(arb_texture_float, 'GL_ARB_texture_float')


oes_texture_float = spec['OES_texture_float']
add_concurrent_test(oes_texture_float, ['oes_texture_float'])
add_concurrent_test(oes_texture_float, ['oes_texture_float', 'half'])
add_concurrent_test(oes_texture_float, ['oes_texture_float', 'linear'])
add_concurrent_test(oes_texture_float, ['oes_texture_float', 'half', 'linear'])


ext_texture_integer = spec['EXT_texture_integer']
# unsupported for int yet
#add_fbo_clear_extension(ext_texture_integer, 'GL_EXT_texture_integer', 'fbo-clear-formats')
ext_texture_integer['api-drawpixels'] = PiglitGLTest(['ext_texture_integer-api-drawpixels'], run_concurrent=True)
ext_texture_integer['api-teximage'] = PiglitGLTest(['ext_texture_integer-api-teximage'], run_concurrent=True)
ext_texture_integer['api-readpixels'] = PiglitGLTest(['ext_texture_integer-api-readpixels'], run_concurrent=True)
ext_texture_integer['fbo-blending'] = PiglitGLTest(['ext_texture_integer-fbo-blending'], run_concurrent=True)
ext_texture_integer['fbo-blending GL_ARB_texture_rg'] = PiglitGLTest(['ext_texture_integer-fbo-blending', 'GL_ARB_texture_rg'], run_concurrent=True)
ext_texture_integer['fbo_integer_precision_clear'] = PiglitGLTest(['ext_texture_integer-fbo_integer_precision_clear'])
ext_texture_integer['fbo_integer_readpixels_sint_uint'] = PiglitGLTest(['ext_texture_integer-fbo_integer_readpixels_sint_uint'])
ext_texture_integer['getteximage-clamping'] = PiglitGLTest(['ext_texture_integer-getteximage-clamping'], run_concurrent=True)
ext_texture_integer['getteximage-clamping GL_ARB_texture_rg'] = PiglitGLTest(['ext_texture_integer-getteximage-clamping', 'GL_ARB_texture_rg'], run_concurrent=True)
ext_texture_integer['texture_integer_glsl130'] = PiglitGLTest(['ext_texture_integer-texture_integer_glsl130'], run_concurrent=True)
add_msaa_formats_tests(ext_texture_integer, 'GL_EXT_texture_integer')
add_texwrap_format_tests(ext_texture_integer, 'GL_EXT_texture_integer')
add_plain_test(ext_texture_integer, ['fbo-integer'])

arb_texture_rg = spec['ARB_texture_rg']
add_fbo_formats_tests(grouptools.join('spec', 'ARB_texture_rg'), 'GL_ARB_texture_rg')
add_fbo_formats_tests(grouptools.join('spec', 'ARB_texture_rg'), 'GL_ARB_texture_rg-float', '-float')
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
add_plain_test(arb_texture_rg, ['depth-tex-modes-rg'])
add_plain_test(arb_texture_rg, ['rg-draw-pixels'])
add_plain_test(arb_texture_rg, ['rg-teximage-01'])
add_plain_test(arb_texture_rg, ['rg-teximage-02'])
add_plain_test(arb_texture_rg, ['texture-rg'])

arb_texture_rgb10_a2ui = spec['ARB_texture_rgb10_a2ui']
arb_texture_rgb10_a2ui['fbo-blending'] = PiglitGLTest(['ext_texture_integer-fbo-blending', 'GL_ARB_texture_rgb10_a2ui'], run_concurrent=True)
add_texwrap_format_tests(arb_texture_rgb10_a2ui, 'GL_ARB_texture_rgb10_a2ui')

ext_texture_shared_exponent = spec['EXT_texture_shared_exponent']
ext_texture_shared_exponent['fbo-generatemipmap-formats'] = PiglitGLTest(['fbo-generatemipmap-formats', 'GL_EXT_texture_shared_exponent'], run_concurrent=True)
add_texwrap_format_tests(ext_texture_shared_exponent, 'GL_EXT_texture_shared_exponent')

ext_texture_snorm = spec['EXT_texture_snorm']
add_fbo_formats_tests(grouptools.join('spec', 'EXT_texture_snorm'), 'GL_EXT_texture_snorm')
add_msaa_formats_tests(ext_texture_snorm, 'GL_EXT_texture_snorm')
add_texwrap_format_tests(ext_texture_snorm, 'GL_EXT_texture_snorm')

ext_texture_srgb = spec['EXT_texture_sRGB']
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_SRGB_S3TC_DXT1_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT'])
add_concurrent_test(ext_texture_compression_s3tc, ['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT'])
add_fbo_generatemipmap_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-generatemipmap-formats')
# TODO: also use GL_ARB_framebuffer_sRGB:
#add_fbo_blending_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-blending-formats')
add_fbo_alphatest_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB', 'fbo-alphatest-formats')
add_fbo_generatemipmap_extension(ext_texture_srgb, 'GL_EXT_texture_sRGB-s3tc', 'fbo-generatemipmap-formats-s3tc')
ext_texture_srgb['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'srgb'], run_concurrent=True)
add_msaa_formats_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB')
add_texwrap_format_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB')
add_texwrap_format_tests(ext_texture_srgb, 'GL_EXT_texture_sRGB-s3tc', '-s3tc')
add_plain_test(ext_texture_srgb, ['fbo-srgb'])
add_plain_test(ext_texture_srgb, ['tex-srgb'])

ext_timer_query = spec['EXT_timer_query']
ext_timer_query['time-elapsed'] = PiglitGLTest(['ext_timer_query-time-elapsed'], run_concurrent=True)
add_plain_test(ext_timer_query, ['timer_query'])

arb_timer_query = spec['ARB_timer_query']
arb_timer_query['query GL_TIMESTAMP'] = PiglitGLTest(['ext_timer_query-time-elapsed', 'timestamp'], run_concurrent=True)
arb_timer_query['query-lifetime'] = PiglitGLTest(['ext_timer_query-lifetime'], run_concurrent=True)
arb_timer_query['timestamp-get'] = PiglitGLTest(['arb_timer_query-timestamp-get'], run_concurrent=True)

ext_transform_feedback = spec['EXT_transform_feedback']
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
    ext_transform_feedback[test_name] = PiglitGLTest(
        'ext_transform_feedback-{0}'.format(test_name).split(),
        run_concurrent=True)
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
    ext_transform_feedback[test_name] = PiglitGLTest(
            'ext_transform_feedback-{0}'.format(test_name).split())
ext_transform_feedback['buffer-usage'] = PiglitGLTest(['ext_transform_feedback-buffer-usage'], run_concurrent=True)
ext_transform_feedback['discard-api'] = PiglitGLTest(['ext_transform_feedback-discard-api'], run_concurrent=True)
ext_transform_feedback['discard-bitmap'] = PiglitGLTest(['ext_transform_feedback-discard-bitmap'], run_concurrent=True)
ext_transform_feedback['discard-clear'] = PiglitGLTest(['ext_transform_feedback-discard-clear'], run_concurrent=True)
ext_transform_feedback['discard-copypixels'] = PiglitGLTest(['ext_transform_feedback-discard-copypixels'], run_concurrent=True)
ext_transform_feedback['discard-drawarrays'] = PiglitGLTest(['ext_transform_feedback-discard-drawarrays'], run_concurrent=True)
ext_transform_feedback['discard-drawpixels'] = PiglitGLTest(['ext_transform_feedback-discard-drawpixels'], run_concurrent=True)
for mode in ['main_binding', 'indexed_binding', 'buffer_start', 'buffer_size']:
    test_name = 'get-buffer-state {0}'.format(mode)
    ext_transform_feedback[test_name] = PiglitGLTest(
        'ext_transform_feedback-{0}'.format(test_name).split(), run_concurrent=True)
ext_transform_feedback['immediate-reuse'] = PiglitGLTest(['ext_transform_feedback-immediate-reuse'], run_concurrent=True)
ext_transform_feedback['immediate-reuse-index-buffer'] = PiglitGLTest(['ext_transform_feedback-immediate-reuse-index-buffer'], run_concurrent=True)
ext_transform_feedback['immediate-reuse-uniform-buffer'] = PiglitGLTest(['ext_transform_feedback-immediate-reuse-uniform-buffer'], run_concurrent=True)
for mode in ['output', 'prims_generated', 'prims_written']:
    for use_gs in ['', ' use_gs']:
        test_name = 'intervening-read {0}{1}'.format(mode, use_gs)
        ext_transform_feedback[test_name] = PiglitGLTest(
            'ext_transform_feedback-{0}'.format(test_name).split(),
            run_concurrent=True)
ext_transform_feedback['max-varyings'] = PiglitGLTest(['ext_transform_feedback-max-varyings'], run_concurrent=True)
ext_transform_feedback['nonflat-integral'] = PiglitGLTest(['ext_transform_feedback-nonflat-integral'], run_concurrent=True)
ext_transform_feedback['overflow-edge-cases'] = PiglitGLTest(['ext_transform_feedback-overflow-edge-cases'], run_concurrent=True)
ext_transform_feedback['overflow-edge-cases use_gs'] = PiglitGLTest(['ext_transform_feedback-overflow-edge-cases', 'use_gs'], run_concurrent=True)
ext_transform_feedback['points'] = PiglitGLTest(['ext_transform_feedback-points'], run_concurrent=True)
ext_transform_feedback['points-large'] = PiglitGLTest(['ext_transform_feedback-points', 'large'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferbase'] = PiglitGLTest(['ext_transform_feedback-position'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferbase-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'discard'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferoffset'] = PiglitGLTest(['ext_transform_feedback-position', 'offset'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferoffset-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'offset', 'discard'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferrange'] = PiglitGLTest(['ext_transform_feedback-position', 'range'], run_concurrent=True)
ext_transform_feedback['position-readback-bufferrange-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'range', 'discard'], run_concurrent=True)

ext_transform_feedback['negative-prims'] = PiglitGLTest(['ext_transform_feedback-negative-prims'], run_concurrent=True)
ext_transform_feedback['primgen-query transform-feedback-disabled'] = PiglitGLTest(['ext_transform_feedback-primgen'], run_concurrent=True)
ext_transform_feedback['pipeline-basic-primgen'] = PiglitGLTest(['ext_transform_feedback-pipeline-basic-primgen'], run_concurrent=True)

ext_transform_feedback['position-render-bufferbase'] = PiglitGLTest(['ext_transform_feedback-position', 'render'], run_concurrent=True)
ext_transform_feedback['position-render-bufferbase-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'render', 'discard'], run_concurrent=True)
ext_transform_feedback['position-render-bufferoffset'] = PiglitGLTest(['ext_transform_feedback-position', 'render', 'offset'], run_concurrent=True)
ext_transform_feedback['position-render-bufferoffset-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'render', 'offset', 'discard'], run_concurrent=True)
ext_transform_feedback['position-render-bufferrange'] = PiglitGLTest(['ext_transform_feedback-position', 'render', 'range'], run_concurrent=True)
ext_transform_feedback['position-render-bufferrange-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'render', 'range', 'discard'], run_concurrent=True)

ext_transform_feedback['query-primitives_generated-bufferbase'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen'], run_concurrent=True)
ext_transform_feedback['query-primitives_generated-bufferbase-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen', 'discard'], run_concurrent=True)
ext_transform_feedback['query-primitives_generated-bufferoffset'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen', 'offset'], run_concurrent=True)
ext_transform_feedback['query-primitives_generated-bufferoffset-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen', 'offset', 'discard'], run_concurrent=True)
ext_transform_feedback['query-primitives_generated-bufferrange'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen', 'range'], run_concurrent=True)
ext_transform_feedback['query-primitives_generated-bufferrange-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primgen', 'range', 'discard'], run_concurrent=True)

ext_transform_feedback['query-primitives_written-bufferbase'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten'], run_concurrent=True)
ext_transform_feedback['query-primitives_written-bufferbase-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten', 'discard'], run_concurrent=True)
ext_transform_feedback['query-primitives_written-bufferoffset'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten', 'offset'], run_concurrent=True)
ext_transform_feedback['query-primitives_written-bufferoffset-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten', 'offset', 'discard'], run_concurrent=True)
ext_transform_feedback['query-primitives_written-bufferrange'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten', 'range'], run_concurrent=True)
ext_transform_feedback['query-primitives_written-bufferrange-discard'] = PiglitGLTest(['ext_transform_feedback-position', 'primwritten', 'range', 'discard'], run_concurrent=True)

ext_transform_feedback['interleaved-attribs'] = PiglitGLTest(['ext_transform_feedback-interleaved'], run_concurrent=True)
ext_transform_feedback['separate-attribs'] = PiglitGLTest(['ext_transform_feedback-separate'], run_concurrent=True)
for drawcall in ['arrays', 'elements']:
    for mode in ['triangles', 'lines', 'points']:
        test_name = 'order {0} {1}'.format(drawcall, mode)
        ext_transform_feedback[test_name] = PiglitGLTest(
            'ext_transform_feedback-{0}'.format(test_name).split(),
            run_concurrent=True)
for draw_mode in ['points', 'lines', 'line_loop', 'line_strip',
                  'triangles', 'triangle_strip', 'triangle_fan',
                  'quads', 'quad_strip', 'polygon']:
    for shade_mode in ['monochrome', 'smooth', 'flat_first', 'flat_last', 'wireframe']:
        if shade_mode == 'wireframe' and \
                    draw_mode in ['points', 'lines', 'line_loop', 'line_strip']:
            continue
        test_name = 'tessellation {0} {1}'.format(
                draw_mode, shade_mode)
        ext_transform_feedback[test_name] = PiglitGLTest(
            'ext_transform_feedback-{0}'.format(test_name).split(),
            run_concurrent=True)
for alignment in [0, 4, 8, 12]:
    test_name = 'alignment {0}'.format(alignment)
    ext_transform_feedback[test_name] = PiglitGLTest(
        'ext_transform_feedback-{0}'.format(test_name).split(), run_concurrent=True)

for output_type in ['float', 'vec2', 'vec3', 'vec4', 'mat2', 'mat2x3',
                    'mat2x4', 'mat3x2', 'mat3', 'mat3x4', 'mat4x2', 'mat4x3',
                    'mat4', 'int', 'ivec2', 'ivec3', 'ivec4', 'uint', 'uvec2',
                    'uvec3', 'uvec4']:
    for suffix in ['', '[2]', '[2]-no-subscript']:
        test_name = 'output-type {0}{1}'.format(output_type, suffix)
        ext_transform_feedback[test_name] = PiglitGLTest(
            'ext_transform_feedback-{0}'.format(test_name).split(),
            run_concurrent=True)

for mode in ['discard', 'buffer', 'prims_generated', 'prims_written']:
    test_name = 'generatemipmap {0}'.format(mode)
    ext_transform_feedback[test_name] = PiglitGLTest(
        'ext_transform_feedback-{0}'.format(test_name).split(), run_concurrent=True)

for test_case in ['base-shrink', 'base-grow', 'offset-shrink', 'offset-grow',
                  'range-shrink', 'range-grow']:
    test_name = 'change-size {0}'.format(test_case)
    ext_transform_feedback[test_name] = PiglitGLTest(
        'ext_transform_feedback-{0}'.format(test_name).split(), run_concurrent=True)
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
                ext_transform_feedback[test_name] = PiglitGLTest(
                    'ext_transform_feedback-{0}'.format(test_name).split(),
                    run_concurrent=True)
ext_transform_feedback['geometry-shaders-basic'] = PiglitGLTest(
    ['ext_transform_feedback-geometry-shaders-basic'], run_concurrent=True)

arb_transform_feedback2 = spec['ARB_transform_feedback2']
arb_transform_feedback2['Change objects while paused'] = PiglitGLTest(['arb_transform_feedback2-change-objects-while-paused'])
arb_transform_feedback2['Change objects while paused (GLES3)'] = PiglitGLTest(['arb_transform_feedback2-change-objects-while-paused_gles3'])
arb_transform_feedback2['draw-auto'] = PiglitGLTest(['arb_transform_feedback2-draw-auto'])
arb_transform_feedback2['istranformfeedback'] = PiglitGLTest(['arb_transform_feedback2-istransformfeedback'])
arb_transform_feedback2['glGenTransformFeedbacks names only'] = PiglitGLTest(['arb_transform_feedback2-gen-names-only'], run_concurrent=True)
arb_transform_feedback2['cannot bind when another object is active'] = PiglitGLTest(['arb_transform_feedback2-cannot-bind-when-active'], run_concurrent=True)
arb_transform_feedback2['misc. API queries'] = PiglitGLTest(['arb_transform_feedback2-api-queries'], run_concurrent=True)
arb_transform_feedback2['counting with pause'] = PiglitGLTest(['arb_transform_feedback2-pause-counting'], run_concurrent=True)

arb_transform_feedback_instanced = spec['ARB_transform_feedback_instanced']
arb_transform_feedback_instanced['draw-auto instanced'] = PiglitGLTest(['arb_transform_feedback2-draw-auto', 'instanced'])

arb_transform_feedback3 = spec['ARB_transform_feedback3']

for param in ['gl_NextBuffer-1', 'gl_NextBuffer-2', 'gl_SkipComponents1-1',
              'gl_SkipComponents1-2', 'gl_SkipComponents1-3', 'gl_SkipComponents2',
              'gl_SkipComponents3', 'gl_SkipComponents4',
              'gl_NextBuffer-gl_SkipComponents1-gl_NextBuffer',
              'gl_NextBuffer-gl_NextBuffer', 'gl_SkipComponents1234']:
    arb_transform_feedback3[param] = PiglitGLTest(
        ['ext_transform_feedback-output-type', param],
        run_concurrent=True)

arb_transform_feedback3['arb_transform_feedback3-bind_buffer_invalid_index'] = PiglitGLTest(['arb_transform_feedback3-bind_buffer_invalid_index'])
arb_transform_feedback3['arb_transform_feedback3-query_with_invalid_index'] = PiglitGLTest(['arb_transform_feedback3-query_with_invalid_index'])
arb_transform_feedback3['arb_transform_feedback3-end_query_with_name_zero'] = PiglitGLTest(['arb_transform_feedback3-end_query_with_name_zero'])
arb_transform_feedback3['arb_transform_feedback3-draw_using_invalid_stream_index'] = PiglitGLTest(['arb_transform_feedback3-draw_using_invalid_stream_index'])
arb_transform_feedback3['arb_transform_feedback3-set_varyings_with_invalid_args'] = PiglitGLTest(['arb_transform_feedback3-set_varyings_with_invalid_args'])
arb_transform_feedback3['arb_transform_feedback3-set_invalid_varyings'] = PiglitGLTest(['arb_transform_feedback3-set_invalid_varyings'])

arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_vs'] = PiglitGLTest(['arb_transform_feedback3-ext_interleaved_two_bufs', 'vs'])
arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_gs'] = PiglitGLTest(['arb_transform_feedback3-ext_interleaved_two_bufs', 'gs'])
arb_transform_feedback3['arb_transform_feedback3-ext_interleaved_two_bufs_gs_max'] = PiglitGLTest(['arb_transform_feedback3-ext_interleaved_two_bufs', 'gs_max'])

arb_uniform_buffer_object = spec['ARB_uniform_buffer_object']
arb_uniform_buffer_object['bindbuffer-general-point'] = PiglitGLTest(['arb_uniform_buffer_object-bindbuffer-general-point'], run_concurrent=True)
arb_uniform_buffer_object['buffer-targets'] = PiglitGLTest(['arb_uniform_buffer_object-buffer-targets'], run_concurrent=True)
arb_uniform_buffer_object['bufferstorage'] = PiglitGLTest(['arb_uniform_buffer_object-bufferstorage'], run_concurrent=True)
arb_uniform_buffer_object['deletebuffers'] = PiglitGLTest(['arb_uniform_buffer_object-deletebuffers'], run_concurrent=True)
arb_uniform_buffer_object['dlist'] = PiglitGLTest(['arb_uniform_buffer_object-dlist'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformblockiv-uniform-block-data-size'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformblockiv-uniform-block-data-size'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformblockname'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformblockname'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformname'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformname'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformsiv-uniform-array-stride'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-array-stride'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformsiv-uniform-block-index'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-block-index'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformsiv-uniform-matrix-stride'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-matrix-stride'], run_concurrent=True)
arb_uniform_buffer_object['getactiveuniformsiv-uniform-type'] = PiglitGLTest(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-type'], run_concurrent=True)
arb_uniform_buffer_object['getintegeri_v'] = PiglitGLTest(['arb_uniform_buffer_object-getintegeri_v'], run_concurrent=True)
arb_uniform_buffer_object['getprogramiv'] = PiglitGLTest(['arb_uniform_buffer_object-getprogramiv'], run_concurrent=True)
arb_uniform_buffer_object['getuniformblockindex'] = PiglitGLTest(['arb_uniform_buffer_object-getuniformblockindex'], run_concurrent=True)
arb_uniform_buffer_object['getuniformindices'] = PiglitGLTest(['arb_uniform_buffer_object-getuniformindices'], run_concurrent=True)
arb_uniform_buffer_object['getuniformlocation'] = PiglitGLTest(['arb_uniform_buffer_object-getuniformlocation'], run_concurrent=True)
arb_uniform_buffer_object['layout-std140'] = PiglitGLTest(['arb_uniform_buffer_object-layout-std140'], run_concurrent=True)
arb_uniform_buffer_object['layout-std140-base-size-and-alignment'] = PiglitGLTest(['arb_uniform_buffer_object-layout-std140-base-size-and-alignment'], run_concurrent=True)
arb_uniform_buffer_object['link-mismatch-blocks'] = PiglitGLTest(['arb_uniform_buffer_object-link-mismatch-blocks'], run_concurrent=True)
arb_uniform_buffer_object['maxblocks'] = PiglitGLTest(['arb_uniform_buffer_object-maxblocks'], run_concurrent=True)
arb_uniform_buffer_object[grouptools.join('maxuniformblocksize', 'vs')] = PiglitGLTest(['arb_uniform_buffer_object-maxuniformblocksize', 'vs'], run_concurrent=True)
arb_uniform_buffer_object[grouptools.join('maxuniformblocksize', 'vsexceed')] = PiglitGLTest(['arb_uniform_buffer_object-maxuniformblocksize', 'vsexceed'], run_concurrent=True)
arb_uniform_buffer_object[grouptools.join('maxuniformblocksize', 'fs')] = PiglitGLTest(['arb_uniform_buffer_object-maxuniformblocksize', 'fs'], run_concurrent=True)
arb_uniform_buffer_object[grouptools.join('maxuniformblocksize', 'fsexceed')] = PiglitGLTest(['arb_uniform_buffer_object-maxuniformblocksize', 'fsexceed'], run_concurrent=True)
arb_uniform_buffer_object['minmax'] = PiglitGLTest(['arb_uniform_buffer_object-minmax'], run_concurrent=True)
arb_uniform_buffer_object['negative-bindbuffer-index'] = PiglitGLTest(['arb_uniform_buffer_object-negative-bindbuffer-index'], run_concurrent=True)
arb_uniform_buffer_object['negative-bindbuffer-target'] = PiglitGLTest(['arb_uniform_buffer_object-negative-bindbuffer-target'], run_concurrent=True)
arb_uniform_buffer_object['negative-bindbufferrange-range'] = PiglitGLTest(['arb_uniform_buffer_object-negative-bindbufferrange-range'], run_concurrent=True)
arb_uniform_buffer_object['negative-getactiveuniformblockiv'] = PiglitGLTest(['arb_uniform_buffer_object-negative-getactiveuniformblockiv'], run_concurrent=True)
arb_uniform_buffer_object['negative-getactiveuniformsiv'] = PiglitGLTest(['arb_uniform_buffer_object-negative-getactiveuniformsiv'], run_concurrent=True)
arb_uniform_buffer_object['referenced-by-shader'] = PiglitGLTest(['arb_uniform_buffer_object-referenced-by-shader'], run_concurrent=True)
arb_uniform_buffer_object['rendering'] = PiglitGLTest(['arb_uniform_buffer_object-rendering'], run_concurrent=True)
arb_uniform_buffer_object['rendering-offset'] = PiglitGLTest(['arb_uniform_buffer_object-rendering', 'offset'], run_concurrent=True)
arb_uniform_buffer_object['row-major'] = PiglitGLTest(['arb_uniform_buffer_object-row-major'], run_concurrent=True)
arb_uniform_buffer_object['uniformblockbinding'] = PiglitGLTest(['arb_uniform_buffer_object-uniformblockbinding'], run_concurrent=True)

ati_draw_buffers = spec['ATI_draw_buffers']
add_plain_test(ati_draw_buffers, ['ati_draw_buffers-arbfp'])
ati_draw_buffers['arbfp-no-index'] = PiglitGLTest(['ati_draw_buffers-arbfp-no-index'])
ati_draw_buffers['arbfp-no-option'] = PiglitGLTest(['ati_draw_buffers-arbfp-no-option'])

ati_envmap_bumpmap = spec['ATI_envmap_bumpmap']
add_plain_test(ati_envmap_bumpmap, ['ati_envmap_bumpmap-bump'])

arb_instanced_arrays = spec['ARB_instanced_arrays']
add_plain_test(arb_instanced_arrays, ['arb_instanced_arrays-vertex-attrib-divisor-index-error'])
add_plain_test(arb_instanced_arrays, ['arb_instanced_arrays-instanced_arrays'])
add_plain_test(arb_instanced_arrays, ['arb_instanced_arrays-drawarrays'])
add_single_param_test_set(arb_instanced_arrays, 'arb_instanced_arrays-instanced_arrays', 'vbo')

arb_internalformat_query = spec['ARB_internalformat_query']
arb_internalformat_query['misc. API error checks'] = PiglitGLTest(['arb_internalformat_query-api-errors'], run_concurrent=True)
arb_internalformat_query['buffer over-run checks'] = PiglitGLTest(['arb_internalformat_query-overrun'], run_concurrent=True)
arb_internalformat_query['minmax'] = PiglitGLTest(['arb_internalformat_query-minmax'], run_concurrent=True)

arb_map_buffer_range = spec['ARB_map_buffer_range']
add_plain_test(arb_map_buffer_range, ['map_buffer_range_error_check'])
add_plain_test(arb_map_buffer_range, ['map_buffer_range_test'])
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT offset=0'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT', 'offset=0'], run_concurrent=True)
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT increment-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT', 'increment-offset'], run_concurrent=True)
arb_map_buffer_range['MAP_INVALIDATE_RANGE_BIT decrement-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT', 'decrement-offset'], run_concurrent=True)
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT offset=0'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT', 'offset=0'], run_concurrent=True)
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT increment-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT', 'increment-offset'], run_concurrent=True)
arb_map_buffer_range['MAP_INVALIDATE_BUFFER_BIT decrement-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT', 'decrement-offset'], run_concurrent=True)
arb_map_buffer_range['CopyBufferSubData offset=0'] = PiglitGLTest(['map_buffer_range-invalidate', 'CopyBufferSubData', 'offset=0'], run_concurrent=True)
arb_map_buffer_range['CopyBufferSubData increment-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'CopyBufferSubData', 'increment-offset'], run_concurrent=True)
arb_map_buffer_range['CopyBufferSubData decrement-offset'] = PiglitGLTest(['map_buffer_range-invalidate', 'CopyBufferSubData', 'decrement-offset'], run_concurrent=True)

arb_multisample = spec['ARB_multisample']
arb_multisample['beginend'] = PiglitGLTest(['arb_multisample-beginend'], run_concurrent=True)
arb_multisample['pushpop'] = PiglitGLTest(['arb_multisample-pushpop'], run_concurrent=True)

arb_seamless_cube_map = spec['ARB_seamless_cube_map']
add_plain_test(arb_seamless_cube_map, ['arb_seamless_cubemap'])
add_plain_test(arb_seamless_cube_map, ['arb_seamless_cubemap-initially-disabled'])
add_plain_test(arb_seamless_cube_map, ['arb_seamless_cubemap-three-faces-average'])

amd_pinned_memory = spec['AMD_pinned_memory']
amd_pinned_memory['offset=0'] = PiglitGLTest(['amd_pinned_memory', 'offset=0'], run_concurrent=True)
amd_pinned_memory['increment-offset'] = PiglitGLTest(['amd_pinned_memory', 'increment-offset'], run_concurrent=True)
amd_pinned_memory['decrement-offset'] = PiglitGLTest(['amd_pinned_memory', 'decrement-offset'], run_concurrent=True)
amd_pinned_memory['map-buffer offset=0'] = PiglitGLTest(['amd_pinned_memory', 'offset=0', 'map-buffer'], run_concurrent=True)
amd_pinned_memory['map-buffer increment-offset'] = PiglitGLTest(['amd_pinned_memory', 'increment-offset', 'map-buffer'], run_concurrent=True)
amd_pinned_memory['map-buffer decrement-offset'] = PiglitGLTest(['amd_pinned_memory', 'decrement-offset', 'map-buffer'], run_concurrent=True)

amd_seamless_cubemap_per_texture = spec['AMD_seamless_cubemap_per_texture']
add_plain_test(amd_seamless_cubemap_per_texture, ['amd_seamless_cubemap_per_texture'])

amd_vertex_shader_layer = spec['AMD_vertex_shader_layer']
add_plain_test(amd_vertex_shader_layer, ['amd_vertex_shader_layer-layered-2d-texture-render'])
add_plain_test(amd_vertex_shader_layer, ['amd_vertex_shader_layer-layered-depth-texture-render'])

amd_vertex_shader_viewport_index = spec['AMD_vertex_shader_viewport_index']
add_concurrent_test(amd_vertex_shader_viewport_index, ['amd_vertex_shader_viewport_index-render'])

ext_fog_coord = spec['EXT_fog_coord']
add_plain_test(ext_fog_coord, ['ext_fog_coord-modes'])

ext_shader_integer_mix = spec['EXT_shader_integer_mix']

nv_texture_barrier = spec['NV_texture_barrier']
add_plain_test(nv_texture_barrier, ['blending-in-shader'])

nv_conditional_render = spec['NV_conditional_render']
nv_conditional_render['begin-while-active'] = PiglitGLTest(['nv_conditional_render-begin-while-active'], run_concurrent=True)
nv_conditional_render['begin-zero'] = PiglitGLTest(['nv_conditional_render-begin-zero'], run_concurrent=True)
nv_conditional_render['bitmap'] = PiglitGLTest(['nv_conditional_render-bitmap'])
nv_conditional_render['blitframebuffer'] = PiglitGLTest(['nv_conditional_render-blitframebuffer'])
nv_conditional_render['clear'] = PiglitGLTest(['nv_conditional_render-clear'])
nv_conditional_render['copypixels'] = PiglitGLTest(['nv_conditional_render-copypixels'])
nv_conditional_render['copyteximage'] = PiglitGLTest(['nv_conditional_render-copyteximage'])
nv_conditional_render['copytexsubimage'] = PiglitGLTest(['nv_conditional_render-copytexsubimage'])
nv_conditional_render['dlist'] = PiglitGLTest(['nv_conditional_render-dlist'])
nv_conditional_render['drawpixels'] = PiglitGLTest(['nv_conditional_render-drawpixels'])
nv_conditional_render['generatemipmap'] = PiglitGLTest(['nv_conditional_render-generatemipmap'])
nv_conditional_render['vertex_array'] = PiglitGLTest(['nv_conditional_render-vertex_array'])

oes_matrix_get = spec['OES_matrix_get']
oes_matrix_get['All queries'] = PiglitGLTest(['oes_matrix_get-api'], run_concurrent=True)

oes_fixed_point = spec['OES_fixed_point']
oes_fixed_point['attribute-arrays'] = PiglitGLTest(['oes_fixed_point-attribute-arrays'], run_concurrent=True)

arb_clear_buffer_object = spec['ARB_clear_buffer_object']
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-formats'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-invalid-internal-format'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-invalid-size'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-mapped'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-no-bound-buffer'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-null-data'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-sub-invalid-size'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-sub-mapped'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-sub-overlap'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-sub-simple'])
add_concurrent_test(arb_clear_buffer_object, ['arb_clear_buffer_object-zero-size'])

arb_clear_texture = spec['ARB_clear_texture']
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-simple'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-error'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-3d'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-cube'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-multisample'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-integer'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-base-formats'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-sized-formats'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-float'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-rg'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-depth-stencil'])
add_concurrent_test(arb_clear_texture, ['arb_clear_texture-srgb'])

arb_copy_buffer = spec['ARB_copy_buffer']
add_plain_test(arb_copy_buffer, ['copy_buffer_coherency'])
add_plain_test(arb_copy_buffer, ['copybuffersubdata'])
arb_copy_buffer['data-sync'] = PiglitGLTest(['arb_copy_buffer-data-sync'], run_concurrent=True)
arb_copy_buffer['dlist'] = PiglitGLTest(['arb_copy_buffer-dlist'], run_concurrent=True)
arb_copy_buffer['get'] = PiglitGLTest(['arb_copy_buffer-get'], run_concurrent=True)
arb_copy_buffer['negative-bound-zero'] = PiglitGLTest(['arb_copy_buffer-negative-bound-zero'], run_concurrent=True)
arb_copy_buffer['negative-bounds'] = PiglitGLTest(['arb_copy_buffer-negative-bounds'], run_concurrent=True)
arb_copy_buffer['negative-mapped'] = PiglitGLTest(['arb_copy_buffer-negative-mapped'], run_concurrent=True)
arb_copy_buffer['overlap'] = PiglitGLTest(['arb_copy_buffer-overlap'], run_concurrent=True)
arb_copy_buffer['targets'] = PiglitGLTest(['arb_copy_buffer-targets'], run_concurrent=True)
arb_copy_buffer['subdata-sync'] = PiglitGLTest(['arb_copy_buffer-subdata-sync'], run_concurrent=True)

arb_copy_image = spec['ARB_copy_image']
add_concurrent_test(arb_copy_image, ['arb_copy_image-simple', '--tex-to-tex'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-simple', '--rb-to-tex'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-simple', '--rb-to-rb'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-srgb-copy'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_1D', '32', '1', '1', '11', '0', '0', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', '11', '0', '0', '5', '0', '9', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_2D', '32', '32', '1', '11', '0', '0', '5', '13', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', '11', '0', '0', '5', '13', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_2D_ARRAY', '32', '32', '10', '11', '0', '0', '5', '13', '4', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '0', '0', '5', '13', '4', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '30', '11', '0', '0', '5', '13', '8', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1', 'GL_TEXTURE_3D', '32', '32', '32', '11', '0', '0', '5', '13', '4', '14', '1', '1'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_1D', '32', '1', '1', '11', '0', '7', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '0', '3', '5', '0', '7', '14', '1', '8'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_2D', '32', '16', '1', '11', '0', '3', '5', '7', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '0', '3', '5', '7', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_2D_ARRAY', '32', '16', '18', '11', '0', '3', '5', '9', '7', '14', '1', '8'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '0', '3', '5', '17', '2', '14', '1', '3'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '0', '3', '5', '17', '2', '14', '1', '7'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12', 'GL_TEXTURE_3D', '32', '16', '18', '11', '0', '3', '5', '9', '2', '14', '1', '7'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '0', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '0', '5', '0', '7', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14', '9', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14', '9', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '0', '5', '7', '12', '14', '8', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '0', '5', '9', '2', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '0', '5', '9', '7', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14', '7', '1'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '0', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '0', '5', '0', '7', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14', '9', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '0', '5', '7', '12', '14', '8', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '0', '5', '9', '2', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '0', '5', '9', '7', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14', '7', '1'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7', '14', '1', '7'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '4', '0', '14', '10', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7', '0', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '7', '2', '14', '9', '9'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '1', '5', '9', '2', '14', '7', '3'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '2', '5', '9', '7', '14', '7', '11'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14', '7', '11'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '3', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '3', '5', '0', '7', '14', '1', '2'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '3', '5', '7', '0', '14', '9', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '3', '5', '3', '0', '14', '12', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '1', '5', '3', '2', '14', '11', '4'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '1', '5', '9', '2', '14', '7', '3'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '1', '5', '9', '9', '14', '7', '5'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14', '7', '4'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7', '14', '1', '7'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '7', '0', '14', '8', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7', '0', '14', '6', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '1', '2', '14', '15', '9'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_CUBE_MAP', '16', '16', '6', '11', '5', '1', '5', '9', '2', '5', '7', '3'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_CUBE_MAP_ARRAY', '16', '16', '18', '11', '5', '2', '5', '9', '7', '5', '7', '11'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14', '7', '11'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14', '1', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7', '14', '1', '7'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '7', '0', '14', '7', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7', '0', '14', '9', '1'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '3', '2', '14', '13', '9'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_CUBE_MAP', '16', '16', '6', '11', '5', '1', '5', '9', '2', '5', '7', '3'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_CUBE_MAP_ARRAY', '16', '16', '18', '11', '5', '2', '5', '9', '7', '5', '7', '11'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17', 'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14', '7', '11'])

add_concurrent_test(arb_copy_image, ['arb_copy_image-formats'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-formats', '--samples=2'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-formats', '--samples=4'])
add_concurrent_test(arb_copy_image, ['arb_copy_image-formats', '--samples=8'])

arb_cull_distance = spec['arb_cull_distance']
add_concurrent_test(arb_cull_distance, ['arb_cull_distance-max-distances'])

arb_half_float_vertex = spec['ARB_half_float_vertex']
add_plain_test(arb_half_float_vertex, ['draw-vertices-half-float'])
arb_half_float_vertex['draw-vertices-half-float-user'] = PiglitGLTest(['draw-vertices-half-float', 'user'])

arb_vertex_type_2_10_10_10_rev = spec['ARB_vertex_type_2_10_10_10_rev']
add_plain_test(arb_vertex_type_2_10_10_10_rev, ['draw-vertices-2101010'])
arb_vertex_type_2_10_10_10_rev['attribs'] = PiglitGLTest(['attribs', 'GL_ARB_vertex_type_2_10_10_10_rev'], run_concurrent=True)
add_concurrent_test(arb_vertex_type_2_10_10_10_rev, ['arb_vertex_type_2_10_10_10_rev-array_types'])

arb_vertex_type_10f_11f_11f_rev = spec['ARB_vertex_type_10f_11f_11f_rev']
add_plain_test(arb_vertex_type_10f_11f_11f_rev, ['arb_vertex_type_10f_11f_11f_rev-api-errors'])
add_concurrent_test(arb_vertex_type_10f_11f_11f_rev, ['arb_vertex_type_10f_11f_11f_rev-draw-vertices'])

arb_draw_buffers = spec['ARB_draw_buffers']
add_plain_test(arb_draw_buffers, ['arb_draw_buffers-state_change'])
add_plain_test(arb_draw_buffers, ['fbo-mrt-alphatest'])

ext_draw_buffers2 = spec['EXT_draw_buffers2']
add_plain_test(ext_draw_buffers2, ['fbo-drawbuffers2-blend'])
add_plain_test(ext_draw_buffers2, ['fbo-drawbuffers2-colormask'])
add_plain_test(ext_draw_buffers2, ['fbo-drawbuffers2-colormask', 'clear'])

arb_draw_buffers_blend = spec['ARB_draw_buffers_blend']
add_plain_test(arb_draw_buffers_blend, ['fbo-draw-buffers-blend'])

arb_blend_func_extended = spec['ARB_blend_func_extended']
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-bindfragdataindexed-invalid-parameters'])
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-blend-api'])
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-error-at-begin'])
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-getfragdataindex'])
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-fbo-extended-blend'])
add_plain_test(arb_blend_func_extended, ['arb_blend_func_extended-fbo-extended-blend-explicit'])

arb_base_instance = spec['ARB_base_instance']
add_plain_test(arb_base_instance, ['arb_base_instance-baseinstance-doesnt-affect-gl-instance-id'])
add_concurrent_test(arb_base_instance, ['arb_base_instance-drawarrays'])

arb_buffer_storage = spec['ARB_buffer_storage']
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'draw'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'draw', 'coherent'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'draw', 'client-storage'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'draw', 'coherent', 'client-storage'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'read'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'read', 'coherent'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'read', 'client-storage'])
add_concurrent_test(arb_buffer_storage, ['bufferstorage-persistent', 'read', 'coherent', 'client-storage'])

apple_object_purgeable = spec['APPLE_object_purgeable']
add_plain_test(apple_object_purgeable, ['object_purgeable-api-pbo'])
add_plain_test(apple_object_purgeable, ['object_purgeable-api-texture'])
add_plain_test(apple_object_purgeable, ['object_purgeable-api-vbo'])

oes_read_format = spec['OES_read_format']
add_plain_test(oes_read_format, ['oes-read-format'])

nv_primitive_restart = spec['NV_primitive_restart']
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

ext_provoking_vertex = spec['EXT_provoking_vertex']
add_plain_test(ext_provoking_vertex, ['provoking-vertex'])

ext_texture_lod_bias = spec['EXT_texture_lod_bias']
add_plain_test(ext_texture_lod_bias, ['lodbias'])

sgis_generate_mipmap = spec['SGIS_generate_mipmap']
add_plain_test(sgis_generate_mipmap, ['gen-nonzero-unit'])
add_plain_test(sgis_generate_mipmap, ['gen-teximage'])
add_plain_test(sgis_generate_mipmap, ['gen-texsubimage'])

arb_map_buffer_alignment = spec['ARB_map_buffer_alignment']
add_plain_test(arb_map_buffer_alignment, ['arb_map_buffer_alignment-sanity_test'])
add_concurrent_test(arb_map_buffer_alignment, ['arb_map_buffer_alignment-map-invalidate-range'])

arb_geometry_shader4 = spec['ARB_geometry_shader4']
for draw in ['', 'indexed']:
    for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY', 'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-ignore-adjacent-vertices', draw, prim])
add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-program-parameter-input-type'])
add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-program-parameter-input-type-draw'])
add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-program-parameter-output-type'])
add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-vertices-in'])
for mode in ['1', 'tf 1', 'max', 'tf max']:
    add_concurrent_test(arb_geometry_shader4, ['arb_geometry_shader4-program-parameter-vertices-out', mode])

arb_compute_shader = spec['ARB_compute_shader']
arb_compute_shader['api_errors'] = PiglitGLTest(['arb_compute_shader-api_errors'], run_concurrent=True)
arb_compute_shader['minmax'] = PiglitGLTest(['arb_compute_shader-minmax'], run_concurrent=True)
arb_compute_shader[grouptools.join('compiler', 'work_group_size_too_large')] = \
    PiglitGLTest(['arb_compute_shader-work_group_size_too_large'], run_concurrent=True)
arb_compute_shader['built-in constants'] = PiglitGLTest(
    ['built-in-constants',
     os.path.join(TESTS_DIR, 'spec', 'arb_compute_shader', 'minimum-maximums.txt')],
    run_concurrent=True)

ext_polygon_offset_clamp = spec['EXT_polygon_offset_clamp']
add_concurrent_test(ext_polygon_offset_clamp, ['ext_polygon_offset_clamp-draw'])
add_concurrent_test(ext_polygon_offset_clamp, ['ext_polygon_offset_clamp-dlist'])

arb_pipeline_statistics_query = spec['ARB_pipeline_statistics_query']
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-vert'])
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-vert_adj'])
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-clip'])
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-geom'])
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-frag'])
add_concurrent_test(arb_pipeline_statistics_query, ['arb_pipeline_statistics_query-comp'])

hiz = profile.tests['hiz']
add_plain_test(hiz, ['hiz-depth-stencil-test-fbo-d0-s8'])
add_plain_test(hiz, ['hiz-depth-stencil-test-fbo-d24-s0'])
add_plain_test(hiz, ['hiz-depth-stencil-test-fbo-d24-s8'])
add_plain_test(hiz, ['hiz-depth-stencil-test-fbo-d24s8'])
add_plain_test(hiz, ['hiz-depth-read-fbo-d24-s0'])
add_plain_test(hiz, ['hiz-depth-read-fbo-d24-s8'])
add_plain_test(hiz, ['hiz-depth-read-fbo-d24s8'])
add_plain_test(hiz, ['hiz-depth-read-window-stencil0'])
add_plain_test(hiz, ['hiz-depth-read-window-stencil1'])
add_plain_test(hiz, ['hiz-depth-test-fbo-d24-s0'])
add_plain_test(hiz, ['hiz-depth-test-fbo-d24-s8'])
add_plain_test(hiz, ['hiz-depth-test-fbo-d24s8'])
add_plain_test(hiz, ['hiz-depth-test-window-stencil0'])
add_plain_test(hiz, ['hiz-depth-test-window-stencil1'])
add_plain_test(hiz, ['hiz-stencil-read-fbo-d0-s8'])
add_plain_test(hiz, ['hiz-stencil-read-fbo-d24-s8'])
add_plain_test(hiz, ['hiz-stencil-read-fbo-d24s8'])
add_plain_test(hiz, ['hiz-stencil-read-window-depth0'])
add_plain_test(hiz, ['hiz-stencil-read-window-depth1'])
add_plain_test(hiz, ['hiz-stencil-test-fbo-d0-s8'])
add_plain_test(hiz, ['hiz-stencil-test-fbo-d24-s8'])
add_plain_test(hiz, ['hiz-stencil-test-fbo-d24s8'])
add_plain_test(hiz, ['hiz-stencil-test-window-depth0'])
add_plain_test(hiz, ['hiz-stencil-test-window-depth1'])

fast_color_clear = profile.tests['fast_color_clear']
for subtest in ('sample', 'read_pixels', 'blit', 'copy'):
    for buffer_type in ('rb', 'tex'):
        if subtest == 'sample' and buffer_type == 'rb':
            continue
        test_name = ['fcc-read-after-clear', subtest, buffer_type]
        add_concurrent_test(fast_color_clear, test_name)
add_concurrent_test(fast_color_clear, ['fcc-blit-between-clears'])
add_plain_test(fast_color_clear, ['fcc-read-to-pbo-after-clear'])

def add_asmparsertest(group, shader):
    profile.test_list[grouptools.join('asmparsertest', group, shader)] = \
        PiglitGLTest(
            ['asmparsertest', group,
             os.path.join(TESTS_DIR, 'asmparsertest', 'shaders', group, shader)],
            run_concurrent=True)

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
add_asmparsertest('ARBfp1.0', 'swz-08.txt')
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

ext_unpack_subimage = spec['EXT_unpack_subimage']
ext_unpack_subimage['basic'] = PiglitGLTest(['ext_unpack_subimage'], run_concurrent=True)

oes_draw_texture = spec['OES_draw_texture']
oes_draw_texture['oes_draw_texture'] = PiglitGLTest(['oes_draw_texture'], run_concurrent=True)

oes_compressed_etc1_rgb8_texture = spec['OES_compressed_ETC1_RGB8_texture']
oes_compressed_etc1_rgb8_texture['basic'] = PiglitGLTest(['oes_compressed_etc1_rgb8_texture-basic'], run_concurrent=True)
oes_compressed_etc1_rgb8_texture['miptree'] = PiglitGLTest(['oes_compressed_etc1_rgb8_texture-miptree'], run_concurrent=True)

oes_compressed_paletted_texture = spec['OES_compressed_paletted_texture']
oes_compressed_paletted_texture['basic API'] = PiglitGLTest(['oes_compressed_paletted_texture-api'], run_concurrent=True)
oes_compressed_paletted_texture['invalid formats'] = PiglitGLTest(['arb_texture_compression-invalid-formats', 'paletted'], run_concurrent=True)
oes_compressed_paletted_texture['basic API'] = PiglitGLTest(['oes_compressed_paletted_texture-api'], run_concurrent=True)

egl14 = spec['EGL 1.4']
egl14['eglCreateSurface'] = PiglitGLTest(['egl-create-surface'], exclude_platforms=['glx'])
egl14['eglQuerySurface EGL_BAD_ATTRIBUTE'] = PiglitGLTest(['egl-query-surface', '--bad-attr'], exclude_platforms=['glx'])
egl14['eglQuerySurface EGL_BAD_SURFACE'] = PiglitGLTest(['egl-query-surface', '--bad-surface'], exclude_platforms=['glx'])
egl14['eglQuerySurface EGL_HEIGHT'] = PiglitGLTest(['egl-query-surface', '--attr=EGL_HEIGHT'], exclude_platforms=['glx'])
egl14['eglQuerySurface EGL_WIDTH'] = PiglitGLTest(['egl-query-surface', '--attr=EGL_WIDTH'], exclude_platforms=['glx'])
egl14['eglTerminate then unbind context'] = PiglitGLTest(['egl-terminate-then-unbind-context'], exclude_platforms=['glx'])
egl14['eglCreatePbufferSurface and then glClear'] = PiglitGLTest(['egl-create-pbuffer-surface'], exclude_platforms=['glx'])

egl_nok_swap_region = spec['EGL_NOK_swap_region']
egl_nok_swap_region['basic'] = PiglitGLTest(['egl-nok-swap-region'], exclude_platforms=['glx'])

egl_nok_texture_from_pixmap = spec['EGL_NOK_texture_from_pixmap']
egl_nok_texture_from_pixmap['basic'] = PiglitGLTest(['egl-nok-texture-from-pixmap'], exclude_platforms=['glx'])

egl_khr_create_context = spec['EGL_KHR_create_context']
egl_khr_create_context['default major version GLES'] = PiglitGLTest(['egl-create-context-default-major-version-gles'], exclude_platforms=['glx'])
egl_khr_create_context['default major version GL'] = PiglitGLTest(['egl-create-context-default-major-version-gl'], exclude_platforms=['glx'])
egl_khr_create_context['default minor version GLES'] = PiglitGLTest(['egl-create-context-default-minor-version-gles'], exclude_platforms=['glx'])
egl_khr_create_context['default minor version GL'] = PiglitGLTest(['egl-create-context-default-minor-version-gl'], exclude_platforms=['glx'])
egl_khr_create_context['valid attribute empty GLES'] = PiglitGLTest(['egl-create-context-valid-attribute-empty-gles'], exclude_platforms=['glx'])
egl_khr_create_context['valid attribute empty GL'] = PiglitGLTest(['egl-create-context-valid-attribute-empty-gl'], exclude_platforms=['glx'])
egl_khr_create_context['NULL valid attribute GLES'] = PiglitGLTest(['egl-create-context-valid-attribute-null-gles'], exclude_platforms=['glx'])
egl_khr_create_context['NULL valid attribute GL'] = PiglitGLTest(['egl-create-context-valid-attribute-null-gl'], exclude_platforms=['glx'])
egl_khr_create_context['invalid OpenGL version'] = PiglitGLTest(['egl-create-context-invalid-gl-version'], exclude_platforms=['glx'])
egl_khr_create_context['invalid attribute GLES'] = PiglitGLTest(['egl-create-context-invalid-attribute-gles'], exclude_platforms=['glx'])
egl_khr_create_context['invalid attribute GL'] = PiglitGLTest(['egl-create-context-invalid-attribute-gl'], exclude_platforms=['glx'])
egl_khr_create_context['invalid flag GLES'] = PiglitGLTest(['egl-create-context-invalid-flag-gles'], exclude_platforms=['glx'])
egl_khr_create_context['invalid flag GL'] = PiglitGLTest(['egl-create-context-invalid-flag-gl'], exclude_platforms=['glx'])
egl_khr_create_context['valid forward-compatible flag GL'] = PiglitGLTest(['egl-create-context-valid-flag-forward-compatible-gl'], exclude_platforms=['glx'])
egl_khr_create_context['invalid profile'] = PiglitGLTest(['egl-create-context-invalid-profile'], exclude_platforms=['glx'])
egl_khr_create_context['3.2 core profile required'] = PiglitGLTest(['egl-create-context-core-profile'], exclude_platforms=['glx'])
egl_khr_create_context['pre-GL3.2 profile'] = PiglitGLTest(['egl-create-context-pre-GL32-profile'], exclude_platforms=['glx'])
egl_khr_create_context['verify GL flavor'] = PiglitGLTest(['egl-create-context-verify-gl-flavor'], exclude_platforms=['glx'])
egl_khr_create_context['valid debug flag GL'] = PiglitGLTest(['egl-create-context-valid-flag-debug-gl', 'gl'], exclude_platforms=['glx'])
for api in ('gles1', 'gles2', 'gles3'):
    egl_khr_create_context['valid debug flag ' + api] = \
        PiglitGLTest(['egl-create-context-valid-flag-debug-gles', api], exclude_platforms=['glx'])

egl_mesa_configless_context = spec['EGL_MESA_configless_context']
egl_mesa_configless_context['basic'] = PiglitGLTest(['egl-configless-context'], run_concurrent=True, exclude_platforms=['glx'])

egl_ext_client_extensions = spec['EGL_EXT_client_extensions']
for i in [1, 2, 3]:
    egl_ext_client_extensions['conformance test {0}'.format(i)] = PiglitGLTest(['egl_ext_client_extensions', str(i)], run_concurrent=True, exclude_platforms=['glx'])

egl_khr_fence_sync = spec['EGL_KHR_fence_sync']
egl_khr_fence_sync['conformance'] = PiglitGLTest(['egl_khr_fence_sync'], run_concurrent=True, exclude_platforms=['glx'])

egl_chromium_sync_control = spec['EGL_CHROMIUM_sync_control']
egl_chromium_sync_control['conformance'] = PiglitGLTest(['egl_chromium_sync_control'], run_concurrent=True, exclude_platforms=['glx'])

gles20 = spec['!OpenGL ES 2.0']
gles20['glsl-fs-pointcoord'] = PiglitGLTest(['glsl-fs-pointcoord_gles2'], run_concurrent=True)
add_concurrent_test(gles20, ['invalid-es3-queries_gles2'])
gles20['link-no-vsfs'] = PiglitGLTest(['link-no-vsfs_gles2'], run_concurrent=True)
add_concurrent_test(gles20, ['minmax_gles2'])
add_concurrent_test(gles20, ['multiple-shader-objects_gles2'])
add_concurrent_test(gles20, ['fbo_discard_gles2'])
add_concurrent_test(gles20, ['draw_buffers_gles2'])

gles30 = spec['!OpenGL ES 3.0']
for tex_format in ('rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11', 'rgb8-punchthrough-alpha1', 'srgb8-punchthrough-alpha1'):
    test_name = ' ' .join(['oes_compressed_etc2_texture-miptree_gles3', tex_format])
    gles30[test_name] = PiglitGLTest(test_name.split(), run_concurrent=True)
gles30['minmax'] = PiglitGLTest(['minmax_gles3'], run_concurrent=True)
for test_mode in ['teximage', 'texsubimage']:
    test_name = 'ext_texture_array-compressed_gles3 {0}'.format(test_mode)
    gles30[test_name] = PiglitGLTest(['ext_texture_array-compressed_gles3', test_mode, '-fbo'])
gles30['texture-immutable-levels'] = PiglitGLTest(['texture-immutable-levels_gles3'], run_concurrent=True)
gles30['gl_VertexID used with glDrawArrays'] = PiglitGLTest(['gles-3.0-drawarrays-vertexid'], run_concurrent=True)

arb_es3_compatibility = spec['ARB_ES3_compatibility']
for tex_format in ('rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11', 'rgb8-punchthrough-alpha1', 'srgb8-punchthrough-alpha1'):
    for context in ('core', 'compat'):
        test_name = ' ' .join(['oes_compressed_etc2_texture-miptree', tex_format, context])
        arb_es3_compatibility[test_name] = PiglitGLTest(test_name.split(), run_concurrent=True)
add_concurrent_test(arb_es3_compatibility, ['es3-primrestart-fixedindex'])
add_concurrent_test(arb_es3_compatibility, ['es3-drawarrays-primrestart-fixedindex'])

arb_shader_atomic_counters = spec['ARB_shader_atomic_counters']
arb_shader_atomic_counters['active-counters'] = PiglitGLTest(['arb_shader_atomic_counters-active-counters'], run_concurrent=True)
arb_shader_atomic_counters['array-indexing'] = PiglitGLTest(['arb_shader_atomic_counters-array-indexing'], run_concurrent=True)
arb_shader_atomic_counters['buffer-binding'] = PiglitGLTest(['arb_shader_atomic_counters-buffer-binding'], run_concurrent=True)
arb_shader_atomic_counters['default-partition'] = PiglitGLTest(['arb_shader_atomic_counters-default-partition'], run_concurrent=True)
arb_shader_atomic_counters['fragment-discard'] = PiglitGLTest(['arb_shader_atomic_counters-fragment-discard'], run_concurrent=True)
arb_shader_atomic_counters['function-argument'] = PiglitGLTest(['arb_shader_atomic_counters-function-argument'], run_concurrent=True)
arb_shader_atomic_counters['max-counters'] = PiglitGLTest(['arb_shader_atomic_counters-max-counters'], run_concurrent=True)
arb_shader_atomic_counters['minmax'] = PiglitGLTest(['arb_shader_atomic_counters-minmax'], run_concurrent=True)
arb_shader_atomic_counters['multiple-defs'] = PiglitGLTest(['arb_shader_atomic_counters-multiple-defs'], run_concurrent=True)
arb_shader_atomic_counters['semantics'] = PiglitGLTest(['arb_shader_atomic_counters-semantics'], run_concurrent=True)
arb_shader_atomic_counters['unique-id'] = PiglitGLTest(['arb_shader_atomic_counters-unique-id'], run_concurrent=True)
arb_shader_atomic_counters['unused-result'] = PiglitGLTest(['arb_shader_atomic_counters-unused-result'], run_concurrent=True)
arb_shader_atomic_counters['respecify-buffer'] = PiglitGLTest(['arb_shader_atomic_counters-respecify-buffer'], run_concurrent=True)

spec['ARB_direct_state_access']['create-transformfeedbacks'] = PiglitGLTest(['arb_direct_state_access-create-transformfeedbacks'], run_concurrent=True)
spec['ARB_direct_state_access']['dsa-textures'] = PiglitGLTest(['arb_direct_state_access-dsa-textures'], run_concurrent=True)
spec['ARB_direct_state_access']['texturesubimage'] = PiglitGLTest(['arb_direct_state_access-texturesubimage'], run_concurrent=True)
spec['ARB_direct_state_access']['bind-texture-unit'] = PiglitGLTest(['arb_direct_state_access-bind-texture-unit'], run_concurrent=True)
spec['ARB_direct_state_access']['create-textures'] = PiglitGLTest(['arb_direct_state_access-create-textures'], run_concurrent=True)
spec['ARB_direct_state_access']['texture-storage'] = PiglitGLTest(['arb_direct_state_access-texture-storage'], run_concurrent=True)
spec['ARB_direct_state_access']['texunits'] = PiglitGLTest(['arb_direct_state_access-texunits'], run_concurrent=True)
spec['ARB_direct_state_access']['texture-params'] = PiglitGLTest(['arb_direct_state_access-texture-params'], run_concurrent=True)
spec['ARB_direct_state_access']['copytexturesubimage'] = PiglitGLTest(['arb_direct_state_access-copytexturesubimage'], run_concurrent=True)
spec['ARB_direct_state_access']['texture-errors'] = PiglitGLTest(['arb_direct_state_access-texture-errors'], run_concurrent=True)
spec['ARB_direct_state_access']['get-textures'] = PiglitGLTest(['arb_direct_state_access-get-textures'], run_concurrent=True)
spec['ARB_direct_state_access']['gettextureimage-formats'] = PiglitGLTest(['arb_direct_state_access-gettextureimage-formats'], run_concurrent=True)
spec['ARB_direct_state_access']['gettextureimage-formats'] = PiglitGLTest(['arb_direct_state_access-gettextureimage-formats init-by-rendering'], run_concurrent=True)
spec['ARB_direct_state_access']['gettextureimage-luminance'] = PiglitGLTest(['arb_direct_state_access-gettextureimage-luminance'], run_concurrent=True)
spec['ARB_direct_state_access']['gettextureimage-simple'] = PiglitGLTest(['arb_direct_state_access-gettextureimage-simple'], run_concurrent=True)
spec['ARB_direct_state_access']['gettextureimage-targets'] = PiglitGLTest(['arb_direct_state_access-gettextureimage-targets'], run_concurrent=True)
spec['ARB_direct_state_access']['compressedtextureimage'] = PiglitGLTest(['arb_direct_state_access-compressedtextureimage GL_COMPRESSED_RGBA_FXT1_3DFX'], run_concurrent=True)
spec['ARB_direct_state_access']['getcompressedtextureimage'] = PiglitGLTest(['arb_direct_state_access-getcompressedtextureimage'], run_concurrent=True)
spec['ARB_direct_state_access']['texture-storage-multisample'] = PiglitGLTest(['arb_direct_state_access-texture-storage-multisample'], run_concurrent=True)
spec['ARB_direct_state_access']['texture-buffer'] = PiglitGLTest(['arb_direct_state_access-texture-buffer'], run_concurrent=True)

arb_shader_image_load_store = spec['ARB_shader_image_load_store']
arb_shader_image_load_store['atomicity'] = PiglitGLTest(['arb_shader_image_load_store-atomicity'], run_concurrent=True)
arb_shader_image_load_store['bitcast'] = PiglitGLTest(['arb_shader_image_load_store-bitcast'], run_concurrent=True)
arb_shader_image_load_store['coherency'] = PiglitGLTest(['arb_shader_image_load_store-coherency'], run_concurrent=True)
arb_shader_image_load_store['dead-fragments'] = PiglitGLTest(['arb_shader_image_load_store-dead-fragments'], run_concurrent=True)
arb_shader_image_load_store['early-z'] = PiglitGLTest(['arb_shader_image_load_store-early-z'], run_concurrent=True)
arb_shader_image_load_store['host-mem-barrier'] = PiglitGLTest(['arb_shader_image_load_store-host-mem-barrier'], run_concurrent=True)
arb_shader_image_load_store['indexing'] = PiglitGLTest(['arb_shader_image_load_store-indexing'], run_concurrent=True)
arb_shader_image_load_store['invalid'] = PiglitGLTest(['arb_shader_image_load_store-invalid'], run_concurrent=True)
arb_shader_image_load_store['layer'] = PiglitGLTest(['arb_shader_image_load_store-layer'], run_concurrent=True)
arb_shader_image_load_store['level'] = PiglitGLTest(['arb_shader_image_load_store-level'], run_concurrent=True)
arb_shader_image_load_store['max-images'] = PiglitGLTest(['arb_shader_image_load_store-max-images'], run_concurrent=True)
arb_shader_image_load_store['max-size'] = PiglitGLTest(['arb_shader_image_load_store-max-size'], run_concurrent=True)
arb_shader_image_load_store['minmax'] = PiglitGLTest(['arb_shader_image_load_store-minmax'], run_concurrent=True)
arb_shader_image_load_store['qualifiers'] = PiglitGLTest(['arb_shader_image_load_store-qualifiers'], run_concurrent=True)
arb_shader_image_load_store['restrict'] = PiglitGLTest(['arb_shader_image_load_store-restrict'], run_concurrent=True)
arb_shader_image_load_store['semantics'] = PiglitGLTest(['arb_shader_image_load_store-semantics'], run_concurrent=True)
arb_shader_image_load_store['shader-mem-barrier'] = PiglitGLTest(['arb_shader_image_load_store-shader-mem-barrier'], run_concurrent=True)
arb_shader_image_load_store['state'] = PiglitGLTest(['arb_shader_image_load_store-state'], run_concurrent=True)
arb_shader_image_load_store['unused'] = PiglitGLTest(['arb_shader_image_load_store-unused'], run_concurrent=True)

profile.tests['hiz'] = hiz
profile.tests['fast_color_clear'] = fast_color_clear
profile.tests['glean'] = glean
profile.tests['shaders'] = shaders
profile.tests['security'] = security
profile.tests['spec'] = spec

if platform.system() is 'Windows':
    del profile.tests['glx']
