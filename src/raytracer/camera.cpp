#include "camera.h"
#include "../utils/math_utils.h"
#include "hittable.h"

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

std::atomic<int> finished_pixels{0};

Camera::Camera(int initial_width, int initial_height)
    : image_width(1080),
      image_height(0),
      samples_per_pixel(1),
      max_depth(10),
      vfov(90),
      lookFrom(Point3d(0, 0, 0)),
      lookAt(Point3d(0, 0, 0)),
      vector_up(Vector3d(0, 0, 0)),
      defocus_angle(0),
      focus_distance(10)
{
    aspect_ratio_width = initial_width;
    aspect_ratio_height = initial_height;
}

void Camera::render_multithread(const IHittable &world, std::vector<Color> &image_buffer, int num_threads, std::string &progress)
{
    std::vector<std::thread> threads(num_threads);
    int total_pixels = image_width * image_height;
    finished_pixels = 0;

    std::clog << "Running on " << num_threads << " threads.\n";

    for (int t = 0; t < num_threads; ++t)
    {

        threads[t] = std::thread([&, t]()
                                 {
                                        int start = t * image_height / num_threads;
                                        int end = (t == num_threads - 1) ? image_height : (t + 1) * image_height / num_threads; // last thread gets the rest :)

                                        for (int j = start; j < end; ++j)
                                        {
                                            for (int i = 0; i < image_width; i++)
                                            {
                                                Color pixel_color(0, 0, 0);

                                                for (int sample = 0; sample < samples_per_pixel; sample++)
                                                {
                                                    Ray r = get_ray(i, j);
                                                    pixel_color += ray_color(r, max_depth, world);
                                                }

                                                image_buffer[j * image_width + i] = pixel_samples_scale * pixel_color;

                                                finished_pixels++;
                                                progress = "Progress " + std::to_string(100 * finished_pixels / total_pixels) + "%";
                                            }
                                        } });
    }

    for (auto &t : threads)
    {
        t.join();
    }
}

void Camera::render_singlethread(const IHittable &world, std::vector<Color> &image_buffer, std::string &progress)
{
    std::clog << "Running on a single thread.\n";
    std::clog << "Image dimensions: " << image_width << "x" << image_height << "\n";
    int total_pixels = image_width * image_height;
    finished_pixels = 0;

    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            Color pixel_color(0, 0, 0);

            for (int sample = 0; sample < samples_per_pixel; sample++)
            {
                Ray r = get_ray(i, j);
                pixel_color += ray_color(r, max_depth, world);
            }

            image_buffer[j * image_width + i] = pixel_samples_scale * pixel_color;
        }

        finished_pixels += image_width;
        progress = "Progress " + std::to_string(100 * finished_pixels / total_pixels) + "%";
    }
}

std::vector<Color> Camera::render(const IHittable &world, unsigned int nthreads, std::string &progress)
{
    initialize();
    if (image_width <= 0 || image_height <= 0)
    {
        std::cerr << "Invalid image dimensions: " << image_width << "x" << image_height << std::endl;
        return {};
    }

    // string buffer for the image
    std::vector<Color> image_buffer(image_width * image_height);

#ifdef __EMSCRIPTEN__
    render_singlethread(world, image_buffer, progress);
    return image_buffer;
#endif

#ifndef __EMSCRIPTEN__
    if (nthreads < 1)
    {
        nthreads = 1;
    }

    if (nthreads > std::thread::hardware_concurrency())
    {
        std::cerr << "Warning: " << nthreads << " threads requested, but only " << std::thread::hardware_concurrency() << " available.\n";
        nthreads = std::thread::hardware_concurrency();
    }

    if (nthreads > 1)
    {
        render_multithread(world, image_buffer, nthreads, progress);
    }
    else
    {
        render_singlethread(world, image_buffer, progress);
    }

    return image_buffer;
#endif
}

void Camera::initialize()
{
    double aspect_ratio = aspect_ratio_width / aspect_ratio_height;
    // image sizes
    image_height = int(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;

    // multisampling
    pixel_samples_scale = 1.0 / samples_per_pixel;

    center = lookFrom;

    // camera
    // double focal_length = (lookFrom - lookAt).length();
    double theta = degrees_to_radians(vfov);
    double h = tan(theta / 2);
    double viewport_height = 2.0 * h * focus_distance;
    double viewport_width = viewport_height * (double(image_width) / image_height);

    w = unit_vector(lookFrom - lookAt);
    u = unit_vector(cross(vector_up, w));
    v = cross(w, u);

    Vector3d viewport_u = viewport_width * u;   // vector across horizontal edge
    Vector3d viewport_v = viewport_height * -v; // vector down viewport vertical edge

    // horizontal and vertical delta vectors, the "step" in each direction
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    // upper left pixel
    Point3d viewport_upper_left = center - (focus_distance * w) - viewport_u / 2 - viewport_v / 2;
    // middlepoint of the first pixel
    origin_pixel_location = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

    double defocus_radius = focus_distance * tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = u * defocus_radius;
    defocus_disk_v = v * defocus_radius;
}

Vector3d Camera::sample_square() const
{
    int factor = 1; // increasing this makes it all blurry
    // Returns the vector to a random point in the [-.5,-.5]-[+.5,+.5] unit square.
    return Vector3d(factor * (random_double() - 0.5), factor * (random_double() - 0.5), 0);
}

Ray Camera::get_ray(int i, int j) const
{
    // Construct a camera ray originating from the defocus disk and directed at a randomly
    // sampled point around the pixel location i, j.

    Vector3d offset = sample_square();
    Point3d target_point_sample = origin_pixel_location + ((i + offset.x()) * pixel_delta_u) + ((j + offset.y()) * pixel_delta_v);

    Point3d ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    Vector3d ray_direction = target_point_sample - ray_origin;

    return Ray(ray_origin, ray_direction);
}

Point3d Camera::defocus_disk_sample() const
{
    Vector3d p = Vector3d::random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
}

Color Camera::ray_color(const Ray &r, int depth, const IHittable &world) const
{
    if (depth <= 0)
        return Color(0.5, 0.5, 0.5);

    HitRecord rec;

    if (world.hit(r, Interval(0.001, infinity), rec))
    {
        Ray scattered;
        Color attenuation;

        if (rec.material->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, depth - 1, world);

        return Color(0.5, 0.5, 0.5);
    }

    Vector3d unit_direction = unit_vector(r.direction());

    double a = 0.5 * (unit_direction.y() + 1.0);
    return (1.0 - a) * Color(1.0, 1.0, 1.0) + a * Color(0.5, 0.7, 1.0); // sky gradient if no hit
}

void Camera::print_image_header(std::ostream &out, int image_width, int image_height)
{
    // render
    out << "P3\n"
        << image_width << ' ' << image_height << "\n255\n";
}
