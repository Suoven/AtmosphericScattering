# AccelerationStructures
- If you want to execute the program, you need to unzip the data folder so instead of a data.zip there is just a data folder.

- Real time atmospheric simulation
![This is a alt text.](/docs/video1.gif "Atmospheric Scattering 1")
![This is a alt text.](/docs/video6.gif "Atmospheric Scattering 6")

- Smooth transitions between outisde and inside atmosphere
![This is a alt text.](/docs/video2.gif "Atmospheric Scattering 2")
![This is a alt text.](/docs/video5.gif "Atmospheric Scattering 3")

- Implemented with Horizontal Ambient Occlusion
![This is a alt text.](/docs/video3.gif "Atmospheric Scattering 4")

- Implemented with Cascaded Shadow Maps to allow for big scenes
![This is a alt text.](/docs/video4.gif "Atmospheric Scattering 5")

# How to use the program
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
