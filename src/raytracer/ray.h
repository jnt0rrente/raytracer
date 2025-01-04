#ifndef RAY_H
#define RAY_H

#include "vector3d.h"

class Ray
{
public:
    Ray();

    Ray(const Point3d &origin, const Vector3d &direction);

    const Point3d &origin() const;
    const Vector3d &direction() const;

    Point3d at(double t) const;

private:
    Point3d orig;
    Vector3d dir;
};


#endif