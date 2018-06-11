#pragma once

#include "Model.h"
#include "Texture.h"


class Object
{
public:
	virtual ~Object();
	virtual void Update();
	Model GetModel();
	virtual void Render();

protected:
	Object(GLuint _shader);

	void GetUniformLocation();

	std::unique_ptr<Model> m_pModel;

	glm::vec3 m_position = glm::vec3(0,0,0);
	glm::vec3 m_scale = glm::vec3(1, 1, 1);
	glm::vec3 m_rotation;



	std::unique_ptr<Texture> pTexture;
	GLuint m_iScaleLocation, m_iRotateLocation, m_iTranslateLocation, m_icurrentTimeLocation;
	GLuint m_shader;

private:

};

