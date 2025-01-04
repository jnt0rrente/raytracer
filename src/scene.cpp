#include "scene.h"

#include <mutex>

extern std::mutex renderTargetMutex;

Scene::Scene(RenderTarget *renderTarget, int camera_initial_width, int camera_initial_height) : world(World()), renderTarget(renderTarget), camera(Camera(camera_initial_width, camera_initial_height))
{
    materials = std::map<std::string, shared_ptr<IMaterial>>();
    addMaterial(MaterialFactory::createLambertian("default", Color(1, 1, 1)));
}

#pragma region camera settings

void Scene::setCameraPositionFrom(Point3d lookFrom)
{
    camera.lookFrom = lookFrom;
}

void Scene::setCameraPositionTo(Point3d lookAt)
{
    camera.lookAt = lookAt;
}

void Scene::setCameraUp(Vector3d vector_up)
{
    camera.vector_up = vector_up;
}

int Scene::getCameraWidth() const
{
    return camera.image_width;
}

int Scene::getCameraHeight() const
{
    return camera.image_height;
}
#pragma endregion

#pragma region world operations

void Scene::addMaterial(shared_ptr<IMaterial> material)
{
    if (materials.find(material->name) != materials.end())
    {
        materials[material->name] = material;
    }

    materials.insert({material->name, material});
}

void Scene::addObject(shared_ptr<IHittable> object)
{
    world.add(object);
}

#pragma endregion

#pragma region material operations

std::vector<const char *> Scene::getMaterialKeys()
{
    std::vector<const char *> keys;
    for (const auto &pair : materials)
    {
        keys.push_back(pair.first.c_str());
    }
    return keys;
}

void Scene::deleteMaterial(const std::string &name)
{
    if (materials.find(name) == materials.end())
    {
        return; // material not found
    }

    if (name == "default")
    {
        return; // cannot delete default material
    }

    // set the material of all objects with this material to the default material
    for (const auto &pair : world.objects)
    {
        shared_ptr<IHittable> object = pair.second;
        if (object->material->name == name)
        {
            object->material = materials["default"];
        }
    }

    materials.erase(name);
}

bool Scene::is_default_material(const std::string &name)
{
    return name == "default";
}

void Scene::setMaterialForObject(std::string object_name, std::string material_name)
{
    shared_ptr<IHittable> object = world.getObject(object_name);
    shared_ptr<IMaterial> material = materials[material_name];

    object->material = material;
}

int Scene::getMaterialIndexForName(std::string material_name)
{
    int i = 0;
    for (const auto &pair : materials)
    {
        if (pair.first == material_name)
        {
            return i;
        }
        i++;
    }

    return -1;
}

void Scene::updateMaterialName(const std::string &old_name, const std::string &new_name)
{
    // preserving order
    auto it = materials.find(old_name);
    if (it != materials.end())
    {
        shared_ptr<IMaterial> material = it->second;
        materials.erase(it);
        material->name = new_name;
        materials.insert({new_name, material});
    }
}

#pragma endregion

#pragma region rendering
void Scene::render(std::string &progress_string, int nthreads)
{
    std::clog << "Rendering..." << std::endl;

    std::vector<Color> rendered_image = camera.render(world, nthreads, progress_string);

    {
        std::lock_guard<std::mutex> lock(renderTargetMutex);
        delete renderTarget;
        renderTarget = new RenderTarget(rendered_image, camera.image_width, camera.image_height);
    }

    std::clog << "Rendering complete." << std::endl;
}

RenderTarget *Scene::getRenderTarget() const
{
    return renderTarget;
}
#pragma endregion

#pragma region init
Scene &Scene::init()
{
    // clear the world
    world.clear();

    // clear materials
    materials.clear();

    addMaterial(MaterialFactory::createLambertian("ground", Color(0.5, 0.8, 0.2)));
    addMaterial(MaterialFactory::createLambertian("matte", Color(0.1, 0.2, 0.5)));
    addMaterial(MaterialFactory::createDielectric("glass", Color(1, 1, 1), 1.5));
    addMaterial(MaterialFactory::createMetal("metal", Color(0.8, 0.8, 0.8), 0.2));

    addObject(HittableFactory::createSphere("ground_sphere", Point3d(0.0, -1000.0, 0.0), 1000, materials["ground"]));
    addObject(HittableFactory::createSphere("center_sphere", Point3d(0.0, 1.0, 0.0), 1, materials["matte"]));
    addObject(HittableFactory::createSphere("glass_sphere", Point3d(-3.0, 1.0, 0.0), 1, materials["glass"]));
    addObject(HittableFactory::createSphere("metal_sphere", Point3d(3.0, 1.0, 0.0), 1, materials["metal"]));

    setCameraPositionFrom(Point3d(0, 2, 8));
    setCameraPositionTo(Point3d(0, 1, 0));
    setCameraUp(Vector3d(0, 1, 0));

    return *this;
}
#pragma endregion