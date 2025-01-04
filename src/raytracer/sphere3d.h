#ifndef SPHERE3D_H
#define SPHERE3D_H

#include "hittable.h"
#include "../utils/visitor.h"
#include <memory>
using std::shared_ptr;

class Sphere3d : public IHittable
{
public:
    Sphere3d(const std::string& name, const Point3d& center, double radius, shared_ptr<IMaterial> material);

    Point3d center;
    double radius;

    bool hit(const Ray &r, Interval ray_t, HitRecord &record) const override;

    void accept(IVisitor *visitor) override;

    virtual ~Sphere3d() override;
};

#endif