#pragma once
/*!
******************************************************************************
\*file   GameObject.h
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS250
\par     Assignment 2
\date	 10-2-2020

\brief
	this file contains the declaration of the gameobject class

*******************************************************************************/

#include "../MyMath.h"
#include "../Graphics/Model/Model.h"
#include <unordered_map>

class GameObject
{
public:
	GameObject() = default;
	//---------------------------------------------------------------------------------------------------------
	//constructor of the gameobject that initialize the position the rotation the scale and the visibility
	GameObject(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, bool visible = true);

	//---------------------------------------------------------------------------------------------------------
	// functions used to define behaviour of custom gameobjects, 
	// they may be defined in their inherited classes
	virtual void Initialize() {}
	virtual void Update() {}
	virtual void Edit();
	virtual void ShutDown() {}

	//---------------------------------------------------------------------------------------------------------
	// function that creates a child object parented to this one
	GameObject* CreateChild(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, bool visible = true);

	//---------------------------------------------------------------------------------------------------------
	// gets the transform of the current object taking into account all the parents if there is any
	glm::mat4 get_transform()const;

	//---------------------------------------------------------------------------------------------------------
	// gets the transform of all the parents
	glm::mat4 get_parent_transform()const;

	//---------------------------------------------------------------------------------------------------------
	//function used to rotate the object around a vector certain angle
	void RotateAroundVec(const glm::vec3& vec, float angle, const glm::vec3& _pos);

	//this fuction will be used to orientate the vectors towards the target provded
	void LookAt(glm::vec3 target);

	//variables to store the position rotation scale and visibility of the object
	glm::vec3	mPosition;
	glm::vec3	mRotation;
	glm::vec3	mScale;

	//direction vectors of the gameobject
	glm::vec3 mViewAxis =  { 0.0f, 0.0f, -1.0f };
	glm::vec3 mUpAxis =    { 0.0f, 1.0f, 0.0f };
	glm::vec3 mRightAxis = { 1.0f, 0.0f, 0.0f };

	bool mVisible;
	Model* mModel = nullptr;
	glm::vec3 color = glm::vec3(0.27f, 0.455f, 0.56f);
protected:
	//pointer to the parent object
	GameObject* parent = nullptr;
	unsigned			mUID;
	static unsigned mLastUID;
};


class Light : public GameObject
{
public:
	enum class LIGTH_TYPE { POINT, SPOT, DIRECTIONAL };

	//---------------------------------------------------------------------------------------------------------
	//constructor of the gameobject that initialize the position the rotation the scale and the visibility
	Light(LIGTH_TYPE type, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, const glm::vec3& dir, bool visible = true);

	//---------------------------------------------------------------------------------------------------------
	// functions used to define behaviour of custom gameobjects, 
	// they may be defined in their inherited classes
	virtual void Initialize() {}
	virtual void Update() {}
	virtual void Edit();
	virtual void ShutDown() {}

	
	struct LightSourceParameters
	{
		LightSourceParameters() : m_color{ glm::vec3(1.0f) }, m_position{ glm::vec3(0.0f) }, m_dir{ glm::vec3(0.0f) },
									m_intensity{ 1.0f },
									 m_radius{ 30.0f }{}

		LIGTH_TYPE m_Type;
		glm::vec3 m_color;
		glm::vec3 m_position;
		glm::vec3 m_dir;
		float m_intensity;
		float m_radius;
	}mStats;

private:
};

struct Decal : public GameObject
{
	std::unordered_map<std::string, GLuint> textures;
};