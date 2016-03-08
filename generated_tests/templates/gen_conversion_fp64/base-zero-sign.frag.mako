${preprocessor}\

uniform ${from_type} from;
uniform ${to_type} to;

out vec4 color;

#define ONE 1.0
#define RED vec4(1.0, 0.0, 0.0, 1.0)
#define GREEN vec4(0.0, 1.0, 0.0, 1.0)

void main()
{
    ${to_type} pre_converted = ${converted_from};
    ${to_type} converted = ONE / pre_converted;
    bool match = converted == to;
    color = match ? GREEN : RED;
}
