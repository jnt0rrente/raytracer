#include "interface.h"
#include "interface_utils.h"
#include "imgui.h"
#include "./misc/cpp/imgui_stdlib.h"

#include "../scene.h"
#include "../raytracer/hittable.h"

#include <iterator>

RayTracerInterface::RayTracerInterface(Scene scene)
    : nthreads(1),
      auto_render(false),
      progress_message(""),
      is_rendering(false),
      selected_world_object(0),
      selected_object_material(0),
      selected_scene_material(0),
      last_elapsed_time(0),
      scene(scene)
{
    scene.render(progress_message, 1);
}

void RayTracerInterface::resetScene()
{
    scene.init();
    scene.render(progress_message, 1);
}

void RayTracerInterface::ShowMainWindow(SDL_Rect &background_rectangle, RenderTarget &renderTarget)
{


    ImGui::Begin("Ray Tracer", nullptr, ImGuiWindowFlags_MenuBar);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Reset scene", "Ctrl+R"))
            {
                resetScene();
            }
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                renderTarget.save_image("png");
            }
            if (ImGui::MenuItem("Close", "Ctrl+W"))
            {
                std::exit(0);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (ImGui::Button("Render"))
    {
        is_rendering = true;
        render_ms_start = SDL_GetTicks64();

        #ifdef __EMSCRIPTEN__
        scene.render(progress_message, nthreads);
        #endif
        #ifndef __EMSCRIPTEN__
        render_future = std::async(std::launch::async, [&]()
                                   { scene.render(progress_message, nthreads); });
        #endif
    }

    // ImGui::SameLine();
    // if (ImGui::Button("Abort"))
    // {
    //     //implement atomic
    // }

    // ImGui::SameLine();
    // if (ImGui::Button("Clear"))
    // {
    //     render_ms_start = 0;
    //     render_ms_end = 0;
    //     last_elapsed_time = 0;
    // }

    #ifdef __EMSCRIPTEN__
    ImGui::Text("Multithread and async are unsupported in web assembly!");

    //defaults for threads and auto render
    nthreads = 1;
    auto_render = false;
    #endif
    #ifndef __EMSCRIPTEN__
    ImGui::InputInt("Threads", &nthreads, 1, 10);
    // ImGui::SameLine();
    // ImGui::Checkbox("Auto Render", &auto_render);
    #endif
    
    if (render_future.valid() && render_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
    {
        ImGui::Text("Rendering...");
        ImGui::Text("%s", progress_message.c_str());
    }
    else
    {
        if (is_rendering)
        {
            render_ms_end = SDL_GetTicks64();
            last_elapsed_time = render_ms_end - render_ms_start;
            is_rendering = false;
        }
    }

    if (!is_rendering)
    {
        ImGui::Text("Finished rendering in %llu ms", last_elapsed_time ? last_elapsed_time : 0);
    }

    if (ImGui::CollapsingHeader("Camera"))
    {
        ImGui::SeparatorText("Location");
        ImGui::Text("Origin");
        CustomInputDoubleWithLabel("X", &scene.camera.lookFrom.e[0]);
        CustomInputDoubleWithLabel("Y", &scene.camera.lookFrom.e[1]);
        CustomInputDoubleWithLabel("Z", &scene.camera.lookFrom.e[2]);
        ImGui::Text("Target");
        CustomInputDoubleWithLabel("X", &scene.camera.lookAt.e[0]);
        CustomInputDoubleWithLabel("Y", &scene.camera.lookAt.e[1]);
        CustomInputDoubleWithLabel("Z", &scene.camera.lookAt.e[2]);
        ImGui::Text("Up vector");
        CustomInputDoubleWithLabel("X", &scene.camera.vector_up.e[0]);
        CustomInputDoubleWithLabel("Y", &scene.camera.vector_up.e[1]);
        CustomInputDoubleWithLabel("Z", &scene.camera.vector_up.e[2]);
    }

    if (ImGui::CollapsingHeader("Image"))
    {
        ImGui::SeparatorText("Size");
        CustomInputDoubleWithLabel("Aspect Ratio Width", &scene.camera.aspect_ratio_width);
        CustomInputDoubleWithLabel("Aspect Ratio Height", &scene.camera.aspect_ratio_height);
        ImGui::InputInt("Image Width", &scene.camera.image_width);

        ImGui::SeparatorText("Focus blur");
        CustomInputDoubleWithLabel("Defocus angle", &scene.camera.defocus_angle);
        CustomInputDoubleWithLabel("Focus distance", &scene.camera.focus_distance);

        ImGui::SeparatorText("Parameters");
        ImGui::InputInt("Samples", &scene.camera.samples_per_pixel, 1, 10);
        ImGui::InputInt("Max Depth", &scene.camera.max_depth);

        ImGui::SeparatorText("Output");
        ImGui::InputInt("Origin X", &background_rectangle.x);
        ImGui::InputInt("Origin Y", &background_rectangle.y);
        ImGui::InputInt("Width", &background_rectangle.w);
        ImGui::InputInt("Height", &background_rectangle.h);
    }

    if (ImGui::CollapsingHeader("World"))
    {
        ImGui::SeparatorText("Objects");
        ImGui::PushID("ObjectsTable##");
        if (scene.world.objects.size() <= 0 || scene.world.getObjectKeys().size() <= 0)
        {
            ImGui::Text("No objects in the world");
        }
        else
        {
            std::vector<std::string> objectKeysStr = scene.world.getObjectKeys();
            std::vector<const char *> objectKeys;
            objectKeys.reserve(objectKeysStr.size());
            for (const auto& key : objectKeysStr) {
                objectKeys.push_back(key.c_str());
            }
            ImGui::ListBox(getLabelForValue("", selected_world_object).c_str(), &selected_world_object, objectKeys.data(), objectKeys.size());
            ImGui::SameLine();
            ImGui::BeginGroup();
            if (ImGui::Button("Delete"))
            {
                const char *toDelete = objectKeys[selected_world_object];

                selected_world_object -= 1;

                scene.world.remove(toDelete);

                if (selected_world_object < 0)
                {
                    selected_world_object = 0;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Edit"))
            {
                ImGui::OpenPopup("world_object_edit_popup");
            }
            
            if (ImGui::BeginPopup("world_object_edit_popup"))
            {
                ImGui::SeparatorText("Object properties");
                IHittable *selectedObject = scene.world.getObject(objectKeys[selected_world_object]).get();
                ImGui::InputText("Object name", &selectedObject->name);
                ImGui::SameLine();
                if (ImGui::Button("Set name"))
                {
                    if (selectedObject->name != "")
                    {
                        scene.world.updateObjectName(objectKeys[selected_world_object], selectedObject->name);
                    }
                }

                ImGui::SeparatorText("Material");
                std::vector<const char *> materialKeys = scene.getMaterialKeys();

                selected_object_material = scene.getMaterialIndexForName(selectedObject->material->name);
                // If the material was not found in the vector, set the index to -1
                if (selected_object_material >= materialKeys.size())
                {
                    selected_object_material = -1;
                }
                // on change list, update object material
                if (ImGui::Combo(getLabelForValue("", selected_object_material).c_str(), &selected_object_material, materialKeys.data(), materialKeys.size()))
                {
                    // Get the new material
                    const char *newMaterial = materialKeys[selected_object_material];

                    // Get the selected object
                    const char *object = objectKeys[selected_world_object];

                    // Update the material of the object
                    scene.setMaterialForObject(object, newMaterial);
                }

                ImGui::SeparatorText("Edit attributes");

                selectedObject->accept(&visitor);

                ImGui::EndPopup();
            }

            ImGui::EndGroup();
        }
        ImGui::PopID();

        ImGui::SeparatorText("Materials");
        ImGui::PushID("MaterialsTable##");
        if (scene.materials.size() <= 0 || scene.world.getObjectKeys().size() <= 0)
        {
            ImGui::Text("No materials in the world");
        }
        else
        {
            std::vector<const char *> materialKeys = scene.getMaterialKeys();
            ImGui::ListBox(getLabelForValue("", selected_scene_material).c_str(), &selected_scene_material, materialKeys.data(), materialKeys.size());
            ImGui::SameLine();

            ImGui::BeginDisabled(scene.is_default_material(materialKeys[selected_scene_material]));
            if (ImGui::Button("Delete"))
            {
                const char *toDelete = materialKeys[selected_scene_material];

                selected_scene_material -= 1;

                scene.deleteMaterial(toDelete);

                if (selected_scene_material < 0)
                {
                    selected_scene_material = 0;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Edit"))
            {
                ImGui::OpenPopup("world_material_edit_popup");
            }
            ImGui::EndDisabled();

            if (ImGui::BeginPopup("world_material_edit_popup"))
            {
                ImGui::SeparatorText("Material properties");
                IMaterial *selectedMaterial = scene.materials[materialKeys[selected_scene_material]].get();
                ImGui::InputText("Material name", &selectedMaterial->name);
                ImGui::SameLine();
                if (ImGui::Button("Set name"))
                {
                    if (selectedMaterial->name != "")
                    {
                        scene.updateMaterialName(materialKeys[selected_scene_material], selectedMaterial->name);
                    }
                }

                selectedMaterial->accept(&visitor);

                ImGui::EndPopup();
            }
        }
        ImGui::PopID();
    }

    ImGui::End();
}

