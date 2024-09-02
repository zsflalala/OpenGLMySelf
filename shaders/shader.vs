#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 crntPos;
out vec3 Normal;
out vec4 fragPosLight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 lightProjection;

void main()
{
	crntPos = vec3(model * vec4(aPos, 1.0f));
	Normal = aNormal;
	fragPosLight = lightProjection * vec4(crntPos, 1.0f);
	
	gl_Position = projection * view * model * vec4(crntPos, 1.0);
}