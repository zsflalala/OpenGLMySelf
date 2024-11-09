#version 460 core

out vec4 oFragColor;

in vec3 vNormal;
in vec3 vColor;

void main()
{
    vec3 Color = vNormal;
    oFragColor = vec4(Color, 1.0);
}