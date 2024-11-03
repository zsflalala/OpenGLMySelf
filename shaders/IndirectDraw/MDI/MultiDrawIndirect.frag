#version 460 core

out vec4 oFragColor;

in vec3 vNormal;
in vec3 vColor;

void main()
{
    vec3 Color = normalize(vNormal) * 0.5 + 0.5;
    //vec3 Color = vColor.rgb;
    oFragColor = vec4(Color, 1.0);
}