#include "material.h"

#include "color.h"
#include "../utils/math_utils.h"

#include "hittable.h"

IMaterial::IMaterial(std::string name) : name(name) {}
IMaterial::~IMaterial() = default;

Lambertian::Lambertian(std::string name, const Color &albedo) : IMaterial(name), albedo(albedo) {}
bool Lambertian::scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const
{
    Vector3d scatter_direction = record.normal + Vector3d::random_unit_vector();
    if (scatter_direction.near_zero())
        scatter_direction = record.normal;

    scattered_ray = Ray(record.p, scatter_direction);
    attenuation = albedo;
    return true;
}
void Lambertian::accept(IVisitor *visitor)
{
    visitor->visit(this);
}

Metal::Metal(std::string name, const Color &albedo, double fuzz) : IMaterial(name), albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}
bool Metal::scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const
{
    Vector3d unit_direction = unit_vector(ray_in.direction());
    Vector3d reflected = reflect(unit_direction, unit_vector(record.normal));

    reflected = unit_vector(reflected) + (fuzz * Vector3d::random_unit_vector());

    scattered_ray = Ray(record.p, reflected);
    attenuation = albedo;
    return (dot(scattered_ray.direction(), record.normal) > 0);
}
void Metal::accept(IVisitor *visitor)
{
    visitor->visit(this);
}

Dielectric::Dielectric(std::string name, Color tint, double refraction_index) : IMaterial(name), tint(tint), refraction_index(refraction_index) {}
bool Dielectric::scatter(const Ray &ray_in, const HitRecord &record, Color &attenuation, Ray &scattered_ray) const
{
    attenuation = tint;
    double ri = record.front_face ? (1.0 / refraction_index) : refraction_index;

    Vector3d unit_direction = unit_vector(ray_in.direction());

    double cos_theta = fmin(dot(-unit_direction, record.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    bool cannot_refract = ri * sin_theta > 1.0;

    Vector3d direction;

    if (cannot_refract || reflectance(cos_theta, ri) > random_double())
        direction = reflect(unit_direction, record.normal);
    else
        direction = refract(unit_direction, record.normal, ri);

    scattered_ray = Ray(record.p, direction);
    return true;
}
void Dielectric::accept(IVisitor *visitor)
{
    visitor->visit(this);
}

double Dielectric::reflectance(double cosine, double refraction_index)
{
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

shared_ptr<IMaterial> MaterialFactory::createLambertian(std::string name, Color albedo)
{
    return make_shared<Lambertian>(name, albedo);
}

shared_ptr<IMaterial> MaterialFactory::createMetal(std::string name, Color albedo, double fuzz = 0.0)
{
    return make_shared<Metal>(name, albedo, fuzz);
}

shared_ptr<IMaterial> MaterialFactory::createDielectric(std::string name, Color tint, double refraction_index = 1.0)
{
    return make_shared<Dielectric>(name, tint, refraction_index);
}