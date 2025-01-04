#include "vector3d.h"

Vector3d::Vector3d() : e{0, 0, 0} {}
Vector3d::Vector3d(double e0, double e1, double e2) : e{e0, e1, e2} {}

double Vector3d::x() const { return e[0]; }
double Vector3d::y() const { return e[1]; }
double Vector3d::z() const { return e[2]; }

Vector3d Vector3d::operator-() const { return Vector3d(-e[0], -e[1], -e[2]); }
double Vector3d::operator[](int i) const { return e[i]; }
double &Vector3d::operator[](int i) { return e[i]; }

Vector3d &Vector3d::operator+=(const Vector3d &v)
{
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
}

Vector3d &Vector3d::operator*=(double t)
{
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
}

Vector3d &Vector3d::operator/=(double t)
{
    return *this *= 1 / t;
}

double Vector3d::length() const
{
    return sqrt(length_squared());
}

double Vector3d::length_squared() const
{
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
}

bool Vector3d::near_zero() const
{
    auto tolerance = 1e-8;
    return (
        (fabs(e[0]) < tolerance) && (fabs(e[1]) < tolerance) && (fabs(e[2]) < tolerance));
}

Vector3d Vector3d::random()
{
    return Vector3d(random_double(), random_double(), random_double());
}

Vector3d Vector3d::random(double min, double max)
{
    return Vector3d(
        random_double(min, max),
        random_double(min, max),
        random_double(min, max));
}

Vector3d Vector3d::random_in_unit_sphere()
{
    while (true)
    {
        auto p = Vector3d::random(-1, 1);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

Vector3d Vector3d::random_in_unit_disk()
{
    while (true)
    {
        auto p = Vector3d(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() >= 1)
            continue;
        return p;
    }
}

Vector3d Vector3d::random_on_hemisphere(const Vector3d &normal)
{
    const Vector3d on_unit_sphere = random_in_unit_sphere();
    if (dot(on_unit_sphere, normal) > 0.0)
        return on_unit_sphere;
    else
        return -on_unit_sphere;
}

Vector3d Vector3d::random_unit_vector()
{
    return unit_vector(random_in_unit_sphere());
}

void Vector3d::accept(IVisitor *visitor)
{
    visitor->visit(this);
}