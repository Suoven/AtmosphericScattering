#include "Model.h"
#include "../RenderManager/RenderManager.h"
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <glm/gtx/hash.hpp>
#include <string>

//---------------------------------------------------------------------------------------------------------
	// gets the transform of the current object taking into account all the parents if there is any
glm::mat4 Mesh::get_transform()const
{
	//get the TxRxS matrix of the current object
	glm::mat4 mTrans = glm::translate(mPosition);
	glm::mat4 mSca = glm::scale(mScale);
	glm::mat4 mRot(mRightAxis.x, mRightAxis.y, mRightAxis.z, 0.0f,
					mUpAxis.x, mUpAxis.y, mUpAxis.z, 0.0f,
					-mViewAxis.x, -mViewAxis.y, -mViewAxis.z, 0.0f,
					0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 mTransform = mTrans * mRot * mSca;

	if (TRS != glm::mat4x4(0.0f))
		mTransform = TRS;
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
glm::mat4 Mesh::get_parent_transform()const
{
	//get the TxR matrix of the parent, we dont take the scale because it isnt related to the child
	glm::mat4 mTrans = glm::translate(mPosition);
	mTrans = glm::transpose(mTrans);
	glm::mat4 mRot(mRightAxis.x, mRightAxis.y, mRightAxis.z, 0.0f,
		mUpAxis.x, mUpAxis.y, mUpAxis.z, 0.0f,
		-mViewAxis.x, -mViewAxis.y, -mViewAxis.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 mTransform = mTrans * mRot;

	if (TRS != glm::mat4x4(0.0f))
		mTransform = TRS;

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
//function used to rotate the object around a vector certain angle
void Mesh::RotateAroundVec(const glm::vec3& vec, float angle, const glm::vec3& _pos)
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
	glm::mat4 transMtx = glm::translate(glm::mat4(1.0f), translation);
	rotMtx = transMtx * rotMtx;

	//rotate the object
	mPosition = rotMtx * glm::vec4(mPosition, 1.0f);

	//normalize the direction vectors of the object
	mViewAxis = glm::normalize(mViewAxis);
	mRightAxis = glm::normalize(mRightAxis);
	mUpAxis = glm::normalize(mUpAxis);
}