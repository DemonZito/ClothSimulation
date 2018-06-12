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

			m_pGrabbedPoint = closest;	
			m_pTriangle = triangles[i];
			m_screenPoint = m_pCamera->GetViewMatrix() * glm::vec4(m_pGrabbedPoint->GetPosition(), 1.0);
			m_offset = glm::vec3(glm::vec4(m_pGrabbedPoint->GetPosition(), 1.0f) - (m_pCamera->GetViewMatrix() * glm::vec4(m_mousePos.x, m_mousePos.y, m_screenPoint.z, 1.0f)));

			return true;
		}
	}

	return false;

}

bool Game::Initialize()
{
	m_bGameOver = false;
	m_windDirection = glm::vec3(0, 0, 0.1f);
	m_windStrength = 0.1f;
	m_clothLength = 15;
	m_clothWidth = 15;
	m_numOfHooks = 4;
	m_stiffyness = 1;
	m_mouseCameraControl = false;
	m_xrayMode = false;

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

	// Create Floor
	Monster* mon;

	m_vecObjects.push_back(std::make_unique<Monster>(g_mapShaders[UNLIT_MODEL], "Resources/Models/rug.obj"));
	mon = dynamic_cast<Monster*>(m_vecObjects.at(m_vecObjects.size() - 1).get());
	mon->SetPosition(glm::vec3(0.0f, -30.5f, 0.0f));
	mon->SetScale(glm::vec3(200.0f, 1.0f, 200.0f));
	//m_vecObjects.push_back(std::make_unique<Monster>(g_mapShaders[UNLIT_MODEL], "Resources/Models/Oriental_rug_01.obj"));
	//dynamic_cast<Monster>(m_vecObjects.at(m_vecObjects.size() - 1))->SetPosition(glm::vec3(0.0f, -30.0f, 0.0f));
	//m_pPlayer->SetPosition(glm::vec3(-10.0f, -7.0f, 0.0f));

	m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
	//mon->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));

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

	m_UIText.push_back(new Text(glm::vec2(10, 190), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Camera:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 170), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Movement: (W),(A),(S),(D)", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 150), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Camera Lock: (Q) Toggle On/Off", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UIText.push_back(new Text(glm::vec2(10, 770), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Curtain Shape:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 750), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Length:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 710), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Width:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 670), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Hooks:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 630), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Reset:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	
	m_UIText.push_back(new Text(glm::vec2(30, 220), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "X-Ray Mode", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(10, 120), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Mouse Modes:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(30, 100), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Pull", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(30, 80), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Push", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(30, 60), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Tear", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(30, 40), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Fire", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(30, 20), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Pin", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UIText.push_back(new Text(glm::vec2(520, 770), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Object: (Arrow Keys to move)", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 750), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "No Object", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 730), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Sphere", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 710), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Capsule", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));
	m_UIText.push_back(new Text(glm::vec2(540, 690), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Pyramid", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UIText.push_back(new Text(glm::vec2(10, 590), glm::vec2(0.55f, 0.55f), glm::vec3(1, 1, 1), "Cloth Stiffness:", "Resources/Fonts/absender1.ttf", g_mapShaders[TEXT]));

	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(130, 55), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Length Knob
	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(130, 95), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Width Knob
	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(530, 755), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Wind Speed Knob

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

	// Mouse Modes
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 685), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Pull
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 705), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Push
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 725), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Tearing
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 745), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Fire

	// Object spawning
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(520, 35), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // No Object
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(520, 55), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Sphere
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(520, 75), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Cylinder
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(520, 95), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Pyramid

	m_UISprites.push_back(new Sprite("Resources/Textures/Knob.png", glm::vec2(80, 215), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Cloth stiffness
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 765), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // Pin
	m_UISprites.push_back(new Sprite("Resources/Textures/Skull.png", glm::vec2(10, 565), glm::vec2(20, 20), glm::vec3(1, 1, 1), g_mapShaders[SPRITE])); // XRay

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

	m_pCloth->Render();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	for (int i = 0; i < m_UIText.size(); ++i)
	{
		m_UIText.at(i)->Render();
	}

	for (int i = 0; i < m_UISprites.size(); ++i)
	{
		m_UISprites.at(i)->Render();
	}
	//m_pPostProcessing->Draw();
	if(m_xrayMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glfwSwapBuffers(m_pWindow);
}

void Game::Update()
{
	glfwPollEvents();
	HandleMouseInput();
	HandleKeyboardInput();
	if(!m_pGrabbedPoint)
		UpdateSliders();
	m_pCloth->AddForce(glm::vec3(0.0f, -0.11f, 0.0f));
	m_pCloth->windForce(m_windDirection * m_windStrength); // generate some wind each frame
	m_pCloth->Step();
	m_pCloth->FloorCollision();
	m_pCloth->SelfCollision();
	if(m_pSphere)
		m_pCloth->ballCollision(m_pSphere->GetPosition(), 3.2f);
	if (m_pPyramid)
		m_pCloth->pyramidCollision(m_pPyramid->GetPosition() , m_pPyramid->GetModel());
	if (m_pCapsule)
	{
		glm::vec3 capsulePos = m_pCapsule->GetPosition();
		m_pCloth->CapsuleCollision(glm::vec3(capsulePos.x + 0.5f, capsulePos.y, capsulePos.z), glm::vec3(capsulePos.x - 0.5f, capsulePos.y, capsulePos.z), 0, 2.0f);

	}
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

		// Mouse picking
		if (!m_pGrabbedPoint && glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
			UpdateMousePicking();
		else if (m_pGrabbedPoint && glfwGetMouseButton(m_pWindow, GLFW_MOUSE_BUTTON_LEFT))
		{
			ProcessMouseInteract();
			
		}
		else
		{
			if (m_pGrabbedPoint)
			{
				m_pGrabbedPoint->RevertFixedState();
				m_pGrabbedPoint = nullptr;
			}

		}

		m_prevMousePos = m_mousePos;
}

void Game::HandleKeyboardInput()
{
	if (m_pSphere != nullptr)
	{
		glm::vec3 monsterPos = m_pSphere->GetPosition();

		// Movement of the monster/player
		float monsterY = -10.0f;

		if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
			m_pSphere->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.1f));
			m_pCloth->MoveCurtain(0.01f);
		}
		if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
			m_pSphere->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.1f));
			//m_pCloth->MoveClothPoint(glm::vec3(-0.1f, 0, 0));
			m_pCloth->MoveCurtain(-0.01f);

		}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
			m_pSphere->SetPosition(glm::vec3(monsterPos.x + 0.1f, monsterY, monsterPos.z));
			//m_pCloth->MoveClothPoint(glm::vec3(0, 0, 1));

		}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
			m_pSphere->SetPosition(glm::vec3(monsterPos.x - 0.1f, monsterY, monsterPos.z));
			//m_pCloth->MoveClothPoint(glm::vec3(0, 0, -1));
		}
	}

	if (m_pPyramid != nullptr)
	{
		glm::vec3 monsterPos = m_pPyramid->GetPosition();

		// Movement of the monster/player
		float monsterY = -10.0f;

		if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
			m_pPyramid->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.1f));
		}
		if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
			m_pPyramid->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.1f));

		}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
			m_pPyramid->SetPosition(glm::vec3(monsterPos.x + 0.1f, monsterY, monsterPos.z));

		}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
			m_pPyramid->SetPosition(glm::vec3(monsterPos.x - 0.1f, monsterY, monsterPos.z));
		}
	}

	if (m_pCapsule != nullptr)
	{
		glm::vec3 monsterPos = m_pCapsule->GetPosition();

		// Movement of the monster/player
		float monsterY = -10.0f;

		if (Input::Instance().GetKeyDown(GLFW_KEY_UP)) {
			m_pCapsule->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z + 0.1f));
		}
		if (Input::Instance().GetKeyDown(GLFW_KEY_DOWN)) {
			m_pCapsule->SetPosition(glm::vec3(monsterPos.x, monsterY, monsterPos.z - 0.1f));

		}if (Input::Instance().GetKeyDown(GLFW_KEY_LEFT)) {
			m_pCapsule->SetPosition(glm::vec3(monsterPos.x + 0.1f, monsterY, monsterPos.z));

		}if (Input::Instance().GetKeyDown(GLFW_KEY_RIGHT)) {
			m_pCapsule->SetPosition(glm::vec3(monsterPos.x - 0.1f, monsterY, monsterPos.z));
		}
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
		m_windDirection = glm::vec3(0, 0, 0.1f);
		m_windStrength = 0.1f;
		m_clothLength = 15;
		m_clothWidth = 15;
		m_numOfHooks = 4;
		m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
		m_UISprites.at(0)->SetPosition(glm::vec2(130, 55));
		m_UISprites.at(1)->SetPosition(glm::vec2(130, 95));
		m_UISprites.at(2)->SetPosition(glm::vec2(630, 755));
	}
}

void Game::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void Game::ProcessMouseInteract()
{
	switch (m_mouseMode)
	{
	case PULL:
	{
		m_pGrabbedPoint->FixPoint();

		glm::vec4 viewPos = m_pCamera->GetViewMatrix() * glm::vec4{ m_pGrabbedPoint->GetPosition(), 1.0f };
		glm::vec2 deltaMouse = m_mousePos - m_prevMousePos;
		viewPos += glm::vec4(deltaMouse.x * 0.01f, -deltaMouse.y * 0.01f, 0.0f, 0.0f);

		glm::vec4 newPos = glm::inverse(m_pCamera->GetViewMatrix()) * viewPos;
		m_pGrabbedPoint->SetPos(glm::vec3(newPos));
		break;
	}
	case PUSH:
	{
		m_pCloth->PushCloth(m_pTriangle, m_mouseRayDirection);
		m_pGrabbedPoint = nullptr;
		break;
	}
	case BURN:
	{
		m_pGrabbedPoint->m_bBurning = true;
		break;
	}
	case TEAR:
	{
		m_pCloth->Tear(m_pGrabbedPoint);
		m_pGrabbedPoint = nullptr;
		break;
	}
	case PIN:
	{
		m_pGrabbedPoint->SetFixed(true);

		glm::vec4 viewPos = m_pCamera->GetViewMatrix() * glm::vec4{ m_pGrabbedPoint->GetPosition(), 1.0f };
		glm::vec2 deltaMouse = m_mousePos - m_prevMousePos;
		viewPos += glm::vec4(deltaMouse.x * 0.01f, -deltaMouse.y * 0.01f, 0.0f, 0.0f);

		glm::vec4 newPos = glm::inverse(m_pCamera->GetViewMatrix()) * viewPos;
		m_pGrabbedPoint->SetPos(glm::vec3(newPos));
		break;
	}
	}


	/**/

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
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
		}

		// Cloth Width
		if (m_mousePos.x > 10 && m_mousePos.x < 300 && m_mousePos.y > 95 && m_mousePos.y < 115)
		{
			m_UISprites.at(1)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(1)->GetPosition().y));
			m_clothWidth = ((m_mousePos.x - 10) / 10) + 1;
			delete m_pCloth;
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
		}

		// Wind Strength
		if (m_mousePos.x > 520 && m_mousePos.x < 760 && m_mousePos.y > 750 && m_mousePos.y < 770)
		{
			m_UISprites.at(2)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(2)->GetPosition().y));
			m_windStrength = (m_mousePos.x - 520) / 100;
		}

		// Cloth Stiffness
		if (m_mousePos.x > 10 && m_mousePos.x < 150 && m_mousePos.y > 215 && m_mousePos.y < 235)
		{
			m_UISprites.at(23)->SetPosition(glm::vec2(m_mousePos.x - 10, m_UISprites.at(23)->GetPosition().y));
			m_stiffyness = ((m_mousePos.x - 10) / 100) + 0.5f;
			m_pCloth->ChangeStiffness(m_stiffyness);
		}

		// Wind Reset
		if ((m_mousePos.x > m_UISprites.at(3)->GetPosition().x) 
			&& (m_mousePos.x < m_UISprites.at(3)->GetPosition().x + 20) 
			&& (m_mousePos.y > m_UISprites.at(3)->GetPosition().y) 
			&& (m_mousePos.y < m_UISprites.at(3)->GetPosition().y + 20))
		{
			m_windDirection = glm::vec3(0, 0, 0.1f);
			m_windStrength = 0.1f;
			m_UISprites.at(2)->SetPosition(glm::vec2(530, m_UISprites.at(2)->GetPosition().y));
		}

		// Reset Cloth
		if ((m_mousePos.x > m_UISprites.at(4)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(4)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(4)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(4)->GetPosition().y + 20))
		{
			delete m_pCloth;
			m_bGameOver = false;
			m_clothLength = 15;
			m_clothWidth = 15;
			m_numOfHooks = 4;
			m_stiffyness = 1;
			m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
			m_UISprites.at(0)->SetPosition(glm::vec2(130, 55));
			m_UISprites.at(1)->SetPosition(glm::vec2(130, 95));
			m_UISprites.at(2)->SetPosition(glm::vec2(630, 755));
			m_UISprites.at(23)->SetPosition(glm::vec2(80, 215));
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

		//Mouse Types
		// Pull
		if ((m_mousePos.x > m_UISprites.at(15)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(15)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(15)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(15)->GetPosition().y + 20))
		{
			m_mouseMode = PULL;
		}
		// push
		if ((m_mousePos.x > m_UISprites.at(16)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(16)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(16)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(16)->GetPosition().y + 20))
		{
			m_mouseMode = PUSH;
		}
		// tear
		if ((m_mousePos.x > m_UISprites.at(17)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(17)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(17)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(17)->GetPosition().y + 20))
		{
			m_mouseMode = TEAR;
		}
		// burn
		if ((m_mousePos.x > m_UISprites.at(18)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(18)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(18)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(18)->GetPosition().y + 20))
		{
			m_mouseMode = BURN;
		}
		// pin
		if ((m_mousePos.x > m_UISprites.at(24)->GetPosition().x)
			&& (m_mousePos.x < m_UISprites.at(24)->GetPosition().x + 20)
			&& (m_mousePos.y > m_UISprites.at(24)->GetPosition().y)
			&& (m_mousePos.y < m_UISprites.at(24)->GetPosition().y + 20))
		{
			m_mouseMode = PIN;
		}

		if (!m_mouseClickDown)
		{
			m_mouseClickDown = true;
			
			// XRay Mode
			if ((m_mousePos.x > m_UISprites.at(25)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(25)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(25)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(25)->GetPosition().y + 20))
			{
				if(m_xrayMode)
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				else
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				m_xrayMode = !m_xrayMode;
			}
			
			// Remove Hooks
			if ((m_mousePos.x > m_UISprites.at(13)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(13)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(13)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(13)->GetPosition().y + 20))
			{
				if (m_clothWidth > 1 && m_numOfHooks > 0)
					--m_numOfHooks;
				delete m_pCloth;
				m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
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
				m_pCloth = new Cloth(m_clothWidth, m_clothLength, m_numOfHooks, g_mapShaders[UNLIT_STANDARD], m_stiffyness);
			}

			// Objects
			// No Object
			if ((m_mousePos.x > m_UISprites.at(19)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(19)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(19)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(19)->GetPosition().y + 20))
			{
				for (int i = 0; i < m_vecObjects.size();)
				{
					if (m_vecObjects.at(i).get() == m_pSphere)
						m_vecObjects.erase(m_vecObjects.begin() + i);
					else if (m_vecObjects.at(i).get() == m_pPyramid)
						m_vecObjects.erase(m_vecObjects.begin() + i);
					else if (m_vecObjects.at(i).get() == m_pCapsule)
						m_vecObjects.erase(m_vecObjects.begin() + i);
					else
						i++;
				}
			}
			// Sphere
			if ((m_mousePos.x > m_UISprites.at(20)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(20)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(20)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(20)->GetPosition().y + 20))
			{
				for (int i = 0; i < m_vecObjects.size();)
				{
					if (m_vecObjects.at(i).get() == m_pSphere)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pSphere = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pPyramid)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pPyramid = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pCapsule)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pCapsule = nullptr;
					}
					else
						i++;
				}

				m_vecObjects.push_back(std::make_unique<Sphere>(g_mapShaders[UNLIT_MODEL]));
				m_pSphere = dynamic_cast<Sphere*>(m_vecObjects[m_vecObjects.size() - 1].get());
				m_pSphere->SetPosition(glm::vec3(0.0f, -10.0f, -10.0f));
				m_pSphere->SetScale(glm::vec3(2.75f, 2.75f, 2.75f));
			}

			// capsule
			if ((m_mousePos.x > m_UISprites.at(21)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(21)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(21)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(21)->GetPosition().y + 20))
			{
				for (int i = 0; i < m_vecObjects.size();)
				{
					if (m_vecObjects.at(i).get() == m_pSphere)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pSphere = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pPyramid)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pPyramid = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pCapsule)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pCapsule = nullptr;
					}
					else
						i++;
				}

				m_vecObjects.push_back(std::make_unique<Capsule>(g_mapShaders[UNLIT_MODEL]));
				m_pCapsule = dynamic_cast<Capsule*>(m_vecObjects[m_vecObjects.size() - 1].get());
				m_pCapsule->SetPosition(glm::vec3(0.0f, -10.0f, -10.0f));
				m_pCapsule->SetScale(glm::vec3(2.0f, 2.0f, 2.0f));
			}

			// pyramid
			if ((m_mousePos.x > m_UISprites.at(22)->GetPosition().x)
				&& (m_mousePos.x < m_UISprites.at(22)->GetPosition().x + 20)
				&& (m_mousePos.y > m_UISprites.at(22)->GetPosition().y)
				&& (m_mousePos.y < m_UISprites.at(22)->GetPosition().y + 20))
			{
				for (int i = 0; i < m_vecObjects.size();)
				{
					if (m_vecObjects.at(i).get() == m_pSphere)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pSphere = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pPyramid)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pPyramid = nullptr;
					}
					else if (m_vecObjects.at(i).get() == m_pCapsule)
					{
						m_vecObjects.erase(m_vecObjects.begin() + i);
						m_pCapsule = nullptr;
					}
					else
						i++;
				}

				m_vecObjects.push_back(std::make_unique<Pyramid>(g_mapShaders[UNLIT_MODEL]));
				m_pPyramid = dynamic_cast<Pyramid*>(m_vecObjects[m_vecObjects.size() - 1].get());
				m_pPyramid->SetPosition(glm::vec3(0.0f, -10.0f, -10.0f));
				m_pPyramid->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
			}
		}
	}
	else if (m_mouseClickDown)
	{
		m_mouseClickDown = false;
	}
}
