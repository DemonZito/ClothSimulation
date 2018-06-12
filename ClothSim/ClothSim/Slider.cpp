#include "Slider.h"



Slider::Slider(GLuint _spriteShader, GLuint _textShader, std::string _title, int _min, int _max)
{
	m_background = new Sprite("Resources/Textures/SliderBackground.PNG", glm::vec2(0, 0), glm::vec2(250, 250), glm::vec3(1, 1, 1), _spriteShader);
	m_slider = new Sprite("Resources/Textures/Slider.PNG", glm::vec2(0, 0), glm::vec2(250, 250), glm::vec3(1, 1, 1), _spriteShader);
	m_title = new Text(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec3(1.0, 0.0, 0.0), _title, "Resources/Fonts/SequentialSans.ttf", _textShader);

}


Slider::~Slider()
{
}

void Slider::Render()
{
	m_background->Render();
	m_title->Render();
	m_slider->Render();
}
