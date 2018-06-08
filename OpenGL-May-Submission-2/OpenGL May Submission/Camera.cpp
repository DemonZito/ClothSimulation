#include "Camera.h"

CCamera::CCamera()
{
	m_cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	m_fYaw = -171.5f;
	m_fPitch = -30.5f;
}


CCamera::~CCamera()
{
}

void CCamera::GetUniformLocation(GLuint _shaders)
{
}

void CCamera::SendDataToShaders(std::unordered_map<ShaderType, GLuint> _shaders)
{
	float aspectRatio = static_cast<float>(WINDOW_WIDTH / WINDOW_HEIGHT);
	glm::mat4 Ortho = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, 0.1f, 100.0f);

	m_viewMatrix = glm::lookAtRH(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
	m_perspectiveMatrix = glm::perspectiveRH(glm::radians(30.0f), aspectRatio, 1.0f, 10000.0f);

	// send camera data to all shaders

	for (auto it = _shaders.begin(); it != _shaders.end(); it++)
	{
		if (it->first != SPRITE)
		{
			glUseProgram(it->second);

			glUniformMatrix4fv(glGetUniformLocation(it->second, "gView"), 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
			glUniformMatrix4fv(glGetUniformLocation(it->second, "gPerspective"), 1, GL_FALSE, glm::value_ptr(m_perspectiveMatrix));
		}
		
	}
}

void CCamera::MoveForward(float _fSpeed)
{
	m_cameraPos += m_cameraFront * _fSpeed;
}

void CCamera::MoveLeft(float _fSpeed)
{
	m_cameraPos -= glm::normalize(glm::cross(m_cameraFront, m_cameraUp)) * _fSpeed;
}

void CCamera::SetCameraFront(glm::vec3 _front)
{
	m_cameraFront = _front;
}

void CCamera::SetPosition(glm::vec3 _position)
{
	m_cameraPos += _position;
}

glm::vec3 CCamera::GetPosition()
{
	return m_cameraPos;
}

glm::vec3 CCamera::GetCameraUp()
{
	return m_cameraUp;
}

float CCamera::GetCameraSpeed()
{
	return m_fCameraSpeed;
}

void CCamera::MoveCamera(glm::vec2 _mousePos)
{
	GLfloat fXOffset = _mousePos.x - 400; //+vexoffsetgives clockwise rotation
	GLfloat fYOffset = _mousePos.y - 400; //+veyoffsetgives clockwise rotation

										 // Set camera sensitivity
	GLfloat fCamSensitivity = 0.025f;

	// Multiply offset by sensitivity
	fXOffset *= fCamSensitivity;
	fYOffset *= fCamSensitivity;

	m_fYaw -= fXOffset; //clockwise rotation decreases angle since
	m_fPitch -= fYOffset; //CCW is +verotation
						  // Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (m_fPitch > 89.0f)
		m_fPitch = 89.0f;
	if (m_fPitch < -89.0f)
		m_fPitch = -89.0f;

	// Update front of the camera
	glm::vec3 FrontVector(-cos(glm::radians(m_fPitch))*sin(glm::radians(m_fYaw)),
		sin(glm::radians(m_fPitch)),
		-cos(glm::radians(m_fPitch)) * cos(glm::radians(m_fYaw)));

	m_cameraFront = glm::normalize(FrontVector);
}

glm::vec3 CCamera::GetFront()
{
	return m_cameraFront;
}

glm::mat4 CCamera::GetViewMatrix()
{
	return m_viewMatrix;
}

glm::mat4 CCamera::GetPerspectiveMatrix()
{
	return m_perspectiveMatrix;
}