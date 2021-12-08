#pragma once

#include <SDL2/SDL.h>

//total number of keys in the keyboard
static const int KEYBOARD_KEY_AMOUNT = SDL_NUM_SCANCODES;
static const int MOUSE_KEY_AMOUNT = 3;

enum class MouseKey {
	LEFT, MID, RIGHT
};

//enum to create shorcuts for the keys that we are going to use of SDL
enum Key
{
	A = SDL_SCANCODE_A, B, C, D, E, F, G, H, I, J, K, L, M,
	N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

	Num1 = SDL_SCANCODE_1, Num2, Num3, Num4, Num5, Num6, Num7, 
	Num8, Num9, Num0,

	Tab			= SDL_SCANCODE_TAB,
	Control		= SDL_SCANCODE_LCTRL,
	LShift		= SDL_SCANCODE_LSHIFT,
	Enter		= SDL_SCANCODE_RETURN,
	Delete		= SDL_SCANCODE_DELETE,
	Esc			= SDL_SCANCODE_ESCAPE,
	KeySpace	= SDL_SCANCODE_SPACE,

	Plus		= SDL_SCANCODE_KP_PLUS,
	Minus		= SDL_SCANCODE_KP_MINUS,

	Supr		= SDL_SCANCODE_BACKSPACE,

	F1 = SDL_SCANCODE_F1,
	F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

	Right		= SDL_SCANCODE_RIGHT,
	Left		= SDL_SCANCODE_LEFT,
	Down		= SDL_SCANCODE_DOWN,
	Up			= SDL_SCANCODE_UP,
};

class InputHandler
{
public:
	//initialize the input manager
	void Initialize();
	//this function will actualize the re initialize the buffers with the keys status
	void StartFrame();

	bool MouseIsDown(MouseKey index);
	bool MouseIsUp(MouseKey index);
	bool MouseIsTriggered(MouseKey index);
	bool MouseIsReleased(MouseKey index);
	bool WheelTriggered();

	//check if the key provided is down
	bool KeyIsDown(Key index);
	//check if the key provided is up
	bool KeyIsUp(Key index);
	//check if the key provided was triggered
	bool KeyIsTriggered(Key index);
	//check if the key provided was released
	bool KeyIsReleased(Key index);

	//this function will set the status of the key associated with the event provided
	void HandleKeyEvent(SDL_Event event);
	void HandleMouseEvent(SDL_Event event);
private:

	bool mMouseCurrent[MOUSE_KEY_AMOUNT] = { false };
	bool mMousePrevious[MOUSE_KEY_AMOUNT] = { false };

	/* KeyboardKeys */
	bool mKeyCurrent[KEYBOARD_KEY_AMOUNT];
	bool mKeyPrevious[KEYBOARD_KEY_AMOUNT];
	bool mWheelCurrent = false;

	/* SINGLETON (We will only create one manager) */
public:
	InputHandler(InputHandler const&) = delete;
	void operator=(InputHandler const&) = delete;

	static InputHandler& getInstance()
	{
		static InputHandler instance;
		return instance;
	}

private:
	InputHandler() {}
};

#define InputManager (InputHandler::getInstance())

#define KeyDown(i)				InputManager.KeyIsDown(i)
#define KeyUp(i)					InputManager.KeyIsUp(i)
#define KeyTriggered(i)		InputManager.KeyIsTriggered(i)
#define KeyReleased(i)		InputManager.KeyIsReleased(i)

#define MouseDown(i)		InputManager.MouseIsDown(i)
#define MouseUp(i)			InputManager.MouseIsUp(i)
#define MouseTriggered(i)	InputManager.MouseIsTriggered(i)
#define MouseReleased(i)	InputManager.MouseIsReleased(i)
