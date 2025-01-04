#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

#include <memory>
using std::make_shared;
using std::shared_ptr;

#include "vector3d.h"
#include "color.h"
#include "ray.h"
#include "../utils/visitor.h"

class HitRecord; // forward declaration

class IMaterial : public virtual IVisitable
{
public:
    IMaterial(std::string name);
    virtual ~IMaterial();
    std::string name;

    virtual bool scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const = 0;
};

class Lambertian : public IMaterial
{
public:
    Lambertian(std::string name, const Color &albedo);

    bool scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const override;
    void accept(IVisitor *visitor) override;
    Color albedo;
};

class Metal : public IMaterial
{
public:
    Metal(std::string name, const Color &albedo, double fuzz);

    bool scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const override;
    void accept(IVisitor *visitor) override;
    Color albedo;
    double fuzz;
};

class Dielectric : public IMaterial
{
public:
    Dielectric(std::string name, Color tint, double refraction_index);

    bool scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const override;
    void accept(IVisitor *visitor) override;
    Color tint;
    double refraction_index;

    static double reflectance(double cosine, double refraction_index);
};

class MaterialFactory
{
public:
    static shared_ptr<IMaterial> createLambertian(std::string name, Color albedo);
    static shared_ptr<IMaterial> createMetal(std::string name, Color albedo, double fuzz);
    static shared_ptr<IMaterial> createDielectric(std::string name, Color tint, double refraction_index);
};

#endif