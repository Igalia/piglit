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
GL_MAX_VERTEX_ATTRIBS >= ${num_vs_in}

[vertex shader]
#version ${glsl_int}
% if ver == 'GL_ARB_vertex_attrib_64bit':
  #extension GL_ARB_gpu_shader_fp64 : require
  #extension GL_ARB_vertex_attrib_64bit : require
% endif

% for idx, in_type in enumerate(in_types):
  uniform ${in_type} expected${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor
% for idx, in_type in enumerate(in_types):
  in ${in_type} value${idx}${'[{}]'.format(arrays[idx]) if arrays[idx] - 1 else ''};
% endfor
in vec3 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    gl_Position = vec4(piglit_vertex, 1.0);
    % for idx, in_type in enumerate(in_types):
        if (value${idx} != expected${idx}) {
            fs_color = RED;
            return;
        }
    ## XXX: should this have a break?
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
% for idx, in_type in enumerate(in_types):
  % if idx == position_order - 1:
    piglit_vertex/vec3/3 \
  % endif
  % for i in range(arrays[idx]):
    % for j in range(cols(in_type)):
    value${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}/${in_type}/${rows(in_type)}${'/{}'.format(j) if cols(in_type) > 1 else ''} \
    % endfor
  % endfor
% endfor
% if position_order > len(in_types):
  piglit_vertex/vec3/3\
% endif

% for d in range(len(dvalues)):
  % for vertex in ('-1.0 -1.0  0.0', ' 1.0 -1.0  0.0', ' 1.0  1.0  0.0', '-1.0  1.0  0.0'):
    % for idx, in_type in enumerate(in_types):
      % if idx == position_order - 1:
        ${vertex}   \
      % endif
      % for i in range(arrays[idx]):
        % for j in range(cols(in_type)):
          % for k in range(rows(in_type)):
            ${dvalues[(d + (i * cols(in_type) + j) * rows(in_type) + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + (i * cols(in_type) + j) * rows(in_type) + k) % len(hvalues)]}  \
          % endfor
         \
        % endfor
      % endfor
    % endfor
    % if position_order > len(in_types):
      ${vertex}\
    % endif

  % endfor
% endfor

[test]
% for d in range(len(dvalues)):

  % for idx, in_type in enumerate(in_types):
    % for i in range(arrays[idx]):
      uniform ${in_type} expected${idx}${'[{}]'.format(i) if arrays[idx] > 1 else ''}\
      % for j in range(cols(in_type)):
        % for k in range(rows(in_type)):
         ${dvalues[(d + (i * cols(in_type) + j) * rows(in_type) + k) % len(dvalues)] if in_type.startswith('d') else hvalues[(d + (i * cols(in_type) + j) * rows(in_type) + k) % len(hvalues)]}\
        % endfor
      % endfor

    % endfor
  % endfor
  clear color 0.0 0.0 1.0 0.0
  clear
  draw arrays GL_TRIANGLE_FAN ${d * 4} 4
  probe all rgba 0.0 1.0 0.0 1.0
% endfor
