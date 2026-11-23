#pragma once

#include <array>

class Input
{
	public :
		void Update();
		bool IsKeyDown(int key) const;
		bool IsKeyPressed(int key) const;
		bool IsKeyReleased(int key) const;
	private :
		std::array<bool, 256> currentKeys{};
		std::array<bool, 256> previousKeys{};
};