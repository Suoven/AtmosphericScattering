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

void RenderManager::ComputeBrightness()
{
	//bind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//compute the brightness of the scene and copy it into the texture
	BrightShaderProgram->Use();
	// Bind the texture to Texture Units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	BrightShaderProgram->SetUniform("HDRTexture", 0);
	BrightShaderProgram->SetUniform("bright_threshold", bright_threshold);

	RenderQuad(BrightShaderProgram);
}

//pass that will apply bloom in the final image
void RenderManager::BloomPass()
{
	bool horizontal = true;
	//check if we are using bloom
	if (mb_use_bloom && texture_mode == 0)
	{
		BlurSceneProgram->Use();
		BlurSceneProgram->SetUniform("TextureData", 0);

		//first pass to blur the data from the bright texture to the horizontal blur texture
		glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
		BlurSceneProgram->SetUniform("horizontal", horizontal);

		//bind the bright resultant texture to read from
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, brightTexture);
		RenderQuad(BlurSceneProgram);

		horizontal = !horizontal;
		// blur horizontal on the horizontal texture binding the frame buffer of the vertical and then perform the same process swaping 
		// the buffer and the texture so it is  vertical blur until we perform all the samples and the resultant blurred image is in the 
		// vertical texture
		for (int i = 1; i < blur_samples; i++)
		{
			//bind horizontal or vertical blur frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER, blurFBO[horizontal]);
			BlurSceneProgram->SetUniform("horizontal", horizontal);

			//bind vertical or horizontal texture to read from previus iteration
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, blurTextures[!horizontal]);

			//render blur on the current frame buffer attached teture and swap
			RenderQuad(BlurSceneProgram);
			horizontal = !horizontal;
		}
	}

	//render onto default buffer combining the scene image with the bloom texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	BloomSceneProgram->Use();

	//set the two textures used for the final image
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	BloomSceneProgram->SetUniform("HDRTexture", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, blurTextures[!horizontal]);
	BloomSceneProgram->SetUniform("BloomTexture", 1);

	//set the parameters to change the final image
	BloomSceneProgram->SetUniform("use_bloom", mb_use_bloom);
	BloomSceneProgram->SetUniform("use_gamma", mb_use_gamma);
	BloomSceneProgram->SetUniform("use_exposure", mb_use_exposure);
	BloomSceneProgram->SetUniform("exposure", exposure);
	BloomSceneProgram->SetUniform("gamma", gamma);

	//render the final image
	RenderQuad(BloomSceneProgram);
}