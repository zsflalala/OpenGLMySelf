#version 460 core

out vec4 oFragColor;

in vec3 voNormal;
in vec2 voTexcoord;
in vec3 voFragPos;

uniform vec3 viewPos;
uniform sampler2D dragonTexture;

void main()
{
    vec3 lightPos   = vec3(1.2f, 1.0f, 2.0f);
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

    vec3 objcetColor = texture(dragonTexture, voTexcoord).rgb;

    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightDir = normalize(lightPos - voFragPos);         
    float diff = max(dot(voNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - voFragPos);           
    vec3 reflectDir = reflect(-lightDir, voNormal);          
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); 
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objcetColor;

    oFragColor = vec4(result, 1.0);
}