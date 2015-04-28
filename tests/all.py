# -*- coding: utf-8 -*-
# All tests that come with piglit, using default settings

from __future__ import print_function, division, absolute_import
import itertools
import os
import platform

from framework import grouptools
from framework.profile import TestProfile
from framework.test import (PiglitGLTest, GleanTest, ShaderTest,
                            GLSLParserTest, GLSLParserNoConfigError)
from .py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR

__all__ = ['profile']

# Disable bad hanging indent errors in pylint
# There is a bug in pyling which causes the profile.group_manager to be tagged
# as bad hanging indent, even though it seems to be correct (and similar syntax
# doesn't trigger an error)
# pylint: disable=bad-continuation


def add_single_param_test_set(adder, name, *params):
    for param in params:
        adder([name, param], '{}-{}'.format(name, param), run_concurrent=False)


def add_depthstencil_render_miplevels_tests(adder, test_types):
    # Note: the buffer sizes below have been chosen to exercise
    # many possible combinations of buffer alignments on i965.
    for texture_size in ['146', '273', '292', '585', '1024']:
        for test_type in test_types:
            adder(['depthstencil-render-miplevels', texture_size, test_type],
                  run_concurrent=False)


def add_fbo_stencil_tests(adder, format):
    g(['fbo-stencil', 'clear', format], 'fbo-stencil-{}-clear'.format(format))
    g(['fbo-stencil', 'readpixels', format],
      'fbo-stencil-{}-readpixels'.format(format))
    g(['fbo-stencil', 'drawpixels', format],
      'fbo-stencil-{}-drawpixels'.format(format))
    g(['fbo-stencil', 'copypixels', format],
      'fbo-stencil-{}-copypixels'.format(format))
    g(['fbo-stencil', 'blit', format], 'fbo-stencil-{}-blit'.format(format))


def add_fbo_depthstencil_tests(group, format, num_samples):
    assert format, \
        'add_fbo_depthstencil_tests argument "format" cannot be empty'

    if format == 'default_fb':
        prefix = ''
        create_test = lambda a: PiglitGLTest(a, run_concurrent=False)
    else:
        prefix = 'fbo-'
        create_test = PiglitGLTest

    if num_samples > 1:
        suffix = ' samples=' + str(num_samples)
        psamples = '-samples=' + str(num_samples)
    else:
        suffix = ''
        psamples = ''

    profile.test_list[grouptools.join(
        group, '{}depthstencil-{}-clear{}'.format(prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'clear', format, psamples])
    profile.test_list[grouptools.join(
        group, '{}depthstencil-{}-readpixels-FLOAT-and-USHORT{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'readpixels', format,
                    'FLOAT-and-USHORT', psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-readpixels-24_8{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'readpixels', format, '24_8',
                    psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-readpixels-32F_24_8_REV{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'readpixels', format,
                    '32F_24_8_REV', psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-drawpixels-FLOAT-and-USHORT{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'drawpixels', format,
                     'FLOAT-and-USHORT', psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-drawpixels-24_8{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'drawpixels', format, '24_8',
                    psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-drawpixels-32F_24_8_REV{}'.format(
            prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'drawpixels', format,
                    '32F_24_8_REV', psamples])
    profile.test_list[grouptools.join(
        group,
        '{}depthstencil-{}-copypixels{}'.format(prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'copypixels', format, psamples])
    profile.test_list[grouptools.join(
        group, '{}depthstencil-{}-blit{}'.format(prefix, format, suffix))] = \
        create_test(['fbo-depthstencil', 'blit', format, psamples])


def add_msaa_visual_plain_tests(adder, args, **kwargs):
    assert isinstance(args, list)

    adder(args, **kwargs)
    for num_samples in MSAA_SAMPLE_COUNTS:
        adder(args + ['-samples={}'.format(num_samples)],
              ' '.join(args + ['samples={}'.format(num_samples)]),
              **kwargs)


def add_fbo_formats_tests(adder, extension, suffix=''):
    adder(['fbo-generatemipmap-formats', extension],
          'fbo-generatemipmap-formats{}'.format(suffix))
    adder(['fbo-clear-formats', extension],
          'fbo-clear-formats{}'.format(suffix))
    adder(['get-renderbuffer-internalformat', extension],
          'get-renderbuffer-internalformat{}'.format(suffix))
    if 'depth' not in extension and 'stencil' not in extension:
        adder(['fbo-blending-formats', extension],
              'fbo-blending-formats{}'.format(suffix))
        adder(['fbo-alphatest-formats', extension],
              'fbo-alphatest-formats{}'.format(suffix))
        adder(['fbo-colormask-formats', extension],
              'fbo-colormask-formats{}'.format(suffix))


def add_msaa_formats_tests(adder, extension):
    for num_samples in MSAA_SAMPLE_COUNTS:
        adder(['ext_framebuffer_multisample-formats', str(num_samples),
               extension],
              'multisample-formats {} {}'.format(num_samples, extension))


def add_vpfpgeneric(adder, name):
    adder(['vpfp-generic',
           os.path.join(TESTS_DIR, 'shaders', 'generic', name + '.vpfp')],
          name)


def add_texwrap_target_tests(adder, target):
    adder(['texwrap', 'GL_RGBA8'], 'texwrap {}'.format(target))
    adder(['texwrap', 'GL_RGBA8', 'bordercolor'],
          'texwrap {} bordercolor'.format(target))
    adder(['texwrap', 'GL_RGBA8', 'proj'], 'texwrap {} proj'.format(target))
    adder(['texwrap', 'GL_RGBA8', 'proj', 'bordercolor'],
          'texwrap {} proj bordercolor'.format(target))


def add_texwrap_format_tests(adder, ext='', suffix=''):
    args = [] if ext == '' else [ext]
    adder(['texwrap'] + args, 'texwrap formats{}'.format(suffix))
    adder(['texwrap'] + args + ['bordercolor'],
          'texwrap formats{} bordercolor'.format(suffix))
    adder(['texwrap'] + args + ['bordercolor', 'swizzled'],
          'texwrap formats{} bordercolor-swizzled'.format(suffix))


def add_fbo_depth_tests(adder, format):
    adder(['fbo-depth-tex1d', format], 'fbo-depth-{}-tex1d'.format(format))
    adder(['fbo-depth', 'clear', format], 'fbo-depth-{}-clear'.format(format))
    adder(['fbo-depth', 'readpixels', format],
          'fbo-depth-{}-readpixels'.format(format))
    adder(['fbo-depth', 'drawpixels', format],
          'fbo-depth-{}-drawpixels'.format(format))
    adder(['fbo-depth', 'copypixels', format],
          'fbo-depth-{}-copypixels'.format(format))
    adder(['fbo-depth', 'blit', format], 'fbo-depth-{}-blit'.format(format))


def power_set(s):
    """Generate all possible subsets of the given set, including the empty set.
    """
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

# Collect and add all asmparsertests
_basedir = os.path.join(TESTS_DIR, 'asmparsertest', 'shaders')
for dirpath, _, filenames in os.walk(_basedir):
    base_group = grouptools.from_path(os.path.join(
        'asmparsertest', os.path.relpath(dirpath, _basedir)))
    type_ = os.path.basename(dirpath)

    for filename in filenames:
        if not os.path.splitext(filename)[1] == '.txt':
            continue

        group = grouptools.join(base_group, filename)
        profile.test_list[group] = PiglitGLTest(
            ['asmparsertest', type_, os.path.join(dirpath, filename)])

# List of all of the MSAA sample counts we wish to test
MSAA_SAMPLE_COUNTS = (2, 4, 6, 8, 16, 32)

with profile.group_manager(GleanTest, 'glean') as g:
    g('basic')
    g('api2')
    g('makeCurrent')
    g('bufferObject')
    g('depthStencil')
    g('fbo')
    g('getString')
    g('occluQry')
    g('paths')
    g('pixelFormats')
    g('pointAtten')
    g('pointSprite')
    # exactRGBA is not included intentionally, because it's too strict and
    # the equivalent functionality is covered by other tests
    g('shaderAPI')
    g('stencil2')
    g('texCombine')
    g('texCube')
    g('texEnv')
    g('texgen')
    g('texCombine4')
    g('texture_srgb')
    g('texUnits')
    g('vertArrayBGRA')
    g('vertattrib')

glean_glsl_tests = ['Primary plus secondary color',
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
        profile.test_list[groupname] = GleanTest(prefix)
        profile.test_list[groupname].env['PIGLIT_TEST'] = name

with profile.group_manager(PiglitGLTest, 'security') as g:
    g(['initialized-texmemory'], run_concurrent=False)
    g(['initialized-fbo'], run_concurrent=False)
    g(['initialized-vbo'], run_concurrent=False)

with profile.group_manager(PiglitGLTest, 'shaders') as g:
    g(['activeprogram-bad-program'])
    g(['activeprogram-get'])
    g(['attribute0'])
    g(['createshaderprogram-bad-type'])
    g(['createshaderprogram-attached-shaders'])
    g(['glsl-arb-fragment-coord-conventions'])
    g(['glsl-arb-fragment-coord-conventions-define'])
    g(['glsl-bug-22603'])
    g(['glsl-bindattriblocation'])
    g(['glsl-dlist-getattriblocation'])
    g(['glsl-getactiveuniform-array-size'])
    g(['glsl-getactiveuniform-length'])
    g(['glsl-getattriblocation'])
    g(['getuniform-01'])
    g(['getuniform-02'])
    g(['glsl-invalid-asm-01'])
    g(['glsl-invalid-asm-02'])
    g(['glsl-novertexdata'])
    g(['glsl-preprocessor-comments'])
    g(['glsl-reload-source'])
    g(['glsl-uniform-out-of-bounds'])
    g(['glsl-uniform-out-of-bounds-2'])
    g(['glsl-uniform-update'])
    g(['glsl-unused-varying'])
    g(['glsl-fs-bug25902'])
    g(['glsl-fs-color-matrix'])
    g(['glsl-fs-discard-02'])
    g(['glsl-fs-exp2'])
    g(['glsl-fs-flat-color'])
    g(['glsl-fs-fogcolor-statechange'])
    g(['glsl-fs-fogscale'])
    g(['glsl-fs-fragcoord'])
    g(['glsl-fs-fragcoord-zw-ortho'])
    g(['glsl-fs-fragcoord-zw-perspective'])
    g(['glsl-fs-loop'])
    g(['glsl-fs-loop-nested'])
    g(['glsl-fs-pointcoord'])
    g(['glsl-fs-raytrace-bug27060'])
    g(['glsl-fs-sampler-numbering'])
    g(['glsl-fs-shader-stencil-export'])
    g(['glsl-fs-sqrt-branch'])
    g(['glsl-fs-texturecube'])
    g(['glsl-fs-texturecube', '-bias'], 'glsl-fs-texturecube-bias')
    g(['glsl-fs-texturecube-2'])
    g(['glsl-fs-texturecube-2', '-bias'], 'glsl-fs-texturecube-2-bias')
    g(['glsl-fs-textureenvcolor-statechange'], run_concurrent=False)
    g(['glsl-fs-texture2drect'])
    g(['glsl-fs-texture2drect', '-proj3'], 'glsl-fs-texture2drect-proj3')
    g(['glsl-fs-texture2drect', '-proj4'], 'glsl-fs-texture2drect-proj4')
    g(['glsl-fs-user-varying-ff'])
    g(['glsl-mat-attribute'])
    g(['glsl-max-varyings'])
    g(['glsl-max-varyings', '--exceed-limits'],
      'glsl-max-varyings >MAX_VARYING_COMPONENTS')
    g(['glsl-orangebook-ch06-bump'])
    g(['glsl-routing'])
    g(['glsl-vs-arrays'])
    g(['glsl-vs-normalscale'])
    g(['glsl-vs-functions'])
    g(['glsl-vs-user-varying-ff'])
    g(['glsl-vs-texturematrix-1'])
    g(['glsl-vs-texturematrix-2'])
    g(['glsl-sin'])
    g(['glsl-cos'])
    g(['glsl-vs-if-bool'])
    g(['glsl-vs-loop'])
    g(['glsl-vs-loop-nested'])
    g(['glsl-vs-mov-after-deref'])
    g(['glsl-vs-mvp-statechange'])
    g(['glsl-vs-raytrace-bug26691'])
    g(['glsl-vs-statechange-1'])
    g(['vp-combined-image-units'])
    g(['glsl-derivs'])
    g(['glsl-fwidth'])
    g(['glsl-lod-bias'])
    g(['vp-ignore-input'])
    g(['glsl-empty-vs-no-fs'])
    g(['glsl-useprogram-displaylist'])
    g(['glsl-vs-point-size'])
    g(['glsl-light-model'])
    g(['glsl-link-bug30552'])
    g(['glsl-link-bug38015'])
    g(['glsl-link-empty-prog-01'])
    g(['glsl-link-empty-prog-02'])
    g(['glsl-max-vertex-attrib'])
    g(['glsl-kwin-blur-1'])
    g(['glsl-kwin-blur-2'])
    g(['gpu_shader4_attribs'])
    g(['link-unresolved-function'])
    g(['sso-simple'])
    g(['sso-uniforms-01'])
    g(['sso-uniforms-02'])
    g(['sso-user-varying-01'])
    g(['sso-user-varying-02'])
    g(['useprogram-flushverts-1'])
    g(['useprogram-flushverts-2'])
    g(['useprogram-inside-begin'])
    g(['useprogram-refcount-1'])
    g(['useshaderprogram-bad-type'])
    g(['useshaderprogram-bad-program'])
    g(['useshaderprogram-flushverts-1'])
    g(['point-vertex-id'])
    g(['glsl-vs-int-attrib'])
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-01a.vert'),
       os.path.join('shaders', 'glsl-link-initializer-01b.vert'), 'pass'],
      'GLSL link single global initializer, 2 shaders')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-01c.vert'),
       os.path.join('shaders', 'glsl-link-initializer-01d.vert'),
       'pass'],
      'GLSL link matched global initializer, 2 shaders')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-01b.vert'),
       os.path.join('shaders', 'glsl-link-initializer-01d.vert'),
       'fail'],
      'GLSL link mismatched global initializer, 2 shaders')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-01a.vert'),
       os.path.join('shaders', 'glsl-link-initializer-01b.vert'),
       os.path.join('shaders', 'glsl-link-initializer-01c.vert'),
       'fail'],
      'GLSL link mismatched global initializer, 3 shaders')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-02a.vert'),
       os.path.join('shaders', 'glsl-link-initializer-02b.vert'),
       'fail'],
      'GLSL link mismatched global const initializer')
    g(['glsl-link-initializer-03'],
      'GLSL link two programs, global initializer')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-05a.vert'),
       os.path.join('shaders', 'glsl-link-initializer-05b.vert'),
       'fail'],
      'GLSL link matched global initializer expression')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-initializer-06a.vert'),
       os.path.join('shaders', 'glsl-link-initializer-06b.vert'),
       'fail'],
      'GLSL link mismatched global initializer expression')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-invariant-01a.vert'),
       os.path.join('shaders', 'glsl-link-invariant-01b.vert'),
       'fail'],
      'GLSL link mismatched invariant')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-centroid-01a.vert'),
       os.path.join('shaders', 'glsl-link-centroid-01b.vert'),
       'fail'],
      'GLSL link mismatched centroid')
    g(['glsl-link-test',
       os.path.join('shaders', 'glsl-link-struct-array.frag'),
       'pass'],
      'GLSL link array-of-struct-of-array')
    g(['glsl-getactiveuniform-count',
       os.path.join('shaders', 'glsl-getactiveuniform-length.vert'), '1'],
      'glsl-getactiveuniform-count: {}'.format('glsl-getactiveuniform-length'))
    g(['glsl-getactiveuniform-count',
       os.path.join('shaders', 'glsl-getactiveuniform-ftransform.vert'), '2'],
      'glsl-getactiveuniform-count: {}'.format(
          'glsl-getactiveuniform-ftransform'))
    g(['glsl-getactiveuniform-count',
       os.path.join('shaders', 'glsl-getactiveuniform-mvp.vert'), '2'],
      'glsl-getactiveuniform-count: {}'.format('glsl-getactiveuniform-mvp'))

    for subtest in ('interstage', 'intrastage', 'vs-gs'):
        g(['version-mixing', subtest])

with profile.group_manager(
        PiglitGLTest, 'glx',
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-destroycontext-1'], run_concurrent=False)
    g(['glx-destroycontext-2'], run_concurrent=False)
    g(['glx-dont-care-mask'], run_concurrent=False)
    g(['glx-close-display'], run_concurrent=False)
    g(['glx-fbconfig-sanity'], run_concurrent=False)
    g(['glx-fbconfig-compliance'], run_concurrent=False)
    g(['glx-fbo-binding'], run_concurrent=False)
    g(['glx-multi-context-ib-1'], run_concurrent=False)
    g(['glx-multithread'], run_concurrent=False)
    g(['glx-multithread-texture'], run_concurrent=False)
    g(['glx-multithread-makecurrent-1'], run_concurrent=False)
    g(['glx-multithread-makecurrent-2'], run_concurrent=False)
    g(['glx-multithread-makecurrent-3'], run_concurrent=False)
    g(['glx-multithread-makecurrent-4'], run_concurrent=False)
    g(['glx-multithread-shader-compile'], run_concurrent=False)
    g(['glx-shader-sharing'], run_concurrent=False)
    g(['glx-swap-exchange'], run_concurrent=False)
    g(['glx-swap-event', '--event'], 'glx-swap-event_event',
      run_concurrent=False)
    g(['glx-swap-event', '--async'], 'glx-swap-event_async',
      run_concurrent=False)
    g(['glx-swap-event', '--interval'], 'glx-swap-event_interval',
      run_concurrent=False)
    g(['glx-swap-pixmap'], run_concurrent=False)
    g(['glx-swap-pixmap-bad'], run_concurrent=False)
    g(['glx-swap-singlebuffer'], run_concurrent=False)
    g(['glx-make-current'], run_concurrent=False)
    g(['glx-make-glxdrawable-current'], run_concurrent=False)
    g(['glx-context-flush-control'], run_concurrent=False)
    g(['glx-buffer-age'], run_concurrent=False)
    g(['glx-pixmap-life'])
    g(['glx-pixmap13-life'])
    g(['glx-pixmap-multi'])
    g(['glx-tfp'], run_concurrent=False)
    g(['glx-visuals-depth'], run_concurrent=False)
    g(['glx-visuals-depth', '-pixmap'])
    g(['glx-visuals-stencil'], run_concurrent=False)
    g(['glx-visuals-stencil', '-pixmap'])
    g(['glx-window-life'])
    g(['glx-pixmap-crosscheck'])
    g(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXWINDOW'],
      'glx-query-drawable-GLXWINDOW-GLX_WIDTH', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXWINDOW'],
      'glx-query-drawable-GLXWINDOW-GLX_HEIGHT', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXPIXMAP'],
      'glx-query-drawable-GLXPIXMAP-GLX_WIDTH', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXPIXMAP'],
      'glx-query-drawable-GLXPIXMAP-GLX_HEIGHT', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=GLXPBUFFER'],
      'glx-query-drawable-GLXPBUFFER-GLX_WIDTH', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=GLXPBUFFER'],
      'glx-query-drawable-GLXPBUFFER-GLX_HEIGHT', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_WIDTH', '--type=WINDOW'],
      'glx-query-drawable-GLX_WIDTH', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_HEIGHT', '--type=WINDOW'],
      'glx-query-drawable-GLX_HEIGHT', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=WINDOW'],
      'glx-query-drawable-GLX_FBCONFIG_ID-WINDOW', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXWINDOW'],
      'glx-query-drawable-GLX_FBCONFIG_ID-GLXWINDOW', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPIXMAP'],
      'glx-query-drawable-GLX_FBCONFIG_ID-GLXPIXMAP', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPBUFFER'],
      'glx-query-drawable-GLX_FBCONFIG_ID-GLXPBUFFER', run_concurrent=False)
    g(['glx-query-drawable', '--attr=GLX_FBCONFIG_ID', '--type=GLXPBUFFER'],
      'glx-query-drawable-GLX_PRESERVED_CONTENTS', run_concurrent=False)
    g(['glx-query-drawable', '--bad-drawable'],
      'glx-query-drawable-GLXBadDrawable', run_concurrent=False)
    g(['glx-string-sanity'], 'extension string sanity')
    add_msaa_visual_plain_tests(g, ['glx-copy-sub-buffer'],
                                run_concurrent=False)
profile.test_list[grouptools.join('glx', 'glx-buffer-age vblank_mode=0')] = \
    PiglitGLTest(['glx-buffer-age'],
                 require_platforms=['glx', 'mixed_glx_egl'],
                 run_concurrent=False)
profile.test_list[grouptools.join(
    'glx', 'glx-buffer-age vblank_mode=0')].env['vblank_mode'] = '0'

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'glx_ext_import_context'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-free-context'], 'free context', run_concurrent=False)
    g(['glx-get-context-id'], 'get context ID', run_concurrent=False)
    g(['glx-get-current-display-ext'], 'get current display',
      run_concurrent=False)
    g(['glx-import-context-has-same-context-id'],
      'imported context has same context ID',
      run_concurrent=False)
    g(['glx-import-context-multi-process'], 'import context, multi process',
      run_concurrent=False)
    g(['glx-import-context-single-process'], 'import context, single process',
      run_concurrent=False)
    g(['glx-make-current-multi-process'], 'make current, multi process',
      run_concurrent=False)
    g(['glx-make-current-single-process'], 'make current, single process',
      run_concurrent=False)
    g(['glx-query-context-info-ext'], 'query context info',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-current-no-framebuffer'],
      'current with no framebuffer')
    g(['glx-create-context-default-major-version'], 'default major version')
    g(['glx-create-context-default-minor-version'], 'default minor version')
    g(['glx-create-context-invalid-attribute'], 'invalid attribute')
    g(['glx-create-context-invalid-flag'], 'invalid flag')
    g(['glx-create-context-invalid-flag-forward-compatible'],
      'forward-compatible flag with pre-3.0')
    g(['glx-create-context-invalid-gl-version'], 'invalid OpenGL version')
    g(['glx-create-context-invalid-render-type'], 'invalid render type')
    g(['glx-create-context-invalid-render-type-color-index'],
      'color-index render type with 3.0')
    g(['glx-create-context-valid-attribute-empty'], 'empty attribute list')
    g(['glx-create-context-valid-attribute-null'], 'NULL attribute list')
    g(['glx-create-context-valid-flag-forward-compatible'],
      'forward-compatible flag with 3.0')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_profile'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-core-profile'], '3.2 core profile required')
    g(['glx-create-context-invalid-profile'], 'invalid profile')
    g(['glx-create-context-pre-GL32-profile'], 'pre-GL3.2 profile')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_robustness'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-invalid-reset-strategy'],
      'invalid reset notification strategy')
    g(['glx-create-context-require-robustness'], 'require GL_ARB_robustness')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_es2_profile'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-indirect-es2-profile'],
      'indirect rendering ES2 profile')
    g(['glx-create-context-invalid-es-version'], 'invalid OpenGL ES version')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_sync_control'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-oml-sync-control-getmscrate'], 'glXGetMscRateOML')
    g(['glx-oml-sync-control-swapbuffersmsc-divisor-zero'],
      'swapbuffersmsc-divisor-zero')
    g(['glx-oml-sync-control-swapbuffersmsc-return'], 'swapbuffersmsc-return')
    g(['glx-oml-sync-control-swapbuffersmsc-return', '0'],
      'swapbuffersmsc-return swap_interval 0')
    g(['glx-oml-sync-control-swapbuffersmsc-return', '1'],
      'swapbuffersmsc-return swap_interval 1')
    g(['glx-oml-sync-control-waitformsc'], 'waitformsc')

oml_sync_control_nonzeros = [
    mode + [kind, period]
    for mode in [[], ['-fullscreen'], ['-waitformsc']]
    for kind in ['-divisor', '-msc-delta']
    for period in ['1', '2']
]
for args in oml_sync_control_nonzeros:
    group = grouptools.join('glx', 'GLX_ARB_sync_control',
                            ' '.join(['timing'] + args))
    profile.test_list[group] = PiglitGLTest(
        ['glx-oml-sync-control-timing'] + args,
        require_platforms=['glx', 'mixed_glx_egl'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_MESA_query_renderer'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-query-renderer-coverage'], 'coverage')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!OpenGL 1.1')) as g:
    g(['copyteximage', '1D'], run_concurrent=False)
    g(['copyteximage', '2D'], run_concurrent=False)
    g(['drawbuffer-modes'], run_concurrent=False)
    g(['fdo10370'], run_concurrent=False)
    g(['fdo23489'], run_concurrent=False)
    g(['fdo23670-depth_test'], run_concurrent=False)
    g(['fdo23670-drawpix_stencil'], run_concurrent=False)
    g(['r300-readcache'], run_concurrent=False)
    g(['tri-tex-crash'], run_concurrent=False)
    g(['vbo-buffer-unmap'], run_concurrent=False)
    g(['array-stride'], run_concurrent=False)
    g(['clear-accum'], run_concurrent=False)
    g(['clipflat'])
    g(['copypixels-draw-sync'], run_concurrent=False)
    g(['copypixels-sync'], run_concurrent=False)
    g(['degenerate-prims'], run_concurrent=False)
    g(['depthfunc'], run_concurrent=False)
    g(['depthrange-clear'], run_concurrent=False)
    g(['dlist-clear'], run_concurrent=False)
    g(['dlist-color-material'], run_concurrent=False)
    g(['dlist-fdo3129-01'], run_concurrent=False)
    g(['dlist-fdo3129-02'], run_concurrent=False)
    g(['dlist-fdo31590'], run_concurrent=False)
    g(['draw-arrays-colormaterial'], run_concurrent=False)
    g(['draw-copypixels-sync'], run_concurrent=False)
    g(['draw-pixel-with-texture'])
    g(['drawpix-z'])
    g(['fog-modes'], run_concurrent=False)
    g(['fragment-center'], run_concurrent=False)
    g(['geterror-invalid-enum'], run_concurrent=False)
    g(['geterror-inside-begin'], run_concurrent=False)
    g(['glinfo'])
    g(['hiz'], run_concurrent=False)
    g(['infinite-spot-light'], run_concurrent=False)
    g(['line-aa-width'], run_concurrent=False)
    g(['line-flat-clip-color'])
    g(['lineloop'], run_concurrent=False)
    g(['linestipple'], run_concurrent=False)
    g(['longprim'], run_concurrent=False)
    g(['masked-clear'])
    g(['point-line-no-cull'], run_concurrent=False)
    g(['polygon-mode'], run_concurrent=False)
    g(['polygon-mode-offset'])
    g(['polygon-offset'], run_concurrent=False)
    g(['push-pop-texture-state'])
    g(['quad-invariance'])
    g(['readpix-z'])
    g(['roundmode-getintegerv'], run_concurrent=False)
    g(['roundmode-pixelstore'], run_concurrent=False)
    g(['scissor-bitmap'], run_concurrent=False)
    g(['scissor-clear'], run_concurrent=False)
    g(['scissor-copypixels'], run_concurrent=False)
    g(['scissor-depth-clear'], run_concurrent=False)
    g(['scissor-many'], run_concurrent=False)
    g(['scissor-offscreen'], run_concurrent=False)
    g(['scissor-polygon'])
    g(['scissor-stencil-clear'], run_concurrent=False)
    g(['select', 'gl11'], 'GL_SELECT - no test function', run_concurrent=False)
    g(['select', 'depth'], 'GL_SELECT - depth-test enabled',
      run_concurrent=False)
    g(['select', 'stencil'], 'GL_SELECT - stencil-test enabled',
      run_concurrent=False)
    g(['select', 'alpha'], 'GL_SELECT - alpha-test enabled',
      run_concurrent=False)
    g(['select', 'scissor'], 'GL_SELECT - scissor-test enabled',
      run_concurrent=False)
    g(['stencil-drawpixels'], run_concurrent=False)
    g(['texgen'], run_concurrent=False)
    g(['two-sided-lighting'], run_concurrent=False)
    g(['user-clip'], run_concurrent=False)
    g(['varray-disabled'], run_concurrent=False)
    g(['windowoverlap'], run_concurrent=False)
    g(['copyteximage-border'], run_concurrent=False)
    g(['copyteximage-clipping'], run_concurrent=False)
    g(['copytexsubimage'], run_concurrent=False)
    g(['getteximage-formats'], run_concurrent=False)
    g(['getteximage-luminance'], run_concurrent=False)
    g(['getteximage-simple'], run_concurrent=False)
    g(['incomplete-texture', 'fixed'], 'incomplete-texture-fixed')
    g(['max-texture-size'], run_concurrent=False)
    g(['max-texture-size-level'])
    g(['proxy-texture'])
    g(['sized-texture-format-channels'])
    g(['streaming-texture-leak'], run_concurrent=False)
    g(['texredefine'], run_concurrent=False)
    g(['texsubimage'], run_concurrent=False)
    g(['texsubimage-depth-formats'], run_concurrent=False)
    g(['texture-al'], run_concurrent=False)
    g(['triangle-guardband-viewport'])
    g(['getteximage-targets', '1D'])
    g(['getteximage-targets', '2D'])
    g(['teximage-scale-bias'])
    add_msaa_visual_plain_tests(g, ['draw-pixels'], run_concurrent=False)
    add_msaa_visual_plain_tests(g, ['read-front'], run_concurrent=False)
    add_msaa_visual_plain_tests(g, ['read-front', 'clear-front-first'],
                                run_concurrent=False)
    add_texwrap_target_tests(g, '1D')
    add_texwrap_target_tests(g, '2D')
    add_texwrap_format_tests(g)

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
        g(['teximage-colors', format], run_concurrent=False)

    for num_samples in (0, ) + MSAA_SAMPLE_COUNTS:
        add_fbo_depthstencil_tests(
            grouptools.join('spec', '!opengl 1.1'), 'default_fb', num_samples)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.0')) as g:
    g(['gl-1.0-beginend-coverage'])
    g(['gl-1.0-dlist-beginend'])
    g(['gl-1.0-dlist-shademodel'])
    g(['gl-1.0-drawpixels-color-index'])
    g(['gl-1.0-edgeflag'])
    g(['gl-1.0-edgeflag-const'])
    g(['gl-1.0-edgeflag-quads'])
    g(['gl-1.0-long-dlist'])
    g(['gl-1.0-rendermode-feedback'])
    g(['gl-1.0-front-invalidate-back'], run_concurrent=False)
    g(['gl-1.0-swapbuffers-behavior'], run_concurrent=False)
    g(['gl-1.0-polygon-line-aa'])
    g(['gl-1.0-blend-func'])
    g(['gl-1.0-fpexceptions'])
    g(['gl-1.0-ortho-pos'])
    g(['gl-1.0-readpixsanity'])
    g(['gl-1.0-logicop'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.2')) as g:
    g(['crash-texparameter-before-teximage'], run_concurrent=False)
    g(['draw-elements-vs-inputs'], run_concurrent=False)
    g(['two-sided-lighting-separate-specular'], run_concurrent=False)
    g(['levelclamp'], run_concurrent=False)
    g(['lodclamp'], run_concurrent=False)
    g(['lodclamp-between'], run_concurrent=False)
    g(['lodclamp-between-max'], run_concurrent=False)
    g(['mipmap-setup'], run_concurrent=False)
    g(['tex-skipped-unit'], run_concurrent=False)
    g(['tex3d'], run_concurrent=False)
    g(['tex3d-maxsize'], run_concurrent=False)
    g(['teximage-errors'], run_concurrent=False)
    g(['texture-packed-formats'], run_concurrent=False)
    g(['getteximage-targets', '3D'])
    add_msaa_visual_plain_tests(g, ['copyteximage', '3D'],
                                run_concurrent=False)
    add_texwrap_target_tests(g, '3D')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.3')) as g:
    g(['texunits'], run_concurrent=False)
    g(['tex-border-1'], run_concurrent=False)
    g(['tex3d-depth1'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.4')) as g:
    g(['fdo25614-genmipmap'], run_concurrent=False)
    g(['tex1d-2dborder'], run_concurrent=False)
    g(['blendminmax'], run_concurrent=False)
    g(['blendsquare'], run_concurrent=False)
    g(['gl-1.4-dlist-multidrawarrays'])
    g(['gl-1.4-polygon-offset'])
    g(['draw-batch'], run_concurrent=False)
    g(['stencil-wrap'], run_concurrent=False)
    g(['triangle-rasterization'], run_concurrent=False)
    g(['triangle-rasterization', '-use_fbo'], 'triangle-rasterization-fbo',
      run_concurrent=False)
    g(['triangle-rasterization-overdraw'], run_concurrent=False)
    g(['tex-miplevel-selection', '-nobias', '-nolod'],
      'tex-miplevel-selection')
    g(['tex-miplevel-selection', '-nobias'], 'tex-miplevel-selection-lod')
    g(['tex-miplevel-selection'], 'tex-miplevel-selection-lod-bias')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.4')) as g:
    add_msaa_visual_plain_tests(g, ['copy-pixels'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.5')) as g:
    g(['draw-elements'], run_concurrent=False)
    g(['draw-elements', 'user'], 'draw-elements-user', run_concurrent=False)
    g(['draw-vertices'], run_concurrent=False)
    g(['draw-vertices', 'user'], 'draw-vertices-user', run_concurrent=False)
    g(['isbufferobj'], run_concurrent=False)
    g(['depth-tex-compare'], run_concurrent=False)
    g(['gl-1.5-normal3b3s-invariance', 'GL_BYTE'],
      'normal3b3s-invariance-byte', run_concurrent=False)
    g(['gl-1.5-normal3b3s-invariance', 'GL_SHORT'],
      'normal3b3s-invariance-short', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 2.0')) as g:
    g(['attribs'])
    g(['gl-2.0-edgeflag'])
    g(['gl-2.0-edgeflag-immediate'])
    g(['gl-2.0-vertexattribpointer'])
    g(['gl-2.0-vertex-const-attr'])
    g(['attrib-assignments'], run_concurrent=False)
    g(['getattriblocation-conventional'], run_concurrent=False)
    g(['clip-flag-behavior'], run_concurrent=False)
    g(['vertex-program-two-side', 'enabled', 'front', 'back', 'front2',
        'back2'])
    g(['vertex-program-two-side', 'enabled', 'front', 'back', 'front2'])
    g(['vertex-program-two-side', 'enabled', 'front', 'back', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'front', 'back'])
    g(['vertex-program-two-side', 'enabled', 'front', 'front2', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'front', 'front2'])
    g(['vertex-program-two-side', 'enabled', 'front', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'front'])
    g(['vertex-program-two-side', 'enabled', 'back', 'front2', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'back', 'front2'])
    g(['vertex-program-two-side', 'enabled', 'back', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'back'])
    g(['vertex-program-two-side', 'enabled', 'front2', 'back2'])
    g(['vertex-program-two-side', 'enabled', 'front2'])
    g(['vertex-program-two-side', 'enabled', 'back2'])
    g(['vertex-program-two-side', 'enabled'])
    g(['vertex-program-two-side', 'front', 'back', 'front2', 'back2'])
    g(['vertex-program-two-side', 'front', 'back', 'front2'])
    g(['vertex-program-two-side', 'front', 'back', 'back2'])
    g(['vertex-program-two-side', 'front', 'back'])
    g(['vertex-program-two-side', 'front', 'front2', 'back2'])
    g(['vertex-program-two-side', 'front', 'front2'])
    g(['vertex-program-two-side', 'front', 'back2'])
    g(['vertex-program-two-side', 'front'])
    g(['vertex-program-two-side', 'back', 'front2', 'back2'])
    g(['vertex-program-two-side', 'back', 'front2'])
    g(['vertex-program-two-side', 'back', 'back2'])
    g(['vertex-program-two-side', 'back'])
    g(['vertex-program-two-side', 'front2', 'back2'])
    g(['vertex-program-two-side', 'front2'])
    g(['vertex-program-two-side', 'back2'])
    g(['vertex-program-two-side'])
    g(['clear-varray-2.0'], run_concurrent=False)
    g(['early-z'], run_concurrent=False)
    g(['occlusion-query-discard'], run_concurrent=False)
    g(['stencil-twoside'], run_concurrent=False)
    g(['vs-point_size-zero'], run_concurrent=False)
    g(['depth-tex-modes-glsl'], run_concurrent=False)
    g(['fragment-and-vertex-texturing'], run_concurrent=False)
    g(['incomplete-texture', 'glsl'], 'incomplete-texture-glsl')
    g(['tex3d-npot'], run_concurrent=False)
    g(['max-samplers'])
    g(['max-samplers', 'border'])
    g(['gl-2.0-active-sampler-conflict'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 2.1')) as g:
    g(['gl-2.1-minmax'], 'minmax')
    g(['gl-2.1-pbo'], 'pbo')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.0')) as g:
    g(['attribs', 'GL3'], 'attribs')
    g(['bindfragdata-invalid-parameters'])
    g(['bindfragdata-link-error'])
    g(['bindfragdata-nonexistent-variable'])
    g(['gl-3.0-bound-resource-limits'],
      'bound-resource-limits')
    g(['clearbuffer-depth'])
    g(['clearbuffer-depth-stencil'])
    g(['clearbuffer-display-lists'], run_concurrent=False)
    g(['clearbuffer-invalid-drawbuffer'])
    g(['clearbuffer-invalid-buffer'])
    g(['clearbuffer-mixed-format'])
    g(['clearbuffer-stencil'])
    g(['genmipmap-errors'])
    g(['getfragdatalocation'])
    g(['integer-errors'])
    g(['gl-3.0-multidrawarrays-vertexid'],
      'gl_VertexID used with glMultiDrawArrays')
    g(['gl-3.0-minmax'], 'minmax')
    g(['gl-3.0-render-integer'], 'render-integer')
    g(['gl-3.0-required-sized-texture-formats', '30'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-renderbuffer-attachment-formats', '30'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-texture-attachment-formats', '30'],
      'required-texture-attachment-formats')
    g(['gl-3.0-forward-compatible-bit', 'yes'],
      'forward-compatible-bit yes')
    g(['gl-3.0-forward-compatible-bit', 'no'],
      'forward-compatible-bit no')
    g(['gl-3.0-texture-integer'])
    g(['gl-3.0-vertexattribipointer'])
    g(['gl30basic'], run_concurrent=False)
    g(['array-depth-roundtrip'], run_concurrent=False)
    g(['depth-cube-map'], run_concurrent=False)
    g(['sampler-cube-shadow'], run_concurrent=False)
    g(['generatemipmap-cubemap'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.1')) as g:
    g(['gl-3.1-default-vao'], 'default-vao')
    g(['gl-3.1-draw-buffers-errors'], 'draw-buffers-errors')
    g(['gl-3.1-genned-names'], 'genned-names')
    g(['gl-3.1-minmax'], 'minmax')
    g(['gl-3.1-vao-broken-attrib'], 'vao-broken-attrib')
    g(['gl-3.0-required-renderbuffer-attachment-formats', '31'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-sized-texture-formats', '31'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-texture-attachment-formats', '31'],
      'required-texture-attachment-formats')
    for subtest in ['generated', 'written', 'flush']:
        g(['gl-3.1-primitive-restart-xfb', subtest],
          'primitive-restart-xfb {0}'.format(subtest))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.2')) as g:
    g(['glsl-resource-not-bound', '1D'])
    g(['glsl-resource-not-bound', '2D'])
    g(['glsl-resource-not-bound', '3D'])
    g(['glsl-resource-not-bound', '2DRect'])
    g(['glsl-resource-not-bound', '1DArray'])
    g(['glsl-resource-not-bound', '2DArray'])
    g(['glsl-resource-not-bound', '2DMS'])
    g(['glsl-resource-not-bound', '2DMSArray'])
    g(['glsl-resource-not-bound', 'Buffer'])
    g(['glsl-resource-not-bound', 'Cube'])
    g(['gl-3.2-basevertex-vertexid'],
      'gl_VertexID used with glMultiDrawElementsBaseVertex')
    g(['gl-3.2-minmax'], 'minmax')
    g(['gl-3.2-clear-no-buffers'], 'clear-no-buffers')
    g(['gl-3.2-depth-tex-sampling'], 'depth-tex-sampling')
    g(['gl-3.2-get-buffer-parameter-i64v'], 'get-buffer-parameter-i64v')
    g(['gl-3.2-get-integer-64iv'], 'get-integer-64iv')
    g(['gl-3.2-get-integer-64v'], 'get-integer-64v')
    g(['gl-3.2-pointsprite-coord'], 'pointsprite-coord')
    g(['gl-3.2-pointsprite-origin'], 'pointsprite-origin')
    g(['gl-coord-replace-doesnt-eliminate-frag-tex-coords'],
      'coord-replace-doesnt-eliminate-frag-tex-coords')
    g(['gl-get-active-attrib-returns-all-inputs'],
      'get-active-attrib-returns-all-inputs')
    g(['gl-3.2-texture-border-deprecated'], 'texture-border-deprecated')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.2', 'layered-rendering')) as g:
    g(['gl-3.2-layered-rendering-blit'], 'blit')
    g(['gl-3.2-layered-rendering-clear-color'], 'clear-color')
    g(['gl-3.2-layered-rendering-clear-color-mismatched-layer-count'],
      'clear-color-mismatched-layer-count')
    g(['gl-3.2-layered-rendering-clear-depth'], 'clear-depth')
    g(['gl-3.2-layered-rendering-framebuffertexture'], 'framebuffertexture')
    g(['gl-3.2-layered-rendering-framebuffertexture-buffer-textures'],
      'framebuffertexture-buffer-textures')
    g(['gl-3.2-layered-rendering-framebuffertexture-defaults'],
      'framebuffertexture-defaults')
    g(['gl-3.2-layered-rendering-readpixels'], 'readpixels')
    g(['gl-3.2-layered-rendering-framebuffer-layer-attachment-mismatch'],
      'framebuffer-layer-attachment-mismatch')
    g(['gl-3.2-layered-rendering-framebuffer-layer-complete'],
      'framebuffer-layer-complete')
    g(['gl-3.2-layered-rendering-framebuffer-layer-count-mismatch'],
      'framebuffer-layer-count-mismatch')
    g(['gl-3.2-layered-rendering-framebuffer-layered-attachments'],
      'framebuffer-layered-attachments')
    g(['gl-3.2-layered-rendering-gl-layer'], 'gl-layer')
    g(['gl-3.2-layered-rendering-gl-layer-cube-map'], 'gl-layer-cube-map')
    g(['gl-3.2-layered-rendering-gl-layer-not-layered'],
      'gl-layer-not-layered')
    g(['gl-3.2-layered-rendering-gl-layer-render'], 'gl-layer-render')

    for texture_type in ['3d', '2d_array', '2d_multisample_array', '1d_array',
                         'cube_map', 'cube_map_array']:
        for test_type in ['single_level', 'mipmapped']:
            if texture_type == '2d_multisample_array' and \
                            test_type == 'mipmapped':
                continue
            g(['gl-3.2-layered-rendering-clear-color-all-types', texture_type,
               test_type],
              'clear-color-all-types {} {}'.format(texture_type, test_type))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.3')) as g:
    g(['gl-3.3-minmax'], 'minmax')
    g(['gl-3.0-required-renderbuffer-attachment-formats', '33'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-sized-texture-formats', '33'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-texture-attachment-formats', '33'],
      'required-texture-attachment-formats')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.2')) as g:
    g(['gl-3.0-required-renderbuffer-attachment-formats', '42'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-sized-texture-formats', '42'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-texture-attachment-formats', '42'],
      'required-texture-attachment-formats')
    g(['gl-4.4-max_vertex_attrib_stride'], 'gl-max-vertex-attrib-stride')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.4')) as g:
    g(['tex-errors'])

# Group spec/glsl-es-1.00
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-es-1.00')) as g:
    g(['built-in-constants_gles2',
       os.path.join(TESTS_DIR, 'spec', 'glsl-es-1.00',
                    'minimum-maximums.txt')],
      'built-in constants')

# Group spec/glsl-1.10
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution')) as g:
    g(['glsl-render-after-bad-attach'])
    g(['glsl-1.10-fragdepth'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution', 'clipping')) as g:
    for mode in ['fixed', 'pos_clipvert', 'clipvert_pos']:
        g(['clip-plane-transformation', mode],
          'clip-plane-transformation {}'.format(mode))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution',
                        'varying-packing')) as g:
    for type_ in ['int', 'uint', 'float', 'vec2', 'vec3', 'vec4', 'ivec2',
                  'ivec3', 'ivec4', 'uvec2', 'uvec3', 'uvec4', 'mat2', 'mat3',
                  'mat4', 'mat2x3', 'mat2x4', 'mat3x2', 'mat3x4', 'mat4x2',
                  'mat4x3']:
        for arrayspec in ['array', 'separate']:
            g(['varying-packing-simple', type_, arrayspec],
              'simple {} {}'.format(type_, arrayspec))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10')) as g:
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-1.10', 'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'api')) as g:
    g(['getactiveattrib', '110'], 'getactiveattrib 110')

# Group spec/glsl-1.20
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20')) as g:
    g(['glsl-1.20-getactiveuniform-constant'])
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-1.20', 'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20', 'api')) as g:
    g(['getactiveattrib', '120'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20', 'recursion')) as g:
    g(['recursion', '-rlmit', '268435456', 'simple'], 'simple',
      run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'unreachable'], 'unreachable',
      run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'unreachable-constant-folding'],
      'unreachable-constant-folding', run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'indirect'], 'indirect',
      run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'indirect-separate'],
      'indirect-separate', run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'indirect-complex'],
      'indirect-complex', run_concurrent=False)
    g(['recursion', '-rlmit', '268435456', 'indirect-complex-separate'],
      'indirect-complex-separate', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20', 'execution')) as g:
    for test in ['1D', '2D', '3D', 'Cube', '1DShadow', '2DShadow']:
        g(['tex-miplevel-selection', 'GL2:texture()', test])
        g(['tex-miplevel-selection', 'GL2:texture(bias)', test])

    for test in ['1D', '1D_ProjVec4', '2D', '2D_ProjVec4', '3D', '1DShadow',
                 '2DShadow']:
        g(['tex-miplevel-selection', 'GL2:textureProj', test])
        g(['tex-miplevel-selection', 'GL2:textureProj(bias)', test])

textureSize_samplers_130 = [
    'sampler1D', 'sampler2D', 'sampler3D', 'samplerCube', 'sampler1DShadow',
    'sampler2DShadow', 'samplerCubeShadow', 'sampler1DArray', 'sampler2DArray',
    'sampler1DArrayShadow', 'sampler2DArrayShadow', 'isampler1D', 'isampler2D',
    'isampler3D', 'isamplerCube', 'isampler1DArray', 'isampler2DArray',
    'usampler1D', 'usampler2D', 'usampler3D', 'usamplerCube',
    'usampler1DArray', 'usampler2DArray']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.30'

    # textureSize():
    # These will be added in the textureSize_samplers_140 loop for gs, because
    # it is a special case and is actually 1.50 feature.
    # Despite the differences in the commands lines of the two lists (this one
    # does not add '140', the two tests are equivalent.
    if stage is not 'gs':
        for sampler in textureSize_samplers_130:
            profile.test_list[grouptools.join(
                'spec', 'glsl-{}'.format(version), 'execution', 'textureSize',
                '{}-textureSize-{}'.format(stage, sampler))] = PiglitGLTest(
                    ['textureSize', stage, sampler])

    # texelFetch():
    for sampler in ['sampler1D', 'sampler2D', 'sampler3D', 'sampler1DArray',
                    'sampler2DArray', 'isampler1D', 'isampler2D', 'isampler3D',
                    'isampler1DArray', 'isampler2DArray', 'usampler1D',
                    'usampler2D', 'usampler3D', 'usampler1DArray',
                    'usampler2DArray']:
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', stage, sampler])
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'texelFetchOffset',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', 'offset', stage, sampler])

    # texelFetch() with EXT_texture_swizzle mode "b0r1":
    for type in ['i', 'u', '']:
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}sampler2Darray-swizzle'.format(stage, type))] = \
            PiglitGLTest(['texelFetch', stage, '{}sampler2DArray'.format(type),
                          'b0r1'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30')) as g:
    g(['glsl-1.30-texel-offset-limits'], 'texel-offset-limits')
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-1.30', 'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'execution')) as g:
    g(['texelFetch', 'fs', 'sampler1D', '1-513'])
    g(['texelFetch', 'fs', 'sampler1DArray', '1x71-501x71'])
    g(['texelFetch', 'fs', 'sampler1DArray', '1x281-501x281'])
    g(['texelFetch', 'fs', 'sampler1DArray', '71x1-71x281'])
    g(['texelFetch', 'fs', 'sampler1DArray', '281x1-281x281'])
    g(['texelFetch', 'fs', 'sampler2D', '1x71-501x71'])
    g(['texelFetch', 'fs', 'sampler2D', '1x281-501x281'])
    g(['texelFetch', 'fs', 'sampler2D', '71x1-71x281'])
    g(['texelFetch', 'fs', 'sampler2D', '281x1-281x281'])
    g(['texelFetch', 'fs', 'sampler3D', '1x129x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler3D', '98x1x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler3D', '98x129x1-98x129x9'])
    g(['texelFetch', 'fs', 'sampler2DArray', '1x129x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler2DArray', '98x1x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler2DArray', '98x129x1-98x129x9'])
    g(['fs-texelFetch-2D'])
    g(['fs-texelFetchOffset-2D'])
    g(['fs-textureOffset-2D'])
    g(['fs-discard-exit-2'])
    g(['vertexid-beginend'])
    g(['vertexid-drawarrays'])
    g(['vertexid-drawelements'])
    g(['fs-execution-ordering'])

    for stage in ['1D', '2D', '3D', 'Cube', '1DShadow', '2DShadow', '1DArray',
                  '2DArray', '1DArrayShadow', 'CubeArray']:
        g(['tex-miplevel-selection', 'textureLod', stage])

    for stage in ['1D', '2D', '3D', 'Cube', '1DShadow', '2DShadow',
                  'CubeShadow', '1DArray', '2DArray', 'CubeArray',
                  '1DArrayShadow']:
        g(['tex-miplevel-selection', 'texture(bias)', stage])

    for stage in ['1D', '2D', '3D', 'Cube', '1DShadow', '2DShadow',
                  'CubeShadow', '1DArray', '2DArray', 'CubeArray',
                  '1DArrayShadow', '2DArrayShadow', '2DRect', '2DRectShadow',
                  'CubeArrayShadow']:
        g(['tex-miplevel-selection', 'texture()', stage])

    for stage in ['1D', '2D', '3D', '2DRect', '2DRectShadow', '1DShadow',
                  '2DShadow', '1DArray', '2DArray', '1DArrayShadow',
                  '2DArrayShadow']:
        g(['tex-miplevel-selection', 'textureOffset', stage])

    for stage in ['1D', '2D', '3D', '1DShadow', '2DShadow', '1DArray',
                  '2DArray', '1DArrayShadow']:
        g(['tex-miplevel-selection', 'textureOffset(bias)', stage])

    for stage in ['1D', '1D_ProjVec4', '2D', '2D_ProjVec4', '3D', '1DShadow',
                  '2DShadow', '2DRect', '2DRect_ProjVec4', '2DRectShadow']:
        g(['tex-miplevel-selection', 'textureProj', stage])

    for stage in ['1D', '1D_ProjVec4', '2D', '2D_ProjVec4', '3D', '1DShadow',
                  '2DShadow']:
        g(['tex-miplevel-selection', 'textureProj(bias)', stage])

    for stage in ['1D', '1D_ProjVec4', '2D', '2D_ProjVec4', '3D', '2DRect',
                  '2DRect_ProjVec4', '2DRectShadow', '1DShadow', '2DShadow']:
        g(['tex-miplevel-selection', 'textureProjOffset', stage])

    for stage in ['1D', '1D_ProjVec4', '2D', '2D_ProjVec4', '3D', '1DShadow',
                  '2DShadow']:
        g(['tex-miplevel-selection', 'textureProjOffset(bias)', stage])

    for stage in ['1D', '2D', '3D', '1DShadow', '2DShadow', '1DArray',
                  '2DArray', '1DArrayShadow']:
        g(['tex-miplevel-selection', 'textureLodOffset', stage])

    for stage in ['1D', '2D', '3D', '1D_ProjVec4', '2D_ProjVec4', '1DShadow',
                  '2DShadow']:
        g(['tex-miplevel-selection', 'textureProjLod', stage])
        g(['tex-miplevel-selection', 'textureProjLodOffset', stage])

    for stage in ['1D', '2D', '3D', 'Cube', '2DRect', '2DRectShadow',
                  '1DShadow', '2DShadow', 'CubeShadow', '1DArray', '2DArray',
                  '1DArrayShadow', '2DArrayShadow', 'CubeArray']:
        g(['tex-miplevel-selection', 'textureGrad', stage])

    for stage in ['1D', '2D', '3D', '2DRect', '2DRectShadow', '1DShadow',
                  '2DShadow', '1DArray', '2DArray', '1DArrayShadow',
                  '2DArrayShadow']:
        g(['tex-miplevel-selection', 'textureGradOffset', stage])

    for stage in ['1D', '2D', '3D', '1D_ProjVec4', '2D_ProjVec4', '2DRect',
                  '2DRect_ProjVec4', '1DShadow', '2DShadow', '2DRectShadow']:
        g(['tex-miplevel-selection', 'textureProjGrad', stage])
        g(['tex-miplevel-selection', 'textureProjGradOffset', stage])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'linker', 'clipping')) as g:
    g(['mixing-clip-distance-and-clip-vertex-disallowed'],
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'execution')) as g:
    for arg in ['vs_basic', 'vs_xfb', 'vs_fbo', 'fs_basic', 'fs_fbo']:
        g(['isinf-and-isnan', arg], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'execution', 'clipping')) as g:
    g(['max-clip-distances'], run_concurrent=False)
    g(['clip-plane-transformation', 'pos'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'api')) as g:
    g(['getactiveattrib', '130'], 'getactiveattrib 130', run_concurrent=False)

# Group spec/glsl-1.40
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.40')) as g:
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-1.40', 'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.40', 'execution')) as g:
    g(['glsl-1.40-tf-no-position'], 'tf-no-position')

textureSize_samplers_140 = textureSize_samplers_130 + [
    'sampler2DRect', 'isampler2DRect', 'sampler2DRectShadow', 'samplerBuffer',
    'isamplerBuffer', 'usamplerBuffer']
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.40'
    # textureSize():
    for sampler in textureSize_samplers_140:
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'textureSize',
            '{}-textureSize-{}'.format(stage, sampler))] = PiglitGLTest(
                ['textureSize', '140', stage, sampler])
    # texelFetch():
    for sampler in ['sampler2DRect', 'usampler2DRect', 'isampler2DRect']:
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'texelFetch',
            '{}-texelFetch-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', '140', stage, sampler])
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'texelFetchOffset',
            '{}-{}'.format(stage, sampler))] = PiglitGLTest(
                ['texelFetch', 'offset', '140', stage, sampler])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.50')) as g:
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-1.50', 'minimum-maximums.txt')],
      'built-in constants')
    g(['glsl-1.50-gs-emits-too-few-verts'], 'gs-emits-too-few-verts')
    g(['glsl-1.50-geometry-end-primitive-optional-with-points-out'],
      'gs-end-primitive-optional-with-points-out')
    g(['glsl-1.50-getshaderiv-may-return-GS'], 'getshaderiv-may-return-GS')
    g(['glsl-1.50-query-gs-prim-types'], 'query-gs-prim-types')
    g(['glsl-1.50-transform-feedback-type-and-size'],
      'transform-feedback-type-and-size')
    g(['glsl-1.50-transform-feedback-vertex-id'],
      'transform-feedback-vertex-id')
    g(['glsl-1.50-transform-feedback-builtins'], 'transform-feedback-builtins')

    for draw in ['', 'indexed']:
        for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                     'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
            g(['arb_geometry_shader4-ignore-adjacent-vertices',
               'core', draw, prim], run_concurrent=False)

    for subtest in ['unnamed', 'named', 'array']:
        g(['glsl-1.50-interface-block-centroid', subtest])

    for layout_type in ['points', 'lines', 'lines_adjacency', 'triangles',
                        'triangles_adjacency']:
        g(['glsl-1.50-gs-mismatch-prim-type', layout_type])

    for layout in ['points', 'lines', 'lines_adjacency', 'triangles',
                   'triangles_adjacency', 'line_strip', 'triangle_strip']:
        g(['glsl-1.50-gs-input-layout-qualifiers', layout])
        g(['glsl-1.50-gs-output-layout-qualifiers', layout])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.50', 'execution')) as g:
    g(['glsl-1.50-get-active-attrib-array'], 'get-active-attrib-array')
    g(['glsl-1.50-interface-blocks-api-access-members'],
      'interface-blocks-api-access-members')
    g(['glsl-1.50-vs-input-arrays'], 'vs-input-arrays')
    g(['glsl-1.50-vs-named-block-no-modify'], 'vs-named-block-no-modify')

# max_vertices of 32 and 128 are important transition points for
# mesa/i965 (they are the number of bits in a float and a vec4,
# respectively), so test around there.  Also test 0, which means the
# maximum number of geometry shader output vertices supported by the
# hardware.
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.50', 'execution', 'geometry')) as g:
    for i in [31, 32, 33, 34, 127, 128, 129, 130, 0]:
        g(['glsl-1.50-geometry-end-primitive', str(i)],
          'end-primitive {0}'.format(i))

    for prim_type in ['GL_POINTS', 'GL_LINE_LOOP', 'GL_LINE_STRIP', 'GL_LINES',
                      'GL_TRIANGLES', 'GL_TRIANGLE_STRIP', 'GL_TRIANGLE_FAN',
                      'GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                      'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        g(['glsl-1.50-geometry-primitive-types', prim_type],
          'primitive-types {0}'.format(prim_type))

        for restart_index in ['ffs', 'other']:
            g(['glsl-1.50-geometry-primitive-id-restart', prim_type,
               restart_index],
              'primitive-id-restart {0} {1}'.format(prim_type, restart_index))

    for prim_type in ['GL_TRIANGLE_STRIP', 'GL_TRIANGLE_STRIP_ADJACENCY']:
        for restart_index in ['ffs', 'other']:
            g(['glsl-1.50-geometry-tri-strip-ordering-with-prim-restart',
               prim_type, restart_index],
              'tri-strip-ordering-with-prim-restart {0} {1}'.format(
                  prim_type, restart_index))

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'glsl-3.30')) as g:
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'glsl-3.30', 'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'glsl-es-3.00')) as g:
    g(['built-in-constants_gles3',
       os.path.join(TESTS_DIR, 'spec', 'glsl-es-3.00',
                    'minimum-maximums.txt')],
      'built-in constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-es-3.00', 'execution')) as g:
    g(['varying-struct-centroid_gles3'])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'glsl-es-3.10')) as g:
    g(['built-in-constants_gles3',
       os.path.join(TESTS_DIR, 'spec', 'glsl-es-3.10',
                    'minimum-maximums.txt')],
      'built-in constants')

# AMD_performance_monitor
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'AMD_performance_monitor')) as g:
    g(['amd_performance_monitor_api'], 'api', run_concurrent=False)
    g(['amd_performance_monitor_measure'], 'measure', run_concurrent=False)

# Group ARB_point_sprite
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_point_sprite')) as g:
    g(['point-sprite'], run_concurrent=False)

# Group ARB_tessellation_shader
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_tessellation_shader')) as g:
    g(['arb_tessellation_shader-get-tcs-params'])
    g(['arb_tessellation_shader-get-tes-params'])
    g(['arb_tessellation_shader-minmax'])
    g(['arb_tessellation_shader-invalid-get-program-params'])
    g(['arb_tessellation_shader-invalid-patch-vertices-range'])
    g(['arb_tessellation_shader-invalid-primitive'])
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'arb_tessellation_shader',
                    'minimum-maximums.txt')],
      'built-in-constants')

# Group ARB_texture_multisample
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_texture_multisample')) as g:
    g(['arb_texture_multisample-minmax'])
    g(['texelFetch', 'fs', 'sampler2DMS', '4', '1x71-501x71'])
    g(['texelFetch', 'fs', 'sampler2DMS', '4', '1x130-501x130'])
    g(['texelFetch', 'fs', 'sampler2DMS', '4', '71x1-71x130'])
    g(['texelFetch', 'fs', 'sampler2DMS', '4', '281x1-281x130'])
    g(['texelFetch', 'fs', 'sampler2DMSArray', '4', '1x129x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler2DMSArray', '4', '98x1x9-98x129x9'])
    g(['texelFetch', 'fs', 'sampler2DMSArray', '4', '98x129x1-98x129x9'])
    g(['arb_texture_multisample-texstate'])
    g(['arb_texture_multisample-errors'])
    g(['arb_texture_multisample-texelfetch', '2'])
    g(['arb_texture_multisample-texelfetch', '4'])
    g(['arb_texture_multisample-texelfetch', '8'])
    g(['arb_texture_multisample-sample-mask'])
    g(['arb_texture_multisample-sample-mask-value'])
    g(['arb_texture_multisample-sample-mask-execution'])
    g(['arb_texture_multisample-sample-mask-execution', '-tex'])
    g(['arb_texture_multisample-negative-max-samples'])
    g(['arb_texture_multisample-teximage-3d-multisample'])
    g(['arb_texture_multisample-teximage-2d-multisample'])
    g(['arb_texture_multisample-sample-depth'])

samplers_atm = ['sampler2DMS', 'isampler2DMS', 'usampler2DMS',
                'sampler2DMSArray', 'isampler2DMSArray', 'usampler2DMSArray']
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'fb-completeness')) as g:

    for sample_count in (str(x) for x in MSAA_SAMPLE_COUNTS):
        # fb-completeness
        g(['arb_texture_multisample-fb-completeness', sample_count],
          sample_count)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample', 'texelFetch')) as g:

    stages = ['vs', 'gs', 'fs']
    for sampler, stage, sample_count in itertools.product(
            samplers_atm, stages, (str(x) for x in MSAA_SAMPLE_COUNTS)):
        g(['texelFetch', stage, sampler, sample_count],
          '{}-{}-{}'.format(sample_count, stage, sampler))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'sample-position')) as g:
    # sample positions
    for sample_count in (str(x) for x in MSAA_SAMPLE_COUNTS):
        g(['arb_texture_multisample-sample-position', sample_count],
          sample_count)


with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'textureSize')) as g:

    stages = ['vs', 'gs', 'fs']
    for stage, sampler in itertools.product(stages, samplers_atm):
        g(['textureSize', stage, sampler],
          '{}-textureSize-{}'.format(stage, sampler))

# Group ARB_texture_gather
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_gather')) as g:
    stages = ['vs', 'fs']
    comps = ['r', 'rg', 'rgb', 'rgba']
    types = ['unorm', 'float', 'int', 'uint']
    samplers = ['2D', '2DArray', 'Cube', 'CubeArray']
    for stage, comp, type_, sampler in itertools.product(
            stages, comps, types, samplers):
        for swiz in ['red', 'green', 'blue', 'alpha'][:len(comp)] + ['', 'zero', 'one']:
            for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset']:
                testname = grouptools.join(
                    func, '{}-{}-{}-{}-{}'.format(
                        stage, comp,
                        swiz if swiz else 'none',
                        type_, sampler))
                g(['textureGather', stage,
                   'offset' if func == 'textureGatherOffset' else '',
                   comp, swiz, type_, sampler], testname)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_stencil_texturing')) as g:
    g(['arb_stencil_texturing-draw'], 'draw')

# Group ARB_sync
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_sync')) as g:
    g(['arb_sync-client-wait-errors'], 'ClientWaitSync-errors')
    g(['arb_sync-delete'], 'DeleteSync')
    g(['arb_sync-fence-sync-errors'], 'FenceSync-errors')
    g(['arb_sync-get-sync-errors'], 'GetSynciv-errors')
    g(['arb_sync-is-sync'], 'IsSync')
    g(['arb_sync-repeat-wait'], 'repeat-wait')
    g(['arb_sync-sync-initialize'], 'sync-initialize')
    g(['arb_sync-timeout-zero'], 'timeout-zero')
    g(['arb_sync-WaitSync-errors'], 'WaitSync-errors')
    g(['arb_sync-ClientWaitSync-timeout'], 'ClientWaitSync-timeout')
    g(['sync_api'], run_concurrent=False)

# Group ARB_ES2_compatibility
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_ES2_compatibility')) as g:
    g(['arb_es2_compatibility-depthrangef'], run_concurrent=False)
    g(['arb_es2_compatibility-drawbuffers'], run_concurrent=False)
    g(['arb_es2_compatibility-getshaderprecisionformat'], run_concurrent=False)
    g(['arb_es2_compatibility-maxvectors'], run_concurrent=False)
    g(['arb_es2_compatibility-shadercompiler'], run_concurrent=False)
    g(['arb_es2_compatibility-releaseshadercompiler'], run_concurrent=False)
    g(['arb_es2_compatibility-fixed-type'], run_concurrent=False)
    g(['fbo-missing-attachment-clear'], run_concurrent=False)
    g(['fbo-missing-attachment-blit', 'es2', 'to'],
      'FBO blit to missing attachment (ES2 completeness rules)')
    g(['fbo-missing-attachment-blit', 'es2', 'from'],
      'FBO blit from missing attachment (ES2 completeness rules)')
    g(['arb_get_program_binary-overrun', 'shader'],
      'NUM_SHADER_BINARY_FORMATS over-run check')
    add_texwrap_format_tests(g, 'GL_ARB_ES2_compatibility')
    add_fbo_formats_tests(g, 'GL_ARB_ES2_compatibility')


# Group ARB_get_program_binary
with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_get_program_binary')) as g:
    g(['arb_get_program_binary-api-errors'],
      'misc. API error checks')
    g(['arb_get_program_binary-overrun', 'program'],
      'NUM_PROGRAM_BINARY_FORMATS over-run check')
    g(['arb_get_program_binary-retrievable_hint'],
      'PROGRAM_BINARY_RETRIEVABLE_HINT')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_depth_clamp')) as g:
    g(['depth_clamp'], run_concurrent=False)
    g(['depth-clamp-range'], run_concurrent=False)
    g(['depth-clamp-status'], run_concurrent=False)

# Group ARB_draw_elements_base_vertex
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_draw_elements_base_vertex')) as g:
    g(['arb_draw_elements_base_vertex-dlist'], 'dlist')
    g(['arb_draw_elements_base_vertex-drawelements'], run_concurrent=False)
    g(['arb_draw_elements_base_vertex-drawelements', 'user_varrays'],
      'arb_draw_elements_base_vertex-drawelements-user_varrays',
      run_concurrent=False)
    g(['arb_draw_elements_base_vertex-negative-index'], run_concurrent=False)
    g(['arb_draw_elements_base_vertex-bounds'], run_concurrent=False)
    g(['arb_draw_elements_base_vertex-negative-index', 'user_varrays'],
      'arb_draw_elements_base_vertex-negative-index-user_varrays',
      run_concurrent=False)
    g(['arb_draw_elements_base_vertex-drawelements-instanced'],
      run_concurrent=False)
    g(['arb_draw_elements_base_vertex-drawrangeelements'],
      run_concurrent=False)
    g(['arb_draw_elements_base_vertex-multidrawelements'],
      run_concurrent=False)

# Group ARB_draw_instanced
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_draw_instanced')) as g:
    g(['arb_draw_instanced-dlist'], 'dlist')
    g(['arb_draw_instanced-elements'], 'elements')
    g(['arb_draw_instanced-negative-arrays-first-negative'],
      'negative-arrays-first-negative')
    g(['arb_draw_instanced-negative-elements-type'],
      'negative-elements-type')
    g(['arb_draw_instanced-drawarrays'], run_concurrent=False)

# Group ARB_draw_indirect
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_draw_indirect')) as g:
    g(['arb_draw_indirect-api-errors'])
    g(['arb_draw_indirect-draw-arrays'])
    g(['arb_draw_indirect-draw-arrays-prim-restart'])
    g(['arb_draw_indirect-draw-elements'])
    g(['arb_draw_indirect-draw-arrays-base-instance'])
    g(['arb_draw_indirect-draw-elements-base-instance'])
    g(['arb_draw_indirect-draw-elements-prim-restart'])
    g(['arb_draw_indirect-draw-elements-prim-restart-ugly'])
    g(['arb_draw_indirect-draw-arrays-instances'])
    g(['arb_draw_indirect-vertexid'],
      'gl_VertexID used with glDrawArraysIndirect')
    g(['arb_draw_indirect-vertexid', 'elements'],
      'gl_VertexID used with glDrawElementsIndirect')

# Group ARB_fragment_program
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_fragment_program')) as g:
    g(['arb_fragment_program-minmax'], 'minmax')
    g(['fp-abs-01'], run_concurrent=False)
    g(['fp-fog'], run_concurrent=False)
    g(['fp-formats'], run_concurrent=False)
    g(['fp-fragment-position'], run_concurrent=False)
    g(['fp-incomplete-tex'], run_concurrent=False)
    g(['fp-indirections'], run_concurrent=False)
    g(['fp-indirections2'], run_concurrent=False)
    g(['fp-kil'], run_concurrent=False)
    g(['fp-lit-mask'], run_concurrent=False)
    g(['fp-lit-src-equals-dst'], run_concurrent=False)
    g(['fp-long-alu'], run_concurrent=False)
    g(['fp-set-01'], run_concurrent=False)
    g(['trinity-fp1'], run_concurrent=False)
    g(['arb_fragment_program-sparse-samplers'], 'sparse-samplers')
    g(['incomplete-texture', 'arb_fp'], 'incomplete-texture-arb_fp')
    add_vpfpgeneric(g, 'fdo30337a')
    add_vpfpgeneric(g, 'fdo30337b')
    add_vpfpgeneric(g, 'fdo38145')
    add_vpfpgeneric(g, 'fp-cmp')
    add_vpfpgeneric(g, 'fp-dst-aliasing-1')
    add_vpfpgeneric(g, 'fp-dst-aliasing-2')
    add_vpfpgeneric(g, 'fp-ex2-sat')
    add_vpfpgeneric(g, 'fp-two-constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'NV_fragment_program_option')) as g:
    g(['fp-abs-02'], run_concurrent=False)
    g(['fp-condition_codes-01'], run_concurrent=False)
    g(['fp-rfl'], run_concurrent=False)
    g(['fp-set-02'], run_concurrent=False)
    g(['fp-unpack-01'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_fragment_coord_conventions')) as g:
    add_vpfpgeneric(g, 'fp-arb-fragment-coord-conventions-none')
    add_vpfpgeneric(g, 'fp-arb-fragment-coord-conventions-integer')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ATI_fragment_shader')) as g:
    g(['ati-fs-bad-delete'])

# Group ARB_framebuffer_object
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_object')) as g:
    g(['same-attachment-glFramebufferTexture2D-GL_DEPTH_STENCIL_ATTACHMENT'])
    g(['same-attachment-glFramebufferRenderbuffer-GL_DEPTH_STENCIL_ATTACHMENT'])
    g(['fdo28551'], run_concurrent=False)
    g(['fbo-alpha'])
    g(['fbo-blit-stretch'], run_concurrent=False)
    g(['fbo-blit-scaled-linear'])
    g(['fbo-attachments-blit-scaled-linear'])
    g(['fbo-deriv'])
    g(['fbo-luminance-alpha'])
    g(['fbo-getframebufferattachmentparameter-01'])
    g(['fbo-gl_pointcoord'])
    g(['fbo-incomplete'])
    g(['fbo-incomplete-invalid-texture'])
    g(['fbo-incomplete-texture-01'])
    g(['fbo-incomplete-texture-02'])
    g(['fbo-incomplete-texture-03'])
    g(['fbo-incomplete-texture-04'])
    g(['fbo-mipmap-copypix'])
    g(['fbo-viewport'], run_concurrent=False)
    g(['fbo-missing-attachment-blit', 'to'], 'FBO blit to missing attachment')
    g(['fbo-missing-attachment-blit', 'from'],
      'FBO blit from missing attachment')
    g(['fbo-scissor-blit', 'fbo'], 'fbo-scissor-blit fbo')
    g(['fbo-scissor-blit', 'window'], 'fbo-scissor-blit window',
      run_concurrent=False)
    g(['fbo-tex-rgbx'], 'fbo-tex-rgbx')
    g(['arb_framebuffer_object-mixed-buffer-sizes'], 'mixed-buffer-sizes')
    g(['arb_framebuffer_object-negative-readpixels-no-rb'],
      'negative-readpixels-no-rb')
    g(['fbo-drawbuffers-none', 'glClear'])
    g(['fbo-drawbuffers-none', 'glClearBuffer'])
    g(['fbo-drawbuffers-none', 'gl_FragColor'])
    g(['fbo-drawbuffers-none', 'gl_FragData'])
    g(['fbo-drawbuffers-none', 'use_frag_out'])
    g(['fbo-drawbuffers-none', 'glColorMaskIndexed'])
    g(['fbo-drawbuffers-none', 'glBlendFunci'])
    g(['fbo-drawbuffers-none', 'glDrawPixels'])
    g(['fbo-drawbuffers-none', 'glBlitFramebuffer'])
    g(['fbo-generatemipmap-cubemap'])
    g(['fbo-generatemipmap-cubemap', 'RGB9_E5'])
    g(['fbo-generatemipmap-cubemap', 'S3TC_DXT1'])
    g(['fbo-generatemipmap-1d'])
    g(['fbo-generatemipmap-1d', 'RGB9_E5'])
    g(['fbo-generatemipmap-3d'])
    g(['fbo-generatemipmap-3d', 'RGB9_E5'])
    for format in ('rgba', 'depth', 'stencil'):
        for test_mode in ('draw', 'read'):
            g(['framebuffer-blit-levels', test_mode, format],
              'framebuffer-blit-levels {} {}'.format(test_mode, format))

# Group ARB_framebuffer_sRGB
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_sRGB')) as g:
    for backing_type in ('texture', 'renderbuffer'):
        for srgb_types in ('linear', 'srgb', 'linear_to_srgb',
                           'srgb_to_linear'):
            for blit_type in ('single_sampled', 'upsample', 'downsample',
                              'msaa', 'scaled'):
                for framebuffer_srgb_setting in ('enabled',
                                                 'disabled'):
                    g(['arb_framebuffer_srgb-blit', backing_type, srgb_types,
                        blit_type, framebuffer_srgb_setting],
                      'blit {} {} {} {}'.format(
                          backing_type, srgb_types, blit_type,
                          framebuffer_srgb_setting))
    g(['framebuffer-srgb'], run_concurrent=False)
    g(['arb_framebuffer_srgb-clear'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader5')) as g:
    stages = ['vs', 'fs']
    types = ['unorm', 'float', 'int', 'uint']
    comps = ['r', 'rg', 'rgb', 'rgba']
    samplers = ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']
    for stage, type_, comp, sampler in itertools.product(
            stages, types, comps, samplers):
        for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets']:
            for cs in xrange(len(comp)):
                assert cs <= 3
                address_mode = 'clamp' if sampler == '2DRect' else 'repeat'
                cmd = ['textureGather', stage,
                       'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                       comp, str(cs), type_, sampler, address_mode]
                testname = grouptools.join(func, '{}-{}-{}-{}-{}'.format(
                    stage, comp, cs, type_, sampler))
                g(cmd, testname)

                if func == 'textureGatherOffset':
                    # also add a constant offset version.
                    testname = grouptools.join(
                        func, '{}-{}-{}-{}-{}-const'.format(
                            stage, comp, cs, type_, sampler))
                    cmd = ['textureGather', stage, 'offset',
                           comp, str(cs), type_, sampler, address_mode]
                    g(cmd, testname)

    # test shadow samplers
    samplers = ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']
    for stage, sampler in itertools.product(stages, samplers):
        for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets']:
            testname = grouptools.join(func, '{}-r-none-shadow-{}'.format(
                stage, sampler))
            cmd = ['textureGather', stage, 'shadow', 'r',
                   'offsets' if func == 'textureGatherOffsets' else 'nonconst' if func == 'textureGatherOffset' else '',
                   sampler,
                   'clamp' if sampler == '2DRect' else 'repeat']
            g(cmd, testname)

    g(['arb_gpu_shader5-minmax'])
    g(['arb_gpu_shader5-invocation-id'])
    g(['arb_gpu_shader5-invocations_count_too_large'])
    g(['arb_gpu_shader5-xfb-streams'])
    g(['arb_gpu_shader5-stream_value_too_large'])
    g(['arb_gpu_shader5-emitstreamvertex_stream_too_large'])
    g(['arb_gpu_shader5-tf-wrong-stream-value'])
    g(['arb_gpu_shader5-xfb-streams-without-invocations'])
    g(['arb_gpu_shader5-emitstreamvertex_nodraw'])
    g(['arb_gpu_shader5-interpolateAtCentroid'])
    g(['arb_gpu_shader5-interpolateAtCentroid-packing'])
    g(['arb_gpu_shader5-interpolateAtCentroid-flat'])
    g(['arb_gpu_shader5-interpolateAtCentroid-centroid'])
    g(['arb_gpu_shader5-interpolateAtCentroid-noperspective'])
    g(['arb_gpu_shader5-interpolateAtSample'])
    g(['arb_gpu_shader5-interpolateAtSample-nonconst'])
    g(['arb_gpu_shader5-interpolateAtOffset'])
    g(['arb_gpu_shader5-interpolateAtOffset-nonconst'])


with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader_fp64',
                        'varying-packing')) as g:
    for type in ['double', 'dvec2', 'dvec3', 'dvec4', 'dmat2', 'dmat3',
                 'dmat4', 'dmat2x3', 'dmat2x4', 'dmat3x2', 'dmat3x4',
                 'dmat4x2', 'dmat4x3']:
        for arrayspec in ['array', 'separate']:
            g(['varying-packing-simple', type, arrayspec],
              'simple {0} {1}'.format(type, arrayspec))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shader_subroutine')) as g:
    g(['arb_shader_subroutine-minmax'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_occlusion_query')) as g:
    g(['occlusion_query'])
    g(['occlusion_query_lifetime'])
    g(['occlusion_query_meta_fragments'])
    g(['occlusion_query_meta_no_fragments'])
    g(['occlusion_query_meta_save'])
    g(['occlusion_query_order'])
    g(['gen_delete_while_active'])

# Group ARB_separate_shader_objects
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_separate_shader_objects')) as g:
    g(['arb_separate_shader_object-ActiveShaderProgram-invalid-program'],
      'ActiveShaderProgram with invalid program')
    g(['arb_separate_shader_object-GetProgramPipelineiv'],
      'GetProgramPipelineiv')
    g(['arb_separate_shader_object-IsProgramPipeline'],
      'IsProgramPipeline')
    g(['arb_separate_shader_object-UseProgramStages-non-separable'],
      'UseProgramStages - non-separable program')
    g(['arb_separate_shader_object-ProgramUniform-coverage'],
      'ProgramUniform coverage')
    g(['arb_separate_shader_object-rendezvous_by_location', '-fbo'],
      'Rendezvous by location', run_concurrent=False)
    g(['arb_separate_shader_object-ValidateProgramPipeline'],
      'ValidateProgramPipeline')
    g(['arb_separate_shader_object-400-combinations', '-fbo', '--by-location'],
      '400 combinations by location', run_concurrent=False)
    g(['arb_separate_shader_object-400-combinations', '-fbo'],
      '400 combinations by name', run_concurrent=False)
    g(['arb_separate_shader_object-active-sampler-conflict'],
      'active sampler conflict')

# Group ARB_sampler_objects
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_sampler_objects')) as g:
    g(['arb_sampler_objects-sampler-objects'], 'sampler-objects',)
    g(['arb_sampler_objects-sampler-incomplete'], 'sampler-incomplete',)
    g(['arb_sampler_objects-srgb-decode'], 'GL_EXT_texture_sRGB_decode',)
    g(['arb_sampler_objects-framebufferblit'], 'framebufferblit',
      run_concurrent=False)

# Group ARB_sample_shading
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_sample_shading')) as g:
    g(['arb_sample_shading-api'], run_concurrent=False)

    for num_samples in (0,) + MSAA_SAMPLE_COUNTS:
        g(['arb_sample_shading-builtin-gl-num-samples', str(num_samples)],
          'builtin-gl-num-samples {0}'.format(num_samples),
          run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-id', str(num_samples)],
          'builtin-gl-sample-id {}'.format(num_samples), run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-mask', str(num_samples)],
          'builtin-gl-sample-mask {}'.format(num_samples),
          run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-position', str(num_samples)],
          'builtin-gl-sample-position {}'.format(num_samples),
          run_concurrent=False)

    for num_samples in MSAA_SAMPLE_COUNTS:
        g(['arb_sample_shading-interpolate-at-sample-position',
           str(num_samples)],
          'interpolate-at-sample-position {}'.format(num_samples),
          run_concurrent=False)
        g(['arb_sample_shading-ignore-centroid-qualifier', str(num_samples)],
          'ignore-centroid-qualifier {}'.format(num_samples),
          run_concurrent=False)

    for num_samples in [0, 2, 4, 6, 8]:
        g(['arb_sample_shading-builtin-gl-sample-mask-simple',
           str(num_samples)],
          'builtin-gl-sample-mask-simple {}'.format(num_samples))

# Group ARB_debug_output
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_debug_output')) as g:
    g(['arb_debug_output-api_error'], run_concurrent=False)

# Group KHR_debug
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'KHR_debug')) as g:
    g(['khr_debug-object-label_gl'], 'object-label_gl')
    g(['khr_debug-object-label_gles2'], 'object-label_gles2')
    g(['khr_debug-object-label_gles3'], 'object-label_gles3')
    g(['khr_debug-push-pop-group_gl'], 'push-pop-group_gl')
    g(['khr_debug-push-pop-group_gles2'], 'push-pop-group_gles2')
    g(['khr_debug-push-pop-group_gles3'], 'push-pop-group_gles3')

# Group ARB_occlusion_query2
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_occlusion_query2')) as g:
    g(['arb_occlusion_query2-api'], 'api')
    g(['arb_occlusion_query2-render'], 'render')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_pixel_buffer_object')) as g:
    g(['fbo-pbo-readpixels-small'], run_concurrent=False)
    g(['pbo-drawpixels'], run_concurrent=False)
    g(['pbo-read-argb8888'], run_concurrent=False)
    g(['pbo-readpixels-small'], run_concurrent=False)
    g(['pbo-teximage'], run_concurrent=False)
    g(['pbo-teximage-tiling'], run_concurrent=False)
    g(['pbo-teximage-tiling-2'], run_concurrent=False)
    g(['texsubimage', 'pbo'])
    g(['texsubimage', 'array', 'pbo'])
    g(['texsubimage', 'cube_map_array', 'pbo'])

# Group ARB_provoking_vertex
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_provoking_vertex')) as g:
    g(['arb-provoking-vertex-control'], run_concurrent=False)
    g(['arb-provoking-vertex-initial'], run_concurrent=False)
    g(['arb-provoking-vertex-render'], run_concurrent=False)
    g(['arb-quads-follow-provoking-vertex'], run_concurrent=False)
    g(['arb-xfb-before-flatshading'], run_concurrent=False)

# Group ARB_robustness
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_robustness')) as g:
    g(['arb_robustness_client-mem-bounds'], run_concurrent=False)

# Group ARB_shader_texture_lod
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shader_texture_lod', 'execution')) as g:
    g(['arb_shader_texture_lod-texgrad'])
    g(['arb_shader_texture_lod-texgradcube'])
    g(['tex-miplevel-selection', '*Lod', '1D'])
    g(['tex-miplevel-selection', '*Lod', '2D'])
    g(['tex-miplevel-selection', '*Lod', '3D'])
    g(['tex-miplevel-selection', '*Lod', 'Cube'])
    g(['tex-miplevel-selection', '*Lod', '1DShadow'])
    g(['tex-miplevel-selection', '*Lod', '2DShadow'])
    g(['tex-miplevel-selection', '*ProjLod', '1D'])
    g(['tex-miplevel-selection', '*ProjLod', '1D_ProjVec4'])
    g(['tex-miplevel-selection', '*ProjLod', '2D'])
    g(['tex-miplevel-selection', '*ProjLod', '2D_ProjVec4'])
    g(['tex-miplevel-selection', '*ProjLod', '3D'])
    g(['tex-miplevel-selection', '*ProjLod', '1DShadow'])
    g(['tex-miplevel-selection', '*ProjLod', '2DShadow'])
    g(['tex-miplevel-selection', '*GradARB', '1D'])
    g(['tex-miplevel-selection', '*GradARB', '2D'])
    g(['tex-miplevel-selection', '*GradARB', '3D'])
    g(['tex-miplevel-selection', '*GradARB', 'Cube'])
    g(['tex-miplevel-selection', '*GradARB', '1DShadow'])
    g(['tex-miplevel-selection', '*GradARB', '2DShadow'])
    g(['tex-miplevel-selection', '*GradARB', '2DRect'])
    g(['tex-miplevel-selection', '*GradARB', '2DRectShadow'])
    g(['tex-miplevel-selection', '*ProjGradARB', '1D'])
    g(['tex-miplevel-selection', '*ProjGradARB', '1D_ProjVec4'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2D'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2D_ProjVec4'])
    g(['tex-miplevel-selection', '*ProjGradARB', '3D'])
    g(['tex-miplevel-selection', '*ProjGradARB', '1DShadow'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2DShadow'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2DRect'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2DRect_ProjVec4'])
    g(['tex-miplevel-selection', '*ProjGradARB', '2DRectShadow'])


# Group ARB_shader_objects
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shader_objects')) as g:
    g(['arb_shader_objects-getuniform'], 'getuniform')
    g(['arb_shader_objects-bindattriblocation-scratch-name'],
      'bindattriblocation-scratch-name')
    g(['arb_shader_objects-getactiveuniform-beginend'],
      'getactiveuniform-beginend')
    g(['arb_shader_objects-getuniformlocation-array-of-struct-of-array'],
      'getuniformlocation-array-of-struct-of-array')
    g(['arb_shader_objects-clear-with-deleted'], 'clear-with-deleted')
    g(['arb_shader_objects-delete-repeat'], 'delete-repeat')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shading_language_420pack')) as g:
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'arb_shading_language_420pack',
                    'minimum-maximums.txt')],
      'built-in constants')
    g(['arb_shading_language_420pack-multiple-layout-qualifiers'],
      'multiple layout qualifiers')
    g(['arb_shading_language_420pack-binding-layout'], 'binding layout')

# Group ARB_explicit_attrib_location
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_explicit_attrib_location')) as g:
    g(['glsl-explicit-location-01'], run_concurrent=False)
    g(['glsl-explicit-location-02'], run_concurrent=False)
    g(['glsl-explicit-location-03'], run_concurrent=False)
    g(['glsl-explicit-location-04'], run_concurrent=False)
    g(['glsl-explicit-location-05'], run_concurrent=False)
    for test_type in ('shader', 'api'):
        g(['overlapping-locations-input-attribs', test_type],
          run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_program_interface_query')) as g:
    g(['arb_program_interface_query-resource-location'], run_concurrent=False)
    g(['arb_program_interface_query-resource-query'], run_concurrent=False)
    g(['arb_program_interface_query-getprograminterfaceiv'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourceindex'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourcename'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourceiv'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_no_attachments')) as g:
    g(['arb_framebuffer_no_attachments-minmax'])
    g(['arb_framebuffer_no_attachments-params'])
    g(['arb_framebuffer_no_attachments-atomic'])

# Group ARB_explicit_uniform_location
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_explicit_uniform_location')) as g:
    g(['arb_explicit_uniform_location-minmax'], run_concurrent=False)
    g(['arb_explicit_uniform_location-boundaries'], run_concurrent=False)
    g(['arb_explicit_uniform_location-array-elements'], run_concurrent=False)
    g(['arb_explicit_uniform_location-inactive-uniform'], run_concurrent=False)
    g(['arb_explicit_uniform_location-use-of-unused-loc'],
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_buffer_object')) as g:
    g(['arb_texture_buffer_object-data-sync'], 'data-sync')
    g(['arb_texture_buffer_object-dlist'], 'dlist')
    g(['arb_texture_buffer_object-formats', 'fs', 'core'],
      'formats (FS, 3.1 core)')
    g(['arb_texture_buffer_object-formats', 'vs', 'core'],
      'formats (VS, 3.1 core)')
    g(['arb_texture_buffer_object-formats', 'fs', 'arb'], 'formats (FS, ARB)')
    g(['arb_texture_buffer_object-formats', 'vs', 'arb'], 'formats (VS, ARB)')
    g(['arb_texture_buffer_object-get'], 'get')
    g(['arb_texture_buffer_object-fetch-outside-bounds'],
      'fetch-outside-bounds')
    g(['arb_texture_buffer_object-minmax'], 'minmax')
    g(['arb_texture_buffer_object-negative-bad-bo'], 'negative-bad-bo')
    g(['arb_texture_buffer_object-negative-bad-format'], 'negative-bad-format')
    g(['arb_texture_buffer_object-negative-bad-target'], 'negative-bad-target')
    g(['arb_texture_buffer_object-negative-unsupported'],
      'negative-unsupported')
    g(['arb_texture_buffer_object-subdata-sync'], 'subdata-sync')
    g(['arb_texture_buffer_object-unused-name'], 'unused-name')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_buffer_range')) as g:
    g(['arb_texture_buffer_range-dlist'], 'dlist')
    g(['arb_texture_buffer_range-errors'], 'errors')
    g(['arb_texture_buffer_range-ranges'], 'ranges')
    g(['arb_texture_buffer_range-ranges-2'], 'ranges-2')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_rectangle')) as g:
    g(['1-1-linear-texture'])
    g(['texrect-many'], run_concurrent=False)
    g(['getteximage-targets', 'RECT'])
    g(['texrect_simple_arb_texrect'], run_concurrent=False)
    g(['arb_texrect-texture-base-level-error'], run_concurrent=False)
    g(['fbo-blit', 'rect'], run_concurrent=False)
    g(['tex-miplevel-selection', 'GL2:texture()', '2DRect'])
    g(['tex-miplevel-selection', 'GL2:texture()', '2DRectShadow'])
    g(['tex-miplevel-selection', 'GL2:textureProj', '2DRect'])
    g(['tex-miplevel-selection', 'GL2:textureProj', '2DRect_ProjVec4'])
    g(['tex-miplevel-selection', 'GL2:textureProj', '2DRectShadow'])
    add_msaa_visual_plain_tests(g, ['copyteximage', 'RECT'],
                                run_concurrent=False)
    add_texwrap_target_tests(g, 'RECT')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_texture_storage')) as g:
    g(['arb_texture_storage-texture-storage'], 'texture-storage',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_storage_multisample')) as g:
    g(['arb_texture_storage_multisample-tex-storage'], 'tex-storage')
    g(['arb_texture_storage_multisample-tex-param'], 'tex-param')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_view')) as g:
    g(['arb_texture_view-cubemap-view'], 'cubemap-view')
    g(['arb_texture_view-texture-immutable-levels'], 'immutable_levels')
    g(['arb_texture_view-max-level'], 'max-level')
    g(['arb_texture_view-params'], 'params')
    g(['arb_texture_view-formats'], 'formats')
    g(['arb_texture_view-targets'], 'targets')
    g(['arb_texture_view-queries'], 'queries')
    g(['arb_texture_view-rendering-target'], 'rendering-target')
    g(['arb_texture_view-rendering-levels'], 'rendering-levels')
    g(['arb_texture_view-rendering-layers'], 'rendering-layers')
    g(['arb_texture_view-rendering-formats'], 'rendering-formats')
    g(['arb_texture_view-lifetime-format'], 'lifetime-format')
    g(['arb_texture_view-getteximage-srgb'], 'getteximage-srgb')
    g(['arb_texture_view-texsubimage-levels'], 'texsubimage-levels')
    g(['arb_texture_view-texsubimage-layers'], 'texsubimage-layers')
    g(['arb_texture_view-clear-into-view-2d'], 'clear-into-view-2d')
    g(['arb_texture_view-clear-into-view-2d-array'],
      'clear-into-view-2d-array')
    g(['arb_texture_view-clear-into-view-layered'], 'clear-into-view-layered')
    g(['arb_texture_view-copytexsubimage-layers'], 'copytexsubimage-layers')
    g(['arb_texture_view-sampling-2d-array-as-cubemap'],
      'sampling-2d-array-as-cubemap')
    g(['arb_texture_view-sampling-2d-array-as-cubemap-array'],
      'sampling-2d-array-as-cubemap-array')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '3DFX_texture_compression_FXT1')) as g:
    g(['compressedteximage', 'GL_COMPRESSED_RGB_FXT1_3DFX'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_FXT1_3DFX'])
    g(['fxt1-teximage'], run_concurrent=False)
    g(['arb_texture_compression-invalid-formats', 'fxt1'],
      'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_3DFX_texture_compression_FXT1'],
      'fbo-generatemipmap-formats')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_clip_control')) as g:
    g(['clip-control'])
    g(['clip-control-depth-precision'])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_color_buffer_float')) as g:

    def f(name, format, p1=None, p2=None):
        testname = '{}-{}{}{}'.format(
            format, name,
            '-{}'.format(p1) if p1 else '',
            '-{}'.format(p2) if p2 else '')
        cmd = ['arb_color_buffer_float-{}'.format(name), format, p1, p2]
        g([c for c in cmd if c is not None], testname)

    f('mrt', 'mixed')
    f('getteximage', 'GL_RGBA8')
    f('queries', 'GL_RGBA8')
    f('readpixels', 'GL_RGBA8')
    f('probepixel', 'GL_RGBA8')
    f('drawpixels', 'GL_RGBA8')
    f('clear', 'GL_RGBA8')
    f('render', 'GL_RGBA8')
    f('render', 'GL_RGBA8', 'fog')
    f('render', 'GL_RGBA8', 'sanity')
    f('render', 'GL_RGBA8', 'sanity', 'fog')
    f('queries', 'GL_RGBA8_SNORM')
    f('readpixels', 'GL_RGBA8_SNORM')
    f('probepixel', 'GL_RGBA8_SNORM')
    f('drawpixels', 'GL_RGBA8_SNORM')
    f('getteximage', 'GL_RGBA8_SNORM')
    f('clear', 'GL_RGBA8_SNORM')
    f('render', 'GL_RGBA8_SNORM')
    f('render', 'GL_RGBA8_SNORM', 'fog')
    f('render', 'GL_RGBA8_SNORM', 'sanity')
    f('render', 'GL_RGBA8_SNORM', 'sanity', 'fog')
    f('getteximage', 'GL_RGBA16F')
    f('queries', 'GL_RGBA16F')
    f('readpixels', 'GL_RGBA16F')
    f('probepixel', 'GL_RGBA16F')
    f('drawpixels', 'GL_RGBA16F')
    f('clear', 'GL_RGBA16F')
    f('render', 'GL_RGBA16F')
    f('render', 'GL_RGBA16F', 'fog')
    f('render', 'GL_RGBA16F', 'sanity')
    f('render', 'GL_RGBA16F', 'sanity', 'fog')
    f('getteximage', 'GL_RGBA32F')
    f('queries', 'GL_RGBA32F')
    f('readpixels', 'GL_RGBA32F')
    f('probepixel', 'GL_RGBA32F')
    f('drawpixels', 'GL_RGBA32F')
    f('clear', 'GL_RGBA32F')
    f('render', 'GL_RGBA32F')
    f('render', 'GL_RGBA32F', 'fog')
    f('render', 'GL_RGBA32F', 'sanity')
    f('render', 'GL_RGBA32F', 'sanity', 'fog')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_depth_texture')) as g:
    g(['depth-level-clamp'], run_concurrent=False)
    g(['depth-tex-modes'], run_concurrent=False)
    g(['texdepth'], run_concurrent=False)
    add_depthstencil_render_miplevels_tests(g, ('d=z24', 'd=z16'))
    add_texwrap_format_tests(g, 'GL_ARB_depth_texture')
    add_fbo_depth_tests(g, 'GL_DEPTH_COMPONENT16')
    add_fbo_depth_tests(g, 'GL_DEPTH_COMPONENT24')
    add_fbo_depth_tests(g, 'GL_DEPTH_COMPONENT32')
    add_fbo_formats_tests(g, 'GL_ARB_depth_texture')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_depth_buffer_float')) as g:
    g(['fbo-clear-formats', 'GL_ARB_depth_buffer_float', 'stencil'],
      'fbo-clear-formats stencil')
    add_depthstencil_render_miplevels_tests(
        g,
        ['d=z32f_s8', 'd=z32f', 'd=z32f_s8_s=z24_s8', 'd=z32f_s=z24_s8',
         's=z24_s8_d=z32f_s8', 's=z24_s8_d=z32f', 'd=s=z32f_s8', 's=d=z32f_s8',
         'ds=z32f_s8'])
    add_fbo_stencil_tests(g, 'GL_DEPTH32F_STENCIL8')
    add_texwrap_format_tests(g, 'GL_ARB_depth_buffer_float')
    add_fbo_depth_tests(g, 'GL_DEPTH_COMPONENT32F')
    add_fbo_depth_tests(g, 'GL_DEPTH32F_STENCIL8')
    add_fbo_formats_tests(g, 'GL_ARB_depth_buffer_float')
    add_fbo_depthstencil_tests(
        grouptools.join('spec', 'arb_depth_buffer_float'),
        'GL_DEPTH32F_STENCIL8', 0)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_env_crossbar')) as g:
    g(['crossbar'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_texture_compression')) as g:
    g(['arb_texture_compression-internal-format-query'],
      'GL_TEXTURE_INTERNAL_FORMAT query')
    g(['arb_texture_compression-invalid-formats', 'unknown'],
      'unknown formats')
    g(['fbo-generatemipmap-formats', 'GL_ARB_texture_compression'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_ARB_texture_compression')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_compression_bptc')) as g:
    g(['arb_texture_compression-invalid-formats', 'bptc'], 'invalid formats')
    g(['bptc-modes'])
    g(['bptc-float-modes'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_BPTC_UNORM'])
    g(['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM'])
    g(['compressedteximage', 'GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT'])
    g(['fbo-generatemipmap-formats', 'GL_ARB_texture_compression_bptc-unorm'],
      'fbo-generatemipmap-formats unorm')
    g(['fbo-generatemipmap-formats', 'GL_ARB_texture_compression_bptc-float'],
      'fbo-generatemipmap-formats float')
    add_texwrap_format_tests(g, 'GL_ARB_texture_compression_bptc')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_vertex_array_bgra')) as g:
    g(['bgra-sec-color-pointer'], run_concurrent=False)
    g(['bgra-vert-attrib-pointer'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'apple_vertex_array_object')) as g:
    g(['vao-01'], run_concurrent=False)
    g(['vao-02'], run_concurrent=False)
    g(['arb_vertex_array-isvertexarray', 'apple'], 'isvertexarray')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_array_bgra')) as g:
    g(['arb_vertex_array_bgra-api-errors'], 'api-errors', run_concurrent=False)
    g(['arb_vertex_array_bgra-get'], 'get', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_array_object')) as g:
    g(['vao-element-array-buffer'])
    g(['arb_vertex_array-isvertexarray'], 'isvertexarray')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_buffer_object')) as g:
    g(['arb_vertex_buffer_object-elements-negative-offset'],
      'elements-negative-offset', run_concurrent=False)
    g(['arb_vertex_buffer_object-mixed-immediate-and-vbo'],
      'mixed-immediate-and-vbo', run_concurrent=False)
    g(['fdo14575'], run_concurrent=False)
    g(['fdo22540'], run_concurrent=False)
    g(['fdo31934'], run_concurrent=False)
    g(['arb_vertex_buffer_object-ib-data-sync'], 'ib-data-sync')
    g(['arb_vertex_buffer_object-ib-subdata-sync'], 'ib-subdata-sync')
    g(['pos-array'], run_concurrent=False)
    g(['vbo-bufferdata'], run_concurrent=False)
    g(['vbo-map-remap'], run_concurrent=False)
    g(['vbo-map-unsync'])
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawarrays'],
      'vbo-subdata-many drawarrays')
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawelements'],
      'vbo-subdata-many drawelements')
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawrangeelements'],
      'vbo-subdata-many drawrangeelements')
    g(['vbo-subdata-sync'], run_concurrent=False)
    g(['vbo-subdata-zero'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_program')) as g:
    g(['arb_vertex_program-getenv4d-with-error'], 'getenv4d-with-error',
      run_concurrent=False)
    g(['arb_vertex_program-getlocal4d-with-error'], 'getlocal4d-with-error',
      run_concurrent=False)
    g(['arb_vertex_program-getlocal4f-max'], 'getlocal4f-max')
    g(['arb_vertex_program-getlocal4-errors'], 'getlocal4-errors')
    g(['clip-plane-transformation', 'arb'],
      'clip-plane-transformation arb')
    g(['arb_vertex_program-minmax'], 'minmax')
    g(['fdo24066'], run_concurrent=False)
    g(['vp-address-01'], run_concurrent=False)
    g(['vp-address-02'], run_concurrent=False)
    g(['vp-address-04'], run_concurrent=False)
    g(['vp-bad-program'], run_concurrent=False)
    g(['vp-max-array'], run_concurrent=False)
    add_vpfpgeneric(g, 'arl')
    add_vpfpgeneric(g, 'big-param')
    add_vpfpgeneric(g, 'dataflow-bug')
    add_vpfpgeneric(g, 'fogcoord-dp3')
    add_vpfpgeneric(g, 'fogcoord-dph')
    add_vpfpgeneric(g, 'fogcoord-dp4')
    add_vpfpgeneric(g, 'vp-arl-constant-array')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge-varying')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge-offset')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge-offset-neg')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge-overwritten')
    add_vpfpgeneric(g, 'vp-arl-constant-array-huge-relative-offset')
    add_vpfpgeneric(g, 'vp-arl-constant-array-varying')
    add_vpfpgeneric(g, 'vp-arl-env-array')
    add_vpfpgeneric(g, 'vp-arl-local-array')
    add_vpfpgeneric(g, 'vp-arl-neg-array')
    add_vpfpgeneric(g, 'vp-arl-neg-array-2')
    add_vpfpgeneric(g, 'vp-constant-array')
    add_vpfpgeneric(g, 'vp-constant-array-huge')
    add_vpfpgeneric(g, 'vp-constant-negate')
    add_vpfpgeneric(g, 'vp-exp-alias')
    add_vpfpgeneric(g, 'vp-max')
    add_vpfpgeneric(g, 'vp-min')
    add_vpfpgeneric(g, 'vp-sge-alias')
    add_vpfpgeneric(g, 'vp-two-constants')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_viewport_array')) as g:
    g(['arb_viewport_array-viewport-indices'], 'viewport-indices')
    g(['arb_viewport_array-depthrange-indices'], 'depthrange-indices')
    g(['arb_viewport_array-scissor-check'], 'scissor-check')
    g(['arb_viewport_array-scissor-indices'], 'scissor-indices')
    g(['arb_viewport_array-bounds'], 'bounds')
    g(['arb_viewport_array-queries'], 'queries')
    g(['arb_viewport_array-minmax'], 'minmax')
    g(['arb_viewport_array-render-viewport'], 'render-viewport')
    g(['arb_viewport_array-render-depthrange'], 'render-depthrange')
    g(['arb_viewport_array-render-scissor'], 'render-scissor')
    g(['arb_viewport_array-clear'], 'clear')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_vertex_program')) as g:
    add_vpfpgeneric(g, 'nv-mov')
    add_vpfpgeneric(g, 'nv-add')
    add_vpfpgeneric(g, 'nv-arl')
    add_vpfpgeneric(g, 'nv-init-zero-reg')
    add_vpfpgeneric(g, 'nv-init-zero-addr')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_vertex_program2_option')) as g:
    g(['vp-address-03'], run_concurrent=False)
    g(['vp-address-05'], run_concurrent=False)
    g(['vp-address-06'], run_concurrent=False)
    g(['vp-clipdistance-01'], run_concurrent=False)
    g(['vp-clipdistance-02'], run_concurrent=False)
    g(['vp-clipdistance-03'], run_concurrent=False)
    g(['vp-clipdistance-04'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_framebuffer_blit')) as g:
    g(['fbo-blit'], run_concurrent=False)
    g(['fbo-copypix'], run_concurrent=False)
    g(['fbo-readdrawpix'], run_concurrent=False)
    g(['fbo-srgb-blit'])
    g(['fbo-sys-blit'], run_concurrent=False)
    g(['fbo-sys-sub-blit'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec',
                        'ext_framebuffer_multisample_blit_scaled')) as g:
    g(['ext_framebuffer_multisample_blit_scaled-negative-blit-scaled'],
      'negative-blit-scaled')

    for num_samples in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample_blit_scaled-blit-scaled',
           str(num_samples)],
          'blit-scaled samples={}'.format(num_samples))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_framebuffer_multisample')) as g:
    g(['ext_framebuffer_multisample-blit-mismatched-samples'],
      'blit-mismatched-samples')
    g(['ext_framebuffer_multisample-blit-mismatched-sizes'],
      'blit-mismatched-sizes')
    g(['ext_framebuffer_multisample-blit-mismatched-formats'],
      'blit-mismatched-formats')
    g(['ext_framebuffer_multisample-dlist'], 'dlist')
    g(['ext_framebuffer_multisample-enable-flag'], 'enable-flag')
    g(['ext_framebuffer_multisample-minmax'], 'minmax')
    g(['ext_framebuffer_multisample-negative-copypixels'],
      'negative-copypixels')
    g(['ext_framebuffer_multisample-negative-copyteximage'],
      'negative-copyteximage')
    g(['ext_framebuffer_multisample-negative-max-samples'],
      'negative-max-samples')
    g(['ext_framebuffer_multisample-negative-mismatched-samples'],
      'negative-mismatched-samples')
    g(['ext_framebuffer_multisample-negative-readpixels'],
      'negative-readpixels')
    g(['ext_framebuffer_multisample-renderbufferstorage-samples'],
      'renderbufferstorage-samples')
    g(['ext_framebuffer_multisample-renderbuffer-samples'],
      'renderbuffer-samples')
    g(['ext_framebuffer_multisample-samples'], 'samples')
    g(['ext_framebuffer_multisample-alpha-blending'], 'alpha-blending')
    g(['ext_framebuffer_multisample-alpha-blending', 'slow_cc'],
      'alpha-blending slow_cc')

    for num_samples in MSAA_SAMPLE_COUNTS:
        if num_samples % 2 != 0:
            continue
        g(['ext_framebuffer_multisample-alpha-blending-after-rendering',
           str(num_samples)],
          'alpha-blending-after-rendering {}'.format(num_samples))

    for num_samples in ('all_samples', ) + MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-formats', str(num_samples)],
          'formats {}'.format(num_samples))

        for test_type in ('color', 'srgb', 'stencil_draw', 'stencil_resolve',
                          'depth_draw', 'depth_resolve'):
            sensible_options = ['small', 'depthstencil']
            if test_type in ('color', 'srgb'):
                sensible_options.append('linear')
            for options in power_set(sensible_options):
                g(['ext_framebuffer_multisample-accuracy', str(num_samples),
                   test_type] + options,
                  ' '.join(['accuracy', str(num_samples), test_type] +
                           options))

    # Note: the interpolation tests also check for sensible behaviour with
    # non-multisampled framebuffers, so go ahead and test them with
    # num_samples==0 as well.
    for num_samples in (0,) + MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-blit-multiple-render-targets',
           str(num_samples)],
          'blit-multiple-render-targets {}'.format(num_samples))

        for test_type in ('non-centroid-disabled', 'centroid-disabled',
                          'centroid-edges', 'non-centroid-deriv',
                          'non-centroid-deriv-disabled', 'centroid-deriv',
                          'centroid-deriv-disabled'):
            g(['ext_framebuffer_multisample-interpolation', str(num_samples),
               test_type],
              'interpolation {} {}'.format(num_samples, test_type))

    for num_samples in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-turn-on-off', str(num_samples)],
          'turn-on-off {}'.format(num_samples), run_concurrent=False)

        for buffer_type in ('color', 'depth', 'stencil'):
            if buffer_type == 'color':
                sensible_options = ['linear']
            else:
                sensible_options = []

            for options in power_set(sensible_options):
                g(['ext_framebuffer_multisample-upsample', str(num_samples),
                   buffer_type] + options,
                  'upsample {} {}'.format(
                      num_samples, ' '.join([buffer_type] + options)))
                g(['ext_framebuffer_multisample-multisample-blit',
                   str(num_samples), buffer_type] + options,
                  'multisample-blit {}'.format(
                      ' '.join([str(num_samples), buffer_type] + options)))

            for blit_type in ('msaa', 'upsample', 'downsample'):
                g(['ext_framebuffer_multisample-unaligned-blit',
                   str(num_samples), buffer_type, blit_type],
                  'unaligned-blit {} {} {}'.format(
                      num_samples, buffer_type, blit_type))

        for test_mode in ('inverted', 'non-inverted'):
            g(['ext_framebuffer_multisample-sample-coverage', str(num_samples),
               test_mode],
              'sample-coverage {} {}'.format(num_samples, test_mode))

        for buffer_type in ('color', 'depth'):
            g(['ext_framebuffer_multisample-sample-alpha-to-coverage',
               str(num_samples), buffer_type],
              'sample-alpha-to-coverage {} {}'.format(
                  num_samples, buffer_type))

        for test in ['line-smooth', 'point-smooth', 'polygon-smooth',
                     'sample-alpha-to-one',
                     'draw-buffers-alpha-to-one',
                     'draw-buffers-alpha-to-coverage',
                     'alpha-coverage-no-draw-buffer-zero',
                     'alpha-to-coverage-dual-src-blend',
                     'alpha-to-coverage-no-draw-buffer-zero',
                     'alpha-to-one-dual-src-blend',
                     'int-draw-buffers-alpha-to-one',
                     'int-draw-buffers-alpha-to-coverage',
                     'alpha-to-one-msaa-disabled',
                     'alpha-to-one-single-sample-buffer',
                     'bitmap', 'polygon-stipple']:
            g(['ext_framebuffer_multisample-{}'.format(test),
               str(num_samples)],
              '{} {}'.format(test, num_samples))

        for blit_type in ('msaa', 'upsample', 'downsample', 'normal'):
            g(['ext_framebuffer_multisample-clip-and-scissor-blit',
               str(num_samples), blit_type],
              'clip-and-scissor-blit {} {}'.format(str(num_samples),
                                                   blit_type))

        for flip_direction in ('x', 'y'):
            g(['ext_framebuffer_multisample-blit-flipped', str(num_samples),
               flip_direction],
              'blit-flipped {} {}'.format(str(num_samples), flip_direction))

        for buffer_type in ('color', 'depth', 'stencil'):
            g(['ext_framebuffer_multisample-clear', str(num_samples),
               buffer_type],
              'clear {} {}'.format(str(num_samples), buffer_type))

        for test_type in ('depth', 'depth-computed', 'stencil'):
            for buffer_config in ('combined', 'separate', 'single'):
                g(['ext_framebuffer_multisample-no-color', str(num_samples),
                   test_type, buffer_config],
                  'no-color {} {} {}'.format(
                      num_samples, test_type, buffer_config))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_framebuffer_object')) as g:
    g(['fbo-generatemipmap-noimage'])
    g(['fdo20701'])
    g(['fbo-1d'])
    g(['fbo-3d'])
    g(['fbo-alphatest-formats'])
    g(['fbo-alphatest-nocolor'])
    g(['fbo-alphatest-nocolor-ff'])
    g(['fbo-blending-formats'])
    g(['fbo-bind-renderbuffer'])
    g(['fbo-clearmipmap'])
    g(['fbo-clear-formats'])
    g(['fbo-colormask-formats'])
    g(['fbo-copyteximage'])
    g(['fbo-copyteximage-simple'])
    g(['fbo-cubemap'])
    g(['fbo-depthtex'])
    g(['fbo-depth-sample-compare'])
    g(['fbo-drawbuffers'])
    g(['fbo-drawbuffers', 'masked-clear'])
    g(['fbo-drawbuffers-arbfp'])
    g(['fbo-drawbuffers-blend-add'])
    g(['fbo-drawbuffers-fragcolor'])
    g(['fbo-drawbuffers-maxtargets'])
    g(['fbo-finish-deleted'])
    g(['fbo-flushing'])
    g(['fbo-flushing-2'])
    g(['fbo-fragcoord'])
    g(['fbo-fragcoord2'])
    g(['fbo-generatemipmap'])
    g(['fbo-generatemipmap-filtering'])
    g(['fbo-generatemipmap-formats'])
    g(['fbo-generatemipmap-scissor'])
    g(['fbo-generatemipmap-swizzle'])
    g(['fbo-generatemipmap-nonsquare'])
    g(['fbo-generatemipmap-npot'])
    g(['fbo-generatemipmap-viewport'])
    g(['fbo-maxsize'])
    g(['fbo-nodepth-test'])
    g(['fbo-nostencil-test'])
    g(['fbo-readpixels'])
    g(['fbo-readpixels-depth-formats'])
    g(['fbo-scissor-bitmap'])
    g(['fbo-storage-completeness'])
    g(['fbo-storage-formats'])
    g(['getteximage-formats', 'init-by-rendering'])
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX1')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX4')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX8')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX16')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_image_dma_buf_import')) as \
        g:
    g(['ext_image_dma_buf_import-invalid_hints'], run_concurrent=False)
    g(['ext_image_dma_buf_import-invalid_attributes'], run_concurrent=False)
    g(['ext_image_dma_buf_import-missing_attributes'], run_concurrent=False)
    g(['ext_image_dma_buf_import-ownership_transfer'], run_concurrent=False)
    g(['ext_image_dma_buf_import-intel_unsupported_format'],
      run_concurrent=False)
    g(['ext_image_dma_buf_import-intel_external_sampler_only'],
      run_concurrent=False)
    g(['ext_image_dma_buf_import-intel_external_sampler_with_dma_only'],
      run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_rgb', '-fmt=AR24'],
      'ext_image_dma_buf_import-sample_argb8888', run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_rgb', '-fmt=XR24', '-alpha-one'],
      'ext_image_dma_buf_import-sample_xrgb8888', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_packed_depth_stencil')) as g:
    g(['fbo-blit-d24s8'], run_concurrent=False)
    g(['fbo-clear-formats', 'GL_EXT_packed_depth_stencil', 'stencil'],
      'fbo-clear-formats stencil')
    g(['ext_packed_depth_stencil-depth-stencil-texture'],
      'DEPTH_STENCIL texture')
    g(['ext_packed_depth_stencil-errors'], 'errors')
    g(['ext_packed_depth_stencil-getteximage'], 'getteximage')
    g(['ext_packed_depth_stencil-readdrawpixels'], 'readdrawpixels')
    g(['ext_packed_depth_stencil-texsubimage'], 'texsubimage')
    g(['ext_packed_depth_stencil-readpixels-24_8'], 'readpixels-24_8',
      run_concurrent=False)
    add_depthstencil_render_miplevels_tests(
        g,
        ['s=z24_s8', 'd=z24_s8', 'd=z24_s8_s=z24_s8', 'd=z24_s=z24_s8',
         's=z24_s8_d=z24_s8', 's=z24_s8_d=z24', 'd=s=z24_s8', 's=d=z24_s8',
         'ds=z24_s8'])
    add_fbo_stencil_tests(g, 'GL_DEPTH24_STENCIL8')
    add_texwrap_format_tests(g, 'GL_EXT_packed_depth_stencil')
    add_fbo_depth_tests(g, 'GL_DEPTH24_STENCIL8')
    add_fbo_formats_tests(g, 'GL_EXT_packed_depth_stencil')
    add_fbo_depthstencil_tests(
        grouptools.join('spec', 'ext_packed_depth_stencil'),
        'GL_DEPTH24_STENCIL8', 0)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_packed_depth_stencil')) as g:
    g(['oes_packed_depth_stencil-depth-stencil-texture_gles2'],
      'DEPTH_STENCIL texture GLES2')
    g(['oes_packed_depth_stencil-depth-stencil-texture_gles1'],
      'DEPTH_STENCIL texture GLES1')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_array')) as g:
    g(['fbo-generatemipmap-array'])
    g(['fbo-generatemipmap-array', 'RGB9_E5'])
    g(['fbo-generatemipmap-array', 'S3TC_DXT1'])
    g(['ext_texture_array-maxlayers'], 'maxlayers')
    g(['ext_texture_array-gen-mipmap'], 'gen-mipmap')
    g(['fbo-array'], run_concurrent=False)
    g(['array-texture'], run_concurrent=False)
    g(['ext_texture_array-errors'])
    g(['getteximage-targets', '1D_ARRAY'])
    g(['getteximage-targets', '2D_ARRAY'])
    g(['texsubimage', 'array'])
    add_msaa_visual_plain_tests(g, ['copyteximage', '1D_ARRAY'],
                                run_concurrent=False)
    add_msaa_visual_plain_tests(g, ['copyteximage', '2D_ARRAY'],
                                run_concurrent=False)
    for test in ('depth-clear', 'depth-layered-clear', 'depth-draw',
                 'fs-writes-depth', 'stencil-clear', 'stencil-layered-clear',
                 'stencil-draw', 'fs-writes-stencil'):
        g(['fbo-depth-array', test])
    for test_mode in ['teximage', 'texsubimage']:
        test_name = 'compressed {0}'.format(test_mode, run_concurrent=False)
        g(['ext_texture_array-{}'.format(test_name), '-fbo'], test_name,
          run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_cube_map')) as g:
    g(['crash-cubemap-order'], run_concurrent=False)
    g(['cubemap'], run_concurrent=False)
    g(['cubemap-getteximage-pbo'])
    g(['cubemap-mismatch'], run_concurrent=False)
    g(['cubemap', 'npot'], 'cubemap npot', run_concurrent=False)
    g(['cubemap-shader'], run_concurrent=False)
    g(['cubemap-shader', 'lod'], 'cubemap-shader lod', run_concurrent=False)
    g(['cubemap-shader', 'bias'], 'cubemap-shader bias', run_concurrent=False)
    g(['getteximage-targets', 'CUBE'])
    add_msaa_visual_plain_tests(g, ['copyteximage', 'CUBE'],
                                run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_cube_map_array')) as g:
    g(['arb_texture_cube_map_array-get'], run_concurrent=False)
    g(['arb_texture_cube_map_array-teximage3d-invalid-values'],
      run_concurrent=False)
    g(['arb_texture_cube_map_array-cubemap'], run_concurrent=False)
    g(['arb_texture_cube_map_array-cubemap-lod'], run_concurrent=False)
    g(['arb_texture_cube_map_array-fbo-cubemap-array'], run_concurrent=False)
    g(['arb_texture_cube_map_array-sampler-cube-array-shadow'],
      run_concurrent=False)
    g(['getteximage-targets', 'CUBE_ARRAY'])
    g(['glsl-resource-not-bound', 'CubeArray'])
    g(['fbo-generatemipmap-cubemap', 'array'])
    g(['fbo-generatemipmap-cubemap', 'array', 'RGB9_E5'])
    g(['fbo-generatemipmap-cubemap', 'array', 'S3TC_DXT1'])
    g(['texsubimage', 'cube_map_array'])

    for stage in ['vs', 'gs', 'fs']:
        # textureSize():
        for sampler in['samplerCubeArray', 'isamplerCubeArray',
                       'usamplerCubeArray', 'samplerCubeArrayShadow']:
            g(['textureSize', stage, sampler],
              grouptools.join('textureSize', '{}-textureSize-{}'.format(
                  stage, sampler)))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_swizzle')) as g:
    g(['ext_texture_swizzle-api'])
    g(['ext_texture_swizzle-swizzle'])
    g(['depth_texture_mode_and_swizzle'], 'depth_texture_mode_and_swizzle')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_compression_latc')) as g:
    g(['arb_texture_compression-invalid-formats', 'latc'], 'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_latc'],
      'fbo-generatemipmap-formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_latc-signed'],
      'fbo-generatemipmap-formats-signed')
    add_texwrap_format_tests(g, 'GL_EXT_texture_compression_latc')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_compression_rgtc')) as g:
    g(['compressedteximage', 'GL_COMPRESSED_RED_RGTC1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RED_GREEN_RGTC2_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_SIGNED_RED_RGTC1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT'])
    g(['arb_texture_compression-invalid-formats', 'rgtc'], 'invalid formats')
    g(['rgtc-teximage-01'], run_concurrent=False)
    g(['rgtc-teximage-02'], run_concurrent=False)
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_rgtc'],
      'fbo-generatemipmap-formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_rgtc-signed'],
      'fbo-generatemipmap-formats-signed')
    add_texwrap_format_tests(g, 'GL_EXT_texture_compression_rgtc')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_compression_s3tc')) as g:
    g(['compressedteximage', 'GL_COMPRESSED_RGB_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT3_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT5_EXT'])
    g(['arb_texture_compression-invalid-formats', 's3tc'], 'invalid formats')
    g(['gen-compressed-teximage'], run_concurrent=False)
    g(['s3tc-errors'])
    g(['s3tc-teximage'], run_concurrent=False)
    g(['s3tc-texsubimage'], run_concurrent=False)
    g(['getteximage-targets', '2D', 'S3TC'])
    g(['getteximage-targets', '2D_ARRAY', 'S3TC'])
    g(['getteximage-targets', 'CUBE', 'S3TC'])
    g(['getteximage-targets', 'CUBE_ARRAY', 'S3TC'])
    g(['compressedteximage', 'GL_COMPRESSED_SRGB_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT'])
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_s3tc'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_EXT_texture_compression_s3tc')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ati_texture_compression_3dc')) as g:
    g(['arb_texture_compression-invalid-formats', '3dc'], 'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_ATI_texture_compression_3dc'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_ATI_texture_compression_3dc')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_packed_float')) as g:
    g(['ext_packed_float-pack'], 'pack')
    g(['getteximage-invalid-format-for-packed-type'],
      'getteximage-invalid-format-for-packed-type')
    add_msaa_formats_tests(g, 'GL_EXT_packed_float')
    add_texwrap_format_tests(g, 'GL_EXT_packed_float')
    add_fbo_formats_tests(g, 'GL_EXT_packed_float')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_float')) as g:
    g(['arb_texture_float-texture-float-formats'], run_concurrent=False)
    add_msaa_formats_tests(g, 'GL_ARB_texture_float')
    add_texwrap_format_tests(g, 'GL_ARB_texture_float')
    add_fbo_formats_tests(g, 'GL_ARB_texture_float')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_texture_float')) as g:
    g(['oes_texture_float'])
    g(['oes_texture_float', 'half'])
    g(['oes_texture_float', 'linear'])
    g(['oes_texture_float', 'half', 'linear'])


with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_integer')) as g:
    g(['ext_texture_integer-api-drawpixels'], 'api-drawpixels')
    g(['ext_texture_integer-api-teximage'], 'api-teximage')
    g(['ext_texture_integer-api-readpixels'], 'api-readpixels')
    g(['ext_texture_integer-fbo-blending'], 'fbo-blending')
    g(['ext_texture_integer-fbo-blending', 'GL_ARB_texture_rg'],
      'fbo-blending GL_ARB_texture_rg')
    g(['ext_texture_integer-fbo_integer_precision_clear'],
      'fbo_integer_precision_clear', run_concurrent=False)
    g(['ext_texture_integer-fbo_integer_readpixels_sint_uint'],
      'fbo_integer_readpixels_sint_uint', run_concurrent=False)
    g(['ext_texture_integer-getteximage-clamping'], 'getteximage-clamping')
    g(['ext_texture_integer-getteximage-clamping', 'GL_ARB_texture_rg'],
      'getteximage-clamping GL_ARB_texture_rg')
    g(['ext_texture_integer-texture_integer_glsl130'],
      'texture_integer_glsl130')
    g(['fbo-integer'], run_concurrent=False)
    # TODO: unsupported for int yet
    # g(['fbo-clear-formats', 'GL_EXT_texture_integer'], 'fbo-clear-formats')
    add_msaa_formats_tests(g, 'GL_EXT_texture_integer')
    add_texwrap_format_tests(g, 'GL_EXT_texture_integer')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_rg')) as g:
    g(['depth-tex-modes-rg'], run_concurrent=False)
    g(['rg-draw-pixels'], run_concurrent=False)
    g(['rg-teximage-01'], run_concurrent=False)
    g(['rg-teximage-02'], run_concurrent=False)
    g(['texture-rg'], run_concurrent=False)
    # TODO: unsupported for int yet
    # g(['fbo-clear-formats', 'GL_ARB_texture_rg-int'],
    #   'fbo-clear-formats-int')
    add_msaa_formats_tests(g, 'GL_ARB_texture_rg')
    add_msaa_formats_tests(g, 'GL_ARB_texture_rg-int')
    add_msaa_formats_tests(g, 'GL_ARB_texture_rg-float')
    add_texwrap_format_tests(g, 'GL_ARB_texture_rg')
    add_texwrap_format_tests(g, 'GL_ARB_texture_rg-float', '-float')
    add_texwrap_format_tests(g, 'GL_ARB_texture_rg-int', '-int')

    for format in ['GL_RED', 'GL_R8', 'GL_R16', 'GL_RG', 'GL_RG8', 'GL_RG16']:
        g(['fbo-rg', format], "fbo-rg-{}".format(format))
    add_fbo_formats_tests(g, 'GL_ARB_texture_rg')
    add_fbo_formats_tests(g, 'GL_ARB_texture_rg-float', '-float')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_rgb10_a2ui')) as g:
    g(['ext_texture_integer-fbo-blending', 'GL_ARB_texture_rgb10_a2ui'],
      'fbo-blending')
    add_texwrap_format_tests(g, 'GL_ARB_texture_rgb10_a2ui')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_shared_exponent')) as g:
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_shared_exponent'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_EXT_texture_shared_exponent')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_snorm')) as g:
    add_msaa_formats_tests(g, 'GL_EXT_texture_snorm')
    add_texwrap_format_tests(g, 'GL_EXT_texture_snorm')
    add_fbo_formats_tests(g, 'GL_EXT_texture_snorm')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_texture_srgb')) as g:
    g(['fbo-srgb'], run_concurrent=False)
    g(['tex-srgb'], run_concurrent=False)
    g(['arb_texture_compression-invalid-formats', 'srgb'], 'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_sRGB'],
      'fbo-generatemipmap-formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_sRGB-s3tc'],
      'fbo-generatemipmap-formats-s3tc')
    # TODO: also use GL_ARB_framebuffer_sRGB:
    # g(['fbo-blending-formats', 'GL_EXT_texture_sRGB'],
    #   'fbo-blending-formats')
    g(['fbo-alphatest-formats', 'GL_EXT_texture_sRGB'],
      'fbo-alphatest-formats')
    add_msaa_formats_tests(g, 'GL_EXT_texture_sRGB')
    add_texwrap_format_tests(g, 'GL_EXT_texture_sRGB')
    add_texwrap_format_tests(g, 'GL_EXT_texture_sRGB-s3tc', '-s3tc')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_timer_query')) as g:
    g(['ext_timer_query-time-elapsed'], 'time-elapsed')
    g(['timer_query'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_timer_query')) as g:
    g(['ext_timer_query-time-elapsed', 'timestamp'], 'query GL_TIMESTAMP')
    g(['ext_timer_query-lifetime'], 'query-lifetime')
    g(['arb_timer_query-timestamp-get'], 'timestamp-get')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_transform_feedback')) as g:
    for mode in ['interleaved_ok_base', 'interleaved_ok_range',
                 'interleaved_ok_offset', 'interleaved_unbound',
                 'interleaved_no_varyings', 'separate_ok_1',
                 'separate_unbound_0_1', 'separate_ok_2',
                 'separate_unbound_0_2', 'separate_unbound_1_2',
                 'separate_no_varyings', 'no_prog_active', 'begin_active',
                 'useprog_active', 'link_current_active', 'link_other_active',
                 'bind_base_active', 'bind_range_active', 'bind_offset_active',
                 'end_inactive', 'bind_base_max', 'bind_range_max',
                 'bind_offset_max', 'bind_range_size_m4', 'bind_range_size_0',
                 'bind_range_size_1', 'bind_range_size_2', 'bind_range_size_3',
                 'bind_range_size_5', 'bind_range_offset_1',
                 'bind_range_offset_2', 'bind_range_offset_3',
                 'bind_range_offset_5', 'bind_offset_offset_1',
                 'bind_offset_offset_2', 'bind_offset_offset_3',
                 'bind_offset_offset_5', 'not_a_program',
                 'useprogstage_noactive', 'useprogstage_active',
                 'bind_pipeline']:
            g(['ext_transform_feedback-api-errors', mode],
              'api-errors {}'.format(mode))

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
        g(['ext_transform_feedback-builtin-varyings', varying],
          'builtin-varyings {}'.format(varying), run_concurrent=False)

    for mode in ['main_binding', 'indexed_binding', 'buffer_start',
                 'buffer_size']:
        g(['ext_transform_feedback-get-buffer-size', mode],
          'get-buffer-size {}'.format(mode))
        g(['ext_transform_feedback-get-buffer-state', mode],
          'get-buffer-state {}'.format(mode))

    for mode in ['output', 'prims_generated', 'prims_written']:
        g(['ext_transform_feedback-intervening-read', mode],
          'intervening-read {0}'.format(mode))
        g(['ext_transform_feedback-intervening-read', mode, 'use_gs'],
          'intervening-read {0} use_gs'.format(mode))

    for drawcall in ['arrays', 'elements']:
        for mode in ['triangles', 'lines', 'points']:
            g(['ext_transform_feedback-order', drawcall, mode],
              'order {0} {1}'.format(drawcall, mode))

    for draw_mode in ['points', 'lines', 'line_loop', 'line_strip',
                      'triangles', 'triangle_strip', 'triangle_fan',
                      'quads', 'quad_strip', 'polygon']:
        for shade_mode in ['monochrome', 'smooth', 'flat_first', 'flat_last',
                           'wireframe']:
            if (draw_mode in ['points', 'lines', 'line_loop', 'line_strip'] and
                    shade_mode == 'wireframe'):
                continue
            g(['ext_transform_feedback-tessellation', draw_mode, shade_mode],
              'tessellation {0} {1}'.format(draw_mode, shade_mode))

    for alignment in [0, 4, 8, 12]:
        g(['ext_transform_feedback-alignment', str(alignment)],
          'alignment {0}'.format(alignment))

    for output_type in ['float', 'vec2', 'vec3', 'vec4', 'mat2', 'mat2x3',
                        'mat2x4', 'mat3x2', 'mat3', 'mat3x4', 'mat4x2',
                        'mat4x3', 'mat4', 'int', 'ivec2', 'ivec3', 'ivec4',
                        'uint', 'uvec2', 'uvec3', 'uvec4']:
        for suffix in ['', '[2]', '[2]-no-subscript']:
            g(['ext_transform_feedback-output-type', output_type, suffix],
              'output-type {0}{1}'.format(output_type, suffix))

    for mode in ['discard', 'buffer', 'prims_generated', 'prims_written']:
        g(['ext_transform_feedback-generatemipmap', mode],
          'generatemipmap {0}'.format(mode))

    for test_case in ['base-shrink', 'base-grow', 'offset-shrink',
                      'offset-grow', 'range-shrink', 'range-grow']:
        g(['ext_transform_feedback-change-size', test_case],
          'change-size {0}'.format(test_case))

    for api_suffix, possible_options in [('', [[], ['interface']]),
                                         ('_gles3', [[]])]:
        for subtest in ['basic-struct', 'struct-whole-array',
                        'struct-array-elem', 'array-struct',
                        'array-struct-whole-array', 'array-struct-array-elem',
                        'struct-struct', 'array-struct-array-struct']:
            for mode in ['error', 'get', 'run', 'run-no-fs']:
                for options in possible_options:
                    g(['ext_transform_feedback-structs{0}'.format(api_suffix),
                       subtest, mode] + options,
                      'structs{0} {1}'.format(
                          api_suffix, ' '.join([subtest, mode] + options)))

    g(['ext_transform_feedback-buffer-usage'], 'buffer-usage')
    g(['ext_transform_feedback-discard-api'], 'discard-api')
    g(['ext_transform_feedback-discard-bitmap'], 'discard-bitmap')
    g(['ext_transform_feedback-discard-clear'], 'discard-clear')
    g(['ext_transform_feedback-discard-copypixels'], 'discard-copypixels')
    g(['ext_transform_feedback-discard-drawarrays'], 'discard-drawarrays')
    g(['ext_transform_feedback-discard-drawpixels'], 'discard-drawpixels')
    g(['ext_transform_feedback-immediate-reuse'], 'immediate-reuse')
    g(['ext_transform_feedback-immediate-reuse-index-buffer'],
      'immediate-reuse-index-buffer')
    g(['ext_transform_feedback-immediate-reuse-uniform-buffer'],
      'immediate-reuse-uniform-buffer')
    g(['ext_transform_feedback-max-varyings'], 'max-varyings')
    g(['ext_transform_feedback-nonflat-integral'], 'nonflat-integral')
    g(['ext_transform_feedback-overflow-edge-cases'], 'overflow-edge-cases')
    g(['ext_transform_feedback-overflow-edge-cases', 'use_gs'],
      'overflow-edge-cases use_gs')
    g(['ext_transform_feedback-points'], 'points')
    g(['ext_transform_feedback-points', 'large'], 'points-large')
    g(['ext_transform_feedback-position'], 'position-readback-bufferbase')
    g(['ext_transform_feedback-position', 'discard'],
      'position-readback-bufferbase-discard')
    g(['ext_transform_feedback-position', 'offset'],
      'position-readback-bufferoffset')
    g(['ext_transform_feedback-position', 'offset', 'discard'],
      'position-readback-bufferoffset-discard')
    g(['ext_transform_feedback-position', 'range'],
      'position-readback-bufferrange')
    g(['ext_transform_feedback-position', 'range', 'discard'],
      'position-readback-bufferrange-discard')
    g(['ext_transform_feedback-negative-prims'], 'negative-prims')
    g(['ext_transform_feedback-primgen'],
      'primgen-query transform-feedback-disabled')
    g(['ext_transform_feedback-pipeline-basic-primgen'],
      'pipeline-basic-primgen')
    g(['ext_transform_feedback-position', 'render'],
      'position-render-bufferbase')
    g(['ext_transform_feedback-position', 'render', 'discard'],
      'position-render-bufferbase-discard')
    g(['ext_transform_feedback-position', 'render', 'offset'],
      'position-render-bufferoffset')
    g(['ext_transform_feedback-position', 'render', 'offset', 'discard'],
      'position-render-bufferoffset-discard')
    g(['ext_transform_feedback-position', 'render', 'range'],
      'position-render-bufferrange')
    g(['ext_transform_feedback-position', 'render', 'range', 'discard'],
      'position-render-bufferrange-discard')
    g(['ext_transform_feedback-position', 'primgen'],
      'query-primitives_generated-bufferbase')
    g(['ext_transform_feedback-position', 'primgen', 'discard'],
      'query-primitives_generated-bufferbase-discard')
    g(['ext_transform_feedback-position', 'primgen', 'offset'],
      'query-primitives_generated-bufferoffset')
    g(['ext_transform_feedback-position', 'primgen', 'offset', 'discard'],
      'query-primitives_generated-bufferoffset-discard')
    g(['ext_transform_feedback-position', 'primgen', 'range'],
      'query-primitives_generated-bufferrange')
    g(['ext_transform_feedback-position', 'primgen', 'range', 'discard'],
      'query-primitives_generated-bufferrange-discard')
    g(['ext_transform_feedback-position', 'primwritten'],
      'query-primitives_written-bufferbase')
    g(['ext_transform_feedback-position', 'primwritten', 'discard'],
      'query-primitives_written-bufferbase-discard')
    g(['ext_transform_feedback-position', 'primwritten', 'offset'],
      'query-primitives_written-bufferoffset')
    g(['ext_transform_feedback-position', 'primwritten', 'offset', 'discard'],
      'query-primitives_written-bufferoffset-discard')
    g(['ext_transform_feedback-position', 'primwritten', 'range'],
      'query-primitives_written-bufferrange')
    g(['ext_transform_feedback-position', 'primwritten', 'range', 'discard'],
      'query-primitives_written-bufferrange-discard')
    g(['ext_transform_feedback-interleaved'], 'interleaved-attribs')
    g(['ext_transform_feedback-separate'], 'separate-attribs')
    g(['ext_transform_feedback-geometry-shaders-basic'],
      'geometry-shaders-basic')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_transform_feedback2')) as g:
    g(['arb_transform_feedback2-change-objects-while-paused'],
      'Change objects while paused', run_concurrent=False)
    g(['arb_transform_feedback2-change-objects-while-paused_gles3'],
      'Change objects while paused (GLES3)', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_transform_feedback2')) as g:
    g(['arb_transform_feedback2-draw-auto'], 'draw-auto', run_concurrent=False)
    g(['arb_transform_feedback2-istransformfeedback'], 'istranformfeedback',
      run_concurrent=False)
    g(['arb_transform_feedback2-gen-names-only'],
      'glGenTransformFeedbacks names only')
    g(['arb_transform_feedback2-cannot-bind-when-active'],
      'cannot bind when another object is active')
    g(['arb_transform_feedback2-api-queries'], 'misc. API queries')
    g(['arb_transform_feedback2-pause-counting'], 'counting with pause')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_transform_instanced')) as g:
    g(['arb_transform_feedback2-draw-auto', 'instanced'],
      'draw-auto instanced', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_transform_feedback3')) as g:
    g(['arb_transform_feedback3-bind_buffer_invalid_index'],
      'arb_transform_feedback3-bind_buffer_invalid_index',
      run_concurrent=False)
    g(['arb_transform_feedback3-query_with_invalid_index'],
      'arb_transform_feedback3-query_with_invalid_index', run_concurrent=False)
    g(['arb_transform_feedback3-end_query_with_name_zero'],
      'arb_transform_feedback3-end_query_with_name_zero', run_concurrent=False)
    g(['arb_transform_feedback3-draw_using_invalid_stream_index'],
      'arb_transform_feedback3-draw_using_invalid_stream_index',
      run_concurrent=False)
    g(['arb_transform_feedback3-set_varyings_with_invalid_args'],
      'arb_transform_feedback3-set_varyings_with_invalid_args',
      run_concurrent=False)
    g(['arb_transform_feedback3-set_invalid_varyings'],
      'arb_transform_feedback3-set_invalid_varyings', run_concurrent=False)
    g(['arb_transform_feedback3-ext_interleaved_two_bufs', 'vs'],
      'arb_transform_feedback3-ext_interleaved_two_bufs_vs',
      run_concurrent=False)
    g(['arb_transform_feedback3-ext_interleaved_two_bufs', 'gs'],
      'arb_transform_feedback3-ext_interleaved_two_bufs_gs',
      run_concurrent=False)
    g(['arb_transform_feedback3-ext_interleaved_two_bufs', 'gs_max'],
      'arb_transform_feedback3-ext_interleaved_two_bufs_gs_max',
      run_concurrent=False)

    for param in ['gl_NextBuffer-1', 'gl_NextBuffer-2', 'gl_SkipComponents1-1',
                  'gl_SkipComponents1-2', 'gl_SkipComponents1-3',
                  'gl_SkipComponents2', 'gl_SkipComponents3',
                  'gl_SkipComponents4',
                  'gl_NextBuffer-gl_SkipComponents1-gl_NextBuffer',
                  'gl_NextBuffer-gl_NextBuffer', 'gl_SkipComponents1234', 'gl_SkipComponents1-gl_NextBuffer']:
        g(['ext_transform_feedback-output-type', param], param)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_uniform_buffer_object')) as g:
    g(['arb_uniform_buffer_object-bindbuffer-general-point'],
      'bindbuffer-general-point')
    g(['arb_uniform_buffer_object-buffer-targets'], 'buffer-targets')
    g(['arb_uniform_buffer_object-bufferstorage'], 'bufferstorage')
    g(['arb_uniform_buffer_object-deletebuffers'], 'deletebuffers')
    g(['arb_uniform_buffer_object-dlist'], 'dlist')
    g(['arb_uniform_buffer_object-getactiveuniformblockiv-uniform-block-data-size'],
      'getactiveuniformblockiv-uniform-block-data-size')
    g(['arb_uniform_buffer_object-getactiveuniformblockname'],
      'getactiveuniformblockname')
    g(['arb_uniform_buffer_object-getactiveuniformname'],
      'getactiveuniformname')
    g(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-array-stride'],
      'getactiveuniformsiv-uniform-array-stride')
    g(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-block-index'],
      'getactiveuniformsiv-uniform-block-index')
    g(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-matrix-stride'],
      'getactiveuniformsiv-uniform-matrix-stride')
    g(['arb_uniform_buffer_object-getactiveuniformsiv-uniform-type'],
      'getactiveuniformsiv-uniform-type')
    g(['arb_uniform_buffer_object-getintegeri_v'], 'getintegeri_v')
    g(['arb_uniform_buffer_object-getprogramiv'], 'getprogramiv')
    g(['arb_uniform_buffer_object-getuniformblockindex'],
      'getuniformblockindex')
    g(['arb_uniform_buffer_object-getuniformindices'], 'getuniformindices')
    g(['arb_uniform_buffer_object-getuniformlocation'], 'getuniformlocation')
    g(['arb_uniform_buffer_object-layout-std140'], 'layout-std140')
    g(['arb_uniform_buffer_object-layout-std140-base-size-and-alignment'],
      'layout-std140-base-size-and-alignment')
    g(['arb_uniform_buffer_object-link-mismatch-blocks'],
      'link-mismatch-blocks')
    g(['arb_uniform_buffer_object-maxblocks'], 'maxblocks')
    g(['arb_uniform_buffer_object-minmax'], 'minmax')
    g(['arb_uniform_buffer_object-negative-bindbuffer-index'],
      'negative-bindbuffer-index')
    g(['arb_uniform_buffer_object-negative-bindbuffer-target'],
      'negative-bindbuffer-target')
    g(['arb_uniform_buffer_object-negative-bindbufferrange-range'],
      'negative-bindbufferrange-range')
    g(['arb_uniform_buffer_object-negative-getactiveuniformblockiv'],
      'negative-getactiveuniformblockiv')
    g(['arb_uniform_buffer_object-negative-getactiveuniformsiv'],
      'negative-getactiveuniformsiv')
    g(['arb_uniform_buffer_object-referenced-by-shader'],
      'referenced-by-shader')
    g(['arb_uniform_buffer_object-rendering'], 'rendering')
    g(['arb_uniform_buffer_object-rendering', 'offset'], 'rendering-offset')
    g(['arb_uniform_buffer_object-row-major'], 'row-major')
    g(['arb_uniform_buffer_object-uniformblockbinding'], 'uniformblockbinding')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_uniform_buffer_object',
                        'maxuniformblocksize')) as g:
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'vs'], 'vs')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'vsexceed'],
      'vsexceed')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'fs'], 'fs')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'fsexceed'],
      'fsexceed')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ati_draw_buffers')) as g:
    g(['ati_draw_buffers-arbfp'], run_concurrent=False)
    g(['ati_draw_buffers-arbfp-no-index'], 'arbfp-no-index',
      run_concurrent=False)
    g(['ati_draw_buffers-arbfp-no-option'], 'arbfp-no-option',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ati_envmap_bumpmap')) as g:
    g(['ati_envmap_bumpmap-bump'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_instanced_arrays')) as g:
    g(['arb_instanced_arrays-vertex-attrib-divisor-index-error'],
      run_concurrent=False)
    g(['arb_instanced_arrays-instanced_arrays'], run_concurrent=False)
    g(['arb_instanced_arrays-drawarrays'], run_concurrent=False)
    add_single_param_test_set(g, 'arb_instanced_arrays-instanced_arrays',
                              'vbo')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_internalformat_query')) as g:
    g(['arb_internalformat_query-api-errors'], 'misc. API error checks')
    g(['arb_internalformat_query-overrun'], 'buffer over-run checks')
    g(['arb_internalformat_query-minmax'], 'minmax')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_map_buffer_range')) as g:
    g(['map_buffer_range_error_check'], run_concurrent=False)
    g(['map_buffer_range_test'], run_concurrent=False)
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT', 'offset=0'],
      'MAP_INVALIDATE_RANGE_BIT offset=0')
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT',
       'increment-offset'], 'MAP_INVALIDATE_RANGE_BIT increment-offset')
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_RANGE_BIT',
       'decrement-offset'], 'MAP_INVALIDATE_RANGE_BIT decrement-offset')
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT', 'offset=0'],
      'MAP_INVALIDATE_BUFFER_BIT offset=0')
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT',
       'increment-offset'], 'MAP_INVALIDATE_BUFFER_BIT increment-offset')
    g(['map_buffer_range-invalidate', 'MAP_INVALIDATE_BUFFER_BIT',
       'decrement-offset'], 'MAP_INVALIDATE_BUFFER_BIT decrement-offset')
    g(['map_buffer_range-invalidate', 'CopyBufferSubData', 'offset=0'],
      'CopyBufferSubData offset=0')
    g(['map_buffer_range-invalidate', 'CopyBufferSubData', 'increment-offset'],
      'CopyBufferSubData increment-offset')
    g(['map_buffer_range-invalidate', 'CopyBufferSubData', 'decrement-offset'],
      'CopyBufferSubData decrement-offset')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_multisample')) as g:
    g(['arb_multisample-beginend'], 'beginend')
    g(['arb_multisample-pushpop'], 'pushpop')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_seamless_cube_map')) as g:
    g(['arb_seamless_cubemap'], run_concurrent=False)
    g(['arb_seamless_cubemap-initially-disabled'], run_concurrent=False)
    g(['arb_seamless_cubemap-three-faces-average'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'AMD_pinned_memory')) as g:
    g(['amd_pinned_memory', 'offset=0'], 'offset=0')
    g(['amd_pinned_memory', 'increment-offset'], 'increment-offset')
    g(['amd_pinned_memory', 'decrement-offset'], 'decrement-offset')
    g(['amd_pinned_memory', 'offset=0', 'map-buffer'], 'map-buffer offset=0')
    g(['amd_pinned_memory', 'increment-offset', 'map-buffer'],
      'map-buffer increment-offset')
    g(['amd_pinned_memory', 'decrement-offset', 'map-buffer'],
      'map-buffer decrement-offset')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_seamless_cubemap_per_texture')) as g:
    g(['amd_seamless_cubemap_per_texture'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_vertex_shader_layer')) as g:
    g(['amd_vertex_shader_layer-layered-2d-texture-render'],
      run_concurrent=False)
    g(['amd_vertex_shader_layer-layered-depth-texture-render'],
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_vertex_shader_viewport_index')) as g:
    g(['amd_vertex_shader_viewport_index-render'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_fog_coord')) as g:
    g(['ext_fog_coord-modes'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_texture_barrier')) as g:
    g(['blending-in-shader'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_conditional_render')) as g:
    g(['nv_conditional_render-begin-while-active'], 'begin-while-active')
    g(['nv_conditional_render-begin-zero'], 'begin-zero')
    g(['nv_conditional_render-bitmap'], 'bitmap', run_concurrent=False)
    g(['nv_conditional_render-blitframebuffer'], 'blitframebuffer',
      run_concurrent=False)
    g(['nv_conditional_render-clear'], 'clear', run_concurrent=False)
    g(['nv_conditional_render-copypixels'], 'copypixels',
      run_concurrent=False)
    g(['nv_conditional_render-copyteximage'], 'copyteximage',
      run_concurrent=False)
    g(['nv_conditional_render-copytexsubimage'], 'copytexsubimage',
      run_concurrent=False)
    g(['nv_conditional_render-dlist'], 'dlist', run_concurrent=False)
    g(['nv_conditional_render-drawpixels'], 'drawpixels', run_concurrent=False)
    g(['nv_conditional_render-generatemipmap'], 'generatemipmap',
      run_concurrent=False)
    g(['nv_conditional_render-vertex_array'], 'vertex_array',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_matrix_get')) as g:
    g(['oes_matrix_get-api'], 'All queries')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_fixed_point')) as g:
    g(['oes_fixed_point-attribute-arrays'], 'attribute-arrays')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_clear_buffer_object')) as g:
    g(['arb_clear_buffer_object-formats'])
    g(['arb_clear_buffer_object-invalid-internal-format'])
    g(['arb_clear_buffer_object-invalid-size'])
    g(['arb_clear_buffer_object-mapped'])
    g(['arb_clear_buffer_object-no-bound-buffer'])
    g(['arb_clear_buffer_object-null-data'])
    g(['arb_clear_buffer_object-sub-invalid-size'])
    g(['arb_clear_buffer_object-sub-mapped'])
    g(['arb_clear_buffer_object-sub-overlap'])
    g(['arb_clear_buffer_object-sub-simple'])
    g(['arb_clear_buffer_object-zero-size'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_clear_texture')) as g:
    g(['arb_clear_texture-clear-max-level'])
    g(['arb_clear_texture-simple'])
    g(['arb_clear_texture-error'])
    g(['arb_clear_texture-3d'])
    g(['arb_clear_texture-cube'])
    g(['arb_clear_texture-multisample'])
    g(['arb_clear_texture-integer'])
    g(['arb_clear_texture-base-formats'])
    g(['arb_clear_texture-sized-formats'])
    g(['arb_clear_texture-float'])
    g(['arb_clear_texture-rg'])
    g(['arb_clear_texture-depth-stencil'])
    g(['arb_clear_texture-srgb'])
    g(['arb_clear_texture-stencil'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_copy_buffer')) as g:
    g(['copy_buffer_coherency'], run_concurrent=False)
    g(['copybuffersubdata'], run_concurrent=False)
    g(['arb_copy_buffer-data-sync'], 'data-sync')
    g(['arb_copy_buffer-dlist'], 'dlist')
    g(['arb_copy_buffer-get'], 'get')
    g(['arb_copy_buffer-negative-bound-zero'], 'negative-bound-zero')
    g(['arb_copy_buffer-negative-bounds'], 'negative-bounds')
    g(['arb_copy_buffer-negative-mapped'], 'negative-mapped')
    g(['arb_copy_buffer-overlap'], 'overlap')
    g(['arb_copy_buffer-targets'], 'targets')
    g(['arb_copy_buffer-subdata-sync'], 'subdata-sync')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_copy_image')) as g:
    g(['arb_copy_image-simple', '--tex-to-tex'])
    g(['arb_copy_image-simple', '--rb-to-tex'])
    g(['arb_copy_image-simple', '--rb-to-rb'])
    g(['arb_copy_image-srgb-copy'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '0', '0', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '12', '11', '0', '0', '5', '0', '9',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_2D', '32', '32', '1', '11', '0', '0', '5', '13', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_RECTANGLE', '32', '32', '1', '11', '0', '0', '5', '13',
       '0', '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_2D_ARRAY', '32', '32', '10', '11', '0', '0', '5', '13',
       '4', '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '0', '0', '5', '13', '4',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '30', '11', '0', '0', '5',
       '13', '8', '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D', '32', '1', '1',
       'GL_TEXTURE_3D', '32', '32', '32', '11', '0', '0', '5', '13', '4',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '0', '7', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '0', '3', '5', '0', '7',
       '14', '1', '8'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '0', '3', '5', '7', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '0', '3', '5', '7', '0',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '18', '11', '0', '3', '5', '9', '7',
       '14', '1', '8'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '0', '3', '5', '17', '2',
       '14', '1', '3'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '0', '3', '5',
       '17', '2', '14', '1', '7'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_1D_ARRAY', '32', '1', '12',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '0', '3', '5', '9', '2', '14',
       '1', '7'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '0', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '0', '5', '0', '7',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14',
       '9', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '0', '5', '7',
       '0', '14', '9', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '0', '5', '7',
       '12', '14', '8', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '0', '5', '9', '2',
       '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '0', '5',
       '9', '7', '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D', '32', '32', '1',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14',
       '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '0', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '0', '5', '0', '7',
       '14', '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '0', '5', '7', '0', '14',
       '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '0', '5', '7',
       '0', '14', '9', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '0', '5', '7',
       '12', '14', '8', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '0', '5', '9', '2',
       '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '0', '5',
       '9', '7', '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_RECTANGLE', '32', '32', '1',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14',
       '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7',
       '14', '1', '7'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '4', '0',
       '14', '10', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7',
       '0', '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '7',
       '2', '14', '9', '9'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '1', '5', '9', '2',
       '14', '7', '3'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '2', '5',
       '9', '7', '14', '7', '11'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_2D_ARRAY', '32', '32', '15',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14',
       '7', '11'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '3', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '3', '5', '0', '7',
       '14', '1', '2'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '3', '5', '7', '0', '14',
       '9', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '3', '5', '3',
       '0', '14', '12', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '1', '5', '3',
       '2', '14', '11', '4'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_CUBE_MAP', '32', '32', '6', '11', '5', '1', '5', '9', '2',
       '14', '7', '3'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18', '11', '5', '1', '5',
       '9', '9', '14', '7', '5'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP', '32', '32', '6',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '0', '5', '9', '7', '14',
       '7', '4'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7',
       '14', '1', '7'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '7', '0',
       '14', '8', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7',
       '0', '14', '6', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '1',
       '2', '14', '15', '9'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_CUBE_MAP', '16', '16', '6', '11', '5', '1', '5', '9', '2',
       '5', '7', '3'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '16', '16', '18', '11', '5', '2', '5',
       '9', '7', '5', '7', '11'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_CUBE_MAP_ARRAY', '32', '32', '18',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14',
       '7', '11'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_1D', '32', '1', '1', '11', '23', '7', '5', '0', '0', '14',
       '1', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_1D_ARRAY', '32', '1', '16', '11', '2', '5', '5', '0', '7',
       '14', '1', '7'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_2D', '32', '16', '1', '11', '12', '13', '5', '7', '0',
       '14', '7', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_RECTANGLE', '32', '16', '1', '11', '12', '13', '5', '7',
       '0', '14', '9', '1'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_2D_ARRAY', '32', '16', '15', '11', '12', '5', '5', '3',
       '2', '14', '13', '9'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_CUBE_MAP', '16', '16', '6', '11', '5', '1', '5', '9', '2',
       '5', '7', '3'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_CUBE_MAP_ARRAY', '16', '16', '18', '11', '5', '2', '5',
       '9', '7', '5', '7', '11'])
    g(['arb_copy_image-targets', 'GL_TEXTURE_3D', '32', '32', '17',
       'GL_TEXTURE_3D', '32', '16', '18', '11', '5', '2', '5', '9', '7', '14',
       '7', '11'])
    g(['arb_copy_image-formats'])
    g(['arb_copy_image-formats', '--samples=2'])
    g(['arb_copy_image-formats', '--samples=4'])
    g(['arb_copy_image-formats', '--samples=8'])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_cull_distance')) as g:
    g(['arb_cull_distance-max-distances'])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_half_float_vertex')) as g:
    g(['draw-vertices-half-float'], run_concurrent=False)
    g(['draw-vertices-half-float', 'user'], 'draw-vertices-half-float-user',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_type_2_10_10_10_rev')) as g:
    g(['draw-vertices-2101010'], run_concurrent=False)
    g(['attribs', 'GL_ARB_vertex_type_2_10_10_10_rev'], 'attribs')
    g(['arb_vertex_type_2_10_10_10_rev-array_types'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_type_10f_11f_11f_rev')) as g:
    g(['arb_vertex_type_10f_11f_11f_rev-api-errors'], run_concurrent=False)
    g(['arb_vertex_type_10f_11f_11f_rev-draw-vertices'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_draw_buffers')) as g:
    g(['arb_draw_buffers-state_change'], run_concurrent=False)
    g(['fbo-mrt-alphatest'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_draw_buffers2')) as g:
    g(['fbo-drawbuffers2-blend'], run_concurrent=False)
    g(['fbo-drawbuffers2-colormask'], run_concurrent=False)
    g(['fbo-drawbuffers2-colormask', 'clear'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_draw_buffers_blend')) as g:
    g(['fbo-draw-buffers-blend'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_blend_func_extended')) as g:
    g(['arb_blend_func_extended-bindfragdataindexed-invalid-parameters'],
      run_concurrent=False)
    g(['arb_blend_func_extended-blend-api'], run_concurrent=False)
    g(['arb_blend_func_extended-error-at-begin'], run_concurrent=False)
    g(['arb_blend_func_extended-getfragdataindex'], run_concurrent=False)
    g(['arb_blend_func_extended-fbo-extended-blend'], run_concurrent=False)
    g(['arb_blend_func_extended-fbo-extended-blend-explicit'],
      run_concurrent=False)
    g(['arb_blend_func_extended-fbo-extended-blend-pattern'],
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_base_instance')) as g:
    g(['arb_base_instance-baseinstance-doesnt-affect-gl-instance-id'],
      run_concurrent=False)
    g(['arb_base_instance-drawarrays'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_buffer_storage')) as g:
    g(['bufferstorage-persistent', 'draw'])
    g(['bufferstorage-persistent', 'draw', 'coherent'])
    g(['bufferstorage-persistent', 'draw', 'client-storage'])
    g(['bufferstorage-persistent', 'draw', 'coherent', 'client-storage'])
    g(['bufferstorage-persistent', 'read'])
    g(['bufferstorage-persistent', 'read', 'coherent'])
    g(['bufferstorage-persistent', 'read', 'client-storage'])
    g(['bufferstorage-persistent', 'read', 'coherent', 'client-storage'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'apple_object_purgeable')) as g:
    g(['object_purgeable-api-pbo'], run_concurrent=False)
    g(['object_purgeable-api-texture'], run_concurrent=False)
    g(['object_purgeable-api-vbo'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_read_format')) as g:
    g(['oes-read-format'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_primitive_restart')) as g:
    add_single_param_test_set(
        g,
        'primitive-restart',
        "DISABLE_VBO",
        "VBO_VERTEX_ONLY", "VBO_INDEX_ONLY",
        "VBO_SEPARATE_VERTEX_AND_INDEX", "VBO_COMBINED_VERTEX_AND_INDEX")
    add_single_param_test_set(
        g,
        'primitive-restart-draw-mode',
        'points', 'lines', 'line_loop', 'line_strip', 'triangles',
        'triangle_strip', 'triangle_fan', 'quads', 'quad_strip', 'polygon')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_provoking_vertex')) as g:
    g(['provoking-vertex'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_lod_bias')) as g:
    g(['lodbias'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'sgis_generate_mipmap')) as g:
    g(['gen-nonzero-unit'], run_concurrent=False)
    g(['gen-teximage'], run_concurrent=False)
    g(['gen-texsubimage'], run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_map_buffer_alignment')) as g:
    g(['arb_map_buffer_alignment-sanity_test'], run_concurrent=False)
    g(['arb_map_buffer_alignment-map-invalidate-range'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_geometry_shader4')) as g:
    g(['arb_geometry_shader4-program-parameter-input-type'])
    g(['arb_geometry_shader4-program-parameter-input-type-draw'])
    g(['arb_geometry_shader4-program-parameter-output-type'])
    g(['arb_geometry_shader4-vertices-in'])

    for draw in ['', 'indexed']:
        for prim in ['GL_LINES_ADJACENCY', 'GL_LINE_STRIP_ADJACENCY',
                     'GL_TRIANGLES_ADJACENCY', 'GL_TRIANGLE_STRIP_ADJACENCY']:
            g(['arb_geometry_shader4-ignore-adjacent-vertices', draw, prim])

    for mode in ['1', 'tf 1', 'max', 'tf max']:
        g(['arb_geometry_shader4-program-parameter-vertices-out', mode])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_compute_shader')) as g:
    g(['arb_compute_shader-api_errors'], 'api_errors')
    g(['arb_compute_shader-minmax'], 'minmax')
    g(['built-in-constants',
       os.path.join(TESTS_DIR, 'spec', 'arb_compute_shader',
                    'minimum-maximums.txt')],
      'built-in constants')
    g(['arb_compute_shader-work_group_size_too_large'],
      grouptools.join('compiler', 'work_group_size_too_large'))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_storage_buffer_object')) as g:
    g(['arb_shader_storage_buffer_object-minmax'], 'minmax')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_polygon_offset_clamp')) as g:
    g(['ext_polygon_offset_clamp-draw'])
    g(['ext_polygon_offset_clamp-dlist'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_pipeline_statistics_query')) as g:
    g(['arb_pipeline_statistics_query-vert'])
    g(['arb_pipeline_statistics_query-vert_adj'])
    g(['arb_pipeline_statistics_query-clip'])
    g(['arb_pipeline_statistics_query-geom'])
    g(['arb_pipeline_statistics_query-frag'])
    g(['arb_pipeline_statistics_query-comp'])

with profile.group_manager(PiglitGLTest, 'hiz') as g:
    g(['hiz-depth-stencil-test-fbo-d0-s8'], run_concurrent=False)
    g(['hiz-depth-stencil-test-fbo-d24-s0'], run_concurrent=False)
    g(['hiz-depth-stencil-test-fbo-d24-s8'], run_concurrent=False)
    g(['hiz-depth-stencil-test-fbo-d24s8'], run_concurrent=False)
    g(['hiz-depth-read-fbo-d24-s0'], run_concurrent=False)
    g(['hiz-depth-read-fbo-d24-s8'], run_concurrent=False)
    g(['hiz-depth-read-fbo-d24s8'], run_concurrent=False)
    g(['hiz-depth-read-window-stencil0'], run_concurrent=False)
    g(['hiz-depth-read-window-stencil1'], run_concurrent=False)
    g(['hiz-depth-test-fbo-d24-s0'], run_concurrent=False)
    g(['hiz-depth-test-fbo-d24-s8'], run_concurrent=False)
    g(['hiz-depth-test-fbo-d24s8'], run_concurrent=False)
    g(['hiz-depth-test-window-stencil0'], run_concurrent=False)
    g(['hiz-depth-test-window-stencil1'], run_concurrent=False)
    g(['hiz-stencil-read-fbo-d0-s8'], run_concurrent=False)
    g(['hiz-stencil-read-fbo-d24-s8'], run_concurrent=False)
    g(['hiz-stencil-read-fbo-d24s8'], run_concurrent=False)
    g(['hiz-stencil-read-window-depth0'], run_concurrent=False)
    g(['hiz-stencil-read-window-depth1'], run_concurrent=False)
    g(['hiz-stencil-test-fbo-d0-s8'], run_concurrent=False)
    g(['hiz-stencil-test-fbo-d24-s8'], run_concurrent=False)
    g(['hiz-stencil-test-fbo-d24s8'], run_concurrent=False)
    g(['hiz-stencil-test-window-depth0'], run_concurrent=False)
    g(['hiz-stencil-test-window-depth1'], run_concurrent=False)

with profile.group_manager(PiglitGLTest, 'fast_color_clear') as g:
    g(['fcc-blit-between-clears'])
    g(['fcc-read-to-pbo-after-clear'], run_concurrent=False)

    for subtest in ('sample', 'read_pixels', 'blit', 'copy'):
        for buffer_type in ('rb', 'tex'):
            if subtest == 'sample' and buffer_type == 'rb':
                continue
            g(['fcc-read-after-clear', subtest, buffer_type])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_unpack_subimage')) as g:
    g(['ext_unpack_subimage'], 'basic')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'oes_draw_texture')) as g:
    g(['oes_draw_texture'])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_compressed_etc1_rgb8_texture')) as g:
    g(['oes_compressed_etc1_rgb8_texture-basic'], 'basic')
    g(['oes_compressed_etc1_rgb8_texture-miptree'], 'miptree')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_compressed_paletted_texture')) as g:
    g(['oes_compressed_paletted_texture-api'], 'basic API')
    g(['arb_texture_compression-invalid-formats', 'paletted'],
      'invalid formats')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl 1.4'),
        exclude_platforms=['glx']) as g:
    g(['egl-create-surface'], 'eglCreateSurface', run_concurrent=False)
    g(['egl-query-surface', '--bad-attr'], 'eglQuerySurface EGL_BAD_ATTRIBUTE',
      run_concurrent=False)
    g(['egl-query-surface', '--bad-surface'],
      'eglQuerySurface EGL_BAD_SURFACE',
      run_concurrent=False)
    g(['egl-query-surface', '--attr=EGL_HEIGHT'], 'eglQuerySurface EGL_HEIGHT',
      run_concurrent=False)
    g(['egl-query-surface', '--attr=EGL_WIDTH'], 'eglQuerySurface EGL_WIDTH',
      run_concurrent=False)
    g(['egl-terminate-then-unbind-context'],
      'eglTerminate then unbind context',
      run_concurrent=False)
    g(['egl-create-pbuffer-surface'],
      'eglCreatePbufferSurface and then glClear',
      run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_nok_swap_region'),
        exclude_platforms=['glx']) as g:
    g(['egl-nok-swap-region'], 'basic', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_nok_texture_from_pixmap'),
        exclude_platforms=['glx']) as g:
    g(['egl-nok-texture-from-pixmap'], 'basic', run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_create_context'),
        exclude_platforms=['glx']) as g:
    g(['egl-create-context-default-major-version-gles'],
      'default major version GLES', run_concurrent=False)
    g(['egl-create-context-default-major-version-gl'],
      'default major version GL', run_concurrent=False)
    g(['egl-create-context-default-minor-version-gles'],
      'default minor version GLES', run_concurrent=False)
    g(['egl-create-context-default-minor-version-gl'],
      'default minor version GL', run_concurrent=False)
    g(['egl-create-context-valid-attribute-empty-gles'],
      'valid attribute empty GLES', run_concurrent=False)
    g(['egl-create-context-valid-attribute-empty-gl'],
      'valid attribute empty GL', run_concurrent=False)
    g(['egl-create-context-valid-attribute-null-gles'],
      'NULL valid attribute GLES', run_concurrent=False)
    g(['egl-create-context-valid-attribute-null-gl'],
      'NULL valid attribute GL', run_concurrent=False)
    g(['egl-create-context-invalid-gl-version'], 'invalid OpenGL version',
      run_concurrent=False)
    g(['egl-create-context-invalid-attribute-gles'], 'invalid attribute GLES',
      run_concurrent=False)
    g(['egl-create-context-invalid-attribute-gl'], 'invalid attribute GL',
      run_concurrent=False)
    g(['egl-create-context-invalid-flag-gles'], 'invalid flag GLES',
      run_concurrent=False)
    g(['egl-create-context-invalid-flag-gl'], 'invalid flag GL',
      run_concurrent=False)
    g(['egl-create-context-valid-flag-forward-compatible-gl'],
      'valid forward-compatible flag GL', run_concurrent=False)
    g(['egl-create-context-invalid-profile'], 'invalid profile',
      run_concurrent=False)
    g(['egl-create-context-core-profile'], '3.2 core profile required',
      run_concurrent=False)
    g(['egl-create-context-pre-GL32-profile'], 'pre-GL3.2 profile',
      run_concurrent=False)
    g(['egl-create-context-verify-gl-flavor'], 'verify GL flavor',
      run_concurrent=False)
    g(['egl-create-context-valid-flag-debug-gl', 'gl'], 'valid debug flag GL',
      run_concurrent=False)

    for api in ('gles1', 'gles2', 'gles3'):
        g(['egl-create-context-valid-flag-debug-gles', api],
          'valid debug flag {}'.format(api), run_concurrent=False)

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_mesa_configless_context'),
        exclude_platforms=['glx']) as g:
    g(['egl-configless-context'], 'basic')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_ext_client_extensions'),
        exclude_platforms=['glx']) as g:
    for i in [1, 2, 3]:
        g(['egl_ext_client_extensions', str(i)],
          'conformance test {0}'.format(i))

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_fence_sync'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_fence_sync'], 'conformance')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_wait_sync'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_fence_sync', 'wait_sync'], 'conformance')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_chromium_sync_control'),
        exclude_platforms=['glx']) as g:
    g(['egl_chromium_sync_control'], 'conformance')

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', '!opengl ES 2.0')) as g:
    g(['glsl-fs-pointcoord_gles2'], 'glsl-fs-pointcoord')
    g(['invalid-es3-queries_gles2'])
    g(['link-no-vsfs_gles2'], 'link-no-vsfs')
    g(['minmax_gles2'])
    g(['multiple-shader-objects_gles2'])
    g(['fbo_discard_gles2'])
    g(['draw_buffers_gles2'])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', '!opengl ES 3.0')) as g:
    g(['minmax_gles3'], 'minmax')
    g(['texture-immutable-levels_gles3'], 'texture-immutable-levels')
    g(['gles-3.0-drawarrays-vertexid'], 'gl_VertexID used with glDrawArrays')

    for test_mode in ['teximage', 'texsubimage']:
        g(['ext_texture_array-compressed_gles3', test_mode, '-fbo'],
          'ext_texture_array-compressed_gles3 {0}'.format(test_mode),
          run_concurrent=False)

    for tex_format in ['rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11',
                       'rgb8-punchthrough-alpha1',
                       'srgb8-punchthrough-alpha1']:
        g(['oes_compressed_etc2_texture-miptree_gles3', tex_format])

with profile.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_es3_compatibility')) as g:
    g(['es3-primrestart-fixedindex'])
    g(['es3-drawarrays-primrestart-fixedindex'])

    for tex_format in ['rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11',
                       'rgb8-punchthrough-alpha1',
                       'srgb8-punchthrough-alpha1']:
        for context in ['core', 'compat']:
            g(['oes_compressed_etc2_texture-miptree', tex_format, context])

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_atomic_counters')) as g:
    g(['arb_shader_atomic_counters-active-counters'], 'active-counters')
    g(['arb_shader_atomic_counters-array-indexing'], 'array-indexing')
    g(['arb_shader_atomic_counters-buffer-binding'], 'buffer-binding')
    g(['arb_shader_atomic_counters-default-partition'], 'default-partition')
    g(['arb_shader_atomic_counters-fragment-discard'], 'fragment-discard')
    g(['arb_shader_atomic_counters-function-argument'], 'function-argument')
    g(['arb_shader_atomic_counters-max-counters'], 'max-counters')
    g(['arb_shader_atomic_counters-minmax'], 'minmax')
    g(['arb_shader_atomic_counters-multiple-defs'], 'multiple-defs')
    g(['arb_shader_atomic_counters-semantics'], 'semantics')
    g(['arb_shader_atomic_counters-unique-id'], 'unique-id')
    g(['arb_shader_atomic_counters-unused-result'], 'unused-result')
    g(['arb_shader_atomic_counters-respecify-buffer'], 'respecify-buffer')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_direct_state_access')) as g:
    g(['arb_direct_state_access-create-transformfeedbacks'],
      'create-transformfeedbacks')
    g(['arb_direct_state_access-transformfeedback-bufferbase'],
      'transformfeedback-bufferbase')
    g(['arb_direct_state_access-transformfeedback-bufferrange'],
      'transformfeedback-bufferrange')
    g(['arb_direct_state_access-gettransformfeedback'], 'gettransformfeedback')
    g(['arb_direct_state_access-create-renderbuffers'], 'create-renderbuffers')
    g(['arb_direct_state_access-namedrenderbuffer'], 'namedrenderbuffer')
    g(['arb_direct_state_access-dsa-textures'], 'dsa-textures')
    g(['arb_direct_state_access-texturesubimage'], 'texturesubimage')
    g(['arb_direct_state_access-bind-texture-unit'], 'bind-texture-unit')
    g(['arb_direct_state_access-create-textures'], 'create-textures')
    g(['arb_direct_state_access-texture-storage'], 'textures-storage')
    g(['arb_direct_state_access-texunits'], 'texunits')
    g(['arb_direct_state_access-texture-params'], 'texture-params')
    g(['arb_direct_state_access-copytexturesubimage'], 'copytexturesubimage')
    g(['arb_direct_state_access-texture-errors'], 'texture-errors')
    g(['arb_direct_state_access-get-textures'], 'get-textures')
    g(['arb_direct_state_access-gettextureimage-formats'],
      'gettextureimage-formats')
    g(['arb_direct_state_access-gettextureimage-formats', 'init-by-rendering'],
      'gettextureimage-formats init-by-rendering')
    g(['arb_direct_state_access-gettextureimage-luminance'],
      'gettextureimage-luminance')
    g(['arb_direct_state_access-gettextureimage-simple'],
      'gettextureimage-simple')
    g(['arb_direct_state_access-gettextureimage-targets'],
      'gettextureimage-targets')
    g(['arb_direct_state_access-compressedtextureimage',
       'GL_COMPRESSED_RGBA_S3TC_DXT5_EXT'],
      'compressedtextureimage GL_COMPRESSED_RGBA_S3TC_DXT5_EXT')
    g(['arb_direct_state_access-getcompressedtextureimage'],
      'getcompressedtextureimage')
    g(['arb_direct_state_access-texture-storage-multisample'],
      'texture-storage-multisample')
    g(['arb_direct_state_access-texture-buffer'], 'texture-buffer')
    g(['arb_direct_state_access-create-samplers'], 'create-samplers')
    g(['arb_direct_state_access-create-programpipelines'],
      'create-programpipelines')
    g(['arb_direct_state_access-create-queries'], 'create-queries')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_image_load_store')) as g:
    g(['arb_shader_image_load_store-atomicity'], 'atomicity')
    g(['arb_shader_image_load_store-bitcast'], 'bitcast')
    g(['arb_shader_image_load_store-coherency'], 'coherency')
    g(['arb_shader_image_load_store-dead-fragments'], 'dead-fragments')
    g(['arb_shader_image_load_store-early-z'], 'early-z')
    g(['arb_shader_image_load_store-host-mem-barrier'], 'host-mem-barrier')
    g(['arb_shader_image_load_store-indexing'], 'indexing')
    g(['arb_shader_image_load_store-invalid'], 'invalid')
    g(['arb_shader_image_load_store-layer'], 'layer')
    g(['arb_shader_image_load_store-level'], 'level')
    g(['arb_shader_image_load_store-max-images'], 'max-images')
    g(['arb_shader_image_load_store-max-size'], 'max-size')
    g(['arb_shader_image_load_store-minmax'], 'minmax')
    g(['arb_shader_image_load_store-qualifiers'], 'qualifiers')
    g(['arb_shader_image_load_store-restrict'], 'restrict')
    g(['arb_shader_image_load_store-semantics'], 'semantics')
    g(['arb_shader_image_load_store-shader-mem-barrier'], 'shader-mem-barrier')
    g(['arb_shader_image_load_store-state'], 'state')
    g(['arb_shader_image_load_store-unused'], 'unused')

with profile.group_manager(
    PiglitGLTest,
    grouptools.join('spec', 'arb_shader_image_size')) as g:
    g(['arb_shader_image_size-builtin'], 'builtin')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_stencil8')) as g:
    g(['arb_texture_stencil8-draw'], 'draw')
    g(['arb_texture_stencil8-getteximage'], 'getteximage')
    g(['arb_texture_stencil8-stencil-texture'], 'stencil-texture')
    g(['arb_texture_stencil8-fbo-stencil8', 'clear', 'GL_STENCIL_INDEX8'], 'fbo-stencil-clear')
    g(['arb_texture_stencil8-fbo-stencil8', 'blit', 'GL_STENCIL_INDEX8'], 'fbo-stencil-blit')
    g(['arb_texture_stencil8-fbo-stencil8', 'readpixels', 'GL_STENCIL_INDEX8'], 'fbo-stencil-readpixels')
    add_fbo_formats_tests(g, 'GL_ARB_texture_stencil8')
    add_texwrap_format_tests(g, 'GL_ARB_texture_stencil8')

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_attrib_64bit')) as g:
    g(['arb_vertex_attrib_64bit-double_attribs'], 'double_attribs')
    g(['arb_vertex_attrib_64bit-check-explicit-location'], 'check-explicit-location')

if platform.system() is 'Windows':
    profile.filter_tests(lambda p, _: not p.startswith('glx'))
