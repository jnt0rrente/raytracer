#ifndef GUI_VISITOR_H
#define GUI_VISITOR_H

#include "../utils/visitor.h"
#include "../raytracer/material.h"
#include <string>

class ImGuiVisitor : public IVisitor
{
public:
    void visit(class IHittable *object) override;
    void visit(class Sphere3d *sphere) override;
    void visit(class Vector3d *vector) override;
    void visit(class IMaterial *material) override;
    void visit(class Lambertian *material) override;
    void visit(class Metal *material) override;
    void visit(class Dielectric *material) override;
};

#endif