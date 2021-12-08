/*!
******************************************************************************
\*file   RenderManager.cpp
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS562
\par     Assignment 3
\date	 10-2-2020

\brief
	this file contains the definition of the render manager class, 
	for example we can find the function used to render an object or
	render lines etc

*******************************************************************************/

#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"						// header file
#include <iostream>
#include "../../imGui/imgui.h"
#include "../../imGui/imgui_impl_opengl3.h"
#include "../../imGui/imgui_impl_sdl.h"
#include "../../Input/Input.h"

//function called in the initalize to initialize the SDL window
void RenderManager::InitWindow(const unsigned WindowWidth, const unsigned WindowHeight)
{
	
	//initialize the window, texture and image of the level and the depth buffer
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "Could not initialize SDL: " << SDL_GetError() << std::endl;
		exit(1);
	}
	
	window = SDL_CreateWindow("CS562", 100, 100, WindowWidth, WindowHeight, SDL_WINDOW_OPENGL);
	
	if (window == nullptr)
	{
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	context_ = SDL_GL_CreateContext(window);
	
	if (context_ == nullptr)
	{
		SDL_DestroyWindow(window);
		std::cout << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		SDL_GL_DeleteContext(context_);
		SDL_DestroyWindow(window);
		std::cout << "GLEW Error: Failed to init" << std::endl;
		SDL_Quit();
		exit(1);
	}
	
}


//functions used to intialize, update and shutdown the render manager
void RenderManager::Initialize(const unsigned WindowWidth, const unsigned WindowHeight)
{
	//initialize the SDL window
	InitWindow(WindowWidth, WindowHeight);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	io = &ImGui::GetIO();

	// Setup Platform/Renderer bindings
	ImGui_ImplSDL2_InitForOpenGL(window, context_);
	ImGui_ImplOpenGL3_Init();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	//create the shader programs
	shaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/gBuffer.vs", "data/shaders/gBuffer.fs");
	if (!shaderProgram->Link())	std::cout << shaderProgram->Log() << std::endl;

	ambientShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/AmbientShader.vs", "data/shaders/AmbientShader.fs");
	if (!ambientShaderProgram->Link())	std::cout << ambientShaderProgram->Log() << std::endl;

	lightsShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_point_shading.vs", "data/shaders/lights_shader.fs");
	if (!lightsShaderProgram->Link())	std::cout << lightsShaderProgram->Log() << std::endl;

	differedPointShadingProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_point_shading.vs", "data/shaders/deferred_point_shading.fs");
	if (!differedPointShadingProgram->Link())	std::cout << differedPointShadingProgram->Log() << std::endl;

	differedDirShadingProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_directional_shading.vs", "data/shaders/deferred_directional_shading.fs");
	if (!differedDirShadingProgram->Link())	std::cout << differedDirShadingProgram->Log() << std::endl;

	BrightShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/Brightness.fs");
	if (!BrightShaderProgram->Link())	std::cout << BrightShaderProgram->Log() << std::endl;

	BloomSceneProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/BloomScene.fs");
	if (!BloomSceneProgram->Link())	std::cout << BloomSceneProgram->Log() << std::endl;

	BlurSceneProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/Blur.fs");
	if (!BlurSceneProgram->Link())	std::cout << BlurSceneProgram->Log() << std::endl;

	ShadowMapShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/ShadowMap.vs", "data/shaders/ShadowMap.fs");
	if (!ShadowMapShaderProgram->Link())	std::cout << ShadowMapShaderProgram->Log() << std::endl;

	shadowRenderProg = GLSLProgram::CreateShaderProgram("data/shaders/shadowRenderVtx.vs", "data/shaders/shadowRenderFrag.fs");
	if (!shadowRenderProg->Link())	std::cout << shadowRenderProg->Log() << std::endl;

	decalsShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/DecalsShader.vs", "data/shaders/DecalsShader.fs");
	if (!decalsShaderProgram->Link())	std::cout << decalsShaderProgram->Log() << std::endl;

	AmbientOcclusionProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/AmbientOcclusion.fs");
	if (!AmbientOcclusionProgram->Link())	std::cout << AmbientOcclusionProgram->Log() << std::endl;

	BilateralBlurProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/BilateralGBlur.fs");
	if (!BilateralBlurProgram->Link())	std::cout << BilateralBlurProgram->Log() << std::endl;

	DensityLookUpTableProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/DLookUpTable.fs");
	if (!DensityLookUpTableProgram->Link())	std::cout << DensityLookUpTableProgram->Log() << std::endl;

	InscatterTableProgram = GLSLProgram::CreateShaderProgram("data/shaders/InscatterTable.vs", "data/shaders/InscatterTable.fs");
	InscatterTableProgram->CompileShaderFromFile("data/shaders/InscatterTable.gs", GLSLShaderType::GEOMETRY);
	if (!InscatterTableProgram->Link())	std::cout << InscatterTableProgram->Log() << std::endl;

	//enable back face removal
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	//enable depth buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glDepthFunc(GL_GREATER);
	glClearDepth(0.0f);

	//load default models
	LoadDefaultModels();

	//load the meshes
	LoadScene();

	//generate gBuffer
	GenerateGBuffer();
	//create hdr and brightness texture
	CreateHDRBrightTexture();
	//create blur buffers and textures for bloom
	CreateBlurBuffers();
	//create shadow maps
	CreateShadowMaps();
	//create decals buffer
	CreateDecalsBuffer();
	//create ambient occlusion buffer
	CreateAOBuffer();

	//create look up table for transmittance
	CreateLookUpTable();
}

//this function will generate the depth buffer
void RenderManager::GenerateGBuffer()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	// configure g-buffer framebuffer
	// ------------------------------
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// color + specular color buffer
	glGenTextures(1, &gDiffuse);
	glBindTexture(GL_TEXTURE_2D, gDiffuse);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gDiffuse, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &gDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, gDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT32F, GL_RENDERBUFFER, gDepth);

	// The depth buffer texture
	glGenTextures(1, &gDepthText);
	glBindTexture(GL_TEXTURE_2D, gDepthText);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, gDepthText, 0);
	
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

//create an hdr texture to render onto and a bright texture to use in for the bloom effect
void RenderManager::CreateHDRBrightTexture()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//create frambe buffer for the hdr and bright texture
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	//generate HDR texture and specify the values and characteristics
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);

	//set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTexture, 0);

	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &hdrDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, hdrDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT32F, GL_RENDERBUFFER, hdrDepth);

	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//---------------------------- CREATE FRAME BUFFER FOR BRIGHT TEXTURE ----------------------------
	//create frambe buffer for the hdr and bright texture
	glGenFramebuffers(1, &brightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);

	//generate brightness texture and specify the values and characteristics
	glGenTextures(1, &brightTexture);
	glBindTexture(GL_TEXTURE_2D, brightTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_FLOAT, NULL);

	//set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brightTexture, 0);
}


//this function will update the render manager and draw all the renderables 
void RenderManager::Update()
{
	//get the window dimensions
	int width, height;
	SDL_GetWindowSize(window, &width, &height);

	//set the clear color and clear the color buffer and depth buffer
	glViewport(0, 0, (int)width, (int)height);
	glCullFace(GL_BACK);
	
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	//editor call
	Edit();

	//------------------ShadowMapGeneration--------------------------
	if(mDirectionalLight)
		DrawShadowMaps();
	
	//-------------------GEOMETRY PASS-----------------------------
	// Bind the glsl program and this object's VAOs
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//draw the scene in the gbuffer
	shaderProgram->Use();
	DrawScene(shaderProgram);

	//-----------------DECALS PASS
	if(draw_decals)
		DecalsDrawPass();

	//-------------------LIGHTS PASS-------------------------------
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//copy the content of the depth buffer from the gBuffer to the hdr depth buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO); // write to hdr buffer
	glBlitFramebuffer(0, 0, static_cast<GLint>(width), static_cast<GLint>(height),
					  0, 0, static_cast<GLint>(width), static_cast<GLint>(height), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	
	//compute the ambient texture
	if(use_ambient_occlusion)
		AmbientOclusionPass();

	//enable blending and disable writing into the depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	
	//draw the scene with the lights
	if (texture_mode == 0)
	{
		if(mDirectionalLight)
			DirLightPass();
		PointLightPass();
	}

	//perform the ambien pass
	AmbientPass();

	//disble blend and enable writing into the depth buffer and draw lights models if needed
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	if(draw_lights_model && texture_mode == 0)
		DrawLightsModels();

	//compute the brightness and perform the bloom pass
	ComputeBrightness();
	BloomPass();

	//check if we need to draw debug shadow maps
	if (draw_shadow_maps)
		DrawDebugShadowMaps();

	//render imgui
	ImGui::Render();
	SDL_GL_MakeCurrent(window, context_);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	//swap the window with the double buffering
	SDL_GL_SwapWindow(window);

	//if we need to take a screenshot
	if (takescreenshot)
		TakeScreenShot();
}

//this function will be used to draw the scene providing a shader program
void RenderManager::DrawScene(GLSLProgram* shaderProg, bool use_custom_matrix, bool use_textures)
{
	//compute the prespective matrix, and world to camera matrix
	glm::mat4 PerspMtx = mCamera->perspective();
	glm::mat4 WorldToCamera = mCamera->viewMtx();
	glm::mat4 WorldToPersp = PerspMtx * WorldToCamera;
	
	//check for errors
	GLenum error = glGetError();

	//iterate through all of the renderables rendering all of them
	for (GameObject* object : renderables)
	{
		//security check
		if (object->mModel == nullptr) return;

		//for each mesh of the model
		for (auto& mesh : object->mModel->mMeshes)
		{
			//get model mtx
			glm::mat4 ModelToWorld = object->get_transform(); //* mesh.get_transform();
			if (!use_custom_matrix)
			{
				//get the model to world matrix and pass it to the shader
				glm::mat4 ModelToCamera = WorldToCamera * ModelToWorld;
				glm::mat4 ModelToPersp = WorldToPersp * ModelToWorld;
				glm::mat3 NormalMtx = glm::mat3(glm::transpose(glm::inverse(ModelToCamera)));
				shaderProg->SetUniform("ModelToCamera", ModelToCamera);
				shaderProg->SetUniform("ModelToPersp", ModelToPersp);
				shaderProg->SetUniform("NormalMtx", NormalMtx);
			}
			else
				shaderProg->SetUniform("ModelToWorld", ModelToWorld);

			//for each primitive
			for (unsigned i = 0; i < mesh.VAOs.size(); i++)
			{
				if (use_textures)
				{
					//bind the textures if there are
					if (!mesh.normal_texture.empty() && mesh.normal_texture[i] >= 0)
					{
						// Bind the texture to Texture Units
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, mesh.normal_texture[i]);
						shaderProg->SetUniform("NormalTexture", 0);
					}

					if (!mesh.diffuse_texture.empty() && mesh.diffuse_texture[i] >= 0)
					{
						//set the diffuse color
						shaderProg->SetUniform("DiffuseColor", mesh.DiffuseColor[i]);

						// Bind the texture to Texture Units
						glActiveTexture(GL_TEXTURE1);
						glBindTexture(GL_TEXTURE_2D, mesh.diffuse_texture[i]);
						shaderProg->SetUniform("DiffuseTexture", 1);
					}

					if (!mesh.specular_texture.empty() && mesh.specular_texture[i] >= 0)
					{
						// Bind the texture to Texture Units
						glActiveTexture(GL_TEXTURE2);
						glBindTexture(GL_TEXTURE_2D, mesh.specular_texture[i]);
						shaderProg->SetUniform("SpecularTexture", 2);
					}
				}

				//check if the mesh is providing tangents
				if (!use_custom_matrix)
				{
					shaderProg->SetUniform("use_model_normal", mesh.use_model_normal);
					shaderProg->SetUniform("use_model_color", mesh.use_model_color);
					shaderProg->SetUniform("model_color", object->color);
				}
					

				//bind the vertex arry object
				glBindVertexArray(mesh.VAOs[i]);

				//draw the object depending if the model is using indices to draw or not
				if(mesh.idx_counts.size() != 0)
					glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.idx_counts[i]), mesh.idx_type[i], 0);
				else
					glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh.vertex_count));
			}
		}
	}
	//unbind vao
	glBindVertexArray(0);
	// Unbind the program
	glUseProgram(0);
}

//ambient ligth and different debug textures pass
void RenderManager::AmbientPass()
{
	//use the shader
	ambientShaderProgram->Use();
	
	//use the shader and pass the textures from the gbuffer to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	ambientShaderProgram->SetUniform("gPosition", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	ambientShaderProgram->SetUniform("gNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gDiffuse);
	ambientShaderProgram->SetUniform("gDiffuse", 2);

	//provide usefull data for the shader
	if (texture_mode == 4)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gDepthText);
		ambientShaderProgram->SetUniform("gDepthText", 3);
		ambientShaderProgram->SetUniform("contrast", depth_contrast);
	}

	//provide usefull data for the shader
	if (texture_mode == 5 || texture_mode == 6 || use_ambient_occlusion)
	{
		
		glActiveTexture(GL_TEXTURE4);
		if (texture_mode == 5)	glBindTexture(GL_TEXTURE_2D, AOText);
		else                    glBindTexture(GL_TEXTURE_2D, blurTextures[1]);
		ambientShaderProgram->SetUniform("AOTexture", 4);
	}

	ambientShaderProgram->SetUniform("Use_AO", use_ambient_occlusion);
	ambientShaderProgram->SetUniform("texture_mode", texture_mode);
	ambientShaderProgram->SetUniform("ambient_intensity", ambient_intensity);

	//render the ambient full scene as a quad
	RenderQuad(ambientShaderProgram);
}

//draw the scene with lights
void RenderManager::PointLightPass()
{
	//use the shader
	differedPointShadingProgram->Use();

	//use the shader and pass the textures from the gbuffer to the shader
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	differedPointShadingProgram->SetUniform("gPosition", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	differedPointShadingProgram->SetUniform("gNormal", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gDiffuse);
	differedPointShadingProgram->SetUniform("gDiffuse", 2);

	//provide the size of the window to the shder to compute the UVs
	int width, height;
	GetWindowSize(width, height);
	glm::vec2 size = { width, height };
	differedPointShadingProgram->SetUniform("size", size);

	//get world to perspective matrix
	glm::mat4 WorldToPersp = mCamera->perspective() * mCamera->viewMtx();

	glCullFace(GL_FRONT);
	glDepthFunc(GL_LESS);
	//pass the lights information to the shader and render the sphere surrounding the light
	for (auto& light : mLights)
	{
		//provide the modeltopersp
		glm::mat4 ModelToPersp = WorldToPersp * light->get_transform() * glm::scale(glm::vec3(light->mStats.m_radius * 2.2f));
		differedPointShadingProgram->SetUniform("ModelToPersp", ModelToPersp);
		SetUniformLight(differedPointShadingProgram, light);
	
		//bind the vertex arry object
		glBindVertexArray(mSphere->mMeshes.back().VAOs.back());
		//draw the object depending if the model is using indices to draw or not
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mSphere->mMeshes.back().idx_counts.back()), GL_UNSIGNED_SHORT, 0);
	}
	glCullFace(GL_BACK);
	glDepthFunc(GL_GREATER);
}

//draw the scene with lights
void RenderManager::DirLightPass()
{
	//use the shader
	differedDirShadingProgram->Use();

	//pass the shadow map textures
	for (int i = 0; i < cascade_levels; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, shadowmapsText[i]);
		differedDirShadingProgram->SetUniform("shadowMaps[" + std::to_string(i) + "]", i);
	}

	//use the shader and pass the textures from the gbuffer to the shader
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, gDepthText);
	differedDirShadingProgram->SetUniform("gDepth", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	differedDirShadingProgram->SetUniform("gNormal", 4);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, gDiffuse);
	differedDirShadingProgram->SetUniform("gDiffuse", 5);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, DLookUpText);
	differedDirShadingProgram->SetUniform("ExtintionTexture", 6);
	
	//provide the size of the window to the shder to compute the UVs
	int width, height;
	GetWindowSize(width, height);
	glm::vec2 size = { width, height };
	differedDirShadingProgram->SetUniform("size", size);

	//set atmospheric stats
	differedDirShadingProgram->SetUniform("planet_center", planet->mPosition);
	differedDirShadingProgram->SetUniform("planet_radius", planet_radius);
	differedDirShadingProgram->SetUniform("atm_radius", planet_radius + top_atmosphere);
	differedDirShadingProgram->SetUniform("radius_epsilon", radius_epsilon);
	differedDirShadingProgram->SetUniform("HR", HR);
	differedDirShadingProgram->SetUniform("HM", HM);
	differedDirShadingProgram->SetUniform("Br", Br);
	differedDirShadingProgram->SetUniform("inscatter_steps", inscatter_steps);
	differedDirShadingProgram->SetUniform("Bme", Bme);
	differedDirShadingProgram->SetUniform("Bms", Bms);
	

	//pass the view to world matrix
	differedDirShadingProgram->SetUniform("camPos", mCamera->position());
	differedDirShadingProgram->SetUniform("PerspToView", glm::inverse(mCamera->perspective()));
	differedDirShadingProgram->SetUniform("ViewToWorld", glm::inverse(mCamera->viewMtx()));
	differedDirShadingProgram->SetUniform("blend_dist", blend_distance);
	differedDirShadingProgram->SetUniform("bias", shadow_bias);
	differedDirShadingProgram->SetUniform("pcf_samples", pcf_samples);
	differedDirShadingProgram->SetUniform("CascadeCount", cascade_levels);
	differedDirShadingProgram->SetUniform("draw_cascade_levels", draw_cascade_levels);

	//set directional light info and render the scene
	SetUniformLight(differedDirShadingProgram, mDirectionalLight);
	RenderQuad(differedDirShadingProgram);
}

void RenderManager::DrawLightsModels()
{
	//use the lights shader
	lightsShaderProgram->Use();
	glm::mat4 PerspMtx = mCamera->perspective();
	glm::mat4 WorldToCamera = mCamera->viewMtx();
	glm::mat4 WorldToPersp = PerspMtx * WorldToCamera;

	//draw the light model representation
	for (Light* light : mLights)
	{
		//get the model to world matrix and pass it to the shader
		glm::mat4 ModelToWorld = light->get_transform();
		glm::mat4 ModelToPersp = WorldToPersp * ModelToWorld;
		lightsShaderProgram->SetUniform("ModelToPersp", ModelToPersp);
		lightsShaderProgram->SetUniform("light_color", light->mStats.m_color);

		//for each mesh of the model
		for (auto& mesh : light->mModel->mMeshes)
		{
			//for each primitive
			for (unsigned i = 0; i < mesh.VAOs.size(); i++)
			{
				//bind the vertex arry object
				glBindVertexArray(mesh.VAOs[i]);
				//draw the object depending if the model is using indices to draw or not
				glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.idx_counts[i]), GL_UNSIGNED_SHORT, 0);
			}
		}
	}
	//unbind vao
	glBindVertexArray(0);
	glUseProgram(0);
}

//this function will delete the gameobjects allocated not free the memory for the models
void RenderManager::DestroyObjects()
{
	//delete the camera
	if (mCamera)
	{
		delete mCamera;
		mCamera = nullptr;
	}
		
	//free all light
	while (!mLights.empty())
	{
		Light* light = mLights.back();
		mLights.pop_back();
		//delete the light
		delete light;
	}

	//free all renderables
	while (!renderables.empty())
	{
		GameObject* obj = renderables.back();
		renderables.pop_back();
		//delete the object
		delete obj;
	}
	lights_count = 0;

	//free all decals
	while (!decals.empty())
	{
		Decal* decal = decals.back();
		decals.pop_back();

		//destroy the texture
		for (auto it : decal->textures)
			glDeleteTextures(1, &it.second);
		
		//free the memory
		delete decal;
	}
}

//this function will unload all the objects allocated
void RenderManager::UnLoadModels()
{
	// Delete the VAOs VBOs
	for (auto& it : mModels)
	{
		//unload all the buffers
		Model* model = it.second;
		UnLoadModel(model);

		//delete the model
		delete model;
	}
	mModels.clear();

	//delete the textures
	for(const auto& text : mTextures)
		glDeleteTextures(1, &text.texture_handler);

	//delete the camera 
	DeleteCamera();
	mCamera = nullptr;
}

//this function will be called at the end of the program to delete and finish everything
void RenderManager::ShutDown()
{
	//free all the objects
	DestroyObjects();
	UnLoadModels();

	//destroy gBuffer
	glDeleteBuffers(1, &gBuffer);
	glDeleteBuffers(1, &gDepth);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gDiffuse);
	glDeleteTextures(1, &gDepthText);

	//destroy hdr buffer
	glDeleteBuffers(1, &hdrFBO);
	glDeleteBuffers(1, &brightFBO);
	glDeleteBuffers(1, &hdrDepth);
	glDeleteTextures(1, &hdrTexture);
	glDeleteTextures(1, &brightTexture);

	//destroy the blur buffers
	glDeleteBuffers(2, blurFBO);
	glDeleteTextures(2, blurTextures);

	//destroy shadow map
	DestroyShadowMaps();

	//destroy decls buffer
	glDeleteBuffers(1, &decalsFBO);
	glDeleteBuffers(1, &decalsDepth);

	//destroy look up table
	glDeleteBuffers(1, &DLookUpFBO);
	glDeleteTextures(1, &DLookUpText);

	//delete the shader programs
	delete shaderProgram;
	delete differedPointShadingProgram;
	delete differedDirShadingProgram;
	delete lightsShaderProgram;
	delete ambientShaderProgram;
	delete BloomSceneProgram;
	delete BlurSceneProgram;
	delete BrightShaderProgram;
	delete ShadowMapShaderProgram;
	delete shadowRenderProg;
	delete decalsShaderProgram;
	delete DensityLookUpTableProgram;
	delete InscatterTableProgram;

	//Clean up
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	//shut down the SDL window
	SDL_GL_DeleteContext(context_);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

//function used to add a new renderable object to the render manager so it will get drawn
void RenderManager::AddRenderable(GameObject* _renderable)
{
	//check if the renderable is valid
	if (!_renderable) return;
	
	//add a new renderble and set its mesh pointer to the plane model by default
	if(_renderable->mModel == nullptr)
		_renderable->mModel = mModels.begin()->second;
	renderables.push_back(_renderable);
}

//it deletes and frees a renderable
void RenderManager::DeleteRenderable(GameObject* _renderable)
{
	//check if the model is valid
	if (!_renderable) return;
	auto it = std::find(renderables.begin(), renderables.end(), _renderable);
	if (it == renderables.end()) return;

	//delete the light
	renderables.erase(it);
	delete _renderable;
}

//function used to restart the shader programs
void RenderManager::RestartShader()
{
	//restart the shaders programs
	if (shaderProgram)
		delete shaderProgram;
	if (differedPointShadingProgram)
		delete differedPointShadingProgram;
	if (differedDirShadingProgram)
		delete differedDirShadingProgram;
	if (lightsShaderProgram)
		delete lightsShaderProgram;
	if (ambientShaderProgram)
		delete ambientShaderProgram;
	if (BloomSceneProgram)
		delete BloomSceneProgram;
	if (BlurSceneProgram)
		delete BlurSceneProgram;
	if (BrightShaderProgram)
		delete BrightShaderProgram;
	if (ShadowMapShaderProgram)
		delete ShadowMapShaderProgram;
	if (decalsShaderProgram)
		delete decalsShaderProgram;
	if (AmbientOcclusionProgram)
		delete AmbientOcclusionProgram;
	if (BilateralBlurProgram)
		delete BilateralBlurProgram;
	if (DensityLookUpTableProgram)
		delete DensityLookUpTableProgram;
	if (InscatterTableProgram)
		delete InscatterTableProgram;

	//recreate the shader programs
	shaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/gBuffer.vs", "data/shaders/gBuffer.fs");
	if (!shaderProgram->Link())	std::cout << shaderProgram->Log() << std::endl;

	ambientShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/AmbientShader.vs", "data/shaders/AmbientShader.fs");
	if (!ambientShaderProgram->Link())	std::cout << ambientShaderProgram->Log() << std::endl;

	lightsShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_point_shading.vs", "data/shaders/lights_shader.fs");
	if (!lightsShaderProgram->Link())	std::cout << lightsShaderProgram->Log() << std::endl;

	differedPointShadingProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_point_shading.vs", "data/shaders/deferred_point_shading.fs");
	if (!differedPointShadingProgram->Link())	std::cout << differedPointShadingProgram->Log() << std::endl;

	differedDirShadingProgram = GLSLProgram::CreateShaderProgram("data/shaders/deferred_directional_shading.vs", "data/shaders/deferred_directional_shading.fs");
	if (!differedDirShadingProgram->Link())	std::cout << differedDirShadingProgram->Log() << std::endl;

	BrightShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/Brightness.fs");
	if (!BrightShaderProgram->Link())	std::cout << BrightShaderProgram->Log() << std::endl;

	BloomSceneProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/BloomScene.fs");
	if (!BloomSceneProgram->Link())	std::cout << BloomSceneProgram->Log() << std::endl;

	BlurSceneProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/Blur.fs");
	if (!BlurSceneProgram->Link())	std::cout << BlurSceneProgram->Log() << std::endl;

	ShadowMapShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/ShadowMap.vs", "data/shaders/ShadowMap.fs");
	if (!ShadowMapShaderProgram->Link())	std::cout << ShadowMapShaderProgram->Log() << std::endl;

	decalsShaderProgram = GLSLProgram::CreateShaderProgram("data/shaders/DecalsShader.vs", "data/shaders/DecalsShader.fs");
	if (!decalsShaderProgram->Link())	std::cout << decalsShaderProgram->Log() << std::endl;

	AmbientOcclusionProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/AmbientOcclusion.fs");
	if (!AmbientOcclusionProgram->Link())	std::cout << AmbientOcclusionProgram->Log() << std::endl;

	BilateralBlurProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/BilateralGBlur.fs");
	if (!BilateralBlurProgram->Link())	std::cout << BilateralBlurProgram->Log() << std::endl;

	DensityLookUpTableProgram = GLSLProgram::CreateShaderProgram("data/shaders/Vertex.vs", "data/shaders/DLookUpTable.fs");
	if (!DensityLookUpTableProgram->Link())	std::cout << DensityLookUpTableProgram->Log() << std::endl;

	InscatterTableProgram = GLSLProgram::CreateShaderProgram("data/shaders/InscatterTable.vs", "data/shaders/InscatterTable.fs");
	if (!InscatterTableProgram->Link())	std::cout << InscatterTableProgram->Log() << std::endl;
}

//create a new light and returns it
Light* RenderManager::CreateLight(Light::LIGTH_TYPE type, const glm::vec3& pos, const glm::vec3& scale, const glm::vec3& rot, const glm::vec3& color, float radius, const glm::vec3& dir, bool visible)
{
	Light* new_light = new Light(type, pos, scale, rot, dir, visible);

	new_light->mStats.m_color = color;
	new_light->mStats.m_radius = radius;

	mLights.push_back(new_light);
	new_light->mModel = mSphere;
	lights_count++;

	return new_light;
}

//remove the light provided from the lights list and renderable list if needed, it frees the memory
void RenderManager::RemoveLight(Light* light)
{
	//check if the model is valid
	if (!light) return;
	auto it = std::find(mLights.begin(), mLights.end(), light);
	if (it == mLights.end()) return;

	//delete the light
	mLights.erase(it);
	lights_count--;
}

//function used to set the lights information to the shader program provided
void RenderManager::SetUniformLight(GLSLProgram* shader, const Light* light)
{
	glm::mat4 WorldToCamera = mCamera->viewMtx();
	std::string temp = std::string("Light");

	if (light->mStats.m_Type == Light::LIGTH_TYPE::POINT)
	{
		shader->SetUniform(temp + ".radius", light->mStats.m_radius);
		shader->SetUniform(temp + ".color", light->mStats.m_color * light->mStats.m_intensity);
		shader->SetUniform(temp + ".position", glm::vec3(WorldToCamera * glm::vec4(light->mPosition, 1.0f)));
	}
	else if (light->mStats.m_Type == Light::LIGTH_TYPE::DIRECTIONAL)
	{
		shader->SetUniform(temp + ".color", light->mStats.m_color * light->mStats.m_intensity);
		shader->SetUniform(temp + ".direction", glm::vec3(WorldToCamera * glm::vec4(glm::normalize(light->mStats.m_dir), 0.0f)));
		shader->SetUniform(temp + ".world_direction", glm::normalize(light->mStats.m_dir));

		//pass shadow parameters
		for (int i = 0; i < cascade_levels; i++)
		{
			shader->SetUniform("cascade_planes[" + std::to_string(i) + "]", far_planes[i]);
			shader->SetUniform("lightMtx[" + std::to_string(i) + "]", lightMtx[i]);
		}
	}
}

void RenderManager::RenderQuad(GLSLProgram* program)
{
	//draw the quad
	glBindVertexArray(mModels["Quad"]->mMeshes.back().VAOs.back());
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

//update the number of lights
void RenderManager::UpdateLightsCount()
{
	int temp_count = lights_count;
	Color color_struct;
	//free all light
	while (!mLights.empty())
	{
		Light* light = mLights.back();
		mLights.pop_back();
		//delete the light
		delete light;
	}

	//create lights with random color and random position
	for (int i = 0; i < temp_count; i++)
	{
		glm::vec3 pos{ glm::linearRand((int)min_spawn.x, (int)max_spawn.x),glm::linearRand((int)min_spawn.y, (int)max_spawn.y),glm::linearRand((int)min_spawn.z, (int)max_spawn.z)};
		glm::vec3 color = color_struct.colors[glm::linearRand(0, 6)];
		CreateLight(Light::LIGTH_TYPE::POINT, pos, glm::vec3(0.5f), glm::vec3(0), color, lights_radius);
	}
	lights_count = static_cast<int>(mLights.size());
}

//update the movement of lights
void RenderManager::UpdateLightsMovement()
{
	//iterate through every ligth
	for (auto& light : mLights)
	{
		//change direction
		if (glm::abs(light->mPosition.x) > max_spawn.x || light->mPosition.y > max_spawn.y || light->mPosition.y < min_spawn.y || glm::abs(light->mPosition.z) > max_spawn.z)
			light->mStats.m_dir *= -1.0f;

		//update the lights position
		light->mPosition += light->mStats.m_dir * lights_velocity;
	}
}

//editor parameters
void RenderManager::Edit()
{
	ImGui::Begin("Editor");
	ImGui::SliderFloat("Radius Epsilon", &radius_epsilon, 0.0f, 100.0f);
	ImGui::SliderFloat("Inscatter Steps", &inscatter_steps, 1.0f, 1000.0f);
	ImGui::SliderFloat("Camera Speed Scale", &camera_speed_scale, 0.0f, 100.0f);
	float dist = (glm::length(mCamera->position() - planet->mPosition) - planet_radius) * camera_speed_scale;
	camera_speed = dist / top_atmosphere;

	ImGui::SliderFloat2("Sun Rotation", &sun_rotation[0], 0.0f, 360.0f);
	glm::vec3 dir = glm::vec3(0.0f, sinf(glm::radians(sun_rotation.y)), cosf(glm::radians(sun_rotation.y)));

	glm::mat4 rotMtxX = glm::rotate(glm::mat4(1.0f), glm::radians(sun_rotation.x), glm::vec3(0.0f, 1.0f, 0.0f));
	dir = normalize(rotMtxX * glm::vec4(dir, 0.0f));
	mDirectionalLight->mStats.m_dir = -normalize(dir);

	if (ImGui::TreeNode("Scene"))
	{
		const char* modes[7] = { "Scene", "Position", "Normals", "Diffuse", "Depth", "HBAO", "BlurHBAO" };
		if (ImGui::Combo("Texture Mode", &texture_mode, modes, 7, 7))
		{
			switch (texture_mode)
			{
			case 0:
				texture_mode = 0;
				break;
			case 1:
				texture_mode = 1;
				break;
			case 2:
				texture_mode = 2;
				break;
			case 3:
				texture_mode = 3;
				break;
			case 4:
				texture_mode = 4;
				break;
			}
		}
		if(texture_mode == 4)
			ImGui::SliderFloat("Depth Contrast", &depth_contrast, 0.0f, 1.0f);

		ImGui::SliderFloat("Ambient Intensity", &ambient_intensity, 0.0f, 1.0f);
		ImGui::Checkbox("Gamma", &mb_use_gamma);
		ImGui::SameLine();
		ImGui::Checkbox("Exposure", &mb_use_exposure);

		if (mb_use_gamma)	ImGui::SliderFloat("Gamma Value", &gamma, 1.0f, 2.2f);
		if (mb_use_exposure)	ImGui::SliderFloat("Exposure Value", &exposure, 0.1f, 3.0f);

		ImGui::TreePop();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("Lights"))
	{
		ImGui::Checkbox("Move Lights", &move_lights);
		ImGui::SameLine();
		ImGui::Checkbox("Draw Lights Model", &draw_lights_model);
		if (move_lights)
		{
			UpdateLightsMovement();
			ImGui::SliderFloat("Lights Velocity", &lights_velocity, 0.01f, 2.0f);
		}

		ImGui::SliderFloat("Lights Intensity", &lights_intensity, 0.1f, 10.0f);
		ImGui::SliderFloat("Lights Radius", &lights_radius, 1.0f, 100.0f);
		for (auto& light : mLights)
		{
			light->mStats.m_radius = lights_radius;
			light->mStats.m_intensity = lights_intensity;
		}

		float temp_count = static_cast<float>(lights_count);
		ImGui::SliderFloat("Lights Number", &temp_count, 0.0f, 3000.0f, "%.0f", 1.0f);
		lights_count = static_cast<int>(temp_count);
		if (lights_count != mLights.size() && MouseUp(MouseKey::LEFT))
			UpdateLightsCount();

		
		ImGui::TreePop();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("Bloom"))
	{
		ImGui::Checkbox("Use Bloom", &mb_use_bloom);
		ImGui::SliderFloat("Bright Threshold", &bright_threshold, 0.1f, 2.0f);
		ImGui::SliderInt("Blur Samples", &blur_samples, 2, 100, "%.2d");
		if (blur_samples % 2 != 0) blur_samples--;

		ImGui::TreePop();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("Cascade Shadows"))
	{
		if (mDirectionalLight)
		{
			ImGui::ColorEdit3("Light Color", &mDirectionalLight->mStats.m_color[0]);
			ImGui::DragFloat3("Light Dir", &mDirectionalLight->mStats.m_dir[0], 0.01f);
			ImGui::Separator();
		}

		int temp_levels = cascade_levels;
		ImGui::SliderInt("Cascade Levels", &temp_levels, 1, 5);
		ImGui::SliderFloat("Cascade Linearity", &cascade_linearity, 0.0f, 1.0f);
		ImGui::SliderFloat("Occluder Max Distance", &occluder_max_distance, 0.0f, 10000.0f);
		ImGui::SliderFloat("Blend Distance", &blend_distance, 0.0f, far_planes[0] / 2.0f);
		ImGui::SliderFloat("Bias", &shadow_bias, 0.0f, 0.055f);
		ImGui::SliderInt("pcf samplers", &pcf_samples, 0, 50);
		glm::ivec2 temp_resolution = near_shadowmap_resolution;
		ImGui::InputInt2("Shadow Map Resolution", &temp_resolution[0]);
		ImGui::Checkbox("Draw Shadow Maps", &draw_shadow_maps);
		ImGui::Checkbox("Draw Cascade Levels", &draw_cascade_levels);

		if (temp_levels != cascade_levels || temp_resolution != near_shadowmap_resolution)
		{
			//destroy shadow maps
			DestroyShadowMaps();
			cascade_levels = temp_levels;
			near_shadowmap_resolution = temp_resolution;

			//create new shadow maps
			CreateShadowMaps();
		}
		ImGui::TreePop();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("Decals"))
	{
		ImGui::Checkbox("Draw Decals", &draw_decals);
		const char* modes[5] = { "Texture", "Cube", "CubeProjected" };
		if (ImGui::Combo("Decals Mode", &decals_mode, modes, 3, 6))
		{
			switch (decals_mode)
			{
			case 0:
				decals_mode = 0;
				break;
			case 1:
				decals_mode = 1;
				break;
			case 2:
				decals_mode = 2;
				break;
			}
		}

		ImGui::SliderFloat("Min Angle", &min_angle, 0.0f, 90.0f);
		ImGui::TreePop();
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Separator();
	ImGui::Spacing();

	if (ImGui::TreeNode("HBAO"))
	{
		ImGui::Checkbox("Use Ambient Occlusion", &use_ambient_occlusion);
		if (use_ambient_occlusion)
		{
			const char* modes[7] = { "Scene", "HBAO", "BlurHBAO"};
			if (ImGui::Combo("Texture Mode", &AO_texture_mode, modes, 3, 3))
			{
				switch (AO_texture_mode)
				{
				case 0:
					AO_texture_mode = 0;
					texture_mode = 0;
					break;
				case 1:
					AO_texture_mode = 1;
					texture_mode = 5;
					break;
				case 2:
					AO_texture_mode = 2;
					texture_mode = 6;
					break;
				}
			}
			ImGui::Checkbox("Use Surface Vectors", &use_surface_normal);
			ImGui::Checkbox("Use Tangent Method", &tangent_method);
			ImGui::SliderInt("Direction Count", &dir_count, 0, 20);
			ImGui::SliderInt("step_count", &step_count, 0, 50);
			ImGui::SliderFloat("Radius", &ao_radious, 0.01f, 10.0f);
			ImGui::SliderFloat("Angle Bias", &angle_bias, 0.0f, 90.0f);
			ImGui::SliderFloat("Occlusion Scale", &ao_scale, 0.1f, 10.0f);
			ImGui::SliderFloat("Attenuation", &ao_attenuation, 0.0f, 1.0f);
			ImGui::Separator();
			ImGui::SliderFloat("Blur Threshold", &blur_threshold, 0.0f, 10.0f);
			ImGui::SliderInt("Blur Samples", &bilateral_blur_samples, 2, 100, "%.2d");
			if (bilateral_blur_samples % 2 != 0) bilateral_blur_samples--;

		}
		ImGui::TreePop();
	}
	ImGui::End();
}

