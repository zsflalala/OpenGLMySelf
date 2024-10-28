#version 460 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 _Normal;
} gs_in[];

uniform float time;
out vec3 Normal;

vec4 explode(vec4 position, vec3 Normal)
{
    float magnitude = 2.0;
    vec3 direction = Normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main() {
    vec3 normal = GetNormal();
    
    gl_Position = explode(gl_in[0].gl_Position, normal);
    Normal = gs_in[0]._Normal;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    Normal = gs_in[1]._Normal;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    Normal = gs_in[2]._Normal;
    EmitVertex();
    EndPrimitive();
}