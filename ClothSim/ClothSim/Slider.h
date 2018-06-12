#pragma once

#include "GlobalIncludes.h"
#include "Sprite.h"
#include "Text.h"

class Slider
{
public:
	Slider(GLuint _spriteShader, GLuint _textShader, std::string _title, int _min, int _max);
	~Slider();

	void Render();

private:
	Sprite* m_background;
	Sprite* m_slider;
	Text* m_title;
};

