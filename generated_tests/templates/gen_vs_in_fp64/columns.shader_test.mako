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
  from six.moves import range

  def _version(ver):
      if ver == 'GL_ARB_vertex_attrib_64bit':
          glsl_version_int = '150'
      else:
          glsl_version_int = ver

      glsl_version = '{}.{}'.format(glsl_version_int[0], glsl_version_int[1:3])

      return (glsl_version, glsl_version_int)
%>
<% glsl, glsl_int = _version(ver) %>

[require]
GLSL >= ${glsl}
% if ver == 'GL_ARB_vertex_attrib_64bit':
  GL_ARB_gpu_shader_fp64
  ${ver}
% endif

[vertex shader]
#version ${glsl_int}
% if ver == 'GL_ARB_vertex_attrib_64bit':
#extension GL_ARB_gpu_shader_fp64 : require
#extension GL_ARB_vertex_attrib_64bit : require
% endif

% for vidx in range(4):
  uniform ${mat.name} expected${vidx};
% endfor

in ${mat.name} value;
in vec3 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
  gl_Position = vec4(piglit_vertex.x, piglit_vertex.y, 0.0, 1.0);
  % for vidx, vertex in enumerate(['-1.0', '0.0', '1.0', '2.0']):
    if (piglit_vertex.z == ${vertex}) {
    % for idx, column in enumerate(columns):
      % if column == 1:
        if (value[${idx}] != expected${vidx}[${idx}]) {
            fs_color = RED;
            return;
        }
      % endif
    % endfor
    }
  % endfor
  fs_color = GREEN;
}

[fragment shader]
#version 150

in vec4 fs_color;
out vec4 color;

void main()
{
  color = fs_color;
}

[vertex data]
piglit_vertex/float/vec3\
  % for i in range(mat.columns):
   value/double/${mat.name}${'/{}'.format(i) if mat.columns > 1 else ''}\
  % endfor

% for d in range(len(dvalues)):
  % for vidx, vertex in enumerate(['-1.0 -1.0  -1.0', ' 1.0 -1.0  0.0', ' 1.0  1.0  1.0', '-1.0  1.0  2.0']):
${vertex} \
    % for i in range(mat.columns):
      % for j in range(mat.rows):
${dvalues[(d + i * mat.rows + j + vidx) % len(dvalues)]}  \
      % endfor
  \
    % endfor

  % endfor
% endfor

[test]
% for d in range(len(dvalues)):

  % for vidx in range(4):
    uniform ${mat.name} expected${vidx}\
    % for i in range(mat.columns):
      % for j in range(mat.rows):
        ${dvalues[(d + i * mat.rows + j + vidx) % len(dvalues)]}\
      % endfor
    % endfor

  % endfor
  clear color 0.0 0.0 1.0 0.0
  clear
  draw arrays GL_TRIANGLE_FAN ${d * 4} 4
  probe all rgba 0.0 1.0 0.0 1.0
% endfor
