#version 460 core
in layout (location = 0) vec3 iPos;
in layout (location = 1) vec3 iNor;

layout(binding = 0, std430) readonly buffer ModelsSSBO
{
    mat4 uModels[];
};

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 voNormal;

void main()
{
    voNormal = iNor;
    gl_Position = projection * view * uModels[gl_DrawID] * vec4(iPos, 1.0);
}