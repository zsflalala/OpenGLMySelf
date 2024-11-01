#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gScreen;
uniform sampler2D gScreen2;

void main()
{             
    //FragColor = texture(gScreen, TexCoords);
    FragColor = texture(gScreen, gl_FragCoord.xy / vec2(800f, 600f)) + texture(gScreen2, gl_FragCoord.xy / vec2(800f, 600f)) * 0.5;
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}