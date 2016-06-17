# encoding=utf-8
# Copyright © 2016 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Generate tests for extensions that affect the shading language.

This generates three tests per extensions.
- The first test asserts that if the extension is required it's defined
- The second test asserts that if the extension isn't available it's not
  defined
- The third test asserts that if the extension isn't available trying to
  require it is an error.

These are glslparsertests.

If the requirement for the test is less than OpenGL Shader Language 140, then
two tests will be made, a core and a compat. If it's greater then only a core
will be made, these will have compat and core added to the name, respectively.

For OpenGL ES only one test will be generated, it will have es added to the
name of the test.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

from templates import template_dir
from modules import utils, glsl

_TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))
ENABLED_TEMPLATE = _TEMPLATES.get_template('enabled.glsl.mako')
DISABLED_TEMPLATE = _TEMPLATES.get_template('disabled.glsl.mako')
UNDEFINED_TEMPLATE = _TEMPLATES.get_template('undefined-require.glsl.mako')

# A list of tuples with the full preprocess defined name, and the minimum
# supported version of that extension.
EXTENSIONS = [
    ("GL_ARB_draw_instanced", "110"),
    ("GL_ARB_draw_buffers", "110"),
    ("GL_ARB_enhanced_layouts", "140"),
    ("GL_ARB_separate_shader_objects", "110"),
    ("GL_ARB_texture_rectangle", "110"),
    ("GL_AMD_shader_trinary_minmax", "110"),
    ("GL_EXT_texture_array", "110"),
    ("GL_ARB_ES3_1_compatibility", "430"),
    ("GL_ARB_arrays_of_arrays", "110"),
    ("GL_ARB_fragment_coord_conventions", "110"),
    ("GL_ARB_fragment_layer_viewport", "150"),
    ("GL_ARB_explicit_attrib_location", "110"),
    ("GL_ARB_explicit_uniform_location", "110"),
    ("GL_ARB_shader_texture_lod", "110"),
    ("GL_AMD_conservative_depth", "110"),
    ("GL_ARB_conservative_depth", "110"),
    ("GL_ARB_shader_bit_encoding", "110"),
    ("GL_ARB_shader_clock", "110"),
    ("GL_ARB_uniform_buffer_object", "110"),
    ("GL_ARB_texture_cube_map_array", "110"),
    ("GL_ARB_shading_language_packing", "110"),
    ("GL_ARB_texture_multisample", "110"),
    ("GL_ARB_texture_query_levels", "110"),
    ("GL_ARB_texture_query_lod", "110"),
    ("GL_ARB_gpu_shader5", "150"),
    ("GL_ARB_gpu_shader_fp64", "150"),
    ("GL_ARB_vertex_attrib_64bit", "150"),
    ("GL_AMD_vertex_shader_layer", "130"),
    ("GL_AMD_vertex_shader_viewport_index", "110"),
    ("GL_ARB_shading_language_420pack", "110"),
    ("GL_ARB_sample_shading", "110"),
    ("GL_ARB_texture_gather", "110"),
    ("GL_ARB_shader_atomic_counters", "110"),
    ("GL_ARB_shader_atomic_counter_ops", "140"),
    ("GL_ARB_viewport_array", "110"),
    ("GL_ARB_compute_shader", "110"),
    ("GL_ARB_shader_image_load_store", "110"),
    ("GL_ARB_shader_image_size", "110"),
    ("GL_ARB_shader_texture_image_samples", "110"),
    # That is what the original hand written test required.
    ("GL_ARB_derivative_control", "150"),
    ("GL_ARB_shader_precision", "110"),
    ("GL_ARB_shader_storage_buffer_object", "110"),
    ("GL_ARB_tessellation_shader", "150"),
    ("GL_ARB_shader_subroutine", "150"),
    ("GL_ARB_shader_draw_parameters", "140"),
    ("GL_EXT_separate_shader_objects", "100"),
    ("GL_EXT_draw_buffers", "100"),
    ("GL_AMD_shader_stencil_export", "120"),
    ("GL_ARB_shader_stencil_export", "120"),
    ("GL_ARB_geometry_shader4", "110"),
    ("GL_OES_EGL_image_external", "100"),
    ("GL_EXT_shader_samples_identical", "110"),
    ("GL_ARB_shader_group_vote", "110"),
    ("GL_EXT_shader_samples_identical", "310 es"),
    ("GL_OES_sample_variables", "300 es"),
    ("GL_OES_multisample_interpolation", "300 es"),
    ("GL_OES_standard_derivatives", "100"),
    ("GL_OES_texture_storage_multisample_2d_array", "300 es"),
    ("GL_OES_blend_func_extended", "100"),
    ("GL_OES_shader_image_atomic", "310 es"),
    ("GL_OES_shader_io_blocks", "310 es"),
    ("GL_OES_geometry_shader", "310 es"),
    ("GL_OES_geometry_point_size", "310 es"),
    ("GL_EXT_gpu_shader5", "310 es"),
    ("GL_OES_gpu_shader5", "310 es"),
    ("GL_EXT_texture_buffer", "310 es"),
    ("GL_OES_texture_buffer", "310 es"),
    ("GL_EXT_clip_cull_distance", "300 es"),
]
EXTENSIONS = [(n, glsl.Version(v)) for n, v in EXTENSIONS]


def _gen_tests(ext, version, stage, path, extra_name, extra_extensions):
    """Generate the actual test files.

    This generates both a disabled test for both disabled and enabled
    configurations.

    Arguments:
    ext -- the extension to be tested
    version -- the minimum required version
    stage -- the shader stage to test
    path -- the path to put the test in, without the test name
    extra_name -- An extra string to put in the test name.
    extra_extensions -- Any extra extensions requires.

    """
    for test, template in [('enabled', ENABLED_TEMPLATE),
                           ('disabled-defined', DISABLED_TEMPLATE),
                           ('disabled-undefined', UNDEFINED_TEMPLATE)]:
        name = os.path.join(path, '{}-{}.{}'.format(test, extra_name, stage))

        # Open in bytes mode to avoid weirdness in python 2/3 compatibility
        with open(name, 'wb') as f:
            f.write(template.render(
                version=version,
                extension=ext,
                extra_extensions=extra_extensions))
        print(name)


def main():
    """Main function."""
    for ext, ver in EXTENSIONS:
        # Lower ext in the path name, but keep it capitalized for the generated
        # tests
        path = os.path.join('spec', ext[3:].lower(), 'preprocessor')
        utils.safe_makedirs(path)

        for stage in ['vert', 'frag', 'geom', 'tesc', 'tese', 'comp']:
            # Calculate the minimum version for the stage, with extensions.
            # This makes the maximum number of tests run on the widest swath of
            # hardware.
            version, extra_ext = glsl.MinVersion.for_stage_with_ext(stage, ver)
            if extra_ext is not None:
                extra_extensions = [extra_ext]
            else:
                extra_extensions = []

            if not version.is_es:
                # if the actual version is less than 140 make a compat test and
                # a core test, otherwise just make a core test
                if version < 140:
                    _gen_tests(ext, version, stage, path, 'compat',
                               extra_extensions)
                    _gen_tests(ext, glsl.Version('140'), stage, path, 'core',
                               extra_extensions)
                else:
                    _gen_tests(ext, version, stage, path, 'core',
                               extra_extensions)
            else:
                _gen_tests(ext, version, stage, path, 'es', extra_extensions)


if __name__ == '__main__':
    main()
