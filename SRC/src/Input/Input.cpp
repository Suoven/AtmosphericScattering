#include "Input.h"

//initialize the input manager
void InputHandler::Initialize()
{
	// Set both Current and previous arrays of booleans to false
	for (unsigned i = 0; i < MOUSE_KEY_AMOUNT; ++i)
	{
		mMouseCurrent[i] = 0;
		mMousePrevious[i] = 0;
	}

	// Set both Current and previous arrays of booleans to false
	for (unsigned i = 0; i < KEYBOARD_KEY_AMOUNT; ++i)
	{
		mKeyCurrent[i] = 0;
		mKeyPrevious[i] = 0;
	}
}

//this function will actualize the re initialize the buffers with the keys status
void InputHandler::StartFrame()
{
	
	/* Reset the Current and Previous arrays */
	for (unsigned i = 0; i < MOUSE_KEY_AMOUNT; ++i)
		mMousePrevious[i] = mMouseCurrent[i];


	/* Reset the Current and Previous arrays */
	for (unsigned i = 0; i < KEYBOARD_KEY_AMOUNT; ++i) 
		mKeyPrevious[i] = mKeyCurrent[i];
}

//this function will set the status of the key associated with the event provided
void InputHandler::HandleKeyEvent(SDL_Event event)
{

	SDL_Keycode ScanCode = event.key.keysym.scancode;

	if (ScanCode > 0 && ScanCode < KEYBOARD_KEY_AMOUNT)
		mKeyCurrent[ScanCode] = event.key.state ? true : false;
}

//check if the key provided is down
bool InputHandler::KeyIsDown(Key index)
{
	return mKeyCurrent[index];
}

//check if the key provided is up
bool InputHandler::KeyIsUp(Key index)
{
	return !KeyIsDown(index);
}

//check if the key provided was triggered
bool InputHandler::KeyIsTriggered(Key index)
{
	if (mKeyCurrent[index] == true)
	{
		if (mKeyCurrent[index] != mKeyPrevious[index])
			return true;
	}
	return false;
}

//check if the key provided was released
bool InputHandler::KeyIsReleased(Key index)
{
	if (mKeyCurrent[index] == false)
	{
		if (mKeyCurrent[index] != mKeyPrevious[index])
			return true;
	}
	return false;
}

void InputHandler::HandleMouseEvent(SDL_Event event)
{
	// Access the index with -1 beacuse they go:
	// LEFT = 1, MIDDLE = 2, RIGHT = 3
	mMouseCurrent[event.button.button - 1] = event.button.state ? true : false;
}
bool InputHandler::WheelTriggered()
{
	return mWheelCurrent;
}
bool InputHandler::MouseIsDown(MouseKey index)
{
	return mMouseCurrent[static_cast<unsigned>(index)];
}
bool InputHandler::MouseIsUp(MouseKey index)
{
	return !mMouseCurrent[static_cast<unsigned>(index)];
}
bool InputHandler::MouseIsTriggered(MouseKey index)
{
	if (mMouseCurrent[static_cast<unsigned>(index)] == true)
	{
		if (mMouseCurrent[static_cast<unsigned>(index)] != mMousePrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}
bool InputHandler::MouseIsReleased(MouseKey index)
{
	if (mMouseCurrent[static_cast<unsigned>(index)] == false)
	{
		if (mMouseCurrent[static_cast<unsigned>(index)] != mMousePrevious[static_cast<unsigned>(index)])
			return true;
	}
	return false;
}

