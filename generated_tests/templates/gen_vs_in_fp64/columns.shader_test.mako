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


  def cols(in_type):
      if 'mat' in in_type:
          if 'x' in in_type:
              return int(in_type[-3:][:1])
          else:
              return int(in_type[-1:])
      else:
          return 1


  def rows(in_type):
      if 'vec' in in_type or 'mat' in in_type:
          return int(in_type[-1:])
      else:
          return 1

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

uniform ${mat} expected;

in ${mat} value;
in vec3 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
  gl_Position = vec4(piglit_vertex, 1.0);
  % for idx, column in enumerate(columns):
    % if column == 1:
      if (value[${idx}] != expected[${idx}]) {
        fs_color = RED;
        return;
      }
      ## XXX: should we break here?
    % endif
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
piglit_vertex/vec3/3\
  % for i in range(cols(mat)):
   value/${mat}/${rows(mat)}${'/{}'.format(i) if cols(mat) > 1 else ''}\
  % endfor

% for d in range(len(dvalues)):
  % for vertex in ('-1.0 -1.0  0.0', ' 1.0 -1.0  0.0', ' 1.0  1.0  0.0', '-1.0  1.0  0.0'):
${vertex} \
    % for i in range(cols(mat)):
      % for j in range(rows(mat)):
${dvalues[(d + i * rows(mat) + j) % len(dvalues)]}  \
      % endfor
  \
    % endfor

  % endfor
% endfor

[test]
% for d in range(len(dvalues)):

  uniform ${mat} expected\
  % for i in range(cols(mat)):
    % for j in range(rows(mat)):
     ${dvalues[(d + i * rows(mat) + j) % len(dvalues)]}\
    % endfor
  % endfor

  clear color 0.0 0.0 1.0 0.0
  clear
  draw arrays GL_TRIANGLE_FAN ${d * 4} 4
  probe all rgba 0.0 1.0 0.0 1.0
% endfor
