#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

//create the shadow map textures
void RenderManager::CreateShadowMaps()
{
	//iterate through all the cascade levels creating them
	glm::ivec2 resolution = near_shadowmap_resolution;
	for (int i = 0; i < cascade_levels; i++, resolution /= 2)
	{
		GLuint depthText, depthFBO;
		// The depth buffer texture
		glGenTextures(1, &depthText);
		glBindTexture(GL_TEXTURE_2D, depthText);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, resolution.x, resolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		// Create and set up the FBO
		glGenFramebuffers(1, &depthFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthText, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		//check for any problem
		GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result == GL_FRAMEBUFFER_COMPLETE)
			printf("Framebuffer is complete.\n");
		else
			printf("Framebuffer is not complete.\n");

		//store the handlers
		shadowmapsFBO.push_back(depthFBO);
		shadowmapsText.push_back(depthText);

		//initialize light matrix and far planes
		lightMtx.push_back(glm::mat4x4(0));
		far_planes.push_back(0.0f);
	}
}

//pass that fill the shadow maps with data
void RenderManager::DrawShadowMaps()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//compute shadows matrices and far planes
	ComputeShadowsMtx();

	glm::ivec2 resoulution = near_shadowmap_resolution;
	//iterate through the shadow maps
	for (int i = 0; i < cascade_levels; i++, resoulution /= 2)
	{
		//bind the frame buffer and clear it and set the current resolution of the shadow map
		glBindFramebuffer(GL_FRAMEBUFFER, shadowmapsFBO[i]);
		glViewport(0, 0, resoulution.x, resoulution.y);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		//render the shadow map
		ShadowMapShaderProgram->Use();
		ShadowMapShaderProgram->SetUniform("WorldToLightPersp", lightMtx[i]);
		DrawScene(ShadowMapShaderProgram, true, false);
	}

	//reset the viewport
	glViewport(0, 0, width, height);
	glCullFace(GL_BACK);
}

std::vector<glm::vec4> RenderManager::ComputeFrustrumPoints(const glm::mat4& projtoview)
{
	glm::mat4x4 inv = glm::inverse(projtoview);

	//since the frustrum corners that we see rendered are in ndc space from -1,1 we can get the 
	// world corners by performing the invers operations for the view and projection matrix 
	std::vector<glm::vec4> frustumCorners;
	for (int x = -1; x <= 1; x += 2)
		for (int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2)
			{
				//compute the point in world space and perform perspective division
				glm::vec4 corner = inv * glm::vec4(x, y, z, 1.0f);
				frustumCorners.push_back(corner / corner.w);
			}
	return frustumCorners;
}

glm::mat4 RenderManager::ComputeLightProjMtx(const float nearPlane, const float farPlane)
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//compute the perspective matrix of the current subfrustrum and get the corners of that subfrustrum in world space
	glm::mat4x4 persp = glm::perspective(glm::radians(mCamera->get_fov()), (float)width / (float)height, nearPlane, farPlane);
	std::vector<glm::vec4> corners = ComputeFrustrumPoints(persp * mCamera->viewMtx());

	//compute the average center of the subfrustrum
	glm::vec3 center = glm::vec3(0, 0, 0);
	for (auto& v : corners)
		center += glm::vec3(v);
	center /= corners.size();

	//compute the view matrix of the light direction with the current subfrustrum
 	glm::mat4x4 lightView = glm::lookAt(center - mDirectionalLight->mStats.m_dir, center, glm::vec3(0.0f, 1.0f, 0.0f));

	float minX = std::numeric_limits<float>::max();
	float maxX = -std::numeric_limits<float>::max();
	float minY = std::numeric_limits<float>::max();
	float maxY = -std::numeric_limits<float>::max();
	float minZ = std::numeric_limits<float>::max();
	float maxZ = -std::numeric_limits<float>::max();

	//get the max and min point of the aabb that encapsulate the subfrustrum
	for (const auto& v : corners)
	{
		const auto trf = lightView * v;
		minX = std::min(minX, trf.x);
		maxX = std::max(maxX, trf.x);
		minY = std::min(minY, trf.y);
		maxY = std::max(maxY, trf.y);
		minZ = std::min(minZ, trf.z);
		maxZ = std::max(maxZ, trf.z);
	}

	//make the x-y equal in length
	float maxdist = std::max(maxX - minX, maxY - minY);
	maxX = minX + maxdist;
	maxY = minY + maxdist;

	//apply occluder max distance
	maxZ += occluder_max_distance;
	minZ -= occluder_max_distance;

	glm::mat4x4 Result(1);
	Result[0][0] = 2.0f / (maxX - minX);
	Result[1][1] = 2.0f / (maxY - minY);
	Result[2][2] = 1.0f / (maxZ - minZ);//-2.0f / (maxZ - minZ);//
	Result[3][0] = -(maxX + minX) / (maxX - minX);
	Result[3][1] = -(maxY + minY) / (maxY - minY);
	Result[3][2] = maxZ / (maxZ - minZ);//-(maxZ + minZ) / (maxZ - minZ);//

	//return the matrix that perform the transformation from world to light projection
	return Result * lightView;//glm::ortho(minX, maxX, minY, maxY, minZ, maxZ) * lightView;
}

void RenderManager::ComputeShadowsMtx()
{
	//get initial parameters
	float n = mCamera->get_near();
	float f = shadow_far;//mCamera->get_far();
	float current_near = n, current_far = f;

	//iterate through the shadow maps
	for (int i = 0; i < cascade_levels; i++)
	{
		//compute the far distance for the current shadow map
		current_far = cascade_linearity * (n * glm::pow(f / n, (float)(i + 1) / (float)cascade_levels));
		current_far += (1 - cascade_linearity) * (n + ((float)(i + 1) / (float)cascade_levels) * (f - n));

		//store the orthogonal matrix
		lightMtx[i] = ComputeLightProjMtx(current_near - blend_distance, current_far + blend_distance);

		//update the near value and store the far plane
		current_near = current_far;
		far_planes[i] = current_far;
	}
}

void RenderManager::DestroyShadowMaps()
{
	lightMtx.clear();
	far_planes.clear();

	//destroy the shadow buffers
	glDeleteBuffers(cascade_levels, shadowmapsFBO.data());
	glDeleteTextures(cascade_levels, shadowmapsText.data());

	//clear the vectors
	shadowmapsFBO.clear();
	shadowmapsText.clear();
}

void RenderManager::DrawDebugShadowMaps()
{
	//get window values
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	float scale = 40.0f;
	float ratioY = (float)width / (scale * 100.0f);
	float ratioX = (float)height / (scale * 100.0f);

	//use the program to draw the texture
	shadowRenderProg->Use();
	for (int i = 0; i < cascade_levels; i++)
	{
		//get position in screen
		glm::mat4 ModelToWorld = glm::translate(glm::vec3(-1.0f + (ratioX)+ratioX * i * 2.0f, -1.0f + (ratioY), 0.05f)) * glm::scale(glm::vec3(ratioX, ratioY, 1));

		//draw the texture in the down-left corner of the screen
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shadowmapsText[i]);
		shadowRenderProg->SetUniform("shadowMap", 1);
		shadowRenderProg->SetUniform("ModelToPersp", ModelToWorld);

		//draw the quad
		glBindVertexArray(mModels["Quad"]->mMeshes.back().VAOs.back());
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}