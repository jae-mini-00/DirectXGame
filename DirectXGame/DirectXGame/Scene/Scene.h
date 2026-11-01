#pragma once

#include "Scene/GameObject.h"
#include <vector>
#include <memory>

class Scene 
{
	public :
		void AddObject(std::unique_ptr<GameObject> object);

		std::size_t size() const;
		GameObject* operator[](std::size_t index);
		const GameObject* operator[](std::size_t index) const;

	private :
		std::vector<std::unique_ptr<GameObject>> objects;

};