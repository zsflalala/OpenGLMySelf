#version 330 core
layout (location = 0) in vec3 iPos;

out vec3 TexCoords;

uniform mat4 skyBoxView;
uniform mat4 skyBoxProjection;

void main()
{
    TexCoords = iPos;
    vec4 pos = skyBoxProjection * skyBoxView * vec4(iPos, 1.0);
    gl_Position = pos.xyww;
}  