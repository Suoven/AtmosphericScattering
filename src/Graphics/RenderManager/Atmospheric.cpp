#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

void RenderManager::CreateLookUpTable()
{
	//get the texture dimensions
	glm::vec2 size = { 256, 64 };

	//--------------------------------- TRANSMITTANCE LOOK UP TABLE ---------------------------------------
	// generate the buffers for Ambien oclusion
	glGenFramebuffers(1, &DLookUpFBO);
	glGenTextures(1, &DLookUpText);

	//bind the texture to the respective FBO and specify the values of it
	glBindFramebuffer(GL_FRAMEBUFFER, DLookUpFBO);
	glBindTexture(GL_TEXTURE_2D, DLookUpText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	DensityLookUpTableProgram->SetUniform("radius_epsilon", radius_epsilon);
	DensityLookUpTableProgram->SetUniform("HR", HR);
	DensityLookUpTableProgram->SetUniform("HM", HM);
	DensityLookUpTableProgram->SetUniform("Br", Br);
	DensityLookUpTableProgram->SetUniform("Bme", Bme);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D_ARRAY, InscatterText);
	//DensityLookUpTableProgram->SetUniform("gg", 0);

	//fill the look up table
	RenderQuad(DensityLookUpTableProgram);

	//--------------------------------- INSCATTER LOOK UP TABLE ---------------------------------------
	// generate the buffers for Ambien oclusion
	glGenFramebuffers(1, &InscatterFBO);
	glGenTextures(1, &InscatterText);

	//bind the texture to the respective FBO and specify the values of it
	glBindTexture(GL_TEXTURE_2D_ARRAY, InscatterText);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB32F, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y), 5, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, InscatterFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, InscatterText, 0);

	GLenum error = glGetError();

	int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!";
		throw 0;
	}

	//draw the scene in the gbuffer
	InscatterTableProgram->Use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, DLookUpText);
	InscatterTableProgram->SetUniform("Transmittance", 0);
	 
	//fill the look up table
	RenderQuad(InscatterTableProgram);
}