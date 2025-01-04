#include "color.h"
#include "../utils/math_utils.h"

inline double linear_to_gamma(double linear_component)
{
    if (linear_component > 0)
    {
        return sqrt(linear_component);
    }

    return 0;
}

void write_ppm_color(std::ostream& out, const Color& pixel_color)
{
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // Translate the [0,1] component values to the byte range [0,255].
    static const Interval intensity(0.000, 0.999);
    int rbyte = int(255.999 * intensity.clamp(r));
    int gbyte = int(255.999 * intensity.clamp(g));
    int bbyte = int(255.999 * intensity.clamp(b));

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}