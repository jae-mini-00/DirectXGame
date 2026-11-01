#include "Scene/Scene.h"
#include <utility>


void Scene::AddObject(std::unique_ptr<GameObject> object) { objects.push_back(std::move(object)); }

std::size_t Scene::size() const { return objects.size(); }

GameObject* Scene::operator[](std::size_t index) { return objects[index].get(); }

const GameObject* Scene::operator[](std::size_t index) const { return objects[index].get(); }
