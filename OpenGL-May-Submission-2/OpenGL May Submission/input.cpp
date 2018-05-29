#include "input.h"

void Input::Initialize(GLFWwindow * _window)
{
	m_pWindow = _window;
}

Input& Input::Instance()
{
	static Input instance; // Guaranteed to be destroyed.
						  // Instantiated on first use.
	return instance;
}


bool Input::GetKeyDown(int _key)
{
	return glfwGetKey(m_pWindow, _key) == GLFW_PRESS;
}

bool Input::GetKeyUp(int _key)
{
	return glfwGetKey(m_pWindow, _key) == GLFW_RELEASE;
}

void Input::Clear()
{

}

glm::vec2 Input::MousePosition()
{
	double mouseX, mouseY;
	glfwGetCursorPos(m_pWindow, &mouseX, &mouseY);
	return glm::vec2(mouseX, mouseY);
}