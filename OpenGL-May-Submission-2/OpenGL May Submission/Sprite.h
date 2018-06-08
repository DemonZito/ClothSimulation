#pragma once

#include "GlobalIncludes.h"
#include "UI.h"
#include "Texture.h"

class Sprite : public UI
{
public:
	Sprite(std::string _image, glm::vec2 _position, glm::vec2 _scale, glm::vec3 _color, GLuint _shader);
	void LoadTexture(std::string _image);
	~Sprite();

	void GLInit();
	void CreateIndicies();
	void GetUniforms();

	void Render() const override;

	void SetPosition(glm::vec2 position);

	glm::vec2 GetPosition();

private:
	glm::vec3 m_color;
	Texture* m_texture;

	std::vector<Vertice> m_vecVertices;
	std::vector<GLuint> m_vecIndicies;
	GLuint m_iProjLocation, m_iSampler, m_iColor;
	GLuint m_iModel, m_iRotationLocation, m_iTranslateLocation;
};

