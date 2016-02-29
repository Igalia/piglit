## encoding=utf-8
## Copyright © 2016 Intel Corporation
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.
<%! 
  import re

  _IS_NVVP3_FAIL = re.compile(r'(TEX|TX[BLP])')

  def dedent(text):
      return '\n'.join(l.lstrip() for l in text.splitlines())

  def newlines(text):
      return '\n'.join(l for l in text.splitlines() if l.strip())
%>

<%def name="require(target)" filter="dedent,newlines">
  % if target.startswith("SHADOW"):
    # REQUIRE GL_ARB_fragment_program_shadow
    OPTION    ARB_fragment_program_shadow;
    ## Remove "SHADOW" portion of string
    <% target = target[len("SHADOW"):] %>
  % endif

  % if target == "RECT":
    # REQUIRE GL_ARB_texture_rectangle
  % elif target == "CUBE":
    # REQUIRE GL_ARB_texture_cube_map
  % elif target == "3D":
    # REQUIRE GL_EXT_texture3D
  % endif
</%def>

<%def name="nvvp3_fail(inst)" filter="dedent">
  % if not _IS_NVVP3_FAIL.match(inst):
    # FAIL - ${inst} not supported by GL_NV_vertex_program3
  % endif
</%def>
