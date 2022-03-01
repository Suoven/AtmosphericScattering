/*!
******************************************************************************
\*file   Level1.cpp
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS250
\par     Assignment 2
\date	 10-2-2020

\brief
	this file contains implementation of the level that will initialize
	update and render everything, here is where everything is created

*******************************************************************************/
#include <vector>
#include "Level1.h"
#include "../Graphics/RenderManager/RenderManager.h"
#include "../GameObject/GameObject.h"
#include "../Graphics/Model/Model.h"
#include "../Input/Input.h"
#include <SDL2/SDL.h>
#include <iostream>
#include "../imGui/imgui.h"

namespace LvL1
{
	//varibles that will be used to create the window, the image etc
	static unsigned WindowWidth = 1280u;
	static unsigned WindowHeight = 720u;

	//vector to store all the objects
	static std::vector<GameObject*> mObjects;
	static std::vector<Light*>		mLights;

	//objects that will form the tank
	static GameObject* body;
	static GameObject* mini_body1;
	static GameObject* mini_body2;
	static GameObject* mini_body3;
	static GameObject* mini_body4;
	static GameObject* body2;

	//camera and model
	static Camera* camera = nullptr;

	//booleans to change between modes of the camera or the rendering
	static bool  stop_lights = true;
	static bool  reload_scene = false;
	
	static Light::LIGTH_TYPE  light_type = Light::LIGTH_TYPE::POINT;
	static float light_count = 1.0f;
	static float angle_step = 0.0f;
	static float bodys_dp = 0.0f;

	static bool left_click = false;
	glm::vec2 m_cursor_pos;
	glm::vec2 last_cursor_pos;
}
using namespace LvL1;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////										GAME LOOP FUNCTIONS														/////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Level1_ReLoad()
{
	//destroy objects
	stop_lights = true;
	reload_scene = false;
	RenderMgr.texture_mode = 0;

	RenderMgr.DestroyObjects();

	//load objects 
	RenderMgr.LoadScene();
	reload_scene = false;
	camera = RenderMgr.GetCamera();
}

void Level1_Init()
{
	//initialize the render manager and add a new camera
	RenderMgr.Initialize(WindowWidth, WindowHeight);
	camera = RenderMgr.GetCamera();
}

void Level1_Update()
{	
	//check if reload
	if (reload_scene)
	{
		Level1_ReLoad();
		return;
	}

	//get the input
	GetInput();
}


void Level1_Render()
{
	//update the render manager
	RenderManager::Instance().Update();
}


void Level1_ShutDown()
{
	//shut down the render manager
	RenderMgr.ShutDown();

	//destroy all the objects if there is any
	if (!mObjects.empty())
	{
		for (GameObject* object : mObjects)
			delete object;
	}

	//clear the vector
	mObjects.clear();
	mLights.clear();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////										HELPER FUNCTIONS														/////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//function used to control the tank depending on the input provided
void GetInput()
{
	//check if we need to reload scene
	if (KeyDown(Key::Control) && KeyTriggered(Key::R))
		reload_scene = true;

	//control number ligths
	if (KeyTriggered(Key::P))
		stop_lights = !stop_lights;

	//control wireframe, textue, and normals
	if (KeyTriggered(Key::T))
		if (++RenderMgr.texture_mode > 4)
			RenderMgr.texture_mode = 0;

	//compute the 3 axis of the camera
	glm::vec3 viewaxis = glm::normalize(camera->view_dir());
	glm::vec3 rightaxis = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), viewaxis));
	glm::vec3 upaxis = glm::cross(viewaxis, rightaxis);

	viewaxis *= RenderMgr.camera_speed;
	rightaxis *= RenderMgr.camera_speed;
	upaxis *= RenderMgr.camera_speed;

	float mult = 1.0f;
	//check if we are in fast mode
	if (!KeyDown(Key::LShift))
	{
		mult = 1.0f / 100.0f;
		viewaxis /= 100;
		rightaxis /= 100;
		upaxis /= 100;
	}
	if (KeyDown(Key::Left))
		camera->rotate_around(0.03f, upaxis);
	if (KeyDown(Key::Right))
		camera->rotate_around(-0.03f, upaxis);
	if (KeyDown(Key::Up))
		camera->rotate_around(0.03f, rightaxis);
	if (KeyDown(Key::Down))
		camera->rotate_around(-0.03f, rightaxis);

	glm::vec3 new_pos = glm::vec3(0.0f);
	bool change = false;
	//move upwards
	if (KeyDown(Key::Q))
		new_pos += glm::vec3(0.0f, 1.0f, 0.0f) * RenderMgr.camera_speed * mult;
	//move downwards
	if (KeyDown(Key::E))
		new_pos += - glm::vec3(0.0f, 1.0f, 0.0f) * RenderMgr.camera_speed * mult;
	//move to the left
	if (KeyDown(Key::A))
		new_pos += rightaxis;
	//move to the right
	if (KeyDown(Key::D))
		new_pos += - rightaxis;
	//move forwards
	if (KeyDown(Key::W))
		new_pos += viewaxis;
	//move backwards
	if (KeyDown(Key::S))
		new_pos += - viewaxis;
	if (new_pos != glm::vec3(0.0f))
	{
		camera->set_position(camera->position() + new_pos);
		if(!RenderMgr.CameraInsideBoundaries())
			camera->set_position(camera->position() - new_pos);
		else
			camera->set_target(camera->target() + new_pos);
	}
	

	//check if the left mosue button is pressed
	if (MouseDown(MouseKey::LEFT) && !ImGui::IsAnyItemHovered())
	{
		//store the position of the mouse
		int posx, posy;
		SDL_GetMouseState(&posx, &posy);
		if (!left_click)
			last_cursor_pos = glm::vec2(posx, posy);

		//compute the direction of the movement
		m_cursor_pos = glm::vec2(posx, posy);
		glm::vec2 direction = (m_cursor_pos - last_cursor_pos);

		//compute the rotated vectors that will form the new direction of the camera
		glm::vec3 rotate_vecx = glm::vec3(glm::rotate(glm::radians(-direction.x / 5), upaxis) * glm::vec4(viewaxis, 0.0f));
		glm::vec3 rotate_vecy = glm::vec3(glm::rotate(glm::radians(direction.y / 5), rightaxis) * glm::vec4(viewaxis, 0.0f));

		//set the new target of the camera
		camera->set_target(glm::normalize(rotate_vecx + rotate_vecy) + camera->position());

		//update values
		left_click = true;
		last_cursor_pos = m_cursor_pos;
	}
	else
		left_click = false;
	//restart level
	if (KeyTriggered(Key::F5))
		RenderMgr.RestartShader();
}
