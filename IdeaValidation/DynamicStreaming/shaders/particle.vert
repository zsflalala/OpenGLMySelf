#version 460 core

in layout(location = 0) vec3 iPos;

void main()
{
    gl_Position = vec4(iPos.xyz, 1.0);
}