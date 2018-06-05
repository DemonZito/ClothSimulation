#version 420 core

in vec3 outColor;
in vec2 textureCord;
in vec3 FragPos;
in vec3 outNormal;

void main(void)
{
	//gl_FragColor = vec4(outColor, 1.0f);

	float ambientIntensity = 0.8f;


	//Do lighting stuff
	vec3 lightPos = vec3(0, 0, 30);
	vec3 lightDir = normalize(lightPos);
	//vec3 lightDir = normalize(lightPos - FragPos);
	vec3 norm = normalize(outNormal);

	float diff = max(dot(lightDir, norm), 0.0);

	//Toon shader stuff
	//if (diff > 0.75f)
	//	diff = 1;
	//else if (diff > 0.5f)
	//	diff = 0.5f;
	//else if (diff > 0.25f)
	//	diff = 0.25f;
	//else
	//	diff = 0;

	vec3 ambient = ambientIntensity * outColor;
	vec3 diffuse = diff * outColor;

	//Apply lighting stuff
	gl_FragColor = vec4(ambient + diffuse, 1.0);
}