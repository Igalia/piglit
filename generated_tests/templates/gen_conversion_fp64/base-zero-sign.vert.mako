${preprocessor}\

uniform ${from_type} from;
uniform ${to_type} to;

in vec4 piglit_vertex;
out vec4 fs_color;

#define ONE 1.0
#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    gl_Position = piglit_vertex;
    ${to_type} pre_converted = ${converted_from};
    ${to_type} converted = ONE / pre_converted;
    bool match = converted == to;
    fs_color = match ? GREEN : RED;
}
