#pragma once

#include "GlobalIncludes.h"
#include "Skybox.h"
#include "PostProcessing.h"
#include "Floor.h"
#include "Camera.h"
#include "Cloth.h"
#include "Sphere.h"
#include "Pyramid.h"
#include "Capsule.h"
//#include "Text.h"
#include "Sprite.h"
#include "Slider.h"

enum MouseMode
{
	PULL,
	PUSH,
	TEAR,
	BURN,
	PIN
};

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
	void UpdateSliders();

	GLFWwindow* GetWindow() { return m_pWindow; }

	Game(Game const&) = delete;
	void operator=(Game const&) = delete;

	// Private functions
private:
	Game() {}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

	void ProcessMouseInteract();


	// Private member variables
private:
	GLFWwindow* m_pWindow;
	std::unique_ptr<CCamera> m_pCamera;
	glm::vec2 m_mousePos;
	glm::vec2 m_prevMousePos;
	glm::vec3 m_mouseRayDirection;

	Floor* m_pFloor;
	Cloth* m_pCloth;
	Sphere* m_pSphere;
	Pyramid* m_pPyramid;
	Capsule* m_pCapsule;

	std::vector<Text*> m_UIText;
	std::vector<Sprite*> m_UISprites;

	std::vector<std::unique_ptr<Object>> m_vecObjects;
	std::unordered_map<ShaderType, GLuint> g_mapShaders;
	bool m_bGameOver;

	float m_windStrength;
	glm::vec3 m_windDirection;

	int m_clothLength;
	int m_clothWidth;
	int m_numOfHooks;
	float m_stiffyness;

	bool m_mouseCameraControl; // The bool to check if the mouse should lock with the camera
	bool m_mouseLockButtonDown; // Bool check so you dont press buttons more than once
	bool m_mouseClickDown; // Bool check so you dont press buttons more than once
	bool m_xrayMode;

	Point* m_pGrabbedPoint = nullptr;
	Triangle* m_pTriangle = nullptr;
	glm::vec4 m_screenPoint;
	glm::vec3 m_offset;

	MouseMode m_mouseMode = PUSH;
};

