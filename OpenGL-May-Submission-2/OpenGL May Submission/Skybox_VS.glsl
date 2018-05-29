#version 430 core

in vec3 a_position;

 uniform mat4 gScale;
 uniform mat4 gRotate;
 uniform mat4 gTranslate;
 uniform mat4 gView;
 uniform mat4 gPerspective;

out vec3 TexCoords;

void main(void)
{
	mat4 WVP = gPerspective * gView * gTranslate * gRotate * gScale;
	WVP[3] = vec4(0,0,0,1);

    gl_Position = WVP * vec4(a_position, 1.0f);
    gl_Position = gl_Position.xyww;

	TexCoords = a_position;
}