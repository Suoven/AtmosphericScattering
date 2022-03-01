#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>


void RenderManager::CreateSpaceTexture()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//--------------------------------- SUN RENDER FBO ---------------------------------------
	// generate the buffers for Ambien oclusion
	glGenFramebuffers(1, &SunRenderFBO);
	glGenTextures(1, &SunRenderText);

	//bind the texture to the respective FBO and specify the values of it
	glBindTexture(GL_TEXTURE_2D, SunRenderText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, SunRenderFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SunRenderText, 0);

	//--------------------------------- LOAD SKYBOX ----------------------------------------------
	std::vector<std::string> faces
	{
		"data/skybox/space_right.png",
		"data/skybox/space_left.png",
		"data/skybox/space_top.png",
		"data/skybox/space_bot.png",
		"data/skybox/space_front.png",
		"data/skybox/space_back.png"
	};

	glGenTextures(1, &skyboxText);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxText);

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		//load the normal map
		SDL_Surface* surface = IMG_Load(faces[i].c_str());
		if (!surface)
		{
			std::cout << "Cubemap text failed to load at path: " << faces[i] << std::endl;
			break;
		}

		int pixelFormat = GL_RGB;
		if (surface->format->BytesPerPixel == 4)
			pixelFormat = GL_RGBA;

		// Give pixel data to opengl and free surface because OpenGL already has the data
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, pixelFormat, surface->w, surface->h, 0, pixelFormat, GL_UNSIGNED_BYTE, surface->pixels);
		SDL_FreeSurface(surface);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void RenderManager::CreateLookUpTable()
{
	//get the texture dimensions
	glm::vec2 size = { 256, 128 };

	//--------------------------------- TRANSMITTANCE LOOK UP TABLE ---------------------------------------
	// generate the buffers for Ambien oclusion
	glGenFramebuffers(1, &DLookUpFBO);
	glGenTextures(1, &DLookUpText);

	//bind the texture to the respective FBO and specify the values of it
	glBindTexture(GL_TEXTURE_2D, DLookUpText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, DLookUpFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DLookUpText, 0);

	// also check if framebuffers are complete (no need for depth buffer)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	//fill the look up table with a render pass
	glBindFramebuffer(GL_FRAMEBUFFER, DLookUpFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw the scene in the gbuffer
	DensityLookUpTableProgram->Use();

	//pass the texture size
	DensityLookUpTableProgram->SetUniform("size", size);
	
	DensityLookUpTableProgram->SetUniform("planet_center", planet->mPosition);
	DensityLookUpTableProgram->SetUniform("planet_radius", planet_radius);
	DensityLookUpTableProgram->SetUniform("atm_radius", planet_radius + top_atmosphere);
	DensityLookUpTableProgram->SetUniform("HR", HR);
	DensityLookUpTableProgram->SetUniform("HM", HM);
	DensityLookUpTableProgram->SetUniform("Br", Br);
	DensityLookUpTableProgram->SetUniform("Bme", Bme);

	//fill the look up table
	RenderQuad(DensityLookUpTableProgram);

	// THIS IS THE NEXT POINT IN THE IMPLEMENTATION IN ORDER TO GAIN A HUGE PERFORMANCE
	//--------------------------------- INSCATTER LOOK UP TABLE ---------------------------------------
	// generate the buffers for Ambien oclusion
	//glGenFramebuffers(1, &InscatterFBO);
	//glGenTextures(1, &InscatterText);
	//
	////bind the texture to the respective FBO and specify the values of it
	//glBindTexture(GL_TEXTURE_2D_ARRAY, InscatterText);
	//glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 5, 0, GL_RGB, GL_FLOAT, nullptr);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//
	//glBindFramebuffer(GL_FRAMEBUFFER, InscatterFBO);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, InscatterText, 0);
	//
	//GLenum error = glGetError();
	//
	//int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	//if (status != GL_FRAMEBUFFER_COMPLETE)
	//{
	//	std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
	//	throw 0;
	//}
	//
	////draw the scene in the gbuffer
	//InscatterTableProgram->Use();
	//
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, DLookUpText);
	//InscatterTableProgram->SetUniform("Transmittance", 0);
	// 
	////fill the look up table
	//RenderQuad(InscatterTableProgram);
}

void RenderManager::SpaceRenderPass()
{
	//compute the prespective matrix, and world to camera matrix
	glm::mat4 PerspMtx = mCamera->perspective();
	glm::mat4 WorldToCamera = mCamera->viewMtx();
	glm::mat4 WorldToPersp = PerspMtx * WorldToCamera;

	//Render the sun in the texture
	glBindFramebuffer(GL_FRAMEBUFFER, SunRenderFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//------------------------------------------------- RENDER SKYBOX----------------------
	if (RenderSkyBox)
	{
		//get the window dimensions
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		//use the program to draw the texture
		SkyboxRenderProgram->Use();

		//setup for the skybox render
		glDepthFunc(GL_GEQUAL);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		//set the matrices of the skybo
		glm::mat4 view = glm::mat4(glm::mat3(WorldToCamera));
		SkyboxRenderProgram->SetUniform("ViewToProj", PerspMtx * view);

		//pass the skybox texure to the shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxText);
		SkyboxRenderProgram->SetUniform("skybox", 0);

		SkyboxRenderProgram->SetUniform("top_atm", top_atmosphere);
		SkyboxRenderProgram->SetUniform("planet_radius", planet_radius);
		SkyboxRenderProgram->SetUniform("planet_center", planet->mPosition);
		SkyboxRenderProgram->SetUniform("sun_direction", mDirectionalLight->mStats.m_dir);
		SkyboxRenderProgram->SetUniform("camPos", mCamera->position());
		SkyboxRenderProgram->SetUniform("PerspToView", glm::inverse(PerspMtx));
		SkyboxRenderProgram->SetUniform("ViewToWorld", glm::inverse(WorldToCamera));
		SkyboxRenderProgram->SetUniform("size", glm::vec2(width, height));

		for (auto& mesh : mCube->mMeshes)
		{
			//for each primitive
			for (unsigned i = 0; i < mesh.VAOs.size(); i++)
			{
				//bind the vertex arry object
				glBindVertexArray(mesh.VAOs[i]);

				//draw the object depending if the model is using indices to draw or not
				if (mesh.idx_counts.size() != 0)
					glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.idx_counts[i]), mesh.idx_type[i], 0);
				else
					glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vertex_count));
			}
		}

		//reset values for the rest of the objects
		glDepthFunc(GL_GREATER);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
	}
	
	//---------------------------------- RENDER SUN --------------------------------------
	SunRenderProgram->Use();

	//check for errors
	GLenum error = glGetError();

	//for each mesh of the model
	for (auto& mesh : sun->mModel->mMeshes)
	{
		glm::mat4 trans = glm::translate(glm::vec3(mCamera->position() - mDirectionalLight->mStats.m_dir * sun_distance));
		glm::mat4 scale = glm::scale(glm::vec3(sun_size));

		//get model mtx
		glm::mat4 ModelToWorld = trans * scale;

		//get the model to world matrix and pass it to the shader
		glm::mat4 ModelToPersp = WorldToPersp * ModelToWorld;
		SunRenderProgram->SetUniform("ModelToPersp", ModelToPersp);

		//for each primitive
		for (unsigned i = 0; i < mesh.VAOs.size(); i++)
		{
			//bind the vertex arry object
			glBindVertexArray(mesh.VAOs[i]);

			//draw the object depending if the model is using indices to draw or not
			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vertex_count));
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//unbind vao
	glBindVertexArray(0);
	// Unbind the program
	glUseProgram(0);
}