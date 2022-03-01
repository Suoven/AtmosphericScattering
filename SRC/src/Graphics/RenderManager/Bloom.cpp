#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

//create blur buffers and textures for bloom
void RenderManager::CreateBlurBuffers()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	// generate the buffers for blurring and the textures to render onto
	glGenFramebuffers(2, blurFBO);
	glGenTextures(2, blurTextures);
	for (unsigned int i = 0; i < 2; i++)
	{
		//bind the texture to the respective FBO and specify the values of it
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[i]);
		glBindTexture(GL_TEXTURE_2D, blurTextures[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurTextures[i], 0);

		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}
}

//pass that will apply bloom in the final image
void RenderManager::BloomPass()
{
	bool horizontal = true;

	//render onto default buffer combining the scene image with the bloom texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BloomSceneProgram->Use();

	//set the two textures used for the final image
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	BloomSceneProgram->SetUniform("HDRTexture", 0);

	//set the parameters to change the final image
	BloomSceneProgram->SetUniform("use_gamma", mb_use_gamma);
	BloomSceneProgram->SetUniform("use_exposure", mb_use_exposure);
	BloomSceneProgram->SetUniform("exposure", exposure);
	BloomSceneProgram->SetUniform("gamma", gamma);

	//render the final image
	RenderQuad(BloomSceneProgram);
}