#pragma once
#include <array>
#include "../../MyMath.h"

/**
	* have all the information about the view and the perspective matrix
	*/
class Camera
{
public:
	//constructor that initialize all the variables
	Camera() : mPos{ glm::vec3(0) }, mTarget{ glm::vec3(0) }, perspMtx{ glm::mat4{0} } {}
	void update();

	void rotate_around(float angle, glm::vec3 dir);
	//settors that are used to set the position, the target and the persp matrix
	void set_position(glm::vec3 _pos) { mPos = _pos; }
	void set_target(glm::vec3 _targ);
	void set_projection(float fovy, glm::vec2 size, float near, float far);

	//gettors that are used to get the perspective and view matrix and the position and target of the camera
	glm::mat4 perspective()const;
	glm::mat4 viewMtx()const { return glm::lookAt(mPos, mPos + mViewDir, glm::vec3(0, 1, 0)); }
	glm::vec3 position()const { return mPos; }
	glm::vec3 target()const { return mTarget; }
	glm::vec3 view_dir()const { return mViewDir; }
	float     get_near()const { return mNear; }
	float     get_far()const { return mFar; }
	float     get_fov()const { return mFov; }
private:
	//variables of the camera
	glm::vec3 mViewDir = { 0.0f, 0.0f, 1.0f };
	glm::vec3 mPos;
	glm::vec3 mTarget;
	glm::mat4 perspMtx;
	float mNear;
	float mFar;
	float mFov;
};
