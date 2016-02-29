#version 150
#extension GL_ARB_gpu_shader_fp64 : require

uniform ${from_type} from;
uniform ${to_type} to;

in vec4 piglit_vertex;
out vec4 fs_color;

#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    gl_Position = piglit_vertex;
    ${to_type} converted = ${converted_from};
    bool match = converted == to;
    fs_color = match ? GREEN : RED;
}
