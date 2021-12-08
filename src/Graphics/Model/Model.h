#pragma once
#include <GL/glew.h>
#include "../../MyMath.h"
#include <vector>
#include <map>
#include <string>

struct Vertex
{
	Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& text) :
		position{ pos }, normal{ norm }, textcoord{ text } {}

	glm::vec3 position = glm::vec3(0);
	glm::vec3 normal = glm::vec3(0);
	glm::vec2 textcoord = glm::vec2(0);

private:

};


struct Mesh 
{
	//---------------------------------------------------------------------------------------------------------
	// gets the transform of the current object taking into account all the parents if there is any
	glm::mat4 get_transform()const;
	//---------------------------------------------------------------------------------------------------------
	// gets the transform of all the parents
	glm::mat4 get_parent_transform()const;
	//---------------------------------------------------------------------------------------------------------
	//function used to rotate the object around a vector certain angle
	void RotateAroundVec(const glm::vec3& vec, float angle, const glm::vec3& _pos);

	//variables to store the position rotation scale and visibility of the object
	glm::vec3	mPosition = glm::vec3(0.0f);
	glm::vec3	mRotation = glm::vec3(0.0f);
	glm::vec3	mScale	  = glm::vec3(1.0f);

	//direction vectors of the gameobject
	glm::vec3 mViewAxis = { 0.0f, 0.0f, -1.0f };
	glm::vec3 mUpAxis = { 0.0f, 1.0f, 0.0f };
	glm::vec3 mRightAxis = { 1.0f, 0.0f, 0.0f };

	glm::mat4x4 TRS = glm::mat4x4(0);

	//parent mesh
	Mesh* parent = nullptr;

	std::vector<GLuint> VAOs;
	std::vector<GLuint> idx_VBOs;
	std::vector<GLuint> VBOs;
	std::vector<unsigned> idx_counts;
	std::vector<unsigned> idx_type;

	std::vector<int> normal_texture;
	std::vector<int> diffuse_texture;
	std::vector<int> specular_texture;
	std::vector<glm::vec4> DiffuseColor;

	unsigned render_mode = GL_TRIANGLES;
	unsigned vertex_count = 0;
	bool use_model_normal = false;
	bool use_model_color = false;
};

struct Model
{
	std::string name;
	std::vector<Mesh> mMeshes;
};

struct Texture
{
	std::string name;
	int sampler_idx;
	int image_idx;
	GLuint texture_handler;
};
