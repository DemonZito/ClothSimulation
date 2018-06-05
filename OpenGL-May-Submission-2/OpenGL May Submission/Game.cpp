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

bool Game::UpdateMousePicking()
{
	//screen pos
	glm::vec2 normalizedScreenPos = glm::vec2(0, 0);

	//screenpos to Proj Space
	glm::vec4 clipCoords = glm::vec4(normalizedScreenPos.x, normalizedScreenPos.y, -1.0f, 1.0f);

	//Proj Space to eye space
	glm::mat4 invProjMat = glm::inverse(m_pCamera->GetPerspectiveMatrix());
	glm::vec4 eyeCoords = invProjMat * clipCoords;
	eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);

	//eyespace to world space
	glm::mat4 invViewMat = glm::inverse(m_pCamera->GetViewMatrix());
	glm::vec4 rayWorld = invViewMat * eyeCoords;
	m_mouseRayDirection = glm::normalize(glm::vec3(rayWorld));


	// Check intersection
	std::vector<Triangle*> triangles = m_pCloth->GetTriangles();

	const float EPSILON = 0.0000001f;

	for (unsigned int i = 0; i < triangles.size(); i++)
	{
		glm::vec3 edge1, edge2, h, s, q;
		float a, f, u, v;
		std::vector<Point*> points = triangles[i]->GetPoints();
		edge1 = points[1]->GetPosition() - points[0]->GetPosition();
		edge2 = points[2]->GetPosition() - points[0]->GetPosition();
		h = glm::cross(m_mouseRayDirection, edge2);
		a = glm::dot(edge1, h);

		if (a > -EPSILON && a < EPSILON)
			continue;

		f = 1 / a;
		s = m_pCamera->GetPosition() - points[0]->GetPosition();
		u = f * (glm::dot(s, h));

		if (u < 0.0 || u > 1.0)
			continue;

		q = glm::cross(s, edge1);
		v = f * glm::dot(m_mouseRayDirection, q);
		if (v < 0.0 || u + v > 1.0)
			continue;

		// At this stage we can compute t to find out where the intersection point is on the line.
		float t = f * glm::dot(edge2, q);

		if (t > EPSILON) // ray intersection
		{
			glm::vec3 outIntersectionPoint = m_pCamera->GetPosition() + m_mouseRayDirection * t;

			std::cout << "x: " << outIntersectionPoint.x << ", ";
			std::cout << "y: " << outIntersectionPoint.y << ", ";
			std::cout << "z: " << outIntersectionPoint.z << std::endl;

			Point* closest = nullptr;
			float smallestDist = 100000;
			for (int ptIdx = 0; ptIdx < 3; ptIdx++)
			{
				if (glm::distance(points[ptIdx]->GetPosition(), outIntersectionPoint) < smallestDist)
				{
					smallestDist = glm::distance(points[ptIdx]->GetPosition(), outIntersectionPoint);
					closest = points[ptIdx];
				}
			}

			m_pCloth->PushCloth(closest, m_mouseRayDirection);

			return true;
		}
	}

	return false;

}

bool Game::Initialize()
{
	m_bGameOver = false;
	m_windDirection = glm::vec3(0, 0, 0);
	m_windStrength = 1.0f;
	m_clothLength = 15;
	m_clothWidth = 15;
	m_numOfHooks = 2;

	// Init glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	m_pWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Summative 2", NULL, NULL);
	glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetCursor(m_pWindow, glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR));


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

	m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	//textLavel = new Text(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec3(1.0, 0.0, 0.0), "Hello?", "Resources/Fonts/SequentialSans.ttf", g_mapShaders[TEXT]);
	//sprit = new Sprite("Resources/Textures/best.PNG", glm::vec2(0, 0), glm::vec2(250, 250), glm::vec3(1, 1, 1), g_mapShaders[SPRITE]);
	m_testSlider = new Slider(g_mapShaders[SPRITE], g_mapShaders[TEXT], "test", 0, 10);
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
	m_pCloth->Render();
	m_testSlider->Render();
	//m_pPostProcessing->Draw();

	glfwSwapBuffers(m_pWindow);
}

void Game::Update()
{
	glfwPollEvents();
	HandleMouseInput();
	HandleKeyboardInput();
	m_pCloth->AddForce(glm::vec3(0.0f, -0.81f, 0.0f));
	m_pCloth->windForce(m_windDirection * m_windStrength); // generate some wind each frame
	m_pCloth->Step();
	Input::Instance().Clear();

}

void Game::HandleMouseInput()
{
	m_mousePos = Input::Instance().MousePosition();
	// If mouse has moved and camera exists
	if (!(m_mousePos.x == 400 && m_mousePos.y == 400) && m_pCamera != nullptr)
	{
		m_pCamera->MoveCamera(m_mousePos);

		// Reset mouse to center
		glfwSetCursorPos(m_pWindow, 400, 400);
	}

	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
		UpdateMousePicking();
}

void Game::HandleKeyboardInput()
{
	glm::vec3 monsterPos = m_pPlayer->GetPosition();

	// Movement of the monster/player
	float monsterY= 0.0f;
	if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x + 0.5f, monsterY, monsterPos.z));
		m_pCloth->MoveClothPoint(glm::vec3(0.1f, 0, 0));
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x - 0.5f, monsterY, monsterPos.z));
		m_pCloth->MoveClothPoint(glm::vec3(-0.1f, 0, 0));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.5f));
		m_pCloth->MoveClothPoint(glm::vec3(0, 0, 1));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
		m_pPlayer->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.5f));
		m_pCloth->MoveClothPoint(glm::vec3(0, 0, -1));

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
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_D)) {
		m_pCamera->SetPosition(glm::normalize(glm::cross(m_pCamera->GetFront(), m_pCamera->GetCameraUp()))* m_pCamera->GetCameraSpeed());
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_ESCAPE)) {
		m_bGameOver = true;
	}

	// Winde Direction
	if (Input::Instance().GetKeyDown(GLFW_KEY_T)) { // North West
		m_windDirection = glm::vec3(0.25f, 0.0f, 0.25f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_Y)) { // North
		m_windDirection = glm::vec3(0.0f, 0.0f, 0.25f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_U)) { // North East
		m_windDirection = glm::vec3(-0.25f, 0.0f, 0.25f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_G)) { // West
		m_windDirection = glm::vec3(0.25f, 0.0f, 0.0f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_J)) { // East
		m_windDirection = glm::vec3(-0.25f, 0.0f, 0.0f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_B)) { // South West
		m_windDirection = glm::vec3(0.25f, 0.0f, -0.25f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_N)) { // South
		m_windDirection = glm::vec3(0.0f, 0.0f, -0.25f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_M)) { // South East
		m_windDirection = glm::vec3(-0.25f, 0.0f, -0.25f);
	}

	//Reset Wind
	if (Input::Instance().GetKeyDown(GLFW_KEY_H)) {
		m_windDirection = glm::vec3(0.0f, 0.0f, 0.0f);
		m_windStrength = 0.0f;
	}

	// Wind Strength
	if (Input::Instance().GetKeyDown(GLFW_KEY_L)) {
		if (m_windStrength < 11.0f)
			m_windStrength += 0.1f;
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_K)) {
		if (m_windStrength > 0.0f)
			m_windStrength -= 0.1f;
	}

	// Cloth length
	if (Input::Instance().GetKeyDown(GLFW_KEY_1)) {
		if(m_clothLength > 2)
			--m_clothLength;
		delete m_pCloth;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_2)) {
		++m_clothLength;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}

	// Cloth Width
	if (Input::Instance().GetKeyDown(GLFW_KEY_3)) {
		if (m_clothWidth > 2)
			--m_clothWidth;
		delete m_pCloth;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_4)) {
		++m_clothWidth;
		delete m_pCloth;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}

	// Number of hooks
	if (Input::Instance().GetKeyDown(GLFW_KEY_5)) {
		if (m_clothWidth > 1)
			--m_numOfHooks;
		delete m_pCloth;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_6)) {
		if(m_numOfHooks < m_clothWidth)
			++m_numOfHooks;
		delete m_pCloth;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	}
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

