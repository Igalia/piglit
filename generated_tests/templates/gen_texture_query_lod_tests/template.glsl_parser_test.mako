/* [config]
% if execution_stage == 'fs':
 * expect_result: pass
% else:
 * expect_result: fail
% endif
 * glsl_version: ${version}
% if extensions:
 * require_extensions: ${" ".join(extensions)}
% endif
 * [end config]
 */

#version ${int(float(version) * 100)}
% for extension in extensions:
#extension ${extension} : enable
% endfor

uniform ${sampler_type} s;
% if execution_stage == 'fs':
varying ${coord_type} coord;
% else:
uniform ${coord_type} coord;
% endif

void main()
{
  % if execution_stage == 'fs':
    gl_FragColor.xy = textureQuery${lod}(s, coord);
  % else:
    gl_Position.xy = textureQuery${lod}(s, coord);
  % endif
}
