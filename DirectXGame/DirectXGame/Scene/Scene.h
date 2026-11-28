#pragma once

#include "Scene/GameObject.h"
#include <vector>
#include <memory>

struct CollidingPair
{
	const GameObject* first;
	const GameObject* second;
};

class Scene 
{
	public :
		void AddObject(std::unique_ptr<GameObject> object);
		std::vector<CollidingPair> DetectCollisions();

		std::size_t size() const;
		GameObject* operator[](std::size_t index);
		const GameObject* operator[](std::size_t index) const;

	private :
		std::vector<std::unique_ptr<GameObject>> objects;

};