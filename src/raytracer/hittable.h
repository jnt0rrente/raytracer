#ifndef HITTABLE_H
#define HITTABLE_H

#include <string>
#include <memory>
using std::shared_ptr;
#include "ray.h"
#include "material.h"
#include "../utils/visitor.h"
#include "../utils/math_utils.h"

class Sphere3d;

class HitRecord
{
public:
    Point3d p;
    Vector3d normal;
    shared_ptr<IMaterial> material;
    double t;
    bool front_face;

    void set_face_normal(const Ray &r, const Vector3d &outwards_normal);
};

class IHittable : public virtual IVisitable
{
public:
    std::string name;
    shared_ptr<IMaterial> material;
    virtual ~IHittable() = default;
    virtual bool hit(const Ray &r, Interval ray_t, HitRecord &record) const = 0;
};


class HittableFactory
{
public:
    static shared_ptr<IHittable> createSphere(std::string name, Point3d center, double radius, shared_ptr<IMaterial> material);
};

#endif