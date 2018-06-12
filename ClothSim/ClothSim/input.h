#pragma once

#include <map>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class Input
{
public:
 //void Initialize(GLFWwindow* _window);
	static Input& Instance();

	void Initialize(GLFWwindow * _window);
							  
	bool GetKeyDown(int _key);
	bool GetKeyUp(int _key);

	//Returns current mouse position in pixels
	glm::vec2 MousePosition();

	//Clears key down/up data for a frame
	void Clear();

	Input(const Input&) = delete;
	void operator=(Input const&) = delete;


private:
	Input() {}

	GLFWwindow* m_pWindow;
};