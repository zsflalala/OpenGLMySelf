#version 460 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT 
{
    vec3 _Normal;
} gs_in[];

const float MAGNITUDE = 0.2;
uniform mat4 projection;
out vec3 Normal;

void GenerateLine(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    Normal = gs_in[index]._Normal;
    gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index]._Normal, 0.0) * MAGNITUDE);
    Normal = gs_in[index]._Normal + 0.3;
    EmitVertex();
    EndPrimitive();
}

void main() {
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}