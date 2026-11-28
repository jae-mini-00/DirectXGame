#include "Physics/Collision.h"
#include "Scene/Scene.h"
#include <utility>


void Scene::AddObject(std::unique_ptr<GameObject> object) { objects.push_back(std::move(object)); }

std::size_t Scene::size() const { return objects.size(); }

GameObject* Scene::operator[](std::size_t index) { return objects[index].get(); }

const GameObject* Scene::operator[](std::size_t index) const { return objects[index].get(); }


std::vector<CollidingPair> Scene::DetectCollisions()
{
    std::vector<CollidingPair> pairs;

    for (size_t i = 0; i < objects.size(); ++i)
    {
        for (size_t j = i + 1; j < objects.size(); ++j)
        {
            const CollisionFilter* worldACollider = Collision::GetFilter(objects[i]->collider);
            const CollisionFilter* worldBCollider = Collision::GetFilter(objects[j]->collider);

            if (!worldACollider || !worldBCollider)
                continue;

            const bool aDetectsB = Collision::CanDetect(*worldACollider, *worldBCollider);
            const bool bDetectsA = Collision::CanDetect(*worldBCollider, *worldACollider);
            
            if (aDetectsB || bDetectsA)
            {
                const WorldCollider worldA = Collision::ToWorldCollider(
                    objects[i]->collider, objects[i]->transform.position, objects[i]->transform.scale);

                const WorldCollider worldB = Collision::ToWorldCollider(
                    objects[j]->collider, objects[j]->transform.position, objects[j]->transform.scale);

                if (Collision::Intersects(worldA, worldB))
                {
                    if (aDetectsB)
                        pairs.push_back({ objects[i].get(), objects[j].get() });
                    if (bDetectsA)
                        pairs.push_back({ objects[j].get(), objects[i].get() });
                }
            }
        }
    }

    return pairs;
}