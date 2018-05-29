#pragma once

#include "GlobalIncludes.h"
#include "Skybox.h"
#include "PostProcessing.h"
#include "Monster.h"
#include "Camera.h"
#include "Cloth.h"
#include "Text.h"
#include "Sprite.h"

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

	//std::unique_ptr<PostProcessing> m_pPostProcessing;
	std::unique_ptr<Monster> m_pPlayer;

	Cloth* clothyBoy;
	Text* textLavel;
	Sprite* sprit;

	std::vector<std::unique_ptr<Object>> m_vecObjects;
	std::unordered_map<ShaderType, GLuint> g_mapShaders;
	bool m_bGameOver;
};

