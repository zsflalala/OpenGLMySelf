#version 460 core

in layout (location = 0) vec3 iPos;
in layout (location = 1) vec3 iNor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 vColor;

void main()
{
    vNormal = normalize(iNor);
    vColor = vNormal.rgb;
    gl_Position = projection * view * model * vec4(iPos, 1.0);
}