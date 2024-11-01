#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 _Normal;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out._Normal = transpose(inverse(mat3(model))) * aNormal;
    gl_Position = projection * view * model * vec4(aPos, 1.0); 
}