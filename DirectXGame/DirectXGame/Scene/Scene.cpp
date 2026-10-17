#include "Scene/Scene.h"
#include <utility>


// TODO: const vector<unique_ptr<GameObject>>& 상태에서도
//       GameObject 자체는 수정 가능하므로 추후 접근 방식 재설계

const std::vector<std::unique_ptr<GameObject>>& Scene::GetObjects() const { return objects; }

void Scene::AddObject(std::unique_ptr<GameObject> object)
{
	objects.push_back(std::move(object));
}