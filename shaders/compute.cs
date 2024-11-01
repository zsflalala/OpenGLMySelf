#version 460 core
layout(local_size_x = 8, local_size_y = 4, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D gScreen;
layout(rgba32f, binding = 1) uniform image2D gScreenPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform vec3 viewPos;

void main()
{     
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);

    // Retrieve data from gbuffer
    vec3 FragPos = texelFetch(gPosition, pixel_coords, 0).rgb;
    vec3 Normal = texelFetch(gNormal, pixel_coords, 0).rgb;
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

    imageStore(gScreen, pixel_coords, vec4(lighting, 1.0));
    imageStore(gScreenPos, pixel_coords, vec4(FragPos, 1.0));
}