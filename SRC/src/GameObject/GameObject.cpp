/*!
******************************************************************************
\*file   GameObject.cpp
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS250
\par     Assignment 2
\date	 10-2-2020

\brief
	this file contains the definition of some mamber functions of the gameobject 
	class
*******************************************************************************/


#include "GameObject.h"
#include "../Graphics/RenderManager/RenderManager.h"
#include "../imGui/imgui.h"
#include <string>

unsigned GameObject::mLastUID = 0u;
//---------------------------------------------------------------------------------------------------------
//constructor of the gameobject that initializes the position, the scale, the rotation and the visibility
GameObject::GameObject(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, bool visible)
									: mPosition{pos}, mScale{scale} , mRotation{rot}, mVisible{ visible },
									  mViewAxis{0.0f, 0.0f, -1.0f}, mRightAxis{1.0f, 0.0f, 0.0f}, mUpAxis{0.0f, 1.0f, 0.0f}
{
	mUID = mLastUID++;
}


void GameObject::Edit()
{
	ImGui::PushID(std::to_string(mUID).c_str());
	if (ImGui::TreeNode("GameObject"))
	{

		ImGui::TreePop();
	}
	ImGui::PopID();
}
//---------------------------------------------------------------------------------------------------------
// gets the transform of the current object taking into account all the parents if there is any 
glm::mat4 GameObject::get_transform()const
{
	//get the TxRxS matrix of the current object
	glm::mat4 mTrans = glm::translate(mPosition);
	glm::mat4 mSca = glm::scale(mScale);
	glm::mat4 mRot(	mRightAxis.x,	mRightAxis.y,	mRightAxis.z ,	0.0f,
					mUpAxis.x,		mUpAxis.y,		mUpAxis.z,		0.0f,
					-mViewAxis.x,	-mViewAxis.y,	-mViewAxis.z,	0.0f,
					0.0f,			0.0f,			0.0f,			1.0f);
	glm::mat4 mTransform = mTrans * mRot * mSca;

	//if there is any parent we get his transform
	if (parent)
	{
		//apply the transform of the parent to the child transform
		glm::mat4 Ptransform = parent->get_parent_transform();
		mTransform = Ptransform * mTransform;
	}

	//return the resultant transform
	return mTransform;
}

//---------------------------------------------------------------------------------------------------------
// gets the transform of all the parents
glm::mat4 GameObject::get_parent_transform()const
{
	//get the TxR matrix of the parent, we dont take the scale because it isnt related to the child
	glm::mat4 mTrans = glm::translate(mPosition);
	mTrans = glm::transpose(mTrans);
	glm::mat4 mRot(	mRightAxis.x,	mRightAxis.y,	mRightAxis.z,	0.0f,
					mUpAxis.x,		mUpAxis.y,		mUpAxis.z,		0.0f,
					-mViewAxis.x,	-mViewAxis.y,	-mViewAxis.z,	0.0f,
					0.0f,			0.0f,			0.0f,			1.0f);
	glm::mat4 mTransform = mTrans * mRot;

	//if this parent has another parent we get his transform
	if (parent)
	{
		//apply the transform of the parent to the current parent transform
		glm::mat4 PTransform = parent->get_parent_transform();
		mTransform = PTransform * mTransform;
	}
	
	//return the resultant transform
	return mTransform;
}

//---------------------------------------------------------------------------------------------------------
// function that creates a child object parented to this one
GameObject* GameObject::CreateChild(const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, bool visible)
{
	//creates a new object in the position, and with the rotation scale and visibility provided
	GameObject* child = new GameObject(pos, scale, rot, visible);

	//set the parent pointer of the object to the current object and return the child object
	child->parent = this;
	return child;
}

//---------------------------------------------------------------------------------------------------------
//function used to rotate the object around a vector certain angle
void GameObject::RotateAroundVec(const glm::vec3& vec, float angle, const glm::vec3& _pos)
{
	//compute the rotation matrix around the vector
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), angle, vec);

	//rotate the direction vectors of the object
	mViewAxis = rotMtx * glm::vec4(mViewAxis, 0.0f);
	mRightAxis = rotMtx * glm::vec4(mRightAxis, 0.0f);
	mUpAxis = rotMtx * glm::vec4(mUpAxis, 0.0f);

	//vector that will represent the b vector of the transformation
	glm::vec3 translation = glm::vec4(_pos, 1.0f) - (rotMtx * glm::vec4(_pos, 1.0f));

	//set that vector into the matrix tranformation
	glm::mat4 transMtx  = glm::translate(glm::mat4(1.0f), translation);
	rotMtx = transMtx * rotMtx;

	//rotate the object
	mPosition = rotMtx * glm::vec4(mPosition, 1.0f);

	//normalize the direction vectors of the object
	mViewAxis = glm::normalize(mViewAxis);
	mRightAxis = glm::normalize(mRightAxis);
	mUpAxis = glm::normalize(mUpAxis);
}

//this fuction will be used to orientate the vectors towards the target provded
void GameObject::LookAt(glm::vec3 target)
{
	mViewAxis = glm::normalize(target - mPosition);
	mRightAxis = glm::normalize(glm::cross(mViewAxis , glm::vec3(0.0f, 1.0f, 0.0f)));
	mUpAxis = glm::cross(mRightAxis , mViewAxis);
}


Light::Light(LIGTH_TYPE type, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, const glm::vec3& dir, bool visible) :
	GameObject(pos, scale, rot, visible), mStats{LightSourceParameters()}
{
	if(type == LIGTH_TYPE::POINT)
		mStats.m_dir[glm::linearRand(0, 2)] = 1.0f;
	else
		mStats.m_dir = dir;

	mStats.m_Type = type;
	mStats.m_position = glm::vec4(pos, 1.0f);
}


void Light::Edit()
{
	ImGui::PushID(std::to_string(mUID).c_str());
	if (ImGui::TreeNode("Light"))
	{
		ImGui::ColorEdit3("color", reinterpret_cast<float*>(&mStats.m_color));
		ImGui::TreePop();
	}
	ImGui::PopID();
}