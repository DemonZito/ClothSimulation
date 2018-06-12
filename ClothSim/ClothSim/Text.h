#pragma once 

#include <map>

#include "UI.h"
#include "GlobalIncludes.h"


struct Character {
	GLuint     TextureID;  // ID handle of the glyph texture
	glm::ivec2 Size;       // Size of glyph
	glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
	GLuint     Advance;    // Offset to advance to next glyph
};

class Text : public UI
{
public:
	Text(glm::vec2 _position, glm::vec2 _scale, glm::vec3 _color, std::string _text, std::string _font, GLuint _shader);
	~Text();
	void Render() const override;
	virtual bool Initialise();
	const std::map<GLchar, Character> GetCharacters() const;
	void SetColor(glm::vec3 _color);
	void SetText(std::string _text);

	void SetPosition(glm::vec2 _position);
private:
	std::string m_text;
	glm::vec3 m_color;

	std::string m_font;
	std::map<GLchar, Character> Characters;
};