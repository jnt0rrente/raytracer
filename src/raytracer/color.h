#ifndef COLOR_H
#define COLOR_H

#include "vector3d.h"
#include <iostream>

using Color = Vector3d;

inline double linear_to_gamma(double linear_component);

void write_ppm_color(std::ostream& out, const Color& pixel_color);

#endif