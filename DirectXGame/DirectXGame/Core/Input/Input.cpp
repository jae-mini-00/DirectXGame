#include "Core/Input/Input.h"
#include <Windows.h>

void Input::Update()
{
	previousKeys = currentKeys;

	HWND foreground = GetForegroundWindow();
	HWND active = GetActiveWindow();
	bool deactivate = (active == nullptr || foreground != active);

	for (unsigned int i = 0; i < currentKeys.size(); ++i)
	{
		if (!deactivate && (GetAsyncKeyState(i) & 0x8000) != 0)
			currentKeys[i] = true;
		else
			currentKeys[i] = false;
	}
}

bool Input::IsKeyDown(int key) const
{
	if (key < 0 || key > 255)
		return false;

	return currentKeys[key];
}

bool Input::IsKeyPressed(int key) const
{
	if (key < 0 || key > 255)
		return false;

	return currentKeys[key] && !previousKeys[key];
}

bool Input::IsKeyReleased(int key) const
{
	if (key < 0 || key > 255)
		return false;

	return !currentKeys[key] && previousKeys[key];
}