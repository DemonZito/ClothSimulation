#pragma once

#include "GlobalIncludes.h"

class CCamera
{
public:
	CCamera();
	~CCamera();
	
	void GetUniformLocation(GLuint _shaders);
	void SendDataToShaders(std::unordered_map<ShaderType, GLuint> _shaders);
	void MoveForward(float _fSpeed);
	void MoveLeft(float _fSpeed);
	void SetCameraFront(glm::vec3 _front);
	void SetPosition(glm::vec3 _position);
	glm::vec3 GetFront();
	glm::vec3 GetPosition();
	glm::vec3 GetCameraUp();
	float GetCameraSpeed();
	void MoveCamera(glm::vec2 _mousePos);

private:
	glm::vec3 m_cameraPos;
	glm::vec3 m_cameraFront;
	glm::vec3 m_cameraUp;
	float m_fYaw;
	float m_fPitch;
	float m_fCameraSpeed = 1.0f;
};