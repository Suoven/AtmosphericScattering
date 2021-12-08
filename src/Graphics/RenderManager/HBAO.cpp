#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

void RenderManager::CreateAOBuffer()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	// generate the buffers for Ambien oclusion
	glGenFramebuffers(1, &AOFBO);
	glGenTextures(1, &AOText);

	//bind the texture to the respective FBO and specify the values of it
	glBindFramebuffer(GL_FRAMEBUFFER, AOFBO);
	glBindTexture(GL_TEXTURE_2D, AOText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, AOText, 0);

	// also check if framebuffers are complete (no need for depth buffer)
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
}

void RenderManager::AmbientOclusionPass()
{
	//bind the buffer and clear it
	glBindFramebuffer(GL_FRAMEBUFFER, AOFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	//use the shader
	AmbientOcclusionProgram->Use();

	//use the shader and pass the textures from the gbuffer to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	AmbientOcclusionProgram->SetUniform("gPosition", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	AmbientOcclusionProgram->SetUniform("gNormals", 1);

	//pass the variables that are used to compute the ambient occlusion of the scene
	AmbientOcclusionProgram->SetUniform("dir_count", dir_count);
	AmbientOcclusionProgram->SetUniform("step_count", step_count);
	AmbientOcclusionProgram->SetUniform("angle_bias", glm::radians(angle_bias));
	AmbientOcclusionProgram->SetUniform("angle_step", glm::radians(360.0f / (float)dir_count));
	AmbientOcclusionProgram->SetUniform("radius", ao_radious);
	AmbientOcclusionProgram->SetUniform("const_attenuation", ao_attenuation);
	AmbientOcclusionProgram->SetUniform("scale", ao_scale);
	AmbientOcclusionProgram->SetUniform("use_surface_normal", use_surface_normal);
	AmbientOcclusionProgram->SetUniform("tangents_method", tangent_method);

	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);
	glm::mat4 PerspMtx = mCamera->perspective();
	AmbientOcclusionProgram->SetUniform("ViewToPersp", PerspMtx);

	//render the ambient full scene as a quad
	RenderQuad(AmbientOcclusionProgram);

	//bilateral blur
	bool horizontal = true;
	BilateralBlurProgram->Use();
	BilateralBlurProgram->SetUniform("TextureData", 0);

	//first pass to blur the data from the bright texture to the horizontal blur texture
	glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
	BilateralBlurProgram->SetUniform("horizontal", horizontal);
	BilateralBlurProgram->SetUniform("threshold", blur_threshold);

	//bind the bright resultant texture to read from
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, AOText);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	BilateralBlurProgram->SetUniform("PosText", 1);

	RenderQuad(BilateralBlurProgram);

	horizontal = !horizontal;
	// blur horizontal on the horizontal texture binding the frame buffer of the vertical and then perform the same process swaping 
	// the buffer and the texture so it is  vertical blur until we perform all the samples and the resultant blurred image is in the 
	// vertical texture
	for (int i = 1; i < bilateral_blur_samples; i++)
	{
		//bind horizontal or vertical blur frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
		BilateralBlurProgram->SetUniform("horizontal", horizontal);

		//bind vertical or horizontal texture to read from previus iteration
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blurTextures[!horizontal]);

		//render blur on the current frame buffer attached teture and swap
		RenderQuad(BilateralBlurProgram);
		horizontal = !horizontal;
	}
}