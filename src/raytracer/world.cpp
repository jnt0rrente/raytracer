#include "world.h"

World::World() {}

void World::clear() { objects.clear(); }

void World::add(shared_ptr<IHittable> object)
{
    objects.insert({object->name, object});
}

bool World::hit(const Ray &r, Interval ray_t, HitRecord &record) const
{
    HitRecord temp_rec;
    bool hit_anything = false;
    double closest_so_far = ray_t.max;

    for (const auto &pair : objects)
    {
        const std::shared_ptr<IHittable> &obj = pair.second;

        if (obj->hit(r, Interval(ray_t.min, closest_so_far), temp_rec)) // closest so far here, to ensure that further detections only happen closer
        {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            record = temp_rec;
        }
    }

    return hit_anything;
}

void World::remove(const std::string& name)
{
    objects.erase(name);
}

std::vector<std::string> World::getObjectKeys() const
{
    std::vector<std::string> keys;
    for (const auto &pair : objects)
    {
        keys.push_back(pair.first.c_str());
    }
    return keys;
}

const shared_ptr<IHittable>& World::getObject(const std::string& name) const
{
    return objects.at(name);
}

std::vector<shared_ptr<IHittable>> World::getObjectsArray() const
{
    std::vector<shared_ptr<IHittable>> objectsArray;
    for (const auto &pair : objects)
    {
        objectsArray.push_back(pair.second);
    }
    return objectsArray;
}

void World::accept(IVisitor *visitor)
{
    visitor->visit(this);
}

void World::updateObjectName(const std::string& oldName, const std::string& newName)
{
    auto it = objects.find(oldName);
    if (it != objects.end()) {
        std::shared_ptr<IHittable> object = it->second;
        objects.erase(it);
        object->name = newName;
        objects.insert({newName, object});
    }
}

World::~World() {}