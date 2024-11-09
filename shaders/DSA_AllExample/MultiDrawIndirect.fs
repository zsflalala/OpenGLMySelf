#version 460 core

out vec4 oFragColor;

in vec3 voNormal;

void main()
{
    vec3 Color = voNormal * 0.5 + 0.5;
    oFragColor = vec4(Color, 1.0);
}