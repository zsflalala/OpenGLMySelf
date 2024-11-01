#version 460 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gScreen;

void main()
{             
    //FragColor = texture(gScreen, TexCoords);
    FragColor = texture(gScreen, gl_FragCoord.xy / vec2(800f, 600f));
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}