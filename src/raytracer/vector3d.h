#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <iostream>
#include "../utils/math_utils.h"
#include "../utils/visitor.h"

class Vector3d : public IVisitable
{
public:
    double e[3];

    Vector3d();
    Vector3d(double e0, double e1, double e2);

    double x() const;
    double y() const;
    double z() const;

    Vector3d operator-() const;
    double operator[](int i) const;
    double &operator[](int i);

    Vector3d &operator+=(const Vector3d &v);

    Vector3d &operator*=(double t);

    Vector3d &operator/=(double t);

    double length() const;

    double length_squared() const;

    bool near_zero() const;

    static Vector3d random();

    static Vector3d random(double min, double max);

    static Vector3d random_in_unit_sphere();

    static Vector3d random_in_unit_disk();

    static Vector3d random_on_hemisphere(const Vector3d &normal);

    static Vector3d random_unit_vector();

    void accept(IVisitor *visitor) override;
};

#pragma region utils
// utilities
inline std::ostream &operator<<(std::ostream &out, const Vector3d &v)
{
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline Vector3d operator+(const Vector3d &u, const Vector3d &v)
{
    return Vector3d(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline Vector3d operator-(const Vector3d &u, const Vector3d &v)
{
    return Vector3d(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline Vector3d operator*(const Vector3d &u, const Vector3d &v)
{
    return Vector3d(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline Vector3d operator*(double t, const Vector3d &v)
{
    return Vector3d(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline Vector3d operator*(const Vector3d &v, double t)
{
    return t * v;
}

inline Vector3d operator/(const Vector3d &v, double t)
{
    return (1 / t) * v;
}

inline double dot(const Vector3d &u, const Vector3d &v)
{
    return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}

inline Vector3d cross(const Vector3d &u, const Vector3d &v)
{
    return Vector3d(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                    u.e[2] * v.e[0] - u.e[0] * v.e[2],
                    u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline Vector3d unit_vector(const Vector3d &v)
{
    return v / v.length();
}

inline Vector3d reflect(const Vector3d& v, const Vector3d& n)
{
    return v - 2 * dot(v, n) * n;
}

inline Vector3d refract(const Vector3d& uv, const Vector3d& n, double etai_over_etat)
{
    double cos_theta = fmin(dot(-uv, n), 1.0);
    Vector3d r_out_perpendicular = etai_over_etat * (uv + cos_theta * n);
    Vector3d r_out_parallel = - sqrt(1 - fabs(r_out_perpendicular.length_squared())) * n;
    return r_out_parallel + r_out_perpendicular;
}

using Point3d = Vector3d;

#pragma endregion

#endif