#version 460 core

out vec4 oFragColor;

uniform vec3 uColor;

void main()
{
    oFragColor = vec4(uColor.rgb, 1.0);
}