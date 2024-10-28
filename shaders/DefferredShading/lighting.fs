#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;

uniform vec3 viewPos;

void main()
{             
    // Retrieve data from gbuffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 lighting = vec3(0.1, 0.1, 0.1);
    vec3 lightPos = vec3(0.5, 1.0, 2.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Then calculate lighting as usual
    vec3 viewDir  = normalize(viewPos - FragPos);
    // Diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * lightColor;
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = lightColor * spec;
    lighting += diffuse + specular;  

    FragColor = vec4(lighting, 1.0);
}