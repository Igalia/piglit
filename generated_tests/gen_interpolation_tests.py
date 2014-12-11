# coding=utf-8
#
# Copyright © 2011, 2014 Intel Corporation
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

"""Generate Interpopation tests.

Correct interpolation of vertex shader outputs depends on (a)
whether an interpolation qualifier is present in the shader source,
and if so which qualifier is used, (b) if no interpolation qualifier
is present, whether the output is a user-defined variable or a
built-in color, and (c) if the output is a built-in color, the
setting of the ShadeModel() setting.  In addition, we would like to
test correct interpolation under various clipping scenarios.

To verify that all the combinations of these possibilities work
correctly, this script generates a shader_runner test to check
proper interpolation for every combination of the following
variables:

- which interpolation qualifier is used ("flat", "noperspective",
  "smooth", or no qualifier)

- which variable is used (gl_FrontColor, gl_BackColor,
  gl_FrontSecondaryColor, gl_BackSecondaryColor, or a non-built-in
  variable)

- the setting of ShadeModel() (either GL_SMOOTH or GL_FLAT)

- whether the triangle in question is clipped using gl_ClipVertex,
  clipped using gl_ClipDistance, clipped against the fixed viewing
  volume, or unclipped.

The tests operate by drawing a triangle with a different value of
the variable at each vertex, and then probing within the interior of
the triangle to verify that interpolation was performed correctly.
The triangle is drawn in a frustum projection, with a different z
value for each vertex, so that there will be a detectable difference
in behavior between noperspective and smooth interpolation.

When testing clipping, we clip off the frontmost corner of the
triangle; this ensures that the proportion of the triangle's screen
real estate that is clipped is significantly larger than the
proportion of the triangle's 3D coordinate space that is clipped.
So if the GL implementation doesn't perform perspective-correct
interpolation generating clipped vertices, we will notice.

This program outputs, to stdout, the name of each file it generates.

"""

from __future__ import print_function
import os

from templates import template_file

TEMPLATE = template_file(os.path.basename(os.path.splitext(__file__)[0]),
                         'template.shader_test.mako')


class Test(object):
    def __init__(self, interpolation_qualifier, variable, shade_model,
                 clipping):
        """Get ready to generate a test using the given settings.

        interpolation_qualifier is a string representing the desired
        interpolation qualifier that should appear in GLSL code
        ('flat', 'noperspective', or 'smooth'), or None if no
        qualifier should appear.

        variable is the name of the variable on which to test
        interpolation.  If the name begins with 'gl_', it should be
        one of the four vertex shader variables that are allowed to be
        redeclared with an interpolation qualifier (see GLSL 1.30
        section 4.3.7 "Interpolation").  Namely: gl_FrontColor,
        gl_BackColor, gl_FrontSecondaryColor, or
        gl_BackSecondaryColor.

        shade_model is which shade model the GL state should be put in
        using the glShadeModel() command--either 'smooth' or 'flat'.

        clipping is the variety of clipping which should be tested:
        either 'fixed' to test a triangle that extends beyond the
        fixed view volume (we test clipping against the "near" plane),
        'vertex' to test a triangle which has one corner clipped using
        gl_ClipVertex, or 'distance' to test a triangle which has one
        corner clipped using gl_ClipDistance.
        """
        self.interpolation_qualifier = interpolation_qualifier
        self.vs_variable = variable
        self.shade_model = shade_model
        self.clipping = clipping

        # When colors are mapped into the fragment shader, the string
        # 'Front' or 'Back' is dropped from the variable name, since
        # e.g. gl_Color is mapped to gl_FrontColor for front-facing
        # triangles, and gl_BackColor for back-facing triangles.
        self.fs_variable = variable.replace('Front', '').replace('Back', '')

        # True if we are testing a BackColor, so we'll need to draw a
        # back-facing triangle.
        self.backfacing = variable.find('Back') != -1

        # True if we are testing a built-in color variable, False if
        # we are testing a generic vertex shader output.
        self.builtin_variable = variable[:3] == 'gl_'

        # Determine whether the test requires GLSL 1.30.  If it does,
        # use "in" and "out" to qualify shader inputs and outputs.
        # Otherwise use the old keywords "attribute" and "varying".
        # shader_runner will insert a #version directive based on
        # glsl_version.
        if self.interpolation_qualifier or self.clipping == 'distance':
            self.glsl_version = '1.30'
            self.vs_input = 'in'
            self.vs_output = 'out'
            self.fs_input = 'in'
        else:
            self.glsl_version = '1.10'
            self.vs_input = 'attribute'
            self.vs_output = 'varying'
            self.fs_input = 'varying'

        # Determine the location of the near and far planes for the
        # frustum projection.  The triangle fits between z coordinates
        # -1 and -3; we use 1.75 as the near plane when we want to
        # force clipping.
        if self.clipping == 'fixed':
            self.frustum_near = 1.75
        else:
            self.frustum_near = 1.0
        self.frustum_far = 3.0

        # Determine whether we expect the GL implementation to use
        # flatshading, non-perspective interpolation, or perspective
        # interpolation.
        if self.interpolation_qualifier:
            # According to GLSL 1.30 section 4.3.7 ("Interpolation"),
            # "When an interpolation qualifier is used, it overrides
            # settings established through the OpenGL API."
            self.expected_behavior = self.interpolation_qualifier
        elif self.builtin_variable:
            # According to GL 3.0 section 2.19.7 ("Flatshading"), "If
            # a vertex shader is active, the flat shading control
            # applies to the built-in varying variables gl FrontColor,
            # gl BackColor, gl FrontSecondaryColor and gl
            # BackSecondaryColor.  Non-color varying variables can be
            # specified as being flat-shaded via the flat qualifier,
            # as described in section 4.3.6 of the OpenGL Shading
            # Language Specification."
            self.expected_behavior = self.shade_model
        else:
            # The specs do not explicitly state how non-built-in
            # variables are to be interpolated in the case where no
            # interpolation qualifier is used.  However, it seems to
            # be heavily implied by the text of GL 3.0 section 2.19.6
            # ("Flatshading"--see above) that smooth
            # (perspective-correct) interpolation is intended,
            # regardless of the setting of glShadeModel().
            self.expected_behavior = 'smooth'

    def filename(self):
        return os.path.join(
            'spec', 'glsl-{0}'.format(self.glsl_version),
            'execution', 'interpolation',
            'interpolation-{0}-{1}-{2}-{3}.shader_test'.format(
                self.interpolation_qualifier or 'none', self.vs_variable,
                self.shade_model, self.clipping or 'none'))

    def vertex_data(self):
        table = ['vertex/float/3  input_data/float/4',
                 '-1.0 -1.0 -1.0  1.0 0.0 0.0 1.0',
                 ' 0.0  2.0 -2.0  0.0 1.0 0.0 1.0',
                 ' 3.0 -3.0 -3.0  0.0 0.0 1.0 1.0']
        if not self.backfacing:
            # The vertices above are ordered such that the front of
            # the triangle faces away from the viewer.  If we are
            # trying to render the front face, then swap the first two
            # vertices.  This shows us the front face of the triangle
            # without changing the provoking vertex (which is the
            # third vertex).
            table = [table[0], table[2], table[1], table[3]]
        return table

    def probe_data(self):
        # Loop over possible barycentric coordinates with a spacing of
        # 1/num_subdivisions.  Skip points on the triangle edges and
        # corners so that rounding does not cause us to accidentally
        # probe a pixel that's outside the triangle.
        num_subdivisions = 6
        for i in xrange(1, num_subdivisions - 1):
            for j in xrange(1, num_subdivisions - i):
                # Compute 3D barycentric coordinates--these will be
                # used to compute the expected interpolated values
                # when using smooth (perspective-correct)
                # interpolation.  The vertex associated with b3d_0=1.0
                # is colored red, the vertex associated with b3d_1=1.0
                # is colored green, and the vertex associated with
                # b3d_2=1.0 is colored blue.
                b3d_0 = float(num_subdivisions - i - j)/num_subdivisions
                b3d_1 = float(i)/num_subdivisions
                b3d_2 = float(j)/num_subdivisions
                # Compute 3D coordinates based on those barycentric
                # coordinates.  These will be used, among other
                # things, to determine whether this part of the
                # triangle is clipped.
                x3d = -b3d_0 + 3.0*b3d_2
                y3d = -b3d_0 + 2.0*b3d_1 - 3.0*b3d_2
                z3d = -b3d_0 - 2.0*b3d_1 - 3.0*b3d_2
                # Use perspective division to compute 2D screen
                # coordinates.  These will be used with "relative
                # probe rgba", which treats the lower left corner of
                # the screen as (0, 0) and the upper right is (1, 1).
                x2d = (-x3d/z3d + 1.0) / 2.0
                y2d = (-y3d/z3d + 1.0) / 2.0
                # Finally, compute a second set of barycentric
                # coordinates based on the 2D screen
                # coordinates--these will be used to compute the
                # expected interpolated values when using
                # noperspective (screen-coordinate) interpolation.
                b2d_0 = 1.0 - x2d - 0.5*y2d
                b2d_1 = y2d
                b2d_2 = x2d - 0.5*y2d

                if self.clipping and -z3d < 1.75:
                    # Points whose -z coordinate is less than 1.75
                    # should be clipped.
                    yield x2d, y2d, 0.0, 0.0, 0.0, 0.0
                elif self.expected_behavior == 'flat':
                    # When flatshading, all points on the triangle
                    # should inherit the color of the third vertex,
                    # which is blue.
                    yield x2d, y2d, 0.0, 0.0, 1.0, 1.0
                elif self.expected_behavior == 'noperspective':
                    # Since the 3 triangle vertices are red, green,
                    # and blue, the interpolated color channels should
                    # be exactly equal to the barycentric coordinates.
                    # For "noperspective" shading, we use the
                    # barycentric coordinates that we computed based
                    # on 2D screen position.
                    yield x2d, y2d, b2d_0, b2d_1, b2d_2, 1.0
                else:
                    # For "smooth" (perspective correct) shading, we
                    # use the barycentric coordinates that we used to
                    # compute the 3D position.
                    assert self.expected_behavior == 'smooth'
                    yield x2d, y2d, b3d_0, b3d_1, b3d_2, 1.0

    def generate(self):
        filename = self.filename()
        dirname = os.path.dirname(filename)

        if not os.path.exists(dirname):
            try:
                os.makedirs(dirname)
            except OSError as e:
                if e.errno == 17:  # file exists
                    pass
                raise

        with open(filename, 'w') as f:
            f.write(TEMPLATE.render(args=self))


def all_tests():
    for interpolation_qualifier in ['flat', 'smooth', 'noperspective', '']:
        for variable in ['gl_FrontColor', 'gl_BackColor',
                         'gl_FrontSecondaryColor', 'gl_BackSecondaryColor',
                         'other']:
            for shade_model in ['smooth', 'flat']:
                for clipping in ['vertex', 'distance', 'fixed', '']:
                    yield Test(interpolation_qualifier, variable, shade_model,
                               clipping)


def main():
    for test in all_tests():
        test.generate()
        print(test.filename())


if __name__ == '__main__':
    main()
