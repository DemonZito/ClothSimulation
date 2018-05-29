#version 420 core

in vec3 outColor;
in vec2 textureCord;

void main(void)
{
	gl_FragColor = vec4(outColor, 1.0f);
}