## coding=utf-8
<%inherit file="shader_base.mako"/>\
<%block name="global_variables"/>\

in vec3 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    gl_Position = vec4(piglit_vertex, 1.0);
<%block name="main"/>\
    fs_color = GREEN;
}
