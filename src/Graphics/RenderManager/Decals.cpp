#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

//create the decals buffer to modify the gbuffer with the current decals
void RenderManager::CreateDecalsBuffer()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	// configure g-buffer framebuffer
	// ------------------------------
	glGenFramebuffers(1, &decalsFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, decalsFBO);

	// position color buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// color + specular color buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffuse, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &decalsDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, decalsDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, decalsDepth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//pass that will apply the decals onto the gbuffer 
void RenderManager::DecalsDrawPass()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//copy the content of the depth buffer from the gBuffer to the decals depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, decalsFBO); // write to hdr buffer
	glBlitFramebuffer(0, 0, static_cast<GLint>(width), static_cast<GLint>(height),
		0, 0, static_cast<GLint>(width), static_cast<GLint>(height), GL_DEPTH_BUFFER_BIT, GL_NEAREST);


	//compute the prespective matrix, and world to camera matrix
	glm::mat4 PerspMtx = mCamera->perspective();
	glm::mat4 WorldToCamera = mCamera->viewMtx();
	glm::mat4 WorldToPersp = PerspMtx * WorldToCamera;

	//use the shader and pass the parameters
	decalsShaderProgram->Use();

	//use the shader and pass the textures from the gbuffer to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gDepthText);
	decalsShaderProgram->SetUniform("gDepth_in", 0);

	//pass information as uniforms to the shader
	glm::vec2 size = { width, height };
	decalsShaderProgram->SetUniform("size", size);
	decalsShaderProgram->SetUniform("mode", decals_mode);
	decalsShaderProgram->SetUniform("min_angle", glm::radians(min_angle));
	decalsShaderProgram->SetUniform("WorldToView", WorldToCamera);
	decalsShaderProgram->SetUniform("PerspToView", glm::inverse(PerspMtx));

	//render backfaces unless we are in debug mode for cubes
	if (decals_mode != 1)
	{
		glCullFace(GL_FRONT);
		glDepthFunc(GL_LESS);
		glDepthMask(GL_FALSE);
	}
	
	//iterate through the decals
	for (auto& decal : decals)
	{
		//compute the model to persp of the current decal
		glm::mat4x4 ModelToWorld = decal->get_transform();
		decalsShaderProgram->SetUniform("ModelToWorld", ModelToWorld);
		decalsShaderProgram->SetUniform("ModelToPersp", WorldToPersp * ModelToWorld);

		//iterate through the textures of the current decal
		int idx = 1;
		for (auto it : decal->textures)
		{
			glActiveTexture(GL_TEXTURE0 + idx);
			glBindTexture(GL_TEXTURE_2D, it.second);
			decalsShaderProgram->SetUniform(it.first, idx);
			idx++;
		}

		//bind the vertex arry object
		glBindVertexArray(mCube->mMeshes.back().VAOs.back());
		//draw the object depending if the model is using indices to draw or not
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mCube->mMeshes.back().idx_counts.back()), GL_UNSIGNED_SHORT, 0);
	}
	//reset values
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_GREATER);
	glCullFace(GL_BACK);
}