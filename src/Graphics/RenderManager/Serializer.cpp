#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../../Serializer/tiny_gltf.h"
#include "../../OpenGL/GLSLProgram.h"
#include "RenderManager.h"
#include <iostream>

//take a screen shot
void RenderManager::TakeScreenShot()
{
	//take a screenshot
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int x = viewport[0];
	int y = viewport[1];
	int width = viewport[2];
	int height = viewport[3];

	std::vector<unsigned char> imageData(width * height * 4, 0);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_FRONT);
	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imageData.data());
	stbi_flip_vertically_on_write(true);
	stbi_write_png(screenshot_path.c_str(), width, height, 4, imageData.data(), 0);
}

//load default models
void RenderManager::LoadDefaultModels()
{
	//---------------------------------LOAD QUAD--------------------------
	Model* quad = new Model();
	Mesh quad_mesh;
	GLuint quadVAO, quadVBO;

	float quadVertices[] = {
		// positions        // texture Coords
		1.0f, 1.0f,   0.5f,  1.0f, 1.0f,
		-1.0f, -1.0f, 0.5f,  0.0f, 0.0f,
		1.0f, -1.0f,  0.5f,  1.0f, 0.0f,
		-1.0f, 1.0f,  0.5f,  0.0f, 1.0f,
		-1.0f, -1.0f, 0.5f,  0.0f, 0.0f,
		1.0f, 1.0f,   0.5f,  1.0f, 1.0f,
	};
	// gen vao and vb
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);

	//Provide the data for the positions and texture coordinates
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

	//set the attributes location for use in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	//add handlers to the model
	quad_mesh.VAOs.push_back(quadVAO);
	quad_mesh.VBOs.push_back(quadVBO);
	quad->mMeshes.push_back(quad_mesh);

	//add the quad to the list of models
	mModels["Quad"] = quad;
	mQuad = quad;

	//--------------------------LOAD SPHERE------------------------------
	//load the sphere
	LoadSphereModel();
	mSphere = LoadModel("./data/gltf/Sphere.gltf");

	//--------------------------LOAD CUBE------------------------------
	//load the cube
	mCube = LoadModel("./data/gltf/Cube.gltf");
}

bool RenderManager::LoadScene()
{
	//open the scene json
	std::ifstream scene_file(load_path.c_str());
	if (!scene_file.is_open() || !scene_file.good()) return false;
	nlohmann::json j;

	//LOAD CAMERA
	scene_file >> j;
	if (j.find("camera") != j.end())
	{
		if (mCamera == nullptr)
			mCamera = new Camera();

		float far_plane = j["camera"]["far"];
		float near_plane = j["camera"]["near"];
		float fovy = j["camera"]["FOVy"];
		int width, height;
		SDL_GetWindowSize(window, &width, &height);
		mCamera->set_projection(fovy, glm::vec2(width, height), near_plane, far_plane);

		glm::vec3 pos;
		pos.x = j["camera"]["translate"]["x"];
		pos.y = j["camera"]["translate"]["y"];
		pos.z = j["camera"]["translate"]["z"];
		mCamera->set_position(pos);

		glm::vec3 rot;
		rot.x = j["camera"]["rotate"]["x"];
		rot.y = j["camera"]["rotate"]["y"];

		mCamera->rotate_around(glm::radians(rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 rightaxis = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), mCamera->view_dir()));
		mCamera->rotate_around(glm::radians(rot.x), rightaxis);
	}

	//LOAD PLANET AND SUN
	planet = new GameObject();
	planet->mModel = CustomSphere;
	planet->mScale = glm::vec3(6370.0f * 2.0f);
	AddRenderable(planet);

	sun = new GameObject();
	sun->mModel = CustomSphere;
	sun->mScale = glm::vec3(100.0f * 2.0f);
	sun->color = glm::vec3(1.0f, 0.0f, 0.0f);
	AddRenderable(sun);

	//LOAD OBJECTS
	nlohmann::json& objects = *j.find("objects");
	for (auto it = objects.begin(); it != objects.end(); it++)
	{
		//get the path of the model
		nlohmann::json& val = *it;
		std::string path("./");
		std::string mesh_name = val["mesh"];
		path += mesh_name;

		//create a new object
		GameObject* new_object = new GameObject();
		new_object->mModel = LoadModel(path);

		glm::vec3 rot;
		rot.x = val["rotate"]["x"];
		rot.y = val["rotate"]["y"];
		rot.z = val["rotate"]["z"];

		new_object->RotateAroundVec(glm::vec3(0.0f, 0.0f, -1.0f), glm::radians(rot.z), glm::vec3(0));
		new_object->RotateAroundVec(glm::vec3(0.0f, 1.0f, 0.0f) , glm::radians(rot.y), glm::vec3(0));
		new_object->RotateAroundVec(glm::vec3(1.0f, 0.0f, 0.0f) , glm::radians(rot.x), glm::vec3(0));

		new_object->mPosition.x = val["translate"]["x"];
		new_object->mPosition.y = val["translate"]["y"];
		new_object->mPosition.z = val["translate"]["z"];

		new_object->mScale.x = val["scale"]["x"];
		new_object->mScale.y = val["scale"]["y"];
		new_object->mScale.z = val["scale"]["z"];

		AddRenderable(new_object);
	}

	//LOAD POINT LIGHTS
	if (j.find("lights") != j.end())
	{
		nlohmann::json& lights = *j.find("lights");
		for (auto it = lights.begin(); it != lights.end(); it++)
		{
			//get the path of the model
			nlohmann::json& val = *it;

			glm::vec3 pos;
			glm::vec3 color;
			float radius;

			pos.x = val["position"]["x"];
			pos.y = val["position"]["y"];
			pos.z = val["position"]["z"];

			color.x = val["color"]["x"];
			color.y = val["color"]["y"];
			color.z = val["color"]["z"];

			radius = val["radius"];

			CreateLight(Light::LIGTH_TYPE::POINT, pos, glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0), color, radius);
		}
	}
	
	//LOAD DIRECTIONAL LIGHT
	if (j.find("directional_light") != j.end())
	{
		glm::vec3 color;
		glm::vec3 dir;

		dir.x = j["directional_light"]["direction"]["x"];
		dir.y = j["directional_light"]["direction"]["y"];
		dir.z = j["directional_light"]["direction"]["z"];

		color.x = j["directional_light"]["color"]["x"];
		color.y = j["directional_light"]["color"]["y"];
		color.z = j["directional_light"]["color"]["z"];

		mDirectionalLight = new Light(Light::LIGTH_TYPE::DIRECTIONAL, glm::vec3(0), glm::vec3(1.0f), glm::vec3(0), dir);
		mDirectionalLight->mStats.m_color = color;
		mDirectionalLight->mModel = mSphere;

		GameObject* obj2 = new GameObject;
		obj2->mModel = CustomSphere;
		obj2->color = glm::vec3(1, 1, 1);
		obj2->mScale = glm::vec3(1000.0f);
		obj2->mPosition = glm::vec3(0.0f, 0.0f, 0.0f) - dir * 20000.0f;
		AddRenderable(obj2);
	}

	//LOAD DECALS
	if (j.find("decals") != j.end())
	{
		nlohmann::json& json_decals = *j.find("decals");
		for (auto it = json_decals.begin(); it != json_decals.end(); it++)
		{
			//get the path of the model
			nlohmann::json& val = *it;
			Decal* new_decal = new Decal;

			glm::vec3 rot;
			rot.x = val["rotate"]["x"];
			rot.y = val["rotate"]["y"];
			rot.z = val["rotate"]["z"];

			new_decal->RotateAroundVec(glm::vec3(0.0f, 0.0f, -1.0f), glm::radians(rot.z), glm::vec3(0));
			new_decal->RotateAroundVec(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rot.y), glm::vec3(0));
			new_decal->RotateAroundVec(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rot.x), glm::vec3(0));

			//set the world parameters of the decal
			new_decal->mPosition.x = val["translate"]["x"];
			new_decal->mPosition.y = val["translate"]["y"];
			new_decal->mPosition.z = val["translate"]["z"];

			new_decal->mScale.x = val["scale"]["x"];
			new_decal->mScale.y = val["scale"]["y"];
			new_decal->mScale.z = val["scale"]["z"];

			//set the texture values
			const unsigned texture_count = 3;
			std::string texture_names[texture_count] = { "diffuse", "normal", "metallic" };
			GLuint texture_handlers[texture_count];
			glGenTextures(3, texture_handlers);

			//iterate throgh the textures
			for (unsigned i = 0; i < texture_count; i++)
			{
				//bind the texture
				glBindTexture(GL_TEXTURE_2D, texture_handlers[i]);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				//load the texture
				std::string substr = "./";
				std::string path = val[texture_names[i].c_str()];
				path = substr + path;
				SDL_Surface* surface = IMG_Load(path.c_str());
				if (!surface)
					continue;

				int pixelFormat = GL_RGB;
				if (surface->format->BytesPerPixel == 4)
					pixelFormat = GL_RGBA;

				// Give pixel data to opengl and free surface because OpenGL already has the data
				glTexImage2D(GL_TEXTURE_2D, 0, pixelFormat, surface->w, surface->h, 0, pixelFormat, GL_UNSIGNED_BYTE, surface->pixels);
				glGenerateMipmap(GL_TEXTURE_2D);
				SDL_FreeSurface(surface);

				//set the texture handler to the decal
				new_decal->textures[texture_names[i]] = texture_handlers[i];
			}
			//store the new decal
			decals.push_back(new_decal);
		}
	}

	//close file
	scene_file.close();
	return true;
}



Model* RenderManager::LoadModel(std::string path)
{
	//check if the model is alredy loaded
	auto find = mModels.find(path);
	if (find != mModels.end())
		return find->second;

	//variables to load the model
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	//load the model
	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

	if (!warn.empty())
		printf("Warn: %s\n", warn.c_str());
	if (!err.empty())
		printf("Err: %s\n", err.c_str());
	if (!ret) {
		printf("Failed to parse glTF\n");
		return nullptr;
	}

	//load all the textures for the model
	for (auto& texture : model.textures)
	{
		GLuint texture_handler;
		tinygltf::Sampler mSampler = model.samplers[texture.sampler];
		tinygltf::Image mImage = model.images[texture.source];

		//check if the texture is already created
		if (get_texture_handler(texture, mImage.name) != -1) continue;

		//bind the current texture
		glGenTextures(1, &texture_handler);
		glBindTexture(GL_TEXTURE_2D, texture_handler);

		//set the paramters of the texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mSampler.magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mSampler.minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mSampler.wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mSampler.wrapT);

		// Give pixel data to opengl
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mImage.width, mImage.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, mImage.image.data());
		glGenerateMipmap(GL_TEXTURE_2D);

		//add the texture handler to the vector
		mTextures.push_back(Texture{ mImage.name, texture.sampler , texture.source , texture_handler});
	}

	Model* new_model = new Model;
	const unsigned Attribute_count = 4;
	std::string attributes_names[Attribute_count] = { "POSITION" ,"NORMAL" ,"TANGENT" , "TEXCOORD_0" };
	std::vector<std::vector<int>> parent_childs;
	for (auto& node : model.nodes)
	{
		//store the childs
		std::vector<int> childs;
		for (auto& id : node.children)
			childs.push_back(id);
		parent_childs.push_back(childs);

		//create a new mesh
		Mesh new_mesh;
		if (!node.scale.empty())
			new_mesh.mScale = { node.scale[0],  node.scale[1], node.scale[2] };
		if (!node.translation.empty())
			new_mesh.mPosition = { node.translation[0],  node.translation[1], node.translation[2] };

		glm::vec3 rot = glm::vec3(0);
		if (node.rotation.size() == 3)
			rot = glm::vec3(node.rotation[0], node.rotation[1], node.rotation[2]);
		else if (node.rotation.size() == 4)
		{
			glm::vec4 quat;
			quat.x = static_cast<float>(node.rotation[0]); 
			quat.y = static_cast<float>(node.rotation[1]); 
			quat.z = static_cast<float>(node.rotation[2]); 
			quat.w = static_cast<float>(node.rotation[3]); 

			glm::fquat Q(quat.w, quat.x, quat.y, quat.z);
			rot = glm::eulerAngles(Q) * 180.0f / PI;
		}
		if (node.matrix.size() != 0)
			new_mesh.TRS = glm::mat4x4(	node.matrix[0], node.matrix[1], node.matrix[2], node.matrix[3],
										node.matrix[4], node.matrix[5], node.matrix[6], node.matrix[7],
										node.matrix[8], node.matrix[9], node.matrix[10], node.matrix[11],
										node.matrix[12], node.matrix[13], node.matrix[14], node.matrix[15]);

		new_mesh.RotateAroundVec(glm::vec3(0.0f, 0.0f, -1.0f), glm::radians(rot.z), glm::vec3(0));
		new_mesh.RotateAroundVec(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rot.y), glm::vec3(0));
		new_mesh.RotateAroundVec(glm::vec3(1.0f, 0.0f, 0.0f), glm::radians(rot.x), glm::vec3(0));

		//security check
		//security check
		if (node.mesh < 0)
		{
			new_model->mMeshes.push_back(new_mesh);
			continue;
		}

		//iterate through all the primitives of the mesh
		for (auto& primitive : model.meshes[node.mesh].primitives)
		{
			//create the handlers
			std::array<GLuint, Attribute_count> VBOs;
			GLuint new_VAO;
			GLuint idx_VBO;

			//generate and bind a vao
			glGenVertexArrays(1, &new_VAO);
			glBindVertexArray(new_VAO);

			//get the buffer view of the of the indices and add the number of indices to the model
			int indices_idx = primitive.indices;
			tinygltf::BufferView& indices_BVidx = model.bufferViews[model.accessors[indices_idx].bufferView];
			new_mesh.idx_counts.push_back(static_cast<unsigned>(model.accessors[indices_idx].count));
			new_mesh.idx_type.push_back(static_cast<unsigned>(model.accessors[indices_idx].componentType));

			// Generate the index buffer and fill it with data
			glGenBuffers(1, &idx_VBO);
			glBindBuffer(indices_BVidx.target, idx_VBO);
			glBufferData(indices_BVidx.target, indices_BVidx.byteLength, model.buffers[indices_BVidx.buffer].data.data() + indices_BVidx.byteOffset, GL_STATIC_DRAW);

			//generate the biffers for the attributes
			glGenBuffers(Attribute_count, VBOs.data());

			//generate buffers for all the atributtes of the vertex and fill them
			for (unsigned i = 0; i < Attribute_count; i++)
			{
				auto it = primitive.attributes.find(attributes_names[i]);
				if (it == primitive.attributes.end()) continue;

				//get the index and the buffer view of the attribute
				int attr_idx = primitive.attributes.find(attributes_names[i])->second;
				tinygltf::BufferView& attrib_BV = model.bufferViews[model.accessors[attr_idx].bufferView];

				// Bind new VBO and Bind a buffer of vertices
				glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

				// Give our vertices to OpenGL.
				glBufferData(attrib_BV.target, attrib_BV.byteLength, model.buffers[attrib_BV.buffer].data.data() + attrib_BV.byteOffset, GL_STATIC_DRAW);
				glEnableVertexAttribArray(i);
				glVertexAttribPointer(i, model.accessors[attr_idx].type, model.accessors[attr_idx].componentType, model.accessors[attr_idx].normalized, static_cast<GLsizei>(attrib_BV.byteStride), nullptr);

				//add the new vbo
				new_mesh.VBOs.push_back(VBOs[i]);
			}

			//Unbind buffer
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);

			//set the vao and vbos of the model
			new_mesh.VAOs.push_back(new_VAO);
			new_mesh.idx_VBOs.push_back(idx_VBO);

			//check if the model has materials
			if (primitive.material < 0) continue;

			//get the material 
			tinygltf::Material material = model.materials[primitive.material];

			//store the handler of the normal texture
			int idx = material.normalTexture.index;
			if(idx < 0)		new_mesh.normal_texture.push_back(idx);
			else			new_mesh.normal_texture.push_back(get_texture_handler(model.textures[idx], model.images[model.textures[idx].source].name));

			//store the handler of the diffuse color
			idx = material.pbrMetallicRoughness.baseColorTexture.index;
			if (idx < 0)	new_mesh.diffuse_texture.push_back(idx);
			else			new_mesh.diffuse_texture.push_back(get_texture_handler(model.textures[idx], model.images[model.textures[idx].source].name));

			//store the handler of the specular color
			idx = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
			if (idx < 0)	new_mesh.specular_texture.push_back(idx);
			else			new_mesh.specular_texture.push_back(get_texture_handler(model.textures[idx], model.images[model.textures[idx].source].name));

			//store the ambient color
			new_mesh.DiffuseColor.push_back(glm::vec4{ (float)material.pbrMetallicRoughness.baseColorFactor[0], (float)material.pbrMetallicRoughness.baseColorFactor[1],
												  (float)material.pbrMetallicRoughness.baseColorFactor[2], (float)material.pbrMetallicRoughness.baseColorFactor[3] });
		}
		new_model->mMeshes.push_back(new_mesh);
	}

	//update the childs parents
	for (int i = 0; i < parent_childs.size(); i++)
		for (auto& child : parent_childs[i])
			new_model->mMeshes[child].parent = &new_model->mMeshes[i];

	mModels[path] = (new_model);
	return new_model;
}

//unload all the vao and vbo
void RenderManager::UnLoadModel(Model* model)
{
	//iterate through the mesh
	for (auto& mesh : model->mMeshes)
	{
		//delete the vbos
		for (auto& vao : mesh.VAOs)
			glDeleteBuffers(1, &vao);

		//delete the vbos
		for (auto& vbo : mesh.VBOs)
			glDeleteBuffers(1, &vbo);

		//delete the idx buffers
		for (auto& idx_vbo : mesh.idx_VBOs)
			glDeleteBuffers(1, &idx_vbo);
	}
}

//get the texture form the textures already loaded
int RenderManager::get_texture_handler(tinygltf::Texture texture, std::string name)
{
	for (auto& mText : mTextures)
	{
		//check if the sampler and image is the same so its the same texture
		if (mText.sampler_idx == texture.sampler && mText.image_idx == texture.source && mText.name == name)
			return mText.texture_handler;
	}

	//return -1 if the texture is not found
	return -1;
}

void RenderManager::LoadSphereModel()
{
	unsigned vertex_count = 500;
	//pointers that will be used later to set the new vertices atributes
	glm::vec3* vertexPositions = nullptr;
	glm::vec3* vertexNormals = nullptr;
	glm::vec3* vertexAveragedNormals = nullptr;
	glm::vec2* vertexTexCoords = nullptr;

	//vectors used to store the vertex atributtes
	std::vector<glm::vec3> in_positions;
	std::vector<glm::vec3> in_averaged_normals;
	std::vector<glm::vec3> in_normals;
	std::vector<glm::vec2> in_textcoords;

	//variable used to store the vertex count
	unsigned vertexCount = 0;

	//number of rings that the sphere will have
	unsigned rings = (vertex_count - 1) / 2;
	//radius of the sphere
	float radius = 0.5f;

	//variables that will be used to compute the vertex positions
	float	xz_projTop, xz_projBot, height1, height2,
		cos_angle1, sin_angle1, cos_angle2, sin_angle2;

	//variable that will update the angle of the current circle we are computing
	float CircleAngleStep = 2 * PI / (float)vertex_count;
	//variable that will update the angle of the rings
	float RingAngleStep = PI / rings;
	//angles used to iterate through the sphere
	float RingAngle = PI / 2, CircleAngle = 0.0f;

	//iterate through all of the rings
	for (unsigned i = 0; i < rings; ++i, RingAngle -= RingAngleStep)
	{
		//compute the projection of the top and bot vertex in the xz plane
		xz_projTop = radius * cosf(RingAngle);
		xz_projBot = radius * cosf(RingAngle - RingAngleStep);
		//compute the heiht of the top and bot vertices
		height1 = radius * sinf(RingAngle);
		height2 = radius * sinf(RingAngle - RingAngleStep);

		//reset the angle
		CircleAngle = 0;
		//iterate through all of the vertices inside a ring
		for (unsigned j = 0; j < vertex_count; ++j, CircleAngle += CircleAngleStep)
		{
			//compute the sins and cosines of the vertices
			cos_angle1 = cosf(CircleAngle);
			sin_angle1 = sinf(CircleAngle);
			cos_angle2 = cosf(CircleAngle + CircleAngleStep);
			sin_angle2 = sinf(CircleAngle + CircleAngleStep);

			//compute all the vertices of the current face
			glm::vec3 top_vtx1(xz_projTop * cos_angle1, height1, xz_projTop * sin_angle1);
			glm::vec3 bot_vtx1(xz_projBot * cos_angle1, height2, xz_projBot * sin_angle1);
			glm::vec3 top_vtx2(xz_projTop * cos_angle2, height1, xz_projTop * sin_angle2);
			glm::vec3 bot_vtx2(xz_projBot * cos_angle2, height2, xz_projBot * sin_angle2);

			//compute the UVs of the vertices
			float sUV1 = j / (float)vertex_count;
			float tUV1 = 1 - i / (float)rings;
			float sUV2 = (j + 1) / (float)vertex_count;
			float tUV2 = 1 - (i + 1) / (float)rings;

			//compute the normals of the triangle
			glm::vec3 normal1 = glm::normalize(glm::cross(top_vtx1 - bot_vtx2, bot_vtx2 - bot_vtx1));
			glm::vec3 normal2 = glm::normalize(glm::cross(top_vtx1 - top_vtx2, top_vtx2 - bot_vtx2));
			//if we are at the top or bot base one of the normals must change the sign
			if (i == 0)			normal2 = normal1;
			if (i == rings - 1) normal1 = normal2;

			//-----------------------------------------First Triangle----------------------------------------
			//add the positions of the vertices
			in_positions.push_back(top_vtx1);
			in_positions.push_back(bot_vtx2);
			in_positions.push_back(bot_vtx1);

			//add the Uvs of the vertices
			//if we are at the top base we need to compute the average between the uvs
			if (i == 0)  in_textcoords.push_back(glm::vec2((sUV1 + sUV2) / 2, tUV1));
			else		 in_textcoords.push_back(glm::vec2(sUV1, tUV1));
			in_textcoords.push_back(glm::vec2(sUV2, tUV2));
			in_textcoords.push_back(glm::vec2(sUV1, tUV2));

			//add the normals of the vertices
			in_normals.push_back(normal1);
			in_normals.push_back(normal1);
			in_normals.push_back(normal1);

			//-----------------------------------------Second Triangle----------------------------------------
			//add the positions of the vertices
			in_positions.push_back(top_vtx1);
			in_positions.push_back(top_vtx2);
			in_positions.push_back(bot_vtx2);

			//add the Uvs of the vertices
			in_textcoords.push_back(glm::vec2(sUV1, tUV1));
			in_textcoords.push_back(glm::vec2(sUV2, tUV1));
			//if we are at the bot base we need to compute the average between the uvs
			if (i == rings - 1)	in_textcoords.push_back(glm::vec2((sUV1 + sUV2) / 2, tUV2));
			else				in_textcoords.push_back(glm::vec2(sUV2, tUV2));

			//add the normals of the vertices
			in_normals.push_back(normal2);
			in_normals.push_back(normal2);
			in_normals.push_back(normal2);
		}
	}
	//compute the average normals
	for (unsigned i = 0; i < in_positions.size(); i++)
	{
		glm::vec3 norm = glm::normalize(in_positions[i]);
		float epsilon = glm::epsilon<float>();
		norm.x = (norm.x <= epsilon && norm.x >= -epsilon) ? 0.0f : norm.x;
		norm.z = (norm.z <= epsilon && norm.z >= -epsilon) ? 0.0f : norm.z;
		in_averaged_normals.push_back(norm);
	}

	//set the vertex count
	vertexCount = static_cast<unsigned>(in_positions.size());

	//set all of the attributes pointer of the vertices
	vertexNormals = in_normals.data();
	vertexAveragedNormals = in_averaged_normals.data();
	vertexPositions = in_positions.data();
	vertexTexCoords = in_textcoords.data();

	std::vector<Vertex> mVertices;
	//iterate through all the vertices
	for (unsigned i = 0; i < vertexCount; i++)
	{
		//create a new vertex with the corresponding attributes and add it to the vector of vertices
		Vertex vtx(vertexPositions[i], vertexAveragedNormals[i], vertexTexCoords[i]);
		mVertices.push_back(vtx);
	}

	Model* sphere = new Model();
	Mesh sphere_mesh;
	GLuint sphereVAO, sphereVBO;

	//gen buffer
	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);

	// Bind the VBO and the VAO
	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);

	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(Vertex), &mVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]), reinterpret_cast<void*>(offsetof(Vertex, position)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]), reinterpret_cast<void*>(offsetof(Vertex, normal)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(mVertices[0]), reinterpret_cast<void*>(offsetof(Vertex, textcoord)));

	sphere_mesh.VAOs.push_back(sphereVAO);
	sphere_mesh.VBOs.push_back(sphereVBO);
	sphere_mesh.vertex_count = in_positions.size();
	sphere_mesh.use_model_normal = true;
	sphere_mesh.use_model_color = true;
	sphere->mMeshes.push_back(sphere_mesh);

	mModels["Sphere"] = sphere;
	CustomSphere = sphere;
}