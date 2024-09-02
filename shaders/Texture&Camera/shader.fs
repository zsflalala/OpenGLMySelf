#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D BoxTexture;
uniform sampler2D SmileTexture;


void main()
{
    FragColor = mix(texture(BoxTexture, TexCoord), texture(SmileTexture, vec2(1.0 - TexCoord.x, TexCoord.y)), 0.2);
}