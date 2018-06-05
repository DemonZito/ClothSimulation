#version 420 core

in vec3 a_position;
in vec3 a_color;
in vec2 a_texCord;
in vec3 a_normal;

 uniform mat4 gScale;
 uniform mat4 gRotate;
 uniform mat4 gTranslate;
 uniform mat4 gView;
 uniform mat4 gPerspective;

out vec3 outNormal;
out vec3 outColor;
out vec2 textureCord;
out vec3 FragPos;

void main(void)
{
	gl_Position = gPerspective * gView * gTranslate * gRotate * gScale * vec4(a_position, 1.0f);
	textureCord = a_texCord;
	outColor = a_color;
	FragPos = gl_Position.xyz;
	outNormal = a_normal;
}