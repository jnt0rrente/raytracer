#ifndef CAMERA_H
#define CAMERA_H

#include <atomic>
#include <vector>
#include "vector3d.h"
#include "material.h"
#include "color.h"
#include "ray.h"

extern std::atomic<int> finished_pixels; // for multithread progress tracking

class Camera
{
public:
    Camera(int initial_width, int initial_height);

    double aspect_ratio_width;
    double aspect_ratio_height;

    int image_width;
    int image_height;
    int samples_per_pixel;
    int max_depth;

    double vfov;
    Point3d lookFrom;
    Point3d lookAt;
    Vector3d vector_up;

    double defocus_angle;
    double focus_distance;

    void render_multithread(const IHittable &world, std::vector<Color> &image_buffer, int num_threads, std::string &progress);
    void render_singlethread(const IHittable &world, std::vector<Color> &image_buffer, std::string &progress);
    std::vector<Color> render(const IHittable &world, unsigned int nthreads, std::string &progress);

private:
    Point3d center;
    Point3d origin_pixel_location;
    Vector3d pixel_delta_u;
    Vector3d pixel_delta_v;
    Vector3d u, v, w; // camera frame basis vectors
    Vector3d defocus_disk_u;
    Vector3d defocus_disk_v;

    double pixel_samples_scale;
    void initialize();
    Vector3d sample_square() const;
    Ray get_ray(int i, int j) const;
    Point3d defocus_disk_sample() const;
    Color ray_color(const Ray &r, int depth, const IHittable &world) const;
    void print_image_header(std::ostream &out, int image_width, int image_height);

};

#endif