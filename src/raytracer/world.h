#ifndef WORLD_H
#define WORLD_H

#include "material.h"
#include "hittable.h"
#include "ray.h"
#include "../utils/math_utils.h"

#include <vector>
#include <string>
#include <map>
#include <memory>
using std::shared_ptr;

class World : public IHittable
{
public:
    std::map<std::string, shared_ptr<IHittable>> objects;

    World();

    void clear();

    void add(shared_ptr<IHittable> object);

    bool hit(const Ray &r, Interval ray_t, HitRecord &record) const override;

    void remove(const std::string& name);

    std::vector<std::string> getObjectKeys() const;

    const shared_ptr<IHittable>& getObject(const std::string& name) const;

    std::vector<shared_ptr<IHittable>> getObjectsArray() const;

    void accept(IVisitor *visitor) override;

    void updateObjectName(const std::string& oldName, const std::string& newName);

    virtual ~World() override;
};

#endif