#include "Scene.h"

const std::vector<std::unique_ptr<GameObject>>& Scene::GetObjects() const { return objects; }

void Scene::AddObject(std::unique_ptr<GameObject> object)
{
	objects.push_back(std::move(object));
}