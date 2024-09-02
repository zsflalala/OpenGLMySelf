#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos; 
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform sampler2D BoxTexture;
uniform sampler2D SmileTexture;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
    
    vec3 PhongResult = (ambient + diffuse + specular) * objectColor;
    vec4 TexColor = mix(texture(BoxTexture, TexCoord), texture(SmileTexture, vec2(1.0 - TexCoord.x, TexCoord.y)), 0.2);
    vec3 result = PhongResult * TexColor.rgb;
    FragColor = vec4(result, TexColor.a);
}