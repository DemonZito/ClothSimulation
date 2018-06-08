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
	glm::vec2 normalizedScreenPos = glm::vec2(((m_mousePos.x*2) / WINDOW_WIDTH) - 1, 1 - (2 * m_mousePos.y/WINDOW_HEIGHT));

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

			//m_pCloth->PushCloth(closest, m_mouseRayDirection);
			m_pGrabbedPoint = closest;	
			m_screenPoint = m_pCamera->GetViewMatrix() * glm::vec4(m_pGrabbedPoint->GetPosition(), 1.0);
			m_offset = glm::vec3(glm::vec4(m_pGrabbedPoint->GetPosition(), 1.0f) - (m_pCamera->GetViewMatrix() * glm::vec4(m_mousePos.x, m_mousePos.y, m_screenPoint.z, 1.0f)));
			m_pGrabbedPoint->SetFixed(true);

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
	m_numOfHooks = 4;
	m_mouseCameraControl = false;

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
	m_pCamera->SetPosition(glm::vec3(-1, 20, -45));
	m_pCamera->SetCameraFront(glm::vec3(0.14f, -0.51f, 0.85f));

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

	m_vecObjects.push_back(std::make_unique<Sphere>(g_mapShaders[UNLIT_MODEL]));
	m_pSphere = dynamic_cast<Sphere*>(m_vecObjects[m_vecObjects.size() - 1].get());
	m_pSphere->SetPosition(glm::vec3(-10.0f, -7.0f, 0.0f));
	m_pSphere->SetScale(glm::vec3(2.75f, 2.75f, 2.75f));

	m_vecObjects.push_back(std::make_unique<Pyramid>(g_mapShaders[UNLIT_MODEL]));
	m_pPyramid = dynamic_cast<Pyramid*>(m_vecObjects[m_vecObjects.size() - 1].get());
	m_pPyramid->SetPosition(glm::vec3(-10.0f, -7.0f, 0.0f));
	m_pPyramid->SetScale(glm::vec3(2.75f, 2.75f, 2.75f));

	//m_pPlayer->SetPosition(glm::vec3(-10.0f, -7.0f, 0.0f));

	m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
	//textLavel = new Text(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec3(1.0, 0.0, 0.0), "Hello?", "Resources/Fonts/SequentialSans.ttf", g_mapShaders[TEXT]);
	//sprit = new Sprite("Resources/Textures/best.PNG", glm::vec2(0, 0), glm::vec2(250, 250), glm::vec3(1, 1, 1), g_mapShaders[SPRITE]);
	
	//UI Stuff
	m_UIText.push_back(new Text(glm::vec2(520, 190),glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Wind:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(520, 170),glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Change Direction:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(620, 150),glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "North", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 130),glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "North West", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(680, 130),glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "North East", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 110), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "West", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(680, 110), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "East", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 90), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "South West", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(680, 90), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "South East", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(620, 70), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "South", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(520, 50), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Speed:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(520, 10), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Reset Wind: ", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UIText.push_back(new Text(glm::vec2(10, 50), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Camera:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 30), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Movement: (W),(A),(S),(D)", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 10), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Camera Lock: (Q) Toggle On/Off", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UIText.push_back(new Text(glm::vec2(10, 770), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Curtain Shape:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 750), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Length:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 710), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Width:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 670), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Hooks:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 630), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Reset:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(130, 55), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Length Knob
	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(130, 95), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Width Knob
	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(630, 755), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Wind Speed Knob

	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(620, 775), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Reset Wind
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(60, 155), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Reset Cloth

	// Wind Buttons
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(595, 635), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // North
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(650, 653), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // North East
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(650, 675), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // East
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(650, 695), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // South East
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(510, 653), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // North West
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(510, 675), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // West
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(510, 695), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // South West
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(595, 715), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // South

	// Hooks
	m_UISprites.push_back(new Sprite("Resources/Textures/Minus.png", glm::vec2(70, 115), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Minus							
	m_UISprites.push_back(new Sprite("Resources/Textures/Plus.png", glm::vec2(100, 115), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Plus

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

	for (int i = 0; i < m_UIText.size(); ++i)
	{
		m_UIText.at(i)->Render();
	}

	for (int i = 0; i < m_UISprites.size(); ++i)
	{
		m_UISprites.at(i)->Render();
	}
	//m_pPostProcessing->Draw();

	glfwSwapBuffers(m_pWindow);
}

void Game::Update()
{
	glfwPollEvents();
	HandleMouseInput();
	HandleKeyboardInput();
	UpdateSliders();
	m_pCloth->AddForce(glm::vec3(0.0f, -0.11f, 0.0f));
	m_pCloth->windForce(m_windDirection * m_windStrength); // generate some wind each frame
	m_pCloth->Step();
	m_pCloth->FloorCollision();
	m_pCloth->ballCollision(m_pSphere->GetPosition(), 3);
	Input::Instance().Clear();
}

void Game::HandleMouseInput()
{
		m_mousePos = Input::Instance().MousePosition();
		// If mouse has moved and camera exists
		if (m_mouseCameraControl)
		{
			if (!(m_mousePos.x == 400 && m_mousePos.y == 400) && m_pCamera != nullptr)
			{				
				m_pCamera->MoveCamera(m_mousePos);

				// Reset mouse to center
				glfwSetCursorPos(m_pWindow, 400, 400);
			}
		}
		if (!m_pGrabbedPoint && glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
			UpdateMousePicking();
		else if (m_pGrabbedPoint && glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
		{
			glm::vec4 viewPos = m_pCamera->GetViewMatrix() * glm::vec4{ m_pGrabbedPoint->GetPosition(), 1.0f };
			glm::vec2 deltaMouse = m_mousePos - m_prevMousePos;
			viewPos += glm::vec4(deltaMouse.x * 0.02f, -deltaMouse.y * 0.02f, 0.0f, 0.0f);

			glm::vec4 newPos = glm::inverse(m_pCamera->GetViewMatrix()) * viewPos;
			m_pGrabbedPoint->SetPos(glm::vec3(newPos));
		}
		else
		{
			if (m_pGrabbedPoint)
			{
				m_pGrabbedPoint->SetFixed(false);
				/*if(m_pGrabbedPoint->m_bOverExtended)
					m_pCloth->PushCloth(m_pGrabbedPoint, glm::vec3(0, 0, 0));*/

				m_pGrabbedPoint = nullptr;
			}

		}

		m_prevMousePos = m_mousePos;
}

void Game::HandleKeyboardInput()
{
	glm::vec3 monsterPos = m_pSphere->GetPosition();

	// Movement of the monster/player
	float monsterY= -10.0f;
	if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
		m_pSphere->SetPosition(glm::vec3(monsterPos.x + 0.1f, monsterY, monsterPos.z));
		//m_pCloth->MoveClothPoint(glm::vec3(0.1f, 0, 0));
	}
	if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
		m_pSphere->SetPosition(glm::vec3(monsterPos.x - 0.1f, monsterY, monsterPos.z));
		//m_pCloth->MoveClothPoint(glm::vec3(-0.1f, 0, 0));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
		m_pSphere->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.1f));
		//m_pCloth->MoveClothPoint(glm::vec3(0, 0, 1));

	}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
		m_pSphere->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.1f));
		//m_pCloth->MoveClothPoint(glm::vec3(0, 0, -1));
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
		m_windDirection = glm::vec3(0.1f, 0.0f, 0.1f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_Y)) { // North
		m_windDirection = glm::vec3(0.0f, 0.0f, 0.1f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_U)) { // North East
		m_windDirection = glm::vec3(-0.1f, 0.0f, 0.1f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_G)) { // West
		m_windDirection = glm::vec3(0.1f, 0.0f, 0.0f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_J)) { // East
		m_windDirection = glm::vec3(-0.1f, 0.0f, 0.0f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_B)) { // South West
		m_windDirection = glm::vec3(0.1f, 0.0f, -0.1f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_N)) { // South
		m_windDirection = glm::vec3(0.0f, 0.0f, -0.1f);
	}

	if (Input::Instance().GetKeyDown(GLFW_KEY_M)) { // South East
		m_windDirection = glm::vec3(-0.1f, 0.0f, -0.1f);
	}

	//Reset Wind
	if (Input::Instance().GetKeyDown(GLFW_KEY_H)) {
		m_windDirection = glm::vec3(0.0f, 0.0f, 0.0f);
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

	// Toggle mouse controls
	if (Input::Instance().GetKeyDown(GLFW_KEY_Q)) {
		if (!m_mouseLockButtonDown)
		{
			m_mouseCameraControl = !m_mouseCameraControl;
			m_mouseLockButtonDown = true;
		}
	}

	// Toggle mouse controls
	if (Input::Instance().GetKeyUp(GLFW_KEY_Q)) {
		if (m_mouseLockButtonDown)
		{
			m_mouseLockButtonDown = false;
		}
	}

	// Reset Cloth
	if (Input::Instance().GetKeyDown(GLFW_KEY_R)) {
		delete m_pCloth;
		m_bGameOver = false;
		m_windDirection = glm::vec3(0, 0, 0);
		m_windStrength = 1.0f;
		m_clothLength = 15;
		m_clothWidth = 15;
		m_numOfHooks = 4;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
		m_UISprites.at(0)->SetPosition(glm::vec2(130, 55));
		m_UISprites.at(1)->SetPosition(glm::vec2(130, 95));
		m_UISprites.at(2)->SetPosition(glm::vec2(630, 755));
	}
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Game::UpdateSliders()
{
	if (glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
	{
		// Cloth Length
		if (m_mousePos.x > 10 && m_mousePos.x < 300 && m_mousePos.y > 55 && m_mousePos.y < 75)
		{
			m_UISprites.at(0)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(0)->GetPosition().y));
			m_clothLength = ((m_mousePos.x - 10) / 10) + 1;
			delete m_pCloth;
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
		}

		// Cloth Width
		if (m_mousePos.x > 10 && m_mousePos.x < 300 && m_mousePos.y > 95 && m_mousePos.y < 115)
		{
			m_UISprites.at(1)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(1)->GetPosition().y));
			m_clothWidth = ((m_mousePos.x - 10) / 10) + 1;
			delete m_pCloth;
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
		}

		// Wind Strength
		if (m_mousePos.x > 520 && m_mousePos.x < 760 && m_mousePos.y > 750 && m_mousePos.y < 770)
		{
			m_UISprites.at(2)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(2)->GetPosition().y));
			m_windStrength = (m_mousePos.x - 520) / 100;
		}

		// Wind Reset
		if ((m_mousePos.x > m_UISprites.at(3)->GetPosition().x) 
			&& (m_mousePos.x < m_UISprites.at(3)->GetPosition().x + 20) 
			&& (m_mousePos.y > m_UISprites.at(3)->GetPosition().y) 
			&& (m_mousePos.y < m_UISprites.at(3)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		// Reset Cloth
		if ((m_mousePos.x > m_UISprites.at(4)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(4)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(4)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(4)->GetPosition().y + 20))
		{
			delete m_pCloth;
			m_bGameOver = false;
			m_windDirection = glm::vec3(0, 0, 0);
			m_windStrength = 1.0f;
			m_clothLength = 15;
			m_clothWidth = 15;
			m_numOfHooks = 4;
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
			m_UISprites.at(0)->SetPosition(glm::vec2(130, 55));
			m_UISprites.at(1)->SetPosition(glm::vec2(130, 95));
			m_UISprites.at(2)->SetPosition(glm::vec2(630, 755));
		}

		
		// Winds
		// North
		if ((m_mousePos.x > m_UISprites.at(5)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(5)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(5)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(5)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.0f, 0.0f, 0.1f);
		}
		// North East
		if ((m_mousePos.x > m_UISprites.at(6)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(6)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(6)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(6)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(-0.1f, 0.0f, 0.1f);
		}
		// East
		if ((m_mousePos.x > m_UISprites.at(7)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(7)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(7)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(7)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(-0.1f, 0.0f, 0.0f);
		}
		// South East
		if ((m_mousePos.x > m_UISprites.at(8)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(8)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(8)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(8)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(-0.1f, 0.0f, -0.1f);
		}
		// North West
		if ((m_mousePos.x > m_UISprites.at(9)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(9)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(9)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(9)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.1f, 0.0f, 0.1f);
		}
		// West
		if ((m_mousePos.x > m_UISprites.at(10)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(10)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(10)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(10)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.1f, 0.0f, 0.0f);
		}
		// South West
		if ((m_mousePos.x > m_UISprites.at(11)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(11)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(11)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(11)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.1f, 0.0f, -0.1f);
		}
		// South
		if ((m_mousePos.x > m_UISprites.at(12)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(12)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(12)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(12)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0.0f, 0.0f, -0.1f);
		}

		if (!m_mouseClickDown)
		{
			m_mouseClickDown = true;
			// Remove Hooks
			if ((m_mousePos.x > m_UISprites.at(13)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(13)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(13)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(13)->GetPosition().y + 20))
			{
				if (m_clothWidth > 1)
					--m_numOfHooks;
				delete m_pCloth;
				m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
			}


			// Add Hooks
			if ((m_mousePos.x > m_UISprites.at(14)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(14)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(14)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(14)->GetPosition().y + 20))
			{
				if (m_numOfHooks < m_clothWidth)
					++m_numOfHooks;
				delete m_pCloth;
				m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD]);
			}
		}
	}
	else if (m_mouseClickDown)
	{
		m_mouseClickDown = false;
	}
}
