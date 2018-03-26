# -*- coding: utf-8 -*-
# All tests that come with piglit, using default settings

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools
import os
import platform

from six.moves import range

from framework import grouptools
from framework import options
from framework.profile import TestProfile
from framework.driver_classifier import DriverClassifier
from framework.test.piglit_test import (
    PiglitGLTest, PiglitBaseTest, BuiltInConstantsTest
)
from .py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR

__all__ = ['profile']

# Disable bad hanging indent errors in pylint
# There is a bug in pylint which causes the profile.test_list.group_manager to
# be tagged as bad hanging indent, even though it seems to be correct (and
# similar syntax doesn't trigger an error)
# pylint: disable=bad-continuation

# Shadowing variables is a bad practice. It's just nearly impossible with the
# format of this module to avoid it.
# pylint: disable=redefined-outer-name


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
    adder(['fbo-stencil', 'clear', format],
          'fbo-stencil-{}-clear'.format(format))
    adder(['fbo-stencil', 'readpixels', format],
          'fbo-stencil-{}-readpixels'.format(format))
    adder(['fbo-stencil', 'drawpixels', format],
          'fbo-stencil-{}-drawpixels'.format(format))
    adder(['fbo-stencil', 'copypixels', format],
          'fbo-stencil-{}-copypixels'.format(format))
    adder(['fbo-stencil', 'blit', format],
          'fbo-stencil-{}-blit'.format(format))


def add_fbo_depthstencil_tests(adder, format, num_samples):
    assert format, \
        'add_fbo_depthstencil_tests argument "format" cannot be empty'

    if format == 'default_fb':
        prefix = ''
        concurrent = False
    else:
        prefix = 'fbo-'
        concurrent = True

    if int(num_samples) > 1:
        suffix = ' samples=' + num_samples
        psamples = '-samples=' + num_samples
    else:
        suffix = ''
        psamples = ''

    adder(['fbo-depthstencil', 'clear', format, psamples],
          '{}depthstencil-{}-clear{}'.format(prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'readpixels', format, 'FLOAT-and-USHORT',
           psamples],
          '{}depthstencil-{}-readpixels-FLOAT-and-USHORT{}'.format(
              prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'readpixels', format, '24_8', psamples],
          '{}depthstencil-{}-readpixels-24_8{}'.format(prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'readpixels', format, '32F_24_8_REV', psamples],
          '{}depthstencil-{}-readpixels-32F_24_8_REV{}'.format(
              prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'drawpixels', format, 'FLOAT-and-USHORT',
           psamples],
          '{}depthstencil-{}-drawpixels-FLOAT-and-USHORT{}'.format(
              prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'drawpixels', format, '24_8', psamples],
          '{}depthstencil-{}-drawpixels-24_8{}'.format(prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'drawpixels', format, '32F_24_8_REV', psamples],
          '{}depthstencil-{}-drawpixels-32F_24_8_REV{}'.format(
              prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'copypixels', format, psamples],
          '{}depthstencil-{}-copypixels{}'.format(prefix, format, suffix),
          run_concurrent=concurrent)
    adder(['fbo-depthstencil', 'blit', format, psamples],
          '{}depthstencil-{}-blit{}'.format(prefix, format, suffix),
          run_concurrent=concurrent)


def add_msaa_visual_plain_tests(adder, args, **kwargs):
    assert isinstance(args, list)

    adder(args, **kwargs)
    for sample_count in MSAA_SAMPLE_COUNTS:
        adder(args + ['-samples={}'.format(sample_count)],
              ' '.join(args + ['samples={}'.format(sample_count)]),
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
        adder(['ext_framebuffer_multisample-fast-clear',
               extension,
               'single-sample'],
              'fbo-fast-clear{}'.format(suffix))


def add_msaa_formats_tests(adder, extension):
    for sample_count in MSAA_SAMPLE_COUNTS:
        adder(['ext_framebuffer_multisample-formats', sample_count,
               extension],
              'multisample-formats {} {}'.format(sample_count, extension))
    adder(['ext_framebuffer_multisample-fast-clear', extension],
          'multisample-fast-clear {}'.format(extension))


def add_texwrap_target_tests(adder, target):
    adder(['texwrap', target, 'GL_RGBA8'],
          'texwrap {}'.format(target))
    if target == '2D':
        adder(['texwrap', target, 'GL_RGBA8', 'offset'],
              'texwrap {} offset'.format(target))
    adder(['texwrap', target, 'GL_RGBA8', 'bordercolor'],
          'texwrap {} bordercolor'.format(target))
    adder(['texwrap', target, 'GL_RGBA8', 'proj'],
          'texwrap {} proj'.format(target))
    adder(['texwrap', target, 'GL_RGBA8', 'proj', 'bordercolor'],
          'texwrap {} proj bordercolor'.format(target))


def add_texwrap_format_tests(adder, ext='', suffix=''):
    args = [] if ext == '' else [ext]
    adder(['texwrap'] + args, 'texwrap formats{}'.format(suffix))
    if 'compression' not in ext and 's3tc' not in ext:
        adder(['texwrap'] + args + ['offset'],
              'texwrap formats{} offset'.format(suffix))
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
profile = TestProfile()  # pylint: disable=invalid-name

# Find and add all apitrace tests.
classifier = DriverClassifier()
for basedir in [os.path.join(TESTS_DIR, 'apitrace', 'traces')]:
    for dirpath, _, filenames in os.walk(basedir):
        base_group = grouptools.from_path(os.path.join(
            'apitrace', os.path.relpath(dirpath, basedir)))

        for filename in filenames:
            if not os.path.splitext(filename)[1] == '.trace':
                continue
            group = grouptools.join(base_group, filename)

            profile.test_list[group] = PiglitBaseTest(
                [os.path.join(TESTS_DIR, 'apitrace', 'test-trace.py'),
                 os.path.join(dirpath, filename),
                 ','.join(classifier.categories)],
                run_concurrent=True)

# List of all of the MSAA sample counts we wish to test
MSAA_SAMPLE_COUNTS = ['2', '4', '6', '8', '16', '32']

with profile.test_list.group_manager(PiglitGLTest, 'security') as g:
    g(['initialized-texmemory'], run_concurrent=False)
    g(['initialized-fbo'], run_concurrent=False)
    g(['initialized-vbo'], run_concurrent=False)

with profile.test_list.group_manager(PiglitGLTest, 'shaders') as g:
    g(['activeprogram-bad-program'])
    g(['activeprogram-get'])
    g(['attribute0'])
    g(['createshaderprogram-bad-type'])
    g(['createshaderprogram-attached-shaders'])
    g(['glsl-arb-fragment-coord-conventions'])
    g(['glsl-bug-22603'])
    g(['glsl-bindattriblocation'])
    g(['glsl-dlist-getattriblocation'])
    g(['glsl-getactiveuniform-array-size'])
    g(['glsl-getactiveuniform-length'])
    g(['glsl-getattriblocation'])
    g(['getuniform-01'])
    g(['getuniform-02'])
    g(['getuniform-03'])
    g(['glsl-invalid-asm-01'])
    g(['glsl-invalid-asm-02'])
    g(['glsl-novertexdata'])
    g(['glsl-preprocessor-comments'])
    g(['glsl-reload-source'])
    g(['glsl-cache-fallback-shader-source'])
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
    g(['glsl-max-vertex-attrib'])
    g(['glsl-kwin-blur-1'])
    g(['glsl-kwin-blur-2'])
    g(['gpu_shader4_attribs'])
    g(['link-unresolved-function'])
    g(['shadersource-no-compile'])
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
    g(['point-vertex-id', 'gl_VertexID'])
    g(['point-vertex-id', 'gl_InstanceID'])
    g(['point-vertex-id', 'gl_VertexID', 'gl_InstanceID'])
    g(['point-vertex-id', 'divisor'])
    g(['point-vertex-id', 'gl_VertexID', 'divisor'])
    g(['point-vertex-id', 'gl_InstanceID', 'divisor'])
    g(['point-vertex-id', 'gl_VertexID', 'gl_InstanceID', 'divisor'])
    g(['glsl-vs-int-attrib'])
    g(['unsuccessful-relink'])
    g(['glsl-link-initializer-03'],
      'GLSL link two programs, global initializer')
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

    for subtest in ('texture2D',
                    'bias',
                    'textureGrad',
                    'texelFetch',
                    'textureLod',
                    'textureSize',
                    'textureQueryLOD',
                    'textureGather'):
        g(['zero-tex-coord', subtest])

with profile.test_list.group_manager(
        PiglitGLTest, 'glx',
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-destroycontext-1'], run_concurrent=False)
    g(['glx-destroycontext-2'], run_concurrent=False)
    g(['glx-dont-care-mask'], run_concurrent=False)
    g(['glx-close-display'], run_concurrent=False)
    g(['glx-fbconfig-sanity'], run_concurrent=False)
    g(['glx-fbconfig-compliance'], run_concurrent=False)
    g(['glx-fbconfig-bad'], run_concurrent=False)
    g(['glx-fbo-binding'], run_concurrent=False)
    g(['glx-multi-context-front'], run_concurrent=False)
    g(['glx-multi-context-ib-1'], run_concurrent=False)
    g(['glx-multi-context-single-window'], run_concurrent=False)
    g(['glx-multi-window-single-context'], run_concurrent=False)
    g(['glx-multithread'], run_concurrent=False)
    g(['glx-multithread-clearbuffer'], run_concurrent=False)
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

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_EXT_no_config_context'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-ext-no-config-context'], 'no fbconfig')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_profile'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-core-profile'], '3.2 core profile required')
    g(['glx-create-context-invalid-profile'], 'invalid profile')
    g(['glx-create-context-pre-GL32-profile'], 'pre-GL3.2 profile')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_robustness'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-invalid-reset-strategy'],
      'invalid reset notification strategy')
    g(['glx-create-context-require-robustness'], 'require GL_ARB_robustness')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_ARB_create_context_es2_profile'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-create-context-indirect-es2-profile'],
      'indirect rendering ES2 profile')
    g(['glx-create-context-invalid-es-version'], 'invalid OpenGL ES version')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('glx', 'GLX_MESA_query_renderer'),
        require_platforms=['glx', 'mixed_glx_egl']) as g:
    g(['glx-query-renderer-coverage'], 'coverage')

with profile.test_list.group_manager(
        PiglitGLTest, 'wgl',
        require_platforms=['wgl']) as g:
    g(['wgl-sanity'])
    g(['wgl-multi-context-single-window'])
    g(['wgl-multi-window-single-context'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.0')) as g:
    g(['gl-1.0-beginend-coverage'])
    g(['gl-1.0-dlist-beginend'])
    g(['gl-1.0-dlist-bitmap'])
    g(['gl-1.0-dlist-materials'])
    g(['gl-1.0-dlist-shademodel'])
    g(['gl-1.0-drawpixels-color-index'])
    g(['gl-1.0-drawpixels-depth-test'])
    g(['gl-1.0-drawpixels-stencil-test'])
    g(['gl-1.0-drawbuffer-modes'], run_concurrent=False)
    g(['gl-1.0-edgeflag'])
    g(['gl-1.0-edgeflag-const'])
    g(['gl-1.0-edgeflag-quads'])
    g(['gl-1.0-empty-begin-end-clause'])
    g(['gl-1.0-long-dlist'])
    g(['gl-1.0-long-line-loop'])
    g(['gl-1.0-readpixels-oob'])
    g(['gl-1.0-rendermode-feedback'])
    g(['gl-1.0-front-invalidate-back'], run_concurrent=False)
    g(['gl-1.0-swapbuffers-behavior'], run_concurrent=False)
    g(['gl-1.0-polygon-line-aa'])
    g(['gl-1.0-push-no-attribs'])
    g(['gl-1.0-blend-func'])
    g(['gl-1.0-fpexceptions'])
    g(['gl-1.0-ortho-pos'])
    g(['gl-1.0-rastercolor'])
    g(['gl-1.0-read-cache-stress-test'])
    g(['gl-1.0-readpixsanity'])
    g(['gl-1.0-logicop'])
    g(['gl-1.0-no-op-paths'])
    g(['gl-1.0-simple-readbuffer'])
    g(['gl-1.0-spot-light'])
    g(['gl-1.0-scissor-bitmap'])
    g(['gl-1.0-scissor-clear'])
    g(['gl-1.0-scissor-copypixels'])
    g(['gl-1.0-scissor-depth-clear'])
    g(['gl-1.0-scissor-depth-clear-negative-xy'])
    g(['gl-1.0-scissor-many'])
    g(['gl-1.0-scissor-offscreen'])
    g(['gl-1.0-scissor-polygon'])
    g(['gl-1.0-scissor-stencil-clear'])
    g(['gl-1.0-texgen'])
    g(['gl-1.0-textured-triangle'])
    g(['gl-1.0-user-clip-all-planes'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.1')) as g:
    # putting slower tests first
    g(['streaming-texture-leak'])
    g(['max-texture-size'])
    g(['max-texture-size-level'])
    g(['copyteximage', '1D'])
    g(['copyteximage', '2D'])
    g(['gl-1.1-color-material-array'])
    g(['gl-1.1-draw-arrays-start'])
    g(['gl-1.1-read-pixels-after-display-list'])
    g(['gl-1.1-set-vertex-color-after-draw'])
    g(['array-stride'])
    g(['clear-accum'])
    g(['clipflat'])
    g(['copypixels-draw-sync'])
    g(['copypixels-sync'])
    g(['degenerate-prims'])
    g(['depthfunc'])
    g(['depthrange-clear'])
    g(['dlist-clear'])
    g(['dlist-color-material'])
    g(['dlist-fdo3129-01'])
    g(['dlist-fdo3129-02'])
    g(['dlist-fdo31590'])
    g(['draw-arrays-colormaterial'])
    g(['draw-copypixels-sync'])
    g(['draw-pixel-with-texture'])
    g(['drawpix-z'])
    g(['draw-sync'])
    g(['fog-modes'])
    g(['fragment-center'])
    g(['geterror-invalid-enum'])
    g(['geterror-inside-begin'])
    g(['glinfo'])
    g(['gl-1.1-xor'])
    g(['gl-1.1-xor-copypixels'])
    g(['gl-1.2-texture-base-level'])
    g(['gl-1.3-alpha_to_coverage_nop'])
    g(['hiz'])
    g(['infinite-spot-light'])
    g(['line-aa-width'])
    g(['line-flat-clip-color'])
    g(['lineloop'])
    g(['lineloop', '-dlist'], 'lineloop-dlist')
    g(['linestipple'], run_concurrent=False)
    g(['longprim'])
    g(['masked-clear'])
    g(['point-line-no-cull'])
    g(['polygon-mode'])
    g(['polygon-mode-facing'])
    g(['polygon-mode-offset'])
    g(['polygon-offset'])
    g(['push-pop-texture-state'])
    g(['quad-invariance'])
    g(['readpix-z'])
    g(['roundmode-getintegerv'])
    g(['roundmode-pixelstore'])
    g(['select', 'gl11'], 'GL_SELECT - no test function')
    g(['select', 'depth'], 'GL_SELECT - depth-test enabled')
    g(['select', 'stencil'], 'GL_SELECT - stencil-test enabled')
    g(['select', 'alpha'], 'GL_SELECT - alpha-test enabled')
    g(['select', 'scissor'], 'GL_SELECT - scissor-test enabled')
    g(['stencil-drawpixels'])
    g(['texgen'])
    g(['two-sided-lighting'])
    g(['user-clip'])
    g(['varray-disabled'])
    g(['windowoverlap'])
    g(['copyteximage-border'])
    g(['copyteximage-clipping'])
    g(['copytexsubimage'])
    g(['getteximage-formats'])
    g(['getteximage-luminance'])
    g(['getteximage-simple'])
    g(['getteximage-depth'])
    g(['incomplete-texture', 'fixed'], 'incomplete-texture-fixed')
    g(['proxy-texture'])
    g(['sized-texture-format-channels'])
    g(['texredefine'])
    g(['texsubimage'])
    g(['texsubimage-unpack'])
    g(['texsubimage-depth-formats'])
    g(['texture-al'])
    g(['triangle-guardband-viewport'])
    g(['getteximage-targets', '1D'])
    g(['getteximage-targets', '2D'])
    g(['teximage-scale-bias'])
    add_msaa_visual_plain_tests(g, ['draw-pixels'])
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
        g(['teximage-colors', format])

    for num_samples in ['0'] + MSAA_SAMPLE_COUNTS:
        add_fbo_depthstencil_tests(g, 'default_fb', num_samples)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.2')) as g:
    g(['gl-1.2-texparameter-before-teximage'])
    g(['draw-elements-vs-inputs'])
    g(['two-sided-lighting-separate-specular'])
    g(['levelclamp'])
    g(['lodclamp'])
    g(['lodclamp-between'])
    g(['lodclamp-between-max'])
    g(['mipmap-setup'])
    g(['gl-1.2-rescale_normal'])
    g(['tex-skipped-unit'])
    g(['tex3d'])
    g(['tex3d-maxsize'], run_concurrent=False)
    g(['teximage-errors'])
    g(['texture-packed-formats'])
    g(['getteximage-targets', '3D'])
    add_msaa_visual_plain_tests(g, ['copyteximage', '3D'])
    add_texwrap_target_tests(g, '3D')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.3')) as g:
    g(['texunits'])
    g(['tex-border-1'])
    g(['gl-1.3-texture-env'])
    g(['tex3d-depth1'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.4')) as g:
    g(['gl-1.4-rgba-mipmap-texture-with-rgb-visual'])
    g(['blendminmax'])
    g(['blendsquare'])
    g(['gl-1.4-dlist-multidrawarrays'])
    g(['gl-1.4-multidrawarrays-errors'])
    g(['gl-1.4-polygon-offset'])
    g(['gl-1.4-tex1d-2dborder'])
    g(['draw-batch'])
    g(['stencil-wrap'])
    g(['triangle-rasterization'])
    g(['triangle-rasterization', '-use_fbo'], 'triangle-rasterization-fbo')
    g(['triangle-rasterization-overdraw'])
    g(['tex-miplevel-selection', '-nobias', '-nolod'],
      'tex-miplevel-selection')
    g(['tex-miplevel-selection', '-nobias'], 'tex-miplevel-selection-lod')
    g(['tex-miplevel-selection'], 'tex-miplevel-selection-lod-bias')
    add_msaa_visual_plain_tests(g, ['copy-pixels'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.5')) as g:
    g(['draw-elements'])
    g(['draw-elements', 'user'], 'draw-elements-user')
    g(['draw-vertices'])
    g(['draw-vertices', 'user'], 'draw-vertices-user')
    g(['isbufferobj'])
    g(['depth-tex-compare'])
    g(['gl-1.5-normal3b3s-invariance', 'GL_BYTE'],
      'normal3b3s-invariance-byte')
    g(['gl-1.5-normal3b3s-invariance', 'GL_SHORT'],
      'normal3b3s-invariance-short')
    g(['gl-1.5-vertex-buffer-offsets'], 'vertex-buffer-offsets')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 2.0')) as g:
    g(['attribs'])
    g(['gl-2.0-edgeflag'])
    g(['gl-2.0-edgeflag-immediate'])
    g(['gl-2.0-large-point-fs'])
    g(['gl-2.0-link-empty-prog'])
    g(['gl-2.0-two-sided-stencil'])
    g(['gl-2.0-vertexattribpointer'])
    g(['gl-2.0-vertexattribpointer-size-3'])
    g(['gl-2.0-vertex-attr-0'])
    g(['gl-2.0-vertex-const-attr'])
    g(['gl-2.0-reuse_fragment_shader'])
    g(['gl-2.0-shader-materials'])
    g(['attrib-assignments'])
    g(['getattriblocation-conventional'])
    g(['clip-flag-behavior'])
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
    g(['clear-varray-2.0'])
    g(['early-z'])
    g(['occlusion-query-discard'])
    g(['stencil-twoside'])
    g(['vs-point_size-zero'])
    g(['depth-tex-modes-glsl'])
    g(['fragment-and-vertex-texturing'])
    g(['incomplete-texture', 'glsl'], 'incomplete-texture-glsl')
    g(['tex3d-npot'])
    g(['max-samplers'])
    g(['max-samplers', 'border'])
    g(['gl-2.0-active-sampler-conflict'])
    g(['incomplete-cubemap', 'size'], 'incomplete-cubemap-size')
    g(['incomplete-cubemap', 'format'], 'incomplete-cubemap-format')
    g(['gl-2.0-texture-units'])
    g(['gl-2.0-uniform-neg-location'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 2.1')) as g:
    g(['gl-2.1-minmax'], 'minmax')
    g(['gl-2.1-pbo'], 'pbo')
    g(['gl-2.1-polygon-stipple-fs'], 'polygon-stipple-fs')
    g(['gl-2.1-fbo-mrt-alphatest-no-buffer-zero-write'], 'fbo-mrt-alphatest-no-buffer-zero-write')

with profile.test_list.group_manager(
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
    g(['clearbuffer-display-lists'])
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
    g(['gl-3.0-texparameteri'])
    g(['gl-3.0-texture-integer'])
    g(['gl-3.0-vertexattribipointer'])
    g(['gl30basic'], run_concurrent=False)
    g(['array-depth-roundtrip'])
    g(['depth-cube-map'])
    g(['sampler-cube-shadow'])
    g(['generatemipmap-base-change', 'size'])
    g(['generatemipmap-base-change', 'format'])
    g(['generatemipmap-cubemap'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.1')) as g:
    g(['gl-3.1-buffer-bindings'], 'buffer-bindings')
    g(['gl-3.1-default-vao'], 'default-vao')
    g(['gl-3.1-draw-buffers-errors'], 'draw-buffers-errors')
    g(['gl-3.1-enable-vertex-array'])
    g(['gl-3.1-genned-names'], 'genned-names')
    g(['gl-3.1-link-empty-prog-core'])
    g(['gl-3.1-minmax'], 'minmax')
    g(['gl-3-1-mixed-int-float-fbo'])
    g(['gl-3-1-mixed-int-float-fbo', 'int_second'])
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

with profile.test_list.group_manager(
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
    g(['gl-3.2-adj-prims', 'pv-first'])
    g(['gl-3.2-adj-prims', 'pv-last'])
    g(['gl-3.2-adj-prims', 'cull-front pv-first'])
    g(['gl-3.2-adj-prims', 'cull-front pv-last'])
    g(['gl-3.2-adj-prims', 'cull-back pv-first'])
    g(['gl-3.2-adj-prims', 'cull-back pv-last'])
    g(['gl-3.2-adj-prims', 'line cull-front pv-first'])
    g(['gl-3.2-adj-prims', 'line cull-front pv-last'])
    g(['gl-3.2-adj-prims', 'line cull-back pv-first'])
    g(['gl-3.2-adj-prims', 'line cull-back pv-last'])
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

with profile.test_list.group_manager(
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
    g(['gl-3.2-layered-rendering-gl-layer-render-clipped'], 'gl-layer-render-clipped')
    g(['gl-3.2-layered-rendering-gl-layer-render-storage'], 'gl-layer-render-storage')

    for texture_type in ['3d', '2d_array', '2d_multisample_array', '1d_array',
                         'cube_map', 'cube_map_array']:
        for test_type in ['single_level', 'mipmapped']:
            if texture_type == '2d_multisample_array' and \
                            test_type == 'mipmapped':
                continue
            g(['gl-3.2-layered-rendering-clear-color-all-types', texture_type,
               test_type],
              'clear-color-all-types {} {}'.format(texture_type, test_type))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 3.3')) as g:
    g(['gl-3.3-minmax'], 'minmax')
    g(['gl-3.0-required-renderbuffer-attachment-formats', '33'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-sized-texture-formats', '33'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-texture-attachment-formats', '33'],
      'required-texture-attachment-formats')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.2')) as g:
    g(['gl-3.0-required-renderbuffer-attachment-formats', '42'],
      'required-renderbuffer-attachment-formats')
    g(['gl-3.0-required-sized-texture-formats', '42'],
      'required-sized-texture-formats')
    g(['gl-3.0-required-texture-attachment-formats', '42'],
      'required-texture-attachment-formats')
    g(['gl-4.4-max_vertex_attrib_stride'], 'gl-max-vertex-attrib-stride')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.5')) as g:
    g(['gl-4.5-named-framebuffer-draw-buffers-errors'], 'named-framebuffer-draw-buffers-errors')
    g(['gl-4.5-named-framebuffer-read-buffer-errors'], 'named-framebuffer-read-buffer-errors')
    g(['gl-4.5-compare-framebuffer-parameter-with-get'], 'compare-framebuffer-parameter-with-get')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.3')) as g:
    g(['get_glsl_versions'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 4.4')) as g:
    g(['tex-errors'])

# Group spec/glsl-es-1.00
with profile.test_list.group_manager(
        BuiltInConstantsTest,
        grouptools.join('spec', 'glsl-es-1.00')) as g:
    g(['built-in-constants_gles2',
       os.path.join('spec', 'glsl-es-1.00', 'minimum-maximums.txt')],
      'built-in constants')

# Group spec/glsl-1.10
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution')) as g:
    g(['glsl-render-after-bad-attach'])
    g(['glsl-1.10-built-in-matrix-state'])
    g(['glsl-1.10-built-in-uniform-state'])
    g(['glsl-1.10-fragdepth'])
    g(['glsl-1.10-linear-fog'])
    g(['glsl-1.10-uniform-matrix-transposed'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution', 'clipping')) as g:
    for mode in ['fixed', 'pos_clipvert', 'clipvert_pos']:
        g(['clip-plane-transformation', mode],
          'clip-plane-transformation {}'.format(mode))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'execution',
                        'varying-packing')) as g:
    for type_ in ['int', 'uint', 'float', 'vec2', 'vec3', 'vec4', 'ivec2',
                  'ivec3', 'ivec4', 'uvec2', 'uvec3', 'uvec4', 'mat2', 'mat3',
                  'mat4', 'mat2x3', 'mat2x4', 'mat3x2', 'mat3x4', 'mat4x2',
                  'mat4x3']:
        for arrayspec in ['array', 'separate', 'arrays_of_arrays']:
            g(['varying-packing-simple', type_, arrayspec],
              'simple {} {}'.format(type_, arrayspec))

with profile.test_list.group_manager(
        BuiltInConstantsTest,
        grouptools.join('spec', 'glsl-1.10')) as g:
    g(['built-in-constants',
       os.path.join('spec', 'glsl-1.10', 'minimum-maximums.txt')],
      'built-in constants')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.10', 'api')) as g:
    g(['getactiveattrib', '110'], 'getactiveattrib 110')

# Group spec/glsl-1.20
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20')) as g:
    g(['glsl-1.20-getactiveuniform-constant'])
    g(['built-in-constants',
       os.path.join('spec', 'glsl-1.20', 'minimum-maximums.txt')],
      'built-in constants',
      override_class=BuiltInConstantsTest)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.20', 'api')) as g:
    g(['getactiveattrib', '120'])

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
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

    for type in ('i', 'u', ''):
        for sampler in ('sampler2DMS', 'sampler2DMSArray'):
            for sample_count in MSAA_SAMPLE_COUNTS:
                stype = '{}{}'.format(type, sampler)
                profile.test_list[grouptools.join(
                    'spec', 'arb_shader_texture_image_samples',
                    'textureSamples', '{}-{}-{}'.format(stage, stype, sample_count))
                ] = PiglitGLTest([
                    'textureSamples', stage, stype, sample_count])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30')) as g:
    g(['glsl-1.30-texel-offset-limits'], 'texel-offset-limits')
    g(['built-in-constants',
       os.path.join('spec', 'glsl-1.30', 'minimum-maximums.txt')],
      'built-in constants',
       override_class=BuiltInConstantsTest)

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'linker', 'clipping')) as g:
    g(['mixing-clip-distance-and-clip-vertex-disallowed'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'execution')) as g:
    for arg in ['vs_basic', 'vs_xfb', 'vs_fbo', 'fs_basic', 'fs_fbo']:
        g(['isinf-and-isnan', arg])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'execution', 'clipping')) as g:
    g(['max-clip-distances'])
    g(['clip-plane-transformation', 'pos'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.30', 'api')) as g:
    g(['getactiveattrib', '130'], 'getactiveattrib 130')

# Group spec/glsl-1.40
with profile.test_list.group_manager(
        BuiltInConstantsTest,
        grouptools.join('spec', 'glsl-1.40')) as g:
    g(['built-in-constants',
       os.path.join('spec', 'glsl-1.40', 'minimum-maximums.txt')],
      'built-in constants')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.40', 'execution')) as g:
    g(['glsl-1.40-tf-no-position'], 'tf-no-position')

textureSize_samplers_140 = textureSize_samplers_130 + [
    'sampler2DRect', 'isampler2DRect', 'sampler2DRectShadow', 'samplerBuffer',
    'isamplerBuffer', 'usamplerBuffer']
for stage in ['vs', 'gs', 'fs', 'tes']:
    if stage == 'gs' or stage == 'tes':
        version = '1.50'
    else:
        version = '1.40'
    # textureSize():
    for sampler in textureSize_samplers_140:
        profile.test_list[grouptools.join(
            'spec', 'glsl-{}'.format(version), 'execution', 'textureSize',
            '{}-textureSize-{}'.format(stage, sampler))] = PiglitGLTest(
                ['textureSize', '140', stage, sampler])
for stage in ['vs', 'gs', 'fs']:
    if stage == 'gs':
        version = '1.50'
    else:
        version = '1.40'
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.50')) as g:
    g(['built-in-constants',
       os.path.join('spec', 'glsl-1.50', 'minimum-maximums.txt')],
      'built-in constants',
      override_class=BuiltInConstantsTest)
    g(['glsl-1.50-gs-emits-too-few-verts'], 'gs-emits-too-few-verts')
    g(['glsl-1.50-geometry-end-primitive-optional-with-points-out'],
      'gs-end-primitive-optional-with-points-out')
    g(['glsl-1.50-gs-max-output-components'], 'gs-max-output-components')
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

with profile.test_list.group_manager(
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
with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        BuiltInConstantsTest, grouptools.join('spec', 'glsl-3.30')) as g:
    g(['built-in-constants',
       os.path.join('spec', 'glsl-3.30', 'minimum-maximums.txt')],
      'built-in constants')

with profile.test_list.group_manager(
        BuiltInConstantsTest, grouptools.join('spec', 'glsl-es-3.00')) as g:
    g(['built-in-constants_gles3',
       os.path.join('spec', 'glsl-es-3.00', 'minimum-maximums.txt')],
      'built-in constants')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-es-3.00', 'execution')) as g:
    g(['varying-struct-centroid_gles3'])

with profile.test_list.group_manager(
        BuiltInConstantsTest, grouptools.join('spec', 'glsl-es-3.10')) as g:
    g(['built-in-constants_gles3',
       os.path.join('spec', 'glsl-es-3.10', 'minimum-maximums.txt')],
      'built-in constants')

# AMD_performance_monitor
with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'AMD_performance_monitor')) as g:
    g(['amd_performance_monitor_api'], 'api', run_concurrent=False)
    g(['amd_performance_monitor_api'], 'vc4')
    g(['amd_performance_monitor_measure'], 'measure', run_concurrent=False)

# Group ARB_arrays_of_arrays
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_arrays_of_arrays')) as g:
    g(['arb_arrays_of_arrays-max-binding'], run_concurrent=False)

# Group ARB_point_sprite
with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_point_sprite')) as g:
    g(['arb_point_sprite-checkerboard'], run_concurrent=False)
    g(['arb_point_sprite-mipmap'])

# Group ARB_tessellation_shader
with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_tessellation_shader')) as g:
    g(['arb_tessellation_shader-get-tcs-params'])
    g(['arb_tessellation_shader-get-tes-params'])
    g(['arb_tessellation_shader-minmax'])
    g(['arb_tessellation_shader-invalid-get-program-params'])
    g(['arb_tessellation_shader-invalid-patch-vertices-range'])
    g(['arb_tessellation_shader-invalid-primitive'])
    g(['built-in-constants',
       os.path.join('spec', 'arb_tessellation_shader', 'minimum-maximums.txt')],
      'built-in-constants',
      override_class=BuiltInConstantsTest)
    g(['arb_tessellation_shader-large-uniforms'])
    g(['arb_tessellation_shader-layout-mismatch'])

# Group ARB_texture_multisample
with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_texture_multisample')) as g:
    g(['arb_texture_multisample-large-float-texture'], 'large-float-texture',
      run_concurrent=False)
    g(['arb_texture_multisample-large-float-texture', '--array'],
      'large-float-texture-array', run_concurrent=False)
    g(['arb_texture_multisample-large-float-texture', '--fp16'],
      'large-float-texture-fp16', run_concurrent=False)
    g(['arb_texture_multisample-large-float-texture', '--array', '--fp16'],
      'large-float-texture-array-fp16', run_concurrent=False)
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
    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['arb_texture_multisample-texelfetch', sample_count])
    g(['arb_texture_multisample-sample-mask'])
    g(['arb_texture_multisample-sample-mask-value'])
    g(['arb_texture_multisample-sample-mask-execution'])
    g(['arb_texture_multisample-sample-mask-execution', '-tex'])
    g(['arb_texture_multisample-negative-max-samples'])
    g(['arb_texture_multisample-teximage-3d-multisample'])
    g(['arb_texture_multisample-teximage-2d-multisample'])
    g(['arb_texture_multisample-sample-depth']),
    g(['arb_texture_multisample-stencil-clear']),
    g(['arb_texture_multisample-clear'])

samplers_atm = ['sampler2DMS', 'isampler2DMS', 'usampler2DMS',
                'sampler2DMSArray', 'isampler2DMSArray', 'usampler2DMSArray']
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'fb-completeness')) as g:

    for sample_count in MSAA_SAMPLE_COUNTS:
        # fb-completeness
        g(['arb_texture_multisample-fb-completeness', sample_count],
          sample_count)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample', 'texelFetch')) as g:

    stages = ['vs', 'gs', 'fs']
    for sampler, stage, sample_count in itertools.product(
            samplers_atm, stages, MSAA_SAMPLE_COUNTS):
        g(['texelFetch', stage, sampler, sample_count],
          '{}-{}-{}'.format(sample_count, stage, sampler))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'sample-position')) as g:
    # sample positions
    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['arb_texture_multisample-sample-position', sample_count],
          sample_count)


with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample',
                        'textureSize')) as g:

    stages = ['vs', 'gs', 'fs', 'tes']
    for stage, sampler in itertools.product(stages, samplers_atm):
        g(['textureSize', stage, sampler],
          '{}-textureSize-{}'.format(stage, sampler))

# Group ARB_texture_gather
with profile.test_list.group_manager(
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


with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_stencil_texturing')) as g:
    g(['arb_stencil_texturing-draw'], 'draw')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_stencil_texturing', 'glBlitFramebuffer corrupts state')) as g:
    for t in ['1D', '2D', 'CUBE_MAP', '1D_ARRAY', '2D_ARRAY', 'CUBE_MAP_ARRAY', '2D_MULTISAMPLE', '2D_MULTISAMPLE_ARRAY', 'RECTANGLE']:
        target = 'GL_TEXTURE_' + t
        g(['arb_stencil_texturing-blit_corrupts_state', target], target)

# Group ARB_sync
with profile.test_list.group_manager(
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
with profile.test_list.group_manager(
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
with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_get_program_binary')) as g:
    g(['arb_get_program_binary-api-errors'],
      'misc. API error checks')
    g(['arb_get_program_binary-overrun', 'program'],
      'NUM_PROGRAM_BINARY_FORMATS over-run check')
    g(['arb_get_program_binary-retrievable_hint'],
      'PROGRAM_BINARY_RETRIEVABLE_HINT')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'EXT_depth_bounds_test')) as g:
    g(['depth_bounds'])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_depth_clamp')) as g:
    g(['depth_clamp'])
    g(['depth-clamp-range'])
    g(['depth-clamp-status'])

# Group ARB_draw_elements_base_vertex
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_draw_elements_base_vertex')) as g:
    g(['arb_draw_elements_base_vertex-dlist'], 'dlist')
    g(['arb_draw_elements_base_vertex-drawelements'])
    g(['arb_draw_elements_base_vertex-drawelements', 'user_varrays'],
      'arb_draw_elements_base_vertex-drawelements-user_varrays')
    g(['arb_draw_elements_base_vertex-negative-index'])
    g(['arb_draw_elements_base_vertex-bounds'])
    g(['arb_draw_elements_base_vertex-negative-index', 'user_varrays'],
      'arb_draw_elements_base_vertex-negative-index-user_varrays')
    g(['arb_draw_elements_base_vertex-drawelements-instanced'])
    g(['arb_draw_elements_base_vertex-drawrangeelements'])
    g(['arb_draw_elements_base_vertex-multidrawelements'])

# Group ARB_draw_instanced
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_draw_instanced')) as g:
    g(['arb_draw_instanced-dlist'], 'dlist')
    g(['arb_draw_instanced-elements'], 'elements')
    g(['arb_draw_instanced-negative-arrays-first-negative'],
      'negative-arrays-first-negative')
    g(['arb_draw_instanced-negative-elements-type'],
      'negative-elements-type')
    g(['arb_draw_instanced-drawarrays'])

# Group ARB_draw_indirect
with profile.test_list.group_manager(
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
    g(['arb_draw_indirect-draw-arrays-shared-binding'])
    g(['arb_draw_indirect-transform-feedback'])
    g(['arb_draw_indirect-vertexid'],
      'gl_VertexID used with glDrawArraysIndirect')
    g(['arb_draw_indirect-vertexid', 'elements'],
      'gl_VertexID used with glDrawElementsIndirect')

# Group ARB_fragment_program
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_fragment_program')) as g:
    g(['arb_fragment_program-minmax'], 'minmax')
    g(['fp-abs-01'])
    g(['fp-fog'])
    g(['fp-formats'])
    g(['fp-fragment-position'])
    g(['fp-incomplete-tex'])
    g(['fp-indirections'])
    g(['fp-indirections2'])
    g(['fp-kil'])
    g(['fp-lit-mask'])
    g(['fp-lit-src-equals-dst'])
    g(['fp-long-alu'])
    g(['fp-set-01'])
    g(['trinity-fp1'])
    g(['arb_fragment_program-sparse-samplers'], 'sparse-samplers')
    g(['incomplete-texture', 'arb_fp'], 'incomplete-texture-arb_fp')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'NV_fragment_program_option')) as g:
    g(['fp-abs-02'])
    g(['fp-condition_codes-01'])
    g(['fp-rfl'])
    g(['fp-set-02'])
    g(['fp-unpack-01'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ATI_fragment_shader')) as g:
    g(['ati_fragment_shader-api-alphafirst'])
    g(['ati_fragment_shader-api-gen'])
    g(['ati_fragment_shader-error01-genzero'])
    g(['ati_fragment_shader-error02-inside'])
    g(['ati_fragment_shader-error03-outside'])
    g(['ati_fragment_shader-error04-endshader'])
    g(['ati_fragment_shader-error05-passes'])
    g(['ati_fragment_shader-error06-regswizzle'])
    g(['ati_fragment_shader-error07-instcount'])
    g(['ati_fragment_shader-error08-secondary'])
    g(['ati_fragment_shader-error09-allconst'])
    g(['ati_fragment_shader-error10-dotx'])
    g(['ati_fragment_shader-error11-invaliddst'])
    g(['ati_fragment_shader-error12-invalidsrc'])
    g(['ati_fragment_shader-error13-invalidarg'])
    g(['ati_fragment_shader-error14-invalidmod'])
    g(['ati_fragment_shader-render-constants'])
    g(['ati_fragment_shader-render-default'])
    g(['ati_fragment_shader-render-fog'])
    g(['ati_fragment_shader-render-notexture'])
    g(['ati_fragment_shader-render-precedence'])
    g(['ati_fragment_shader-render-sources'])
    g(['ati_fragment_shader-render-textargets'])

# Group ARB_framebuffer_object
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_object')) as g:
    g(['same-attachment-glFramebufferTexture2D-GL_DEPTH_STENCIL_ATTACHMENT'])
    g(['same-attachment-glFramebufferRenderbuffer-GL_DEPTH_STENCIL_ATTACHMENT'])
    g(['arb_framebuffer_object-get-attachment-parameter-default-framebuffer'], run_concurrent=False)
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
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH_COMPONENT16'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH_COMPONENT24'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH_COMPONENT32'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH_COMPONENT32F'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_STENCIL_INDEX1'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_STENCIL_INDEX4'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_STENCIL_INDEX8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_STENCIL_INDEX16'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH24_STENCIL8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_DEPTH24_STENCIL8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth_stencil', 'GL_DEPTH24_STENCIL8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth', 'GL_DEPTH32F_STENCIL8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'stencil', 'GL_DEPTH32F_STENCIL8'])
    g(['arb_framebuffer_object-depth-stencil-blit', 'depth_stencil', 'GL_DEPTH32F_STENCIL8'])
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
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_sRGB')) as g:
    for backing_type in ('texture', 'renderbuffer'):
        for srgb_types in ('linear', 'srgb', 'linear_to_srgb',
                           'srgb_to_linear'):
            for blit_type in ('single_sampled', 'upsample', 'downsample',
                              'msaa', 'scaled'):
                for framebuffer_srgb_setting in ('enabled',
                                                 'disabled'):
                    for src_fill_mode in ('clear', 'render'):
                        g(['arb_framebuffer_srgb-blit', backing_type,
                            srgb_types, blit_type, framebuffer_srgb_setting,
                            src_fill_mode],
                          'blit {} {} {} {} {}'.format(
                              backing_type, srgb_types, blit_type,
                              framebuffer_srgb_setting, src_fill_mode))
    g(['framebuffer-srgb'], run_concurrent=False)
    g(['arb_framebuffer_srgb-clear'])
    g(['arb_framebuffer_srgb-pushpop'])
    g(['ext_framebuffer_multisample-fast-clear',
       'GL_EXT_texture_sRGB',
       'enable-fb-srgb'],
      'msaa-fast-clear')
    g(['ext_framebuffer_multisample-fast-clear',
       'GL_EXT_texture_sRGB',
       'enable-fb-srgb',
       'single-sample'],
      'fbo-fast-clear')
    g(['arb_framebuffer_srgb-fast-clear-blend'])
    g(['arb_framebuffer_srgb-srgb_conformance'])
    g(['arb_framebuffer_srgb-srgb_pbo'])


with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader5')) as g:
    stages = ['vs', 'fs']
    types = ['unorm', 'float', 'int', 'uint']
    comps = ['r', 'rg', 'rgb', 'rgba']
    samplers = ['2D', '2DArray', 'Cube', 'CubeArray', '2DRect']
    for stage, type_, comp, sampler in itertools.product(
            stages, types, comps, samplers):
        for func in ['textureGather'] if 'Cube' in sampler else ['textureGather', 'textureGatherOffset', 'textureGatherOffsets']:
            for cs in range(len(comp)):
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
    g(['arb_gpu_shader5-interpolateAtSample-different'])
    g(['arb_gpu_shader5-interpolateAtSample-different', 'uniform'])
    g(['arb_gpu_shader5-interpolateAtSample-dynamically-nonuniform'])
    g(['arb_gpu_shader5-interpolateAtOffset'])
    g(['arb_gpu_shader5-interpolateAtOffset-nonconst'])


with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader_fp64',
                        'varying-packing')) as g:
    for type in ['double', 'dvec2', 'dvec3', 'dvec4', 'dmat2', 'dmat3',
                 'dmat4', 'dmat2x3', 'dmat2x4', 'dmat3x2', 'dmat3x4',
                 'dmat4x2', 'dmat4x3']:
        for arrayspec in ['array', 'separate', 'arrays_of_arrays']:
            g(['varying-packing-simple', type, arrayspec],
              'simple {0} {1}'.format(type, arrayspec))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader_fp64', 'execution')) as g:
    g(['arb_gpu_shader_fp64-tf-separate'])
    g(['arb_gpu_shader_fp64-double-gettransformfeedbackvarying'])
    g(['arb_gpu_shader_fp64-tf-interleaved'])
    g(['arb_gpu_shader_fp64-tf-interleaved-aligned'])
    g(['arb_gpu_shader_fp64-vs-getuniformdv'])
    g(['arb_gpu_shader_fp64-fs-getuniformdv'])
    g(['arb_gpu_shader_fp64-gs-getuniformdv'])
    g(['arb_gpu_shader_fp64-wrong-type-setter'])
    g(['arb_gpu_shader_fp64-double_in_bool_uniform'])
    g(['arb_gpu_shader_fp64-uniform-invalid-operation'])
    g(['arb_gpu_shader_fp64-vs-non-uniform-control-flow-const'])
    g(['arb_gpu_shader_fp64-fs-non-uniform-control-flow-const'])
    g(['arb_gpu_shader_fp64-vs-non-uniform-control-flow-ubo'])
    g(['arb_gpu_shader_fp64-fs-non-uniform-control-flow-ubo'])
    g(['arb_gpu_shader_fp64-vs-non-uniform-control-flow-ssbo'])
    g(['arb_gpu_shader_fp64-fs-non-uniform-control-flow-ssbo'])
    g(['arb_gpu_shader_fp64-vs-non-uniform-control-flow-alu'])
    g(['arb_gpu_shader_fp64-fs-non-uniform-control-flow-alu'])
    g(['arb_gpu_shader_fp64-vs-non-uniform-control-flow-packing'])
    g(['arb_gpu_shader_fp64-fs-non-uniform-control-flow-packing'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_gpu_shader_fp64', 'shader_storage')) as g:
    g(['arb_gpu_shader_fp64-layout-std140-fp64-shader'], 'layout-std140-fp64-shader')
    g(['arb_gpu_shader_fp64-layout-std140-fp64-mixed-shader'], 'layout-std140-fp64-mixed-shader')
    g(['arb_gpu_shader_fp64-layout-std430-fp64-shader'], 'layout-std430-fp64-shader')
    g(['arb_gpu_shader_fp64-layout-std430-fp64-mixed-shader'], 'layout-std430-fp64-mixed-shader')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shader_subroutine')) as g:
    g(['arb_shader_subroutine-minmax'])
    g(['arb_shader_subroutine-uniformsubroutinesuiv'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_occlusion_query')) as g:
    g(['occlusion_query'])
    g(['occlusion_query_conform'])
    g(['occlusion_query_lifetime'])
    g(['occlusion_query_meta_fragments'])
    g(['occlusion_query_meta_no_fragments'])
    g(['occlusion_query_meta_save'])
    g(['occlusion_query_order'])
    g(['gen_delete_while_active'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_point_parameters')) as g:
    g(['arb_point_parameters-point-attenuation'])

# Group ARB_separate_shader_objects
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_separate_shader_objects')) as g:
    g(['arb_separate_shader_object-ActiveShaderProgram-invalid-program'],
      'ActiveShaderProgram with invalid program')
    g(['arb_separate_shader_object-GetProgramPipelineiv'],
      'GetProgramPipelineiv')
    g(['arb_separate_shader_object-dlist'], 'Display lists (Compat)')
    g(['arb_separate_shader_object-IsProgramPipeline'],
      'IsProgramPipeline')
    g(['arb_separate_shader_object-UseProgramStages-non-separable'],
      'UseProgramStages - non-separable program')
    g(['arb_separate_shader_object-ProgramUniform-coverage'],
      'ProgramUniform coverage')
    g(['arb_separate_shader_object-rendezvous_by_name'],
      'Rendezvous by name')
    g(['arb_separate_shader_object-rendezvous_by_name_interpolation'],
      'Rendezvous by name with multiple interpolation qualifier')
    g(['arb_separate_shader_object-mix-and-match-tcs-tes'],
      'mix-and-match-tcs-tes'),
    g(['arb_separate_shader_object-mixed_explicit_and_non_explicit_locations'],
      'Mixed explicit and non-explicit locations')
    g(['arb_separate_shader_object-api-errors'], 'misc. API error checks')
    g(['arb_separate_shader_object-rendezvous_by_location', '-fbo'],
      'Rendezvous by location', run_concurrent=False)
    g(['arb_separate_shader_object-rendezvous_by_location-5-stages'],
      'Rendezvous by location (5 stages)')
    g(['arb_separate_shader_object-ValidateProgramPipeline'],
      'ValidateProgramPipeline')
    g(['arb_separate_shader_object-400-combinations', '-fbo', '--by-location'],
      '400 combinations by location', run_concurrent=False)
    g(['arb_separate_shader_object-400-combinations', '-fbo'],
      '400 combinations by name', run_concurrent=False)
    g(['arb_separate_shader_object-active-sampler-conflict'],
      'active sampler conflict')
    g(['arb_separate_shader_object-atomic-counter'],
      'atomic counter')
    g(['arb_separate_shader_object-compat-builtins'], 'compat-builtins')
    g(['arb_separate_shader_object-rendezvous_by_location-3-stages'],
       'rendezvous_by_location-3-stages')
    g(['arb_separate_shader_object-uniform-namespace'],
       'uniform namespace is per-program')
    g(['arb_separate_shader_object-xfb-rendezvous_by_location'],
       'Transform feedback with rendezvous by location')

# Group ARB_sampler_objects
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_sampler_objects')) as g:
    g(['arb_sampler_objects-sampler-objects'], 'sampler-objects',)
    g(['arb_sampler_objects-sampler-incomplete'], 'sampler-incomplete',)
    g(['arb_sampler_objects-srgb-decode'], 'GL_EXT_texture_sRGB_decode',)
    g(['arb_sampler_objects-framebufferblit'], 'framebufferblit',
      run_concurrent=False)

# Group ARB_sample_shading
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_sample_shading')) as g:
    g(['arb_sample_shading-api'], run_concurrent=False)

    for num_samples in ['0'] + MSAA_SAMPLE_COUNTS:
        g(['arb_sample_shading-builtin-gl-num-samples', num_samples],
          'builtin-gl-num-samples {0}'.format(num_samples),
          run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-id', num_samples],
          'builtin-gl-sample-id {}'.format(num_samples), run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-mask', num_samples],
          'builtin-gl-sample-mask {}'.format(num_samples),
          run_concurrent=False)
        g(['arb_sample_shading-builtin-gl-sample-position', num_samples],
          'builtin-gl-sample-position {}'.format(num_samples),
          run_concurrent=False)

    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['arb_sample_shading-interpolate-at-sample-position', sample_count],
          'interpolate-at-sample-position {}'.format(sample_count))
        g(['arb_sample_shading-ignore-centroid-qualifier', sample_count],
          'ignore-centroid-qualifier {}'.format(sample_count))
        g(['arb_sample_shading-samplemask', sample_count, 'all', 'all'],
          'samplemask {}'.format(sample_count))

    for num_samples in ['0'] + MSAA_SAMPLE_COUNTS:
        g(['arb_sample_shading-builtin-gl-sample-mask-simple',
           num_samples],
          'builtin-gl-sample-mask-simple {}'.format(num_samples))
        g(['arb_sample_shading-samplemask', num_samples, 'all', 'all'],
          'samplemask {} all'.format(num_samples))

    g(['arb_sample_shading-builtin-gl-sample-mask-mrt-alpha'])

# Group ARB_debug_output
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_debug_output')) as g:
    g(['arb_debug_output-api_error'], run_concurrent=False)

# Group KHR_debug
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'KHR_debug')) as g:
    g(['khr_debug-object-label_gl'], 'object-label_gl')
    g(['khr_debug-object-label_gles2'], 'object-label_gles2')
    g(['khr_debug-object-label_gles3'], 'object-label_gles3')
    g(['khr_debug-push-pop-group_gl'], 'push-pop-group_gl')
    g(['khr_debug-push-pop-group_gles2'], 'push-pop-group_gles2')
    g(['khr_debug-push-pop-group_gles3'], 'push-pop-group_gles3')

# Group ARB_occlusion_query2
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_occlusion_query2')) as g:
    g(['arb_occlusion_query2-api'], 'api')
    g(['arb_occlusion_query2-render'], 'render')

# Group EXT_memory_object tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_memory_object')) as g:
    g(['ext_memory_object-api-errors'], 'api-errors')

# Group EXT_memory_object_fd tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_memory_object_fd')) as g:
    g(['ext_memory_object_fd-api-errors'], 'api-errors')

# Group EXT_semaphore tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_semaphore')) as g:
    g(['ext_semaphore-api-errors'], 'api-errors')

# Group EXT_semaphore_fd tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_semaphore_fd')) as g:
    g(['ext_semaphore_fd-api-errors'], 'api-errors')

# Group EXT_texture_format_BGRA8888 tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_texture_format_BGRA8888')) as g:
    g(['ext_texture_format_bgra8888-api-errors'], 'api-errors')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_pixel_buffer_object')) as g:
    g(['cubemap', 'pbo'])
    g(['cubemap', 'npot', 'pbo'])
    g(['fbo-pbo-readpixels-small'], run_concurrent=False)
    g(['pbo-drawpixels'], run_concurrent=False)
    g(['pbo-read-argb8888'], run_concurrent=False)
    g(['pbo-readpixels-small'], run_concurrent=False)
    g(['pbo-teximage'], run_concurrent=False)
    g(['pbo-teximage-tiling'], run_concurrent=False)
    g(['pbo-teximage-tiling-2'], run_concurrent=False)
    g(['texsubimage', 'pbo'])
    g(['texsubimage', 'pbo', 'manual', 'GL_TEXTURE_2D', 'GL_RGB8', '6',
        '10', '0', '94', '53', '0'])
    g(['texsubimage', 'array', 'pbo'])
    g(['texsubimage', 'cube_map_array', 'pbo'])
    g(['texsubimage-depth-formats', 'pbo'])
    g(['texsubimage-unpack', 'pbo'])

# Group ARB_provoking_vertex
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_provoking_vertex')) as g:
    g(['arb-provoking-vertex-control'], run_concurrent=False)
    g(['arb-provoking-vertex-initial'], run_concurrent=False)
    g(['arb-provoking-vertex-render'], run_concurrent=False)
    g(['arb-quads-follow-provoking-vertex'], run_concurrent=False)
    g(['arb-xfb-before-flatshading'], run_concurrent=False)

# Group ARB_robustness
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_robustness')) as g:
    g(['arb_robustness_client-mem-bounds'], run_concurrent=False)

# Group ARB_shader_texture_lod
with profile.test_list.group_manager(
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
with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shading_language_420pack')) as g:
    g(['built-in-constants',
       os.path.join('spec', 'arb_shading_language_420pack', 'minimum-maximums.txt')],
      'built-in constants',
      override_class=BuiltInConstantsTest)
    g(['arb_shading_language_420pack-multiple-layout-qualifiers'],
      'multiple layout qualifiers')
    g(['arb_shading_language_420pack-active-sampler-conflict'], 'active sampler conflict')
    g(['arb_shading_language_420pack-binding-layout'], 'binding layout')

# Group ARB_enhanced_layouts
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_enhanced_layouts')) as g:
    g(['arb_enhanced_layouts-explicit-offset-bufferstorage'],
       'explicit-offset-bufferstorage')
    g(['arb_enhanced_layouts-gs-stream-location-aliasing'],
       'gs-stream-location-aliasing')
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'vs'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_vs',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'vs_ifc'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_vs_interface',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'vs_named_ifc'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_vs_named_interface',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'vs_struct'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_vs_struct',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'gs'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_gs',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-qualifiers', 'gs_max'],
      'arb_enhanced_layouts-transform-feedback-layout-qualifiers_gs_max',
      run_concurrent=False)
    g(['arb_enhanced_layouts-transform-feedback-layout-query-api'],
       'transform-feedback-layout-query-api')

# Group ARB_explicit_attrib_location
with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_program_interface_query')) as g:
    g(['arb_program_interface_query-resource-location'], run_concurrent=False)
    g(['arb_program_interface_query-resource-query'], run_concurrent=False)
    g(['arb_program_interface_query-getprograminterfaceiv'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourceindex'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourcename'], run_concurrent=False)
    g(['arb_program_interface_query-getprogramresourceiv'], run_concurrent=False)
    g(['arb_program_interface_query-compare-with-shader-subroutine'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_framebuffer_no_attachments')) as g:
    g(['arb_framebuffer_no_attachments-minmax'])
    g(['arb_framebuffer_no_attachments-params'])
    g(['arb_framebuffer_no_attachments-atomic'])
    g(['arb_framebuffer_no_attachments-query'])
    g(['arb_framebuffer_no_attachments-roundup-samples'])

# Group ARB_explicit_uniform_location
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_explicit_uniform_location')) as g:
    g(['arb_explicit_uniform_location-minmax'], run_concurrent=False)
    g(['arb_explicit_uniform_location-boundaries'], run_concurrent=False)
    g(['arb_explicit_uniform_location-array-elements'], run_concurrent=False)
    g(['arb_explicit_uniform_location-inactive-uniform'], run_concurrent=False)
    g(['arb_explicit_uniform_location-use-of-unused-loc'],
      run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_buffer_object')) as g:
    g(['arb_texture_buffer_object-bufferstorage'], 'bufferstorage')
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
    g(['arb_texture_buffer_object-max-size'], 'max-size')
    g(['arb_texture_buffer_object-minmax'], 'minmax')
    g(['arb_texture_buffer_object-negative-bad-bo'], 'negative-bad-bo')
    g(['arb_texture_buffer_object-negative-bad-format'], 'negative-bad-format')
    g(['arb_texture_buffer_object-negative-bad-target'], 'negative-bad-target')
    g(['arb_texture_buffer_object-negative-unsupported'],
      'negative-unsupported')
    g(['arb_texture_buffer_object-subdata-sync'], 'subdata-sync')
    g(['arb_texture_buffer_object-unused-name'], 'unused-name')
    g(['arb_texture_buffer_object-render-no-bo'], 'render-no-bo')
    g(['arb_texture_buffer_object-indexed'], 'indexed')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_buffer_range')) as g:
    g(['arb_texture_buffer_range-dlist'], 'dlist')
    g(['arb_texture_buffer_range-errors'], 'errors')
    g(['arb_texture_buffer_range-ranges'], 'ranges')
    g(['arb_texture_buffer_range-ranges-2'], 'ranges-2')
    g(['arb_texture_buffer_range-ranges-2', '-compat'], 'ranges-2 compat')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ARB_texture_storage')) as g:
    g(['arb_texture_storage-texture-storage'], 'texture-storage',
      run_concurrent=False)
    g(['arb_texture_storage-texture-storage-attach-before'], 'attach-before',
      run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_storage_multisample')) as g:
    g(['arb_texture_storage_multisample-tex-storage'], 'tex-storage')
    g(['arb_texture_storage_multisample-tex-param'], 'tex-param')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_view')) as g:
    g(['arb_texture_view-cubemap-view'], 'cubemap-view')
    g(['arb_texture_view-texture-immutable-levels'], 'immutable_levels')
    g(['arb_texture_view-max-level'], 'max-level')
    g(['arb_texture_view-mipgen'], 'mipgen')
    g(['arb_texture_view-params'], 'params')
    g(['arb_texture_view-formats'], 'formats')
    g(['arb_texture_view-targets'], 'targets')
    g(['arb_texture_view-queries'], 'queries')
    g(['arb_texture_view-rendering-target'], 'rendering-target')
    g(['arb_texture_view-rendering-levels'], 'rendering-levels')
    g(['arb_texture_view-rendering-layers'], 'rendering-layers')
    g(['arb_texture_view-rendering-formats'], 'rendering-formats')
    g(['arb_texture_view-rendering-r32ui'], 'rendering-r32ui')
    g(['arb_texture_view-lifetime-format'], 'lifetime-format')
    g(['arb_texture_view-getteximage-srgb'], 'getteximage-srgb')
    g(['arb_texture_view-texsubimage-levels'], 'texsubimage-levels')
    g(['arb_texture_view-texsubimage-levels', 'pbo'], 'texsubimage-levels pbo')
    g(['arb_texture_view-texsubimage-layers'], 'texsubimage-layers')
    g(['arb_texture_view-texsubimage-layers', 'pbo'], 'texsubimage-layers pbo')
    g(['arb_texture_view-clear-into-view-2d'], 'clear-into-view-2d')
    g(['arb_texture_view-clear-into-view-2d-array'],
      'clear-into-view-2d-array')
    g(['arb_texture_view-clear-into-view-layered'], 'clear-into-view-layered')
    g(['arb_texture_view-copytexsubimage-layers'], 'copytexsubimage-layers')
    g(['arb_texture_view-sampling-2d-array-as-cubemap'],
      'sampling-2d-array-as-cubemap')
    g(['arb_texture_view-sampling-2d-array-as-cubemap-array'],
      'sampling-2d-array-as-cubemap-array')
    g(['arb_texture_view-sampling-2d-array-as-2d-layer'],
      'sampling-2d-array-as-2d-layer')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'OES_texture_view')) as g:
    g(['arb_texture_view-rendering-formats_gles3'], 'rendering-formats')
    g(['arb_texture_view-rendering-layers_gles3'], 'rendering-layers')
    g(['arb_texture_view-rendering-levels_gles3'], 'rendering-levels')
    g(['arb_texture_view-rendering-target_gles3'], 'rendering-target')
    g(['arb_texture_view-sampling-2d-array-as-cubemap_gles3'],
      'sampling-2d-array-as-cubemap')
    g(['arb_texture_view-sampling-2d-array-as-cubemap-array_gles3'],
      'sampling-2d-array-as-cubemap-array')
    g(['arb_texture_view-sampling-2d-array-as-2d-layer_gles3'],
      'sampling-2d-array-as-2d-layer')
    g(['arb_texture_view-texture-immutable-levels_gles3'], 'immutable_levels')
    g(['arb_texture_view-formats_gles3'], 'formats')
    g(['arb_texture_view-queries_gles3'], 'queries')
    g(['arb_texture_view-targets_gles3'], 'targets')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '3DFX_texture_compression_FXT1')) as g:
    g(['compressedteximage', 'GL_COMPRESSED_RGB_FXT1_3DFX'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_FXT1_3DFX'])
    g(['fxt1-teximage'], run_concurrent=False)
    g(['arb_texture_compression-invalid-formats', 'fxt1'],
      'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_3DFX_texture_compression_FXT1'],
      'fbo-generatemipmap-formats')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_clip_control')) as g:
    g(['arb_clip_control-clip-control'])
    g(['arb_clip_control-depth-precision'])
    g(['arb_clip_control-viewport'])

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
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
    add_fbo_depthstencil_tests(g, 'GL_DEPTH32F_STENCIL8', 0)

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_get_texture_sub_image')) as g:
    g(['arb_get_texture_sub_image-cubemap'])
    g(['arb_get_texture_sub_image-errors'])
    g(['arb_get_texture_sub_image-get'])
    g(['arb_get_texture_sub_image-getcompressed'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_env_combine')) as g:
    g(['ext_texture_env_combine-combine'], 'texture-env-combine')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_env_crossbar')) as g:
    g(['crossbar'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_texture_compression')) as g:
    g(['arb_texture_compression-internal-format-query'],
      'GL_TEXTURE_INTERNAL_FORMAT query')
    g(['arb_texture_compression-invalid-formats', 'unknown'],
      'unknown formats')
    g(['fbo-generatemipmap-formats', 'GL_ARB_texture_compression'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_ARB_texture_compression')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_vertex_array_bgra')) as g:
    g(['bgra-sec-color-pointer'], run_concurrent=False)
    g(['bgra-vert-attrib-pointer'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'apple_vertex_array_object')) as g:
    g(['vao-01'], run_concurrent=False)
    g(['vao-02'], run_concurrent=False)
    g(['arb_vertex_array-isvertexarray', 'apple'], 'isvertexarray')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_array_bgra')) as g:
    g(['arb_vertex_array_bgra-api-errors'], 'api-errors', run_concurrent=False)
    g(['arb_vertex_array_bgra-get'], 'get', run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_array_object')) as g:
    g(['vao-element-array-buffer'])
    g(['arb_vertex_array-isvertexarray'], 'isvertexarray')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_buffer_object')) as g:
    g(['arb_vertex_buffer_object-combined-vertex-index'],
      'combined-vertex-index')
    g(['arb_vertex_buffer_object-elements-negative-offset'],
      'elements-negative-offset')
    g(['arb_vertex_buffer_object-mixed-immediate-and-vbo'],
      'mixed-immediate-and-vbo')
    g(['arb_vertex_buffer_object-delete-mapped-buffer'])
    g(['arb_vertex_buffer_object-map-after-draw'])
    g(['arb_vertex_buffer_object-map-empty'])
    g(['arb_vertex_buffer_object-ib-data-sync'], 'ib-data-sync')
    g(['arb_vertex_buffer_object-ib-subdata-sync'], 'ib-subdata-sync')
    g(['pos-array'])
    g(['vbo-bufferdata'])
    g(['vbo-map-remap'])
    g(['vbo-map-unsync'])
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawarrays'],
      'vbo-subdata-many drawarrays')
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawelements'],
      'vbo-subdata-many drawelements')
    g(['arb_vertex_buffer_object-vbo-subdata-many', 'drawrangeelements'],
      'vbo-subdata-many drawrangeelements')
    g(['vbo-subdata-sync'])
    g(['vbo-subdata-zero'])

with profile.test_list.group_manager(
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
    g(['arb_vertex_program-matrix-property-bindings'])
    g(['arb_vertex_program-minmax'], 'minmax')
    g(['arb_vertex_program-property-bindings'])
    g(['arb_vertex_program-get-limits-without-fp'], run_concurrent=False)
    g(['vp-address-01'], run_concurrent=False)
    g(['vp-address-02'], run_concurrent=False)
    g(['vp-address-04'], run_concurrent=False)
    g(['vp-bad-program'], run_concurrent=False)
    g(['vp-max-array'], run_concurrent=False)

with profile.test_list.group_manager(
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
    g(['arb_viewport_array-render-viewport-2'], 'render-viewport-2')
    g(['arb_viewport_array-render-depthrange'], 'render-depthrange')
    g(['arb_viewport_array-render-scissor'], 'render-scissor')
    g(['arb_viewport_array-clear'], 'clear')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_viewport_array')) as g:
    g(['arb_viewport_array-viewport-indices_gles3'], 'viewport-indices')
    g(['arb_viewport_array-depthrange-indices_gles3'], 'depthrange-indices')
    g(['arb_viewport_array-scissor-check_gles3'], 'scissor-check')
    g(['arb_viewport_array-scissor-indices_gles3'], 'scissor-indices')
    g(['arb_viewport_array-bounds_gles3'], 'bounds')
    g(['arb_viewport_array-queries_gles3'], 'queries')
    g(['arb_viewport_array-minmax_gles3'], 'minmax')
    g(['arb_viewport_array-render-viewport_gles3'], 'render-viewport')
    g(['arb_viewport_array-render-viewport-2_gles3'], 'render-viewport-2')
    g(['arb_viewport_array-render-depthrange_gles3'], 'render-depthrange')
    g(['arb_viewport_array-render-scissor_gles3'], 'render-scissor')
    g(['arb_viewport_array-clear_gles3'], 'clear')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_vertex_program2_option')) as g:
    g(['vp-address-03'], run_concurrent=False)
    g(['vp-address-05'], run_concurrent=False)
    g(['vp-address-06'], run_concurrent=False)
    g(['vp-clipdistance-01'], run_concurrent=False)
    g(['vp-clipdistance-02'], run_concurrent=False)
    g(['vp-clipdistance-03'], run_concurrent=False)
    g(['vp-clipdistance-04'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_framebuffer_blit')) as g:
    g(['fbo-blit'], run_concurrent=False)
    g(['fbo-copypix'], run_concurrent=False)
    g(['fbo-readdrawpix'], run_concurrent=False)
    g(['fbo-sys-blit'], run_concurrent=False)
    g(['fbo-sys-sub-blit'], run_concurrent=False)
    g(['fbo-generatemipmap-versus-READ_FRAMEBUFFER'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec',
                        'ext_framebuffer_multisample_blit_scaled')) as g:
    g(['ext_framebuffer_multisample_blit_scaled-negative-blit-scaled'],
      'negative-blit-scaled')

    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample_blit_scaled-blit-scaled',
           sample_count],
          'blit-scaled samples={}'.format(sample_count))

    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample_blit_scaled-blit-scaled',
           sample_count, 'array'],
          'blit-scaled samples={} with GL_TEXTURE_2D_MULTISAMPLE_ARRAY'.format(sample_count))

with profile.test_list.group_manager(
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
    g(['ext_framebuffer_multisample-fast-clear'], 'fast-clear')

    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-alpha-blending-after-rendering',
           sample_count],
          'alpha-blending-after-rendering {}'.format(sample_count))

    for num_samples in ['all_samples'] + MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-formats', num_samples],
          'formats {}'.format(num_samples))

        for test_type in ('color', 'srgb', 'stencil_draw', 'stencil_resolve',
                          'depth_draw', 'depth_resolve'):
            sensible_options = ['small', 'depthstencil']
            if test_type in ('color', 'srgb'):
                sensible_options.append('linear')
            for options in power_set(sensible_options):
                g(['ext_framebuffer_multisample-accuracy', num_samples,
                   test_type] + options,
                  ' '.join(['accuracy', num_samples, test_type] + options))

    # Note: the interpolation tests also check for sensible behaviour with
    # non-multisampled framebuffers, so go ahead and test them with
    # num_samples==0 as well.
    for num_samples in ['0'] + MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-blit-multiple-render-targets',
           num_samples],
          'blit-multiple-render-targets {}'.format(num_samples))

        for test_type in ('non-centroid-disabled', 'centroid-disabled',
                          'centroid-edges', 'non-centroid-deriv',
                          'non-centroid-deriv-disabled', 'centroid-deriv',
                          'centroid-deriv-disabled'):
            g(['ext_framebuffer_multisample-interpolation', num_samples,
               test_type],
              'interpolation {} {}'.format(num_samples, test_type))

    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['ext_framebuffer_multisample-turn-on-off', sample_count],
          'turn-on-off {}'.format(sample_count), run_concurrent=False)

        for buffer_type in ('color', 'depth', 'stencil'):
            if buffer_type == 'color':
                sensible_options = ['linear']
            else:
                sensible_options = []

            for options in power_set(sensible_options):
                g(['ext_framebuffer_multisample-upsample', sample_count,
                   buffer_type] + options,
                  'upsample {} {}'.format(
                      sample_count, ' '.join([buffer_type] + options)))
                g(['ext_framebuffer_multisample-multisample-blit',
                   sample_count, buffer_type] + options,
                  'multisample-blit {}'.format(
                      ' '.join([sample_count, buffer_type] + options)))

            for blit_type in ('msaa', 'upsample', 'downsample'):
                g(['ext_framebuffer_multisample-unaligned-blit',
                   sample_count, buffer_type, blit_type],
                  'unaligned-blit {} {} {}'.format(
                      sample_count, buffer_type, blit_type))

        for test_mode in ('inverted', 'non-inverted'):
            g(['ext_framebuffer_multisample-sample-coverage', sample_count,
               test_mode],
              'sample-coverage {} {}'.format(sample_count, test_mode))

        for buffer_type in ('color', 'depth'):
            g(['ext_framebuffer_multisample-sample-alpha-to-coverage',
               sample_count, buffer_type],
              'sample-alpha-to-coverage {} {}'.format(
                  sample_count, buffer_type))

        for test in ['line-smooth', 'point-smooth', 'polygon-smooth',
                     'sample-alpha-to-one',
                     'draw-buffers-alpha-to-one',
                     'draw-buffers-alpha-to-coverage',
                     'alpha-to-coverage-dual-src-blend',
                     'alpha-to-coverage-no-draw-buffer-zero',
                     'alpha-to-coverage-no-draw-buffer-zero-write',
                     'alpha-to-one-dual-src-blend',
                     'int-draw-buffers-alpha-to-one',
                     'int-draw-buffers-alpha-to-coverage',
                     'alpha-to-one-msaa-disabled',
                     'alpha-to-one-single-sample-buffer',
                     'bitmap', 'polygon-stipple']:
            g(['ext_framebuffer_multisample-{}'.format(test),
               sample_count],
              '{} {}'.format(test, sample_count))

        for blit_type in ('msaa', 'upsample', 'downsample', 'normal'):
            g(['ext_framebuffer_multisample-clip-and-scissor-blit',
               sample_count, blit_type],
              'clip-and-scissor-blit {} {}'.format(sample_count, blit_type))

        for flip_direction in ('x', 'y'):
            g(['ext_framebuffer_multisample-blit-flipped', sample_count,
               flip_direction],
              'blit-flipped {} {}'.format(sample_count, flip_direction))

        for buffer_type in ('color', 'depth', 'stencil'):
            g(['ext_framebuffer_multisample-clear', sample_count, buffer_type],
              'clear {} {}'.format(sample_count, buffer_type))

        for test_type in ('depth', 'depth-computed', 'stencil'):
            for buffer_config in ('combined', 'separate', 'single'):
                g(['ext_framebuffer_multisample-no-color', sample_count,
                   test_type, buffer_config],
                  'no-color {} {} {}'.format(
                      sample_count, test_type, buffer_config))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_framebuffer_object')) as g:
    g(['fbo-generatemipmap-noimage'])
    g(['fbo-1d'])
    g(['fbo-3d'])
    g(['fbo-alphatest-formats'])
    g(['fbo-alphatest-nocolor'])
    g(['fbo-alphatest-nocolor-ff'])
    g(['fbo-blending-formats'])
    g(['fbo-blending-snorm'])
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
    g(['ext_framebuffer_object-error-handling'])
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
    g(['ext_framebuffer_object-mipmap'])
    g(['fbo-nodepth-test'])
    g(['fbo-nostencil-test'])
    g(['fbo-readpixels'])
    g(['fbo-readpixels-depth-formats'])
    g(['fbo-scissor-bitmap'])
    g(['fbo-storage-completeness'])
    g(['fbo-storage-formats'])
    g(['getteximage-formats', 'init-by-rendering'])
    g(['getteximage-formats', 'init-by-clear-and-render'])
    g(['ext_framebuffer_multisample-fast-clear', 'single-sample'],
      'fbo-fast-clear')
    g(['ext_framebuffer_object-border-texture-finish'])
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX1')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX4')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX8')
    add_fbo_stencil_tests(g, 'GL_STENCIL_INDEX16')

with profile.test_list.group_manager(
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
    g(['ext_image_dma_buf_import-refcount'])
    g(['ext_image_dma_buf_import-sample_rgb', '-fmt=AR24'],
      'ext_image_dma_buf_import-sample_argb8888', run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_rgb', '-fmt=XR24', '-alpha-one'],
      'ext_image_dma_buf_import-sample_xrgb8888', run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_yuv', '-fmt=NV12', '-alpha-one'],
      'ext_image_dma_buf_import-sample_nv12', run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_yuv', '-fmt=YU12', '-alpha-one'],
      'ext_image_dma_buf_import-sample_yuv420', run_concurrent=False)
    g(['ext_image_dma_buf_import-sample_yuv', '-fmt=YV12', '-alpha-one'],
      'ext_image_dma_buf_import-sample_yvu420', run_concurrent=False)
    g(['ext_image_dma_buf_import-transcode-nv12-as-r8-gr88'],
      'ext_image_dma_buf_import-transcode-nv12-as-r8-gr88',
      run_concurrent=False)

with profile.test_list.group_manager(
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
    add_fbo_depthstencil_tests(g, 'GL_DEPTH24_STENCIL8', 0)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_packed_depth_stencil')) as g:
    g(['oes_packed_depth_stencil-depth-stencil-texture_gles2'],
      'DEPTH_STENCIL texture GLES2')
    g(['oes_packed_depth_stencil-depth-stencil-texture_gles1'],
      'DEPTH_STENCIL texture GLES1')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_required_internalformat')) as g:
    g(['oes_required_internalformat-renderbuffer'], 'renderbuffer')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_occlusion_query_boolean')) as g:
    g(['ext_occlusion_query_boolean-any-samples'], 'any-samples')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_disjoint_timer_query')) as g:
    g(['ext_disjoint_timer_query-simple'], 'simple')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_norm16')) as g:
    g(['ext_texture_norm16-render'], 'render')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_frag_depth')) as g:
    g(['fragdepth_gles2'])

with profile.test_list.group_manager(
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
        g(['ext_texture_array-compressed', test_mode, '-fbo'],
          'compressed {0}'.format(test_mode),
          run_concurrent=False)
        g(['ext_texture_array-compressed', test_mode, 'pbo', '-fbo'],
          'compressed {0} pbo'.format(test_mode),
          run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_cube_map')) as g:
    g(['arb_texture_cube_map-unusual-order'], run_concurrent=False)
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

with profile.test_list.group_manager(
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

    for stage in ['vs', 'gs', 'fs', 'tes']:
        # textureSize():
        for sampler in['samplerCubeArray', 'isamplerCubeArray',
                       'usamplerCubeArray', 'samplerCubeArrayShadow']:
            g(['textureSize', stage, sampler],
              grouptools.join('textureSize', '{}-textureSize-{}'.format(
                  stage, sampler)))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_swizzle')) as g:
    g(['ext_texture_swizzle-api'])
    g(['ext_texture_swizzle-swizzle'])
    g(['depth_texture_mode_and_swizzle'], 'depth_texture_mode_and_swizzle')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_compression_latc')) as g:
    g(['arb_texture_compression-invalid-formats', 'latc'], 'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_latc'],
      'fbo-generatemipmap-formats')
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_compression_latc-signed'],
      'fbo-generatemipmap-formats-signed')
    add_texwrap_format_tests(g, 'GL_EXT_texture_compression_latc')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_compression_s3tc')) as g:
    g(['compressedteximage', 'GL_COMPRESSED_RGB_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT1_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT3_EXT'])
    g(['compressedteximage', 'GL_COMPRESSED_RGBA_S3TC_DXT5_EXT'])
    g(['arb_texture_compression-invalid-formats', 's3tc'], 'invalid formats')
    g(['gen-compressed-teximage'], run_concurrent=False)
    g(['s3tc-errors'])
    g(['s3tc-targeted'])
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ati_texture_compression_3dc')) as g:
    g(['arb_texture_compression-invalid-formats', '3dc'], 'invalid formats')
    g(['fbo-generatemipmap-formats', 'GL_ATI_texture_compression_3dc'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_ATI_texture_compression_3dc')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_packed_float')) as g:
    g(['ext_packed_float-pack'], 'pack')
    g(['getteximage-invalid-format-for-packed-type'],
      'getteximage-invalid-format-for-packed-type')
    add_msaa_formats_tests(g, 'GL_EXT_packed_float')
    add_texwrap_format_tests(g, 'GL_EXT_packed_float')
    add_fbo_formats_tests(g, 'GL_EXT_packed_float')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_float')) as g:
    g(['arb_texture_float-texture-float-formats'], run_concurrent=False)
    g(['arb_texture_float-get-tex3d'], run_concurrent=False)
    add_msaa_formats_tests(g, 'GL_ARB_texture_float')
    add_texwrap_format_tests(g, 'GL_ARB_texture_float')
    add_fbo_formats_tests(g, 'GL_ARB_texture_float')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_texture_float')) as g:
    g(['oes_texture_float'])
    g(['oes_texture_float', 'half'])
    g(['oes_texture_float', 'linear'])
    g(['oes_texture_float', 'half', 'linear'])


with profile.test_list.group_manager(
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
    g(['ext_texture_integer-texformats']),
    g(['ext_texture_integer-texture_integer_glsl130'],
      'texture_integer_glsl130')
    g(['fbo-integer'], run_concurrent=False)
    # TODO: unsupported for int yet
    # g(['fbo-clear-formats', 'GL_EXT_texture_integer'], 'fbo-clear-formats')
    add_msaa_formats_tests(g, 'GL_EXT_texture_integer')
    add_texwrap_format_tests(g, 'GL_EXT_texture_integer')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_rgb10_a2ui')) as g:
    g(['ext_texture_integer-fbo-blending', 'GL_ARB_texture_rgb10_a2ui'],
      'fbo-blending')
    add_texwrap_format_tests(g, 'GL_ARB_texture_rgb10_a2ui')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_shared_exponent')) as g:
    g(['fbo-generatemipmap-formats', 'GL_EXT_texture_shared_exponent'],
      'fbo-generatemipmap-formats')
    add_texwrap_format_tests(g, 'GL_EXT_texture_shared_exponent')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_snorm')) as g:
    add_msaa_formats_tests(g, 'GL_EXT_texture_snorm')
    add_texwrap_format_tests(g, 'GL_EXT_texture_snorm')
    add_fbo_formats_tests(g, 'GL_EXT_texture_snorm')

with profile.test_list.group_manager(
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
    g(['ext_framebuffer_multisample-fast-clear',
       'GL_EXT_texture_sRGB',
       'single-sample'],
      'fbo-fast-clear')
    add_msaa_formats_tests(g, 'GL_EXT_texture_sRGB')
    add_texwrap_format_tests(g, 'GL_EXT_texture_sRGB')
    add_texwrap_format_tests(g, 'GL_EXT_texture_sRGB-s3tc', '-s3tc')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_timer_query')) as g:
    g(['ext_timer_query-time-elapsed'], 'time-elapsed', run_concurrent=False)
    g(['timer_query'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_timer_query')) as g:
    g(['ext_timer_query-time-elapsed', 'timestamp'], 'query GL_TIMESTAMP', run_concurrent=False)
    g(['ext_timer_query-lifetime'], 'query-lifetime')
    g(['arb_timer_query-timestamp-get'], 'timestamp-get', run_concurrent=False)

with profile.test_list.group_manager(
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
        if api_suffix == '_gles3':
            subtest_list = ['basic-struct']
        else:
            subtest_list = ['basic-struct', 'struct-whole-array',
                            'struct-array-elem', 'array-struct',
                            'array-struct-whole-array',
                            'array-struct-array-elem', 'struct-struct',
                            'array-struct-array-struct']
        for subtest in subtest_list:
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

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_transform_feedback2')) as g:
    g(['arb_transform_feedback2-change-objects-while-paused'],
      'Change objects while paused', run_concurrent=False)
    g(['arb_transform_feedback2-change-objects-while-paused_gles3'],
      'Change objects while paused (GLES3)', run_concurrent=False)

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_transform_instanced')) as g:
    g(['arb_transform_feedback2-draw-auto', 'instanced'],
      'draw-auto instanced', run_concurrent=False)

with profile.test_list.group_manager(
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
    g(['arb_transform_feedback3-begin_end'], run_concurrent=False)

    for param in ['gl_NextBuffer-1', 'gl_NextBuffer-2', 'gl_SkipComponents1-1',
                  'gl_SkipComponents1-2', 'gl_SkipComponents1-3',
                  'gl_SkipComponents2', 'gl_SkipComponents3',
                  'gl_SkipComponents4',
                  'gl_NextBuffer-gl_SkipComponents1-gl_NextBuffer',
                  'gl_NextBuffer-gl_NextBuffer', 'gl_SkipComponents1234', 'gl_SkipComponents1-gl_NextBuffer']:
        g(['ext_transform_feedback-output-type', param], param)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_transform_feedback_overflow_query')) as g:
    g(['arb_transform_feedback_overflow_query-basic'])
    g(['arb_transform_feedback_overflow_query-errors'])

with profile.test_list.group_manager(
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
    g(['arb_uniform_buffer_object-rendering-array'], 'rendering-array')
    g(['arb_uniform_buffer_object-rendering-array', 'offset'], 'rendering-array-offset')
    g(['arb_uniform_buffer_object-rendering-dsa'], 'rendering-dsa')
    g(['arb_uniform_buffer_object-rendering-dsa', 'offset'], 'rendering-dsa-offset')
    g(['arb_uniform_buffer_object-row-major'], 'row-major')
    g(['arb_uniform_buffer_object-uniformblockbinding'], 'uniformblockbinding')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_uniform_buffer_object',
                        'maxuniformblocksize')) as g:
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'vs'], 'vs')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'vsexceed'],
      'vsexceed')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'fs'], 'fs')
    g(['arb_uniform_buffer_object-maxuniformblocksize', 'fsexceed'],
      'fsexceed')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ati_draw_buffers')) as g:
    g(['ati_draw_buffers-arbfp'])
    g(['ati_draw_buffers-arbfp-no-index'], 'arbfp-no-index')
    g(['ati_draw_buffers-arbfp-no-option'], 'arbfp-no-option')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ati_envmap_bumpmap')) as g:
    g(['ati_envmap_bumpmap-bump'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_instanced_arrays')) as g:
    g(['arb_instanced_arrays-vertex-attrib-divisor-index-error'])
    g(['arb_instanced_arrays-instanced_arrays'])
    g(['arb_instanced_arrays-drawarrays'])
    add_single_param_test_set(g, 'arb_instanced_arrays-instanced_arrays',
                              'vbo')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_internalformat_query')) as g:
    g(['arb_internalformat_query-api-errors'], 'misc. API error checks')
    g(['arb_internalformat_query-overrun'], 'buffer over-run checks')
    g(['arb_internalformat_query-minmax'], 'minmax')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_internalformat_query2')) as g:
    g(['arb_internalformat_query2-api-errors'], 'API error checks')
    g(['arb_internalformat_query2-generic-pname-checks'], 'Individual most generic pname checks')
    g(['arb_internalformat_query2-samples-pnames'], 'SAMPLES and NUM_SAMPLE_COUNTS pname checks')
    g(['arb_internalformat_query2-internalformat-size-checks'], 'All INTERNALFORMAT_<X>_SIZE pname checks')
    g(['arb_internalformat_query2-internalformat-type-checks'], 'All INTERNALFORMAT_<X>_TYPE pname checks')
    g(['arb_internalformat_query2-image-format-compatibility-type'], 'IMAGE_FORMAT_COMPATIBILITY_TYPE pname checks')
    g(['arb_internalformat_query2-max-dimensions'], 'Max dimensions related pname checks')
    g(['arb_internalformat_query2-color-encoding'], 'COLOR_ENCODING pname check')
    g(['arb_internalformat_query2-texture-compressed-block'], 'All TEXTURE_COMPRESSED_BLOCK_<X> pname checks')
    g(['arb_internalformat_query2-minmax'], 'minmax check for SAMPLES/NUM_SAMPLE_COUNTS')
    g(['arb_internalformat_query2-image-texture'], 'Checks for pnames related to ARB_image_load_store that return values from Table 3.22 (OpenGL 4.2)')
    g(['arb_internalformat_query2-filter'], 'FILTER pname checks.')
    g(['arb_internalformat_query2-format-components'], '{COLOR,DEPTH,STENCIL}_COMPONENTS pname checks')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_multisample')) as g:
    g(['arb_multisample-beginend'], 'beginend')
    g(['arb_multisample-pushpop'], 'pushpop')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_seamless_cube_map')) as g:
    g(['arb_seamless_cubemap'])
    g(['arb_seamless_cubemap-initially-disabled'])
    g(['arb_seamless_cubemap-three-faces-average'])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'AMD_pinned_memory')) as g:
    g(['amd_pinned_memory', 'offset=0'], 'offset=0')
    g(['amd_pinned_memory', 'increment-offset'], 'increment-offset')
    g(['amd_pinned_memory', 'decrement-offset'], 'decrement-offset')
    g(['amd_pinned_memory', 'offset=0', 'map-buffer'], 'map-buffer offset=0')
    g(['amd_pinned_memory', 'increment-offset', 'map-buffer'],
      'map-buffer increment-offset')
    g(['amd_pinned_memory', 'decrement-offset', 'map-buffer'],
      'map-buffer decrement-offset')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_seamless_cubemap_per_texture')) as g:
    g(['amd_seamless_cubemap_per_texture'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_vertex_shader_layer')) as g:
    g(['amd_vertex_shader_layer-layered-2d-texture-render'],
      run_concurrent=False)
    g(['amd_vertex_shader_layer-layered-depth-texture-render'],
      run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'amd_vertex_shader_viewport_index')) as g:
    g(['amd_vertex_shader_viewport_index-render'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_fog_coord')) as g:
    g(['ext_fog_coord-modes'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_texture_barrier')) as g:
    g(['blending-in-shader'], run_concurrent=False)
    g(['arb_texture_barrier-texture-halves-ping-pong-operation-chain'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_texture_env_combine4')) as g:
    g(['nv_texture_env_combine4-combine'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_conditional_render')) as g:
    g(['nv_conditional_render-begin-while-active'], 'begin-while-active')
    g(['nv_conditional_render-begin-zero'], 'begin-zero')
    g(['nv_conditional_render-bitmap'], 'bitmap')
    g(['nv_conditional_render-blitframebuffer'], 'blitframebuffer')
    g(['nv_conditional_render-clear'], 'clear')
    g(['nv_conditional_render-copypixels'], 'copypixels')
    g(['nv_conditional_render-copyteximage'], 'copyteximage')
    g(['nv_conditional_render-copytexsubimage'], 'copytexsubimage')
    g(['nv_conditional_render-dlist'], 'dlist')
    g(['nv_conditional_render-drawpixels'], 'drawpixels')
    g(['nv_conditional_render-generatemipmap'], 'generatemipmap')
    g(['nv_conditional_render-vertex_array'], 'vertex_array')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'nv_fill_rectangle')) as g:
    g(['nv_fill_rectangle-invalid-draw-mode'], 'invalid-draw-mode')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_matrix_get')) as g:
    g(['oes_matrix_get-api'], 'All queries')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_fixed_point')) as g:
    g(['oes_fixed_point-attribute-arrays'], 'attribute-arrays')

with profile.test_list.group_manager(
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
    g(['arb_clear_buffer_object-unaligned'])
    g(['arb_clear_buffer_object-zero-size'])

with profile.test_list.group_manager(
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
    g(['arb_clear_texture-texview'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_copy_buffer')) as g:
    g(['copy_buffer_coherency'], run_concurrent=False)
    g(['copybuffersubdata'], run_concurrent=False)
    g(['arb_copy_buffer-data-sync'], 'data-sync')
    g(['arb_copy_buffer-dlist'], 'dlist')
    g(['arb_copy_buffer-get'], 'get')
    g(['arb_copy_buffer-intra-buffer-copy'], 'intra-buffer-copy')
    g(['arb_copy_buffer-negative-bound-zero'], 'negative-bound-zero')
    g(['arb_copy_buffer-negative-bounds'], 'negative-bounds')
    g(['arb_copy_buffer-negative-mapped'], 'negative-mapped')
    g(['arb_copy_buffer-overlap'], 'overlap')
    g(['arb_copy_buffer-targets'], 'targets')
    g(['arb_copy_buffer-subdata-sync'], 'subdata-sync')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_copy_image')) as g:
    g(['arb_copy_image-simple', '--tex-to-tex'])
    g(['arb_copy_image-simple', '--rb-to-tex'])
    g(['arb_copy_image-simple', '--rb-to-rb'])
    g(['arb_copy_image-srgb-copy'])
    g(['arb_copy_image-api_errors'])
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
    g(['arb_copy_image-format-swizzle'])
    g(['arb_copy_image-texview'])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_cull_distance')) as g:
    g(['arb_cull_distance-max-distances'])
    g(['arb_cull_distance-exceed-limits', 'cull'])
    g(['arb_cull_distance-exceed-limits', 'clip'])
    g(['arb_cull_distance-exceed-limits', 'total'])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_half_float_vertex')) as g:
    g(['draw-vertices-half-float'])
    g(['draw-vertices-half-float', 'user'], 'draw-vertices-half-float-user')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'oes_vertex_half_float')) as g:
    g(['draw-vertices-half-float_gles2'], run_concurrent=False)
    g(['draw-vertices-half-float_gles2', 'user'], 'draw-vertices-half-float-user_gles2',
      run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_type_2_10_10_10_rev')) as g:
    g(['draw-vertices-2101010'], run_concurrent=False)
    g(['attribs', 'GL_ARB_vertex_type_2_10_10_10_rev'], 'attribs')
    g(['arb_vertex_type_2_10_10_10_rev-array_types'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_type_10f_11f_11f_rev')) as g:
    g(['arb_vertex_type_10f_11f_11f_rev-api-errors'], run_concurrent=False)
    g(['arb_vertex_type_10f_11f_11f_rev-draw-vertices'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_draw_buffers')) as g:
    g(['arb_draw_buffers-state_change'])
    g(['fbo-mrt-alphatest'])
    g(['fbo-mrt-new-bind'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_draw_buffers2')) as g:
    g(['fbo-drawbuffers2-blend'])
    g(['fbo-drawbuffers2-colormask'])
    g(['fbo-drawbuffers2-colormask', 'clear'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_draw_buffers_blend')) as g:
    g(['arb_draw_buffers_blend-state_set_get'])
    g(['fbo-draw-buffers-blend'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_blend_func_extended')) as g:
    g(['arb_blend_func_extended-bindfragdataindexed-invalid-parameters'])
    g(['arb_blend_func_extended-blend-api'])
    g(['arb_blend_func_extended-error-at-begin'])
    g(['arb_blend_func_extended-getfragdataindex'])
    g(['arb_blend_func_extended-output-location'])
    g(['arb_blend_func_extended-fbo-extended-blend'])
    g(['arb_blend_func_extended-fbo-extended-blend-explicit'])
    g(['arb_blend_func_extended-fbo-extended-blend-pattern'])
    g(['arb_blend_func_extended-blend-api_gles2'])
    g(['arb_blend_func_extended-builtins_gles2'])
    g(['arb_blend_func_extended-bindfragdataindexed-invalid-parameters_gles3'])
    g(['arb_blend_func_extended-output-location_gles3'])
    g(['arb_blend_func_extended-getfragdataindex_gles3'])
    g(['arb_blend_func_extended-fbo-extended-blend-pattern_gles2'])
    g(['arb_blend_func_extended-fbo-extended-blend-pattern_gles3'])
    g(['arb_blend_func_extended-fbo-extended-blend_gles3'])
    g(['arb_blend_func_extended-fbo-extended-blend-explicit_gles3'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_base_instance')) as g:
    g(['arb_base_instance-baseinstance-doesnt-affect-gl-instance-id'],
      run_concurrent=False)
    g(['arb_base_instance-drawarrays'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_base_instance')) as g:
    g(['arb_base_instance-baseinstance-doesnt-affect-gl-instance-id_gles3'],
      run_concurrent=False)
    g(['arb_base_instance-drawarrays_gles3'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_buffer_storage')) as g:
    for mode in ['read', 'draw']:
        g(['bufferstorage-persistent', mode])
        g(['bufferstorage-persistent', mode, 'coherent'])
        g(['bufferstorage-persistent', mode, 'client-storage'])
        g(['bufferstorage-persistent', mode, 'coherent', 'client-storage'])
        g(['bufferstorage-persistent_gles3', mode])
        g(['bufferstorage-persistent_gles3', mode, 'coherent'])
        g(['bufferstorage-persistent_gles3', mode, 'client-storage'])
        g(['bufferstorage-persistent_gles3', mode, 'coherent', 'client-storage'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'apple_object_purgeable')) as g:
    g(['object_purgeable-api-pbo'], run_concurrent=False)
    g(['object_purgeable-api-texture'], run_concurrent=False)
    g(['object_purgeable-api-vbo'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'mesa_pack_invert')) as g:
    g(['mesa_pack_invert-readpixels'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_read_format')) as g:
    g(['oes-read-format'], run_concurrent=False)

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_provoking_vertex')) as g:
    g(['provoking-vertex'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_lod_bias')) as g:
    g(['lodbias'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'sgis_generate_mipmap')) as g:
    g(['gen-nonzero-unit'], run_concurrent=False)
    g(['gen-teximage'], run_concurrent=False)
    g(['gen-texsubimage'], run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_map_buffer_alignment')) as g:
    g(['arb_map_buffer_alignment-sanity_test'], run_concurrent=False)
    g(['arb_map_buffer_alignment-map-invalidate-range'])

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_compute_shader')) as g:
    g(['arb_compute_shader-api_errors'], 'api_errors')
    g(['arb_compute_shader-minmax'], 'minmax')
    g(['built-in-constants',
       os.path.join('spec', 'arb_compute_shader', 'minimum-maximums.txt')],
      'built-in constants',
      override_class=BuiltInConstantsTest)
    g(['arb_compute_shader-work_group_size_too_large'],
      grouptools.join('compiler', 'work_group_size_too_large'))
    g(['arb_compute_shader-indirect-compute'], 'indirect-compute')
    g(['arb_compute_shader-local-id'], 'local-id' + '-explosion')
    g(['arb_compute_shader-render-and-compute'], 'render-and-compute')
    g(['arb_compute_shader-zero-dispatch-size'], 'zero-dispatch-size')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_storage_buffer_object')) as g:
    g(['arb_shader_storage_buffer_object-minmax'], 'minmax')
    g(['arb_shader_storage_buffer_object-rendering'], 'rendering')
    g(['arb_shader_storage_buffer_object-getintegeri_v'], 'getintegeri_v')
    g(['arb_shader_storage_buffer_object-deletebuffers'], 'deletebuffers')
    g(['arb_shader_storage_buffer_object-maxblocks'], 'maxblocks')
    g(['arb_shader_storage_buffer_object-ssbo-binding'], 'ssbo-binding')
    g(['arb_shader_storage_buffer_object-array-ssbo-binding'], 'array-ssbo-binding')
    g(['arb_shader_storage_buffer_object-layout-std430-write-shader'], 'layout-std430-write-shader')
    g(['arb_shader_storage_buffer_object-layout-std140-write-shader'], 'layout-std140-write-shader')
    g(['arb_shader_storage_buffer_object-program_interface_query'], 'program-interface-query')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_storage_buffer_object',
                        'max-ssbo-size')) as g:
    g(['arb_shader_storage_buffer_object-max-ssbo-size', 'vs'], 'vs')
    g(['arb_shader_storage_buffer_object-max-ssbo-size', 'vsexceed'],
      'vsexceed')
    g(['arb_shader_storage_buffer_object-max-ssbo-size', 'fs'], 'fs')
    g(['arb_shader_storage_buffer_object-max-ssbo-size', 'fsexceed'],
      'fsexceed')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_sparse_buffer')) as g:
    g(['arb_sparse_buffer-basic'], 'basic')
    g(['arb_sparse_buffer-buffer-data'], 'buffer-data')
    g(['arb_sparse_buffer-commit'], 'commit')
    g(['arb_sparse_buffer-minmax'], 'minmax')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_polygon_offset_clamp')) as g:
    g(['ext_polygon_offset_clamp-draw'])
    g(['ext_polygon_offset_clamp-draw_gles2'])
    g(['ext_polygon_offset_clamp-dlist'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_pipeline_statistics_query')) as g:
    g(['arb_pipeline_statistics_query-vert'])
    g(['arb_pipeline_statistics_query-vert_adj'])
    g(['arb_pipeline_statistics_query-clip'])
    g(['arb_pipeline_statistics_query-geom'])
    g(['arb_pipeline_statistics_query-frag'])
    g(['arb_pipeline_statistics_query-comp'])

with profile.test_list.group_manager(PiglitGLTest, 'hiz') as g:
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

with profile.test_list.group_manager(PiglitGLTest, 'fast_color_clear') as g:
    g(['fcc-blit-between-clears'])
    g(['fcc-read-to-pbo-after-clear'], run_concurrent=False)
    g(['fcc-front-buffer-distraction'], run_concurrent=False)

    for subtest in ('sample', 'read_pixels', 'blit', 'copy'):
        for buffer_type in ('rb', 'tex'):
            if subtest == 'sample' and buffer_type == 'rb':
                continue
            g(['fcc-read-after-clear', subtest, buffer_type])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'ext_unpack_subimage')) as g:
    g(['ext_unpack_subimage'], 'basic')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'oes_draw_texture')) as g:
    g(['oes_draw_texture'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_compressed_etc1_rgb8_texture')) as g:
    g(['oes_compressed_etc1_rgb8_texture-basic'], 'basic')
    g(['oes_compressed_etc1_rgb8_texture-miptree'], 'miptree')

with profile.test_list.group_manager(
         PiglitGLTest,
         grouptools.join('spec', 'khr_texture_compression_astc')) as g:
    g(['arb_texture_compression-invalid-formats', 'astc'], 'invalid formats')
    g(['khr_compressed_astc-array_gl'], 'array-gl')
    g(['khr_compressed_astc-array_gles3'], 'array-gles')
    g(['khr_compressed_astc-basic_gl'], 'basic-gl')
    g(['khr_compressed_astc-basic_gles2'], 'basic-gles')

    for subtest in ('hdr', 'ldr', 'srgb', "srgb-fp", "srgb-sd"):
        g(['khr_compressed_astc-miptree_gl', '-subtest', subtest],
           'miptree-gl {}'.format(subtest))
        g(['khr_compressed_astc-miptree_gles2', '-subtest', subtest],
           'miptree-gles {}'.format(subtest))
    for subtest in ('hdr', 'ldr', 'srgb', 'srgb-fp'):
        g(['khr_compressed_astc-sliced-3d-miptree_gl', '-subtest', subtest],
           'sliced-3d-miptree-gl {}'.format(subtest))
        g(['khr_compressed_astc-sliced-3d-miptree_gles3', '-subtest', subtest],
           'sliced-3d-miptree-gles {}'.format(subtest))

with profile.test_list.group_manager(
         PiglitGLTest,
         grouptools.join('spec', 'oes_texture_compression_astc')) as g:
    for subtest in ('hdr', 'ldr', 'srgb'):
        g(['oes_compressed_astc-miptree-3d_gl', '-subtest', subtest],
           'miptree-3d-gl {}'.format(subtest))
        g(['oes_compressed_astc-miptree-3d_gles3', '-subtest', subtest],
           'miptree-3d-gles {}'.format(subtest))

with profile.test_list.group_manager(
         PiglitGLTest,
         grouptools.join('spec', 'nv_read_depth')) as g:
    g(['read_depth_gles3'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'oes_compressed_paletted_texture')) as g:
    g(['oes_compressed_paletted_texture-api'], 'basic API')
    g(['arb_texture_compression-invalid-formats', 'paletted'],
      'invalid formats')

with profile.test_list.group_manager(
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
    g(['egl-create-msaa-pbuffer-surface'],
      'eglCreatePbufferSurface with EGL_SAMPLES set',
      run_concurrent=False)
    g(['egl-create-largest-pbuffer-surface'],
      'largest possible eglCreatePbufferSurface and then glClear',
      run_concurrent=False)
    g(['egl-invalid-attr'])
    g(['egl-context-priority'])
    g(['egl-blob-cache'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_nok_swap_region'),
        exclude_platforms=['glx']) as g:
    g(['egl-nok-swap-region'], 'basic', run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_nok_texture_from_pixmap'),
        exclude_platforms=['glx']) as g:
    g(['egl-nok-texture-from-pixmap'], 'basic', run_concurrent=False)

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_gl_image'),
        exclude_platforms=['glx']) as g:
    for internal_format in ('GL_RGBA', 'GL_DEPTH_COMPONENT24'):
        g(['egl_khr_gl_renderbuffer_image-clear-shared-image', internal_format],
           run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_surfaceless_context'),
        exclude_platforms=['glx']) as g:
    g(['egl-surfaceless-context-viewport'], 'viewport',
      run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_mesa_configless_context'),
        exclude_platforms=['glx']) as g:
    g(['egl-configless-context'], 'basic')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_ext_client_extensions'),
        exclude_platforms=['glx']) as g:
    for i in [1, 2, 3]:
        g(['egl_ext_client_extensions', str(i)],
          'conformance test {0}'.format(i))

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_fence_sync'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_fence_sync'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_android_native_fence_sync'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_fence_sync', 'android_native'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_gl_colorspace'),
        exclude_platforms=['glx']) as g:
    g(['egl-gl-colorspace'], 'linear')
    g(['egl-gl-colorspace', 'srgb'], 'srgb')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_wait_sync'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_fence_sync', 'wait_sync'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_khr_get_all_proc_addresses'),
        exclude_platforms=['glx']) as g:
    g(['egl_khr_get_all_proc_addresses'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_chromium_sync_control'),
        exclude_platforms=['glx']) as g:
    g(['egl_chromium_sync_control'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_ext_device_query'),
        exclude_platforms=['glx']) as g:
    g(['egl_ext_device_query'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_ext_device_enumeration'),
        exclude_platforms=['glx']) as g:
    g(['egl_ext_device_enumeration'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'egl_mesa_platform_surfaceless'),
        exclude_platforms=['glx']) as g:
    g(['egl_mesa_platform_surfaceless'], 'conformance')

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', '!opengl ES 2.0')) as g:
    g(['glsl-fs-pointcoord_gles2'], 'glsl-fs-pointcoord')
    g(['invalid-es3-queries_gles2'])
    g(['link-no-vsfs_gles2'], 'link-no-vsfs')
    g(['minmax_gles2'])
    g(['multiple-shader-objects_gles2'])
    g(['fbo_discard_gles2'])
    g(['draw_buffers_gles2'])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', '!opengl ES 3.0')) as g:
    g(['minmax_gles3'], 'minmax')
    g(['texture-immutable-levels_gles3'], 'texture-immutable-levels')
    g(['gles-3.0-drawarrays-vertexid'], 'gl_VertexID used with glDrawArrays')
    g(['gles-3.0-transform-feedback-uniform-buffer-object'])

    for test_mode in ['teximage', 'texsubimage']:
        g(['ext_texture_array-compressed_gles3', test_mode, '-fbo'],
          'ext_texture_array-compressed_gles3 {0}'.format(test_mode),
          run_concurrent=False)

    for tex_format in ['rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11',
                       'rgb8-punchthrough-alpha1',
                       'srgb8-punchthrough-alpha1']:
        g(['oes_compressed_etc2_texture-miptree_gles3', tex_format])

with profile.test_list.group_manager(
        PiglitGLTest, grouptools.join('spec', 'arb_es3_compatibility')) as g:
    g(['es3-primrestart-fixedindex'])
    g(['es3-drawarrays-primrestart-fixedindex'])

    for tex_format in ['rgb8', 'srgb8', 'rgba8', 'srgb8-alpha8', 'r11', 'rg11',
                       'rgb8-punchthrough-alpha1',
                       'srgb8-punchthrough-alpha1']:
        for context in ['core', 'compat']:
            g(['oes_compressed_etc2_texture-miptree', tex_format, context])

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
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
    g(['arb_direct_state_access-generatetexturemipmap'], 'generatetexturemipmap')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_post_depth_coverage')) as g:
    g(['arb_post_depth_coverage-basic'])
    g(['arb_post_depth_coverage-multisampling'])
    g(['arb_post_depth_coverage-sample-shading'])

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_fragment_shader_interlock')) as g:
    g(['arb_fragment_shader_interlock-image-load-store'])

with profile.test_list.group_manager(
    PiglitGLTest,
    grouptools.join('spec', 'arb_shader_image_size')) as g:
    g(['arb_shader_image_size-builtin'], 'builtin')

with profile.test_list.group_manager(
    PiglitGLTest,
    grouptools.join('spec', 'arb_shader_texture_image_samples')) as g:
    g(['arb_shader_texture_image_samples-builtin-image'], 'builtin-image')

with profile.test_list.group_manager(
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

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_vertex_attrib_64bit')) as g:
    g(['arb_vertex_attrib_64bit-double_attribs'], 'double_attribs')
    g(['arb_vertex_attrib_64bit-check-explicit-location'], 'check-explicit-location')
    g(['arb_vertex_attrib_64bit-getactiveattrib'], 'getactiveattrib')
    g(['arb_vertex_attrib_64bit-max-vertex-attrib'], 'max-vertex-attrib')
    for test_type in ('shader', 'api'):
        g(['arb_vertex_attrib_64bit-overlapping-locations', test_type],
          run_concurrent=False)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_query_buffer_object')) as g:
    g(['arb_query_buffer_object-qbo'], 'qbo')
    g(['arb_query_buffer_object-coherency'], 'coherency')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_framebuffer_blit')) as g:
    g(['ext_framebuffer_blit-blit-early'], 'blit-early')

# Group OES_draw_elements_base_vertex
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'OES_draw_elements_base_vertex')) as g:
    g(['oes_draw_elements_base_vertex-drawelements'], run_concurrent=False)
    g(['oes_draw_elements_base_vertex-drawelements-instanced'],
      run_concurrent=False)
    g(['oes_draw_elements_base_vertex-drawrangeelements'],
      run_concurrent=False)
    g(['oes_draw_elements_base_vertex-multidrawelements'],
      run_concurrent=False)

with profile.test_list.group_manager(
        BuiltInConstantsTest,
        grouptools.join('spec', 'oes_geometry_shader')) as g:
    g(['built-in-constants_gles3',
       os.path.join('spec', 'oes_geometry_shader', 'minimum-maximums.txt')],
      'built-in constants')

# Group EXT_shader_samples_identical
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_shader_samples_identical')) as g:
    for sample_count in MSAA_SAMPLE_COUNTS:
        g(['ext_shader_samples_identical', sample_count])
    g(['ext_shader_samples_identical-simple-fs'], 'simple-fs')

# Group ARB_shader_draw_parameters
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_shader_draw_parameters')) as g:
    g(['arb_shader_draw_parameters-drawid', 'drawid'], 'drawid')
    g(['arb_shader_draw_parameters-drawid', 'vertexid'], 'drawid-vertexid')
    g(['arb_shader_draw_parameters-drawid-indirect', 'drawid'], 'drawid-indirect')
    g(['arb_shader_draw_parameters-drawid-indirect', 'basevertex'], 'drawid-indirect-basevertex')
    g(['arb_shader_draw_parameters-drawid-indirect', 'baseinstance'], 'drawid-indirect-baseinstance')
    g(['arb_shader_draw_parameters-drawid-indirect', 'vertexid'], 'drawid-indirect-vertexid')

    variables = ('basevertex', 'baseinstance', 'basevertex-baseinstance', 'vertexid-zerobased')
    for v in variables:
        g(['arb_shader_draw_parameters-basevertex', v], v)
    for v in variables:
        g(['arb_shader_draw_parameters-basevertex', v, 'indirect'], v + '-indirect')

# Group ARB_indirect_parameters
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_indirect_parameters')) as g:
    g(['arb_indirect_parameters-tf-count-arrays'], 'tf-count-arrays')
    g(['arb_indirect_parameters-tf-count-elements'], 'tf-count-elements')

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('object namespace pollution')) as g:
    for object_type in ("buffer", "framebuffer", "program", "renderbuffer", "texture", "vertex-array"):
        for operation in ("glBitmap", "glBlitFramebuffer", "glClear", "glClearTexSubImage", "glCopyImageSubData", "glCopyPixels", "glCopyTexSubImage2D", "glDrawPixels", "glGenerateMipmap", "glGetTexImage", "glGetTexImage-compressed", "glTexSubImage2D"):
            g(['object-namespace-pollution', operation, object_type],
              '{} with {}'.format(object_type, operation))

# Group ARB_texture_barrier
resolution_set = ['32', '512']
blend_passes_set = ['1', '42']
num_textures_set = ['1', '8']
granularity_set = ['8', '64', '128']
draw_passes_set = ['1', '2', '3', '4', '7', '8']

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_texture_barrier')) as g:
    for resolution, blend_passes, num_textures, granularity, draw_passes in itertools.product(
            resolution_set, blend_passes_set, num_textures_set, granularity_set, draw_passes_set):
        g(['arb_texture_barrier-blending-in-shader', resolution,
           blend_passes, num_textures, granularity, draw_passes])


# Group ARB_invalidate_subdata
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_invalidate_subdata')) as g:
    g(['arb_invalidate_subdata-buffer'], 'buffer')

# Group EXT_window_rectangles
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'EXT_window_rectangles')) as g:
    g(['ext_window_rectangles-dlist'], 'dlist')
    g(['ext_window_rectangles-errors'], 'errors')
    g(['ext_window_rectangles-render'], 'render')

    g(['ext_window_rectangles-errors_gles3'], 'errors_gles3')
    g(['ext_window_rectangles-render_gles3'], 'render_gles3')

# Group ARB_compute_variable_group_size
with profile.test_list.group_manager(
	PiglitGLTest,
	grouptools.join('spec', 'ARB_compute_variable_group_size')) as g:
    g(['arb_compute_variable_group_size-errors'], 'errors')
    g(['arb_compute_variable_group_size-local-size'], 'local-size')
    g(['arb_compute_variable_group_size-minmax'], 'minmax')

# Group INTEL_conservative_rasterization
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'INTEL_conservative_rasterization')) as g:
    g(['intel_conservative_rasterization-depthcoverage'])
    g(['intel_conservative_rasterization-innercoverage'])
    g(['intel_conservative_rasterization-invalid'])
    g(['intel_conservative_rasterization-tri'])
    g(['intel_conservative_rasterization-depthcoverage_gles3'])
    g(['intel_conservative_rasterization-innercoverage_gles3'])
    g(['intel_conservative_rasterization-tri_gles3'])

# Group INTEL_blackhole_render
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'INTEL_blackhole_render')) as g:
    g(['intel_blackhole-draw'])
    g(['intel_blackhole-dispatch'])
    g(['intel_blackhole-draw_gles2'])
    g(['intel_blackhole-draw_gles3'])

# Group ARB_bindless_texture
with profile.test_list.group_manager(
	PiglitGLTest,
	grouptools.join('spec', 'ARB_bindless_texture')) as g:
    g(['arb_bindless_texture-border-color'], 'border-color')
    g(['arb_bindless_texture-conversions'], 'conversions')
    g(['arb_bindless_texture-errors'], 'errors')
    g(['arb_bindless_texture-handles'], 'handles')
    g(['arb_bindless_texture-illegal'], 'illegal')
    g(['arb_bindless_texture-legal'], 'legal')
    g(['arb_bindless_texture-limit'], 'limit')
    g(['arb_bindless_texture-uint64_attribs'], 'uint64_attribs')
    g(['arb_bindless_texture-uniform'], 'uniform')

if platform.system() is 'Windows':
    profile.filters.append(lambda p, _: not p.startswith('glx'))
