- IMPORTANT NOTES:
- There are two extra check box values in the HBAO imgui that change the method for obtaining the occlusion, by default it is using the normal
form the gbuffer and using the angle from the horizontal vector to the tangent vector as an input for the occlusion computation. If we select the 
tangent method it will use as an input the horizontal angle only and the tangent angle will be used to check the first sample to satisfy. The other 
check box is used to change between using the normal in the first method, instead of using the normal of the gbuffer we use the normal computed by dFdx, dFdy.
These three modes create different results but at the end I think that the more realistic and the one that gives less artifacts is the first method, so none 
of the 2 extra checkbox should be enabled.

- In order to make the decals work there needs to be a Cube.gltf inside the data/gltf folder
- Is important for the solution to work if you put point lights to have inside the data/gltf folder a Shpere.gltf model 
- The screenshot will be saved inside the data/screenshots folder

1. How to use your program: 
	LShift: move faster with WASD
	WS: move the camera forward/backward along its forward vector.
	AD: move the camera left/right along its side vector.
	QE: move the camera up/down along the global up vector (0,1,0).
	Click + Drag: the mouse should tilt the camera around its yaw and pitch.
	Up/Down: rotate the camera around the pitch.
	Left/right: rotate the camera around the yaw.
	T: change to different texture modes of the gBuffer or the final image
	Ctrl+R: Reload scene
	F5:  reload shaders
	The rest of the controls are controlled by imgui within the application

2. Important parts of the code: 
	- HBAO.cpp : in this file we have the implementation of the ambient occlusion pass

	-RenderManager.cpp/.h : In this file is where every important part from diferred shading is located, in functions like GenerateGBuffer where the
				gBuffer is created and the Update function where the all the Rendering passes such as the light pass, ambient pass, 
				bloom pass and other small passes are called. You can also find the load function where the gltf models are loaded 
				with the corresponding textures and vao. 
	
	-AmbientOcclusion.fs: shader that performs all the computations to compute the ambient occlusion for each pixel
	-BilateralGBlur.fs: shader performs a bilateral gaussian blur with 2 passes
	-gBuffer.vs/.fs : shader that fills the gBuffer
	-deferred_dierectional_shading.vs/.fs: shaders that compute the final color of the scene using taking into account the shadow with different parameters
	-Blur.fs : shader that given a texture blurries it into the current framebuffer texture
	-BloomScene.fs : shader that performs the final pass with the bloom texture and the scene texture to combine them, it also applies if needed gamma correction and 
			exposure.
3. Known issues and problems: 
