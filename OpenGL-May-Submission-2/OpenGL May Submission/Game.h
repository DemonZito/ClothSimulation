#pragma once

#include "GlobalIncludes.h"
#include "Skybox.h"
#include "PostProcessing.h"
#include "Monster.h"
#include "Camera.h"
#include "Cloth.h"
#include "Sphere.h"
//#include "Text.h"
//#include "Sprite.h"
#include "Slider.h"

class Game
{
	// Public functions
public:
	static Game& GetInstance();
	bool Initialize();

	void Render() const;
	void Update();
	
	void HandleMouseInput();
	void HandleKeyboardInput();
	bool UpdateMousePicking();

	GLFWwindow* GetWindow() { return m_pWindow; }

	Game(Game const&) = delete;
	void operator=(Game const&) = delete;

	// Private functions
private:
	Game() {}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);


	// Private member variables
private:
	GLFWwindow* m_pWindow;
	std::unique_ptr<CCamera> m_pCamera;
	glm::vec2 m_mousePos;
	glm::vec3 m_mouseRayDirection;

	//std::unique_ptr<PostProcessing> m_pPostProcessing;
	std::unique_ptr<Monster> m_pPlayer;

	Cloth* m_pCloth;
	std::unique_ptr<Sphere> m_pSphere;
	//Sprite* sprit;
	//Slider* m_testSlider;

	std::vector<Text*> m_textLabels;

	std::vector<std::unique_ptr<Object>> m_vecObjects;
	std::unordered_map<ShaderType, GLuint> g_mapShaders;
	bool m_bGameOver;

	float m_windStrength;
	glm::vec3 m_windDirection;

	int m_clothLength;
	int m_clothWidth;
	int m_numOfHooks;

	bool m_mouseCameraControl;
	bool m_mouseLockButtonDown;
};

