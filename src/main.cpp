#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/GL.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

#include "imGui/imgui_impl_sdl.h"

#include "Input/input.h"
#include "Levels/Level1.h"
#include "Graphics/RenderManager/RenderManager.h"

#undef main
int main(int argc, char* args[])
{
	// Parse arguments
	std::string Lpath = { "./data/scenes/" };
	std::string Spath = { "./data/screenshots/" };
	if(argc > 1)
		RenderMgr.load_path = Lpath + args[1];
	if (argc > 2)
	{
		RenderMgr.screenshot_path = Spath + args[2];
		RenderMgr.takescreenshot = true;
	}

	//init level and input manager
	Level1_Init();
	InputManager.Initialize();

	bool quit = false;
	SDL_Event event;
	//gameloop
	while (!quit)
	{
		//update input manager
		InputManager.StartFrame();

		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			//check for inputs
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				InputManager.HandleKeyEvent(event);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				InputManager.HandleMouseEvent(event);
				break;
			}
		}

		if (KeyTriggered(Key::Esc))
			quit = true;

		//update the level and render it
		Level1_Update();
		Level1_Render();

		//check if we are done
		if (RenderMgr.takescreenshot)
			break;
	}
	//shut down the level
	Level1_ShutDown();

	return 0;
}