#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>
#include <limits>

const double infinity = std::numeric_limits<double>::infinity();

#pragma region definitions
const double pi = 3.1415926535897932385;

// utilities
inline double degrees_to_radians(double angle_in_degrees)
{
    return angle_in_degrees * (pi / 180.0);
}

inline double radians_to_degrees(double angle_in_radians)
{
    return angle_in_radians * (180.0 / pi);
}

/**
 * Return a random double in the interval [0,1)
 */
inline double random_double()
{
    return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

#pragma endregion

#pragma region interval

class Interval
{
public:
    double min, max;
    Interval();
    Interval(double min, double max);
    double size() const;
    bool contains(double x) const;
    bool surrounds(double x) const;
    double clamp(double x) const;

    static const Interval empty, universe;
};

#pragma endregion

#endif