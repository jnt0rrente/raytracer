#ifndef INTERFACE_H
#define INTERFACE_H

#include <SDL2/SDL.h>

#include <future>
#include <string>
#include "../scene.h"
#include "gui_visitor.h"

class RayTracerInterface
{
public:
    int nthreads;
    std::future<void> render_future;
    bool auto_render;

    std::string progress_message;
    bool is_rendering;
    Uint64 render_ms_start;
    Uint64 render_ms_end;
    Uint64 last_elapsed_time;

    int selected_world_object;
    int selected_object_material;
    int selected_scene_material;

    ImGuiVisitor visitor;
    
    RayTracerInterface(Scene scene);

    void ShowMainWindow(SDL_Rect &background_rectangle, RenderTarget &renderTarget);
    void resetScene();

    Scene scene;
};

#endif