#include "ray.h"
#include "../utils/math_utils.h"

Ray::Ray() {}

Ray::Ray(const Point3d &origin, const Vector3d &direction) : orig(origin), dir(direction) {}

const Point3d &Ray::origin() const { return orig; }
const Vector3d &Ray::direction() const { return dir; }

Point3d Ray::at(double t) const
{
    return orig + t * dir;
}