#ifndef VISITOR_H
#define VISITOR_H

#include <string>

class Sphere3d;
class Vector3d;
class IHittable;

class IVisitor
{
public:
    virtual void visit(IHittable *object) = 0;
    virtual void visit(Sphere3d *sphere) = 0;
    virtual void visit(Vector3d *vector) = 0;
    virtual void visit(class IMaterial *material) = 0;
    virtual void visit(class Lambertian *material) = 0;
    virtual void visit(class Metal *material) = 0;
    virtual void visit(class Dielectric *material) = 0;
};

class IVisitable
{
public:
    virtual void accept(IVisitor *visitor) = 0;
};
#endif