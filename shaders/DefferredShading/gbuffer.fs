#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;

in vec3 Normal;
in vec3 FragPos;

void main()
{    
    gPosition = FragPos;
    gNormal = normalize(Normal);
}