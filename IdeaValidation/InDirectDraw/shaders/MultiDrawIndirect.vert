#version 460 core

in layout (location = 0) vec3 iPos;
in layout (location = 1) vec3 iNormal;
in layout (location = 2) vec2 iTexCoords;

uniform mat4 uViewProjection;

layout(binding = 0, std430) readonly buffer ModelsSSBO
{
    mat4 uModels[];
};

layout(binding = 1, std430) readonly buffer ColorsSSBO
{
    vec4 uColors[];
};

out vec3 vNormal;
out vec3 vColor;

void main()
{
    vNormal = iNormal;
    vColor = uColors[gl_DrawID].rgb;
    gl_Position = uViewProjection * uModels[gl_DrawID] * vec4(iPos, 1.0);
}