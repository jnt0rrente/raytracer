#ifndef SCENE_H
#define SCENE_H

#include <map>
#include <string>
#include <iostream>
#include <memory>
using std::shared_ptr;

#include "raytracer/camera.h"
#include "raytracer/world.h"
#include "raytracer/material.h"
#include "raytracer/hittable.h"
#include "raytracer/vector3d.h"
#include "raytracer/renderTarget.h"

class Scene
{
public:
    World world;
    Camera camera;
    RenderTarget* renderTarget;
    std::map<std::string, shared_ptr<IMaterial>> materials;

    Scene(RenderTarget* renderTarget, int camera_initial_width, int camera_initial_height);
    Scene& init();

#pragma region camera settings
    void setCameraPositionFrom(Point3d lookFrom);
    void setCameraPositionTo(Point3d lookAt);
    void setCameraUp(Vector3d vector_up);

    int getCameraWidth() const;
    int getCameraHeight() const;
#pragma endregion

#pragma region world operations
    void addMaterial(shared_ptr<IMaterial> material);
    void addObject(shared_ptr<IHittable> object);
#pragma endregion

#pragma region material operations
    std::vector<const char *> getMaterialKeys();
    void setMaterialForObject(std::string object_name, std::string material_name);  
    int getMaterialIndexForName(std::string material_name);
    void deleteMaterial(const std::string& name);
    bool is_default_material(const std::string& name);
    void updateMaterialName(const std::string& old_name, const std::string& new_name);
#pragma endregion

#pragma region rendering
    void render(std::string &progress_string, int nthreads);
    RenderTarget* getRenderTarget() const;
#pragma endregion 
};

#endif