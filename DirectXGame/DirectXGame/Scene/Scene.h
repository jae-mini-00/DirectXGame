#pragma once

#include "Scene/GameObject.h"
#include <vector>
#include <memory>

class Scene 
{
	public :
		const std::vector<std::unique_ptr<GameObject>>& GetObjects() const;
		void AddObject(std::unique_ptr<GameObject> object);

	private :
		std::vector<std::unique_ptr<GameObject>> objects;

};