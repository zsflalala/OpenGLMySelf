#version 460 core

in layout (location = 0) vec3 iPos;
in layout (location = 1) vec3 iNor;
in layout (location = 2) vec2 iTex;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

out vec3 voNormal;
out vec2 voTexcoord;
out vec3 voFragPos;

void main()
{
    voFragPos   = vec3(model * vec4(iPos, 1.0));
    voNormal    = normalize(iNor);
    voTexcoord  = iTex;
    gl_Position = projection * view * model * vec4(iPos, 1.0);
}