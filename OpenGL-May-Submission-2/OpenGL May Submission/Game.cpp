#include "Game.h"
#include "Shader.h"
#include "Camera.h"
#include "input.h"

Game& Game::GetInstance()
{
	static Game instance; // Guaranteed to be destroyed.
						  // Instantiated on first use.
	return instance;
}

bool Game::Initialize()
{
	m_bGameOver = false;

	// Init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	m_pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Summative 2", NULL, NULL);
	glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);


	if (m_pWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_pWindow);
	glfwSetFramebufferSizeCallback(m_pWindow, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Load all shaders
	GLuint curShader;

	ShaderHelper::compileAndLinkShaders("Skybox_VS.glsl", "Skybox_FS.glsl", curShader);
	g_mapShaders[SKYBOX] = curShader;

	ShaderHelper::compileAndLinkShaders("Framebuffer_VS.glsl", "Framebuffer_FS.glsl", curShader);
	g_mapShaders[FRAMEBUFFER] = curShader;

	ShaderHelper::compileAndLinkShaders("Unlit_VS.glsl", "Unlit_FS.glsl", curShader);
	g_mapShaders[UNLIT_MODEL] = curShader;

	ShaderHelper::compileAndLinkShaders("vertex_shader.glsl", "fragment_shader.glsl", curShader);
	g_mapShaders[UNLIT_STANDARD] = curShader;

	ShaderHelper::compileAndLinkShaders("Text_VS.glsl", "Text_FS.glsl", curShader);
	g_mapShaders[TEXT] = curShader;

	ShaderHelper::compileAndLinkShaders("Sprite_VS.glsl", "Sprite_FS.glsl", curShader);
	g_mapShaders[SPRITE] = curShader;

//	m_pPostProcessing = std::make_unique<PostProcessing>(g_mapShaders[FRAMEBUFFER]);
	//m_pPostProcessing->Initialize();
	
	Input::Instance().Initialize(m_pWindow);
	
	// Create camera
	m_pCamera = std::make_unique<CCamera>();

	// Create skybox
	std::vector<std::string> strImagePaths;
	strImagePaths.push_back("Resources/Textures/skybox1/right.tga");
	strImagePaths.push_back("Resources/Textures/skybox1/left.tga");
	strImagePaths.push_back("Resources/Textures/skybox1/top.tga");
	strImagePaths.push_back("Resources/Textures/skybox1/bottom.tga");
	strImagePaths.push_back("Resources/Textures/skybox1/back.tga");
	strImagePaths.push_back("Resources/Textures/skybox1/front.tga");
	m_vecObjects.push_back(std::make_unique<Skybox>(strImagePaths, g_mapShaders[SKYBOX]));

	// Create objects and player
	m_pPlayer = std::make_unique<Monster>(g_mapShaders[UNLIT_MODEL], "Resources/Models/Bullet.obj");

	clothyBoy = new Cloth(20, 20, 20, 20, g_mapShaders[UNLIT_STANDARD]);
	textLavel = new Text(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec3(1.0, 0.0, 0.0), "Hello?", "Resources/Fonts/SequentialSans.ttf", g_mapShaders[TEXT]);
	sprit = new Sprite("Resources/Textures/best.PNG", glm::vec2(0, 0), glm::vec2(250, 250), glm::vec3(1, 1, 1), g_mapShaders[SPRITE]);

	while (!glfwWindowShouldClose(m_pWindow) && !m_bGameOver)
	{
		Update();
		if(!m_bGameOver) Render();
	}

	glfwTerminate();

	//=== Run Game here ===
	//ApplicationCleanUp();

	return true;
}

void Game::Render() const
{
	//// bind to framebuffer and draw scene as we normally would to color texture 
	//glBindFramebuffer(GL_FRAMEBUFFER, m_pPostProcessing->GetFramebuffer());
	glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

	// make sure we clear the framebuffer's content
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	m_pCamera->SendDataToShaders(g_mapShaders);

	for (int i = 0; i < m_vecObjects.size(); i++)
		m_vecObjects[i]->Render();

	m_pPlayer->Render();
	clothyBoy->Render();
	textLavel->Render();
	sprit->Render();
	//m_pPostProcessing->Draw();

	glfwSwapBuffers(m_pWindow);
}

void Game::Update()
{
	glfwPollEvents();
	HandleMouseInput();
	HandleKeyboardInput();
	clothyBoy->AddForce(glm::vec3(0.0f, -0.5f, sin(glfwGetTime()) * .5f)*g_kfTimeStep);
	clothyBoy->Step();
	Input::Instance().Clear();

}

void Game::HandleMouseInput()
{
	glm::vec2 mousePos = Input::Instance().MousePosition();
	// If mouse has moved and camera exists
	if (!(mousePos.x == 400 && mousePos.y == 400) && m_pCamera != nullptr)
	{
		m_pCamera->MoveCamera(mousePos);

		// Reset mouse to center
		glfwSetCursorPos(m_pWindow, 400, 400);
	}
}

void Game::HandleKeyboardInput()
{
	glm::vec3 monsterPos = m_pPlayer->GetPosition();

	// Movement of the monster/player
	float monsterY= 0.0f;
	if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x + 0.5f, monsterY, monsterPos.z));
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x - 0.5f, monsterY, monsterPos.z));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.5f));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.5f));

	}

	// Movement of camera
	if (Input::Instance().GetKeyDown(GLFW_KEY_W)) {
		m_pCamera->SetPosition(m_pCamera->GetFront() * m_pCamera->GetCameraSpeed());
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_S)) {
		m_pCamera->SetPosition(-(m_pCamera->GetFront() * m_pCamera->GetCameraSpeed()));
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_A)) {
		m_pCamera->SetPosition(-(glm::normalize(glm::cross(m_pCamera->GetFront(), m_pCamera->GetCameraUp()))* m_pCamera->GetCameraSpeed()));
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_D)) {
		m_pCamera->SetPosition(glm::normalize(glm::cross(m_pCamera->GetFront(), m_pCamera->GetCameraUp()))* m_pCamera->GetCameraSpeed());
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_ESCAPE)) {
		m_bGameOver = true;
	}
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

