#ifndef _SHADER_HELPER__
#define _SHADER_HELPER__

#include <glad\glad.h>
#include <soil\SOIL.h>

#include <cstring>
#include <string>

class ShaderHelper {
private:
	//static const std::string TAG;

public:
	ShaderHelper();
	static std::string readShaderFileFromResource(const char* pFileName);

	static GLuint compileVertexShader(const char* shaderCode);
	static GLuint compileFragmentShader(const char* shaderCode);
	static GLuint compileGeometryShader(const char* shaderCode);

	static GLuint compileTessControlShader(const char* shaderCode);
	static GLuint compileTessEvalShader(const char* shaderCode);


	static GLuint compileShader(GLenum ShaderType, const char* shaderCode);

	static GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId);
	static GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId, GLuint geometryShaderId);
	static GLuint linkProgram(GLuint vertexShaderId, GLuint fragmentShaderId, GLuint tessControlId, GLuint tessEvalId);

	static GLint validateProgram(GLuint programObjectId);

	static void compileAndLinkShaders(std::string vertex_shader, std::string fragment_shader, GLuint& program);
	static void compileAndLinkShaders(std::string vertex_shader, std::string fragment_shader, std::string geometry_shader, GLuint& program);

	static void compileAndLinkShaders(std::string vertex_shader, std::string fragment_shader, std::string tesscontrol_shader, std::string tesseval_shader, GLuint& program);
};

//std::string TAG = "ShaderHelper";
#endif