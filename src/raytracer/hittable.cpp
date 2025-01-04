#include "hittable.h"
#include "sphere3d.h"

using std::make_shared;

void HitRecord::set_face_normal(const Ray &r, const Vector3d &outwards_normal)
{
    front_face = dot(r.direction(), outwards_normal) < 0;
    normal = front_face ? outwards_normal : -outwards_normal;
}

shared_ptr<IHittable> HittableFactory::createSphere(std::string name, Point3d center, double radius, shared_ptr<IMaterial> material)
{
    return make_shared<Sphere3d>(name, center, radius, material);
}