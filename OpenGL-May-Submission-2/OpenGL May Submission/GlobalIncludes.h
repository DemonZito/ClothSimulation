#pragma once

//Internal Includes
#include <conio.h>
#include <iostream>
#include <time.h>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <soil\SOIL.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "glm\gtc\quaternion.hpp"
#include "glm\gtx\quaternion.hpp"


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define BUFFER_OFFSET(i) ((GLvoid *)(i*sizeof(float)))

static const float g_kfTimeStep = 0.25f;
static const int g_kiConstraintIterations = 15;

// Vertex structure of models
struct Vertice
{
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec2 TexCoord;
};

enum ShaderType
{
	SKYBOX,
	UNLIT_MODEL,
	UNLIT_STANDARD,
	FRAMEBUFFER,
	TEXT,
	SPRITE
};


