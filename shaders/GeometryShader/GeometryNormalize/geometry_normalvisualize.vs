#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT 
{
    vec3 _Normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    mat3 normalMatix = mat3(transpose(inverse(view * model)));
    vs_out._Normal = normalize(vec3(vec4(normalMatix * aNormal, 0.0)));
    gl_Position = view * model * vec4(aPos, 1.0); 
}