/*!
* \file Camera.cpp
* \brief this file has the definitions of some functions that are used create a camera that moves within the world.
* \details in this file we have functions that move the camera or compute its matrix.
* \author inigo fernandez/ arenas.f@digipen.edu
* \date 17/6/2019
* \copyright Copyright DigiPen Institute Of Technology. All Rights Reserved
*
*/
#include "Camera.h"
#include "../RenderManager/RenderManager.h"

/**
	* Updates the matrices of the camera
	* @return		- void
	*/
void Camera::update()
{
}

/**
* sets the perspective projection of the camera
* @param fovy	- fovy of the frustrum
* @param size	- size of the viewport
* @param near	- near plane
* @param far	- far plane
* @return		- void
*/
void Camera::set_projection(float fovy, glm::vec2 size, float near, float far)
{
	auto gg = glm::translate(glm::vec3(10.0f, 0.0f, 1.0f));
	perspMtx = glm::perspective(glm::radians(fovy), size.x / size.y, near, far);
	
	perspMtx[2][2] = near / (far - near);
	perspMtx[3][2] = (near*far) / (far - near);

	mNear = near;
	mFar = far;
	mFov = fovy;
}

void Camera::set_target(glm::vec3 _targ)
{
	mTarget = _targ;

	glm::vec3 dir = mTarget - mPos;
	if(dir.x != 0.0f || dir.y != 0.0f || dir.z != 0.0f)
		mViewDir = glm::normalize(mTarget - mPos);
}

void Camera::rotate_around(float angle, glm::vec3 dir)
{
	//compute the rotation matrix around the vector
	glm::mat4 rotMtx = glm::rotate(glm::mat4(1.0f), angle, dir);

	//rotate the direction vectors of the object
	mViewDir = glm::normalize(rotMtx * glm::vec4(mViewDir, 0.0f));
	mTarget = mPos + mViewDir;
}

glm::mat4 Camera::perspective()const
{

	return perspMtx;
}