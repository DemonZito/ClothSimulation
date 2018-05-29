#version 420 core

in vec3 outColor;
in vec2 textureCord;

uniform sampler2D gSampler;

void main(void)
{
	gl_FragColor = texture(gSampler, textureCord.st) * vec4(outColor, 1.0f);
}