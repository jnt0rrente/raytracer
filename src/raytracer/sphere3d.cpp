#include "sphere3d.h"

Sphere3d::Sphere3d(const std::string& name, const Point3d& center, double radius, shared_ptr<IMaterial> material)
    : IHittable()
{
    this->name = name;
    this->center = center;
    this->radius = radius;
    this->material = material;
}

bool Sphere3d::hit(const Ray &r, Interval ray_t, HitRecord &record) const
{
    Vector3d oc = center - r.origin();
    double a = r.direction().length_squared();
    double h = dot(r.direction(), oc);
    double c = oc.length_squared() - radius * radius;

    double discriminant = h * h - a * c;
    double tolerance = 1e-6;

    if (discriminant < 0)
    {
        return false;
    }

    double sqrtd = sqrt(discriminant);

    // find nearest root in acceptable range
    double root = (h - sqrtd) / a;
    if (fabs(a) < tolerance || !ray_t.surrounds(root))
    {
        root = (h + sqrtd) / a;
        if (fabs(a) < tolerance || !ray_t.surrounds(root))
        {
            return false;
        }
    }

    record.t = root;
    record.p = r.at(record.t);
    Vector3d outwards_normal = (record.p - center) / radius;
    if (outwards_normal.length() < 1 - tolerance || outwards_normal.length() > 1 + tolerance)
    {
        return false;
    }
    record.set_face_normal(r, outwards_normal);
    record.material = this->material;
    return true;
}

void Sphere3d::accept(IVisitor *visitor)
{
    visitor->visit(this);
}

Sphere3d::~Sphere3d() {}