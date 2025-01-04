#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <mutex>

#include "utils/math_utils.h"
#include "scene.h"
#include "raytracer/material.h"
#include "raytracer/color.h"
#include "gui/interface.h"

using std::make_shared;
using std::shared_ptr;
using std::sqrt;
using std::fabs;

std::mutex renderTargetMutex;

bool initSDL(SDL_Window *&window, SDL_Renderer *&renderer, const char *glsl_version = nullptr)
{
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


    #ifdef __EMSCRIPTEN__
    int canvas_width = EM_ASM_INT({
        return Math.min(window.innerWidth, 1920);
    });
    int canvas_height = EM_ASM_INT({
        return Math.min(window.innerWidth, 1080);
    });
    window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, canvas_width, canvas_height, SDL_WINDOW_SHOWN);
    #endif
    #ifndef __EMSCRIPTEN__
    window = SDL_CreateWindow("Ray Tracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    #endif
    if (window == nullptr)
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
    {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    return true;
}

SDL_Texture *loadTexture(const RenderTarget& renderTarget, SDL_Renderer *renderer)
{
    int width = renderTarget.getWidth();
    int height = renderTarget.getHeight();

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == nullptr)
    {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    void *pixels;
    int pitch;
    if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
    {
        std::cerr << "SDL_LockTexture Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    uint32_t *pixels32 = static_cast<uint32_t *>(pixels);
    const std::vector<Color>& renderPixels = renderTarget.getPixels();
    for (int i = 0; i < width * height; ++i)
    {
        if (i >= renderPixels.size())
        {
            std::cerr << "Index out of bounds: " << i << std::endl;
            break;
        }

        Color pixel = renderPixels[i];
        pixels32[i] = SDL_MapRGBA(SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888), 
                                  static_cast<uint8_t>(pixel.x() * 255.999), 
                                  static_cast<uint8_t>(pixel.y() * 255.999), 
                                  static_cast<uint8_t>(pixel.z() * 255.999), 
                                  255);
    }

    SDL_UnlockTexture(texture);

    return texture;
}
struct MainLoopArgs {
    Uint32 frameStart;
    int frameTime;
    int frameDelay;
    bool done;
    SDL_Renderer *renderer;
    RayTracerInterface *rayTracerInterface;
    SDL_Texture *background_texture;
    SDL_Rect &background_rectangle;
    RenderTarget *renderTarget;
};

void mainLoop(void *arg)
{
    MainLoopArgs *args = static_cast<MainLoopArgs *>(arg);

    Uint32 frameStart = args->frameStart;
    int frameTime = args->frameTime;
    int frameDelay = args->frameDelay;
    SDL_Renderer *renderer = args->renderer;
    RayTracerInterface &rayTracerInterface = *args->rayTracerInterface;
    SDL_Texture *&background_texture = args->background_texture; // Reference to the texture
    SDL_Rect &background_rectangle = args->background_rectangle;
    RenderTarget *renderTarget = args->renderTarget;

    static std::string previousIdentifier;

    frameStart = SDL_GetTicks64();

    SDL_Event event{};
    while (SDL_PollEvent(&event) == 1)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT)
        {
            args->done = true;
        }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImVec2 windowSize = ImVec2(400, 600); // Set the desired width and height
    ImVec2 windowPos = ImVec2(50, 100);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Once);
    ImGui::SetNextWindowBgAlpha(0.8f);

    rayTracerInterface.ShowMainWindow(background_rectangle, *renderTarget);

    ImGui::Render();

    SDL_SetRenderDrawColor(renderer, 70, 80, 90, 255);
    SDL_RenderClear(renderer);

    {
        std::lock_guard<std::mutex> lock(renderTargetMutex);
        if (renderTarget != nullptr && renderTarget->getIdentifier() != previousIdentifier)
        {
            std::clog << "Updating texture" << std::endl;

            previousIdentifier = renderTarget->getIdentifier();
            if (background_texture != nullptr)
            {
                SDL_DestroyTexture(background_texture);
            }

            background_texture = loadTexture(*renderTarget, renderer);
        }
    }

    if (background_texture != nullptr)
    {
        SDL_RenderCopy(renderer, background_texture, NULL, &background_rectangle); // Copy the entire texture to the entire rendering target
    }

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

    SDL_RenderPresent(renderer);

    frameTime = SDL_GetTicks64() - frameStart;
    if (frameDelay > frameTime)
    {
        SDL_Delay(frameDelay - frameTime);
    }
}

int main()
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    const char *glsl_version = nullptr;

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;

    if (!initSDL(window, renderer, glsl_version))
    {
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    SDL_Rect background_rectangle;
    #ifdef __EMSCRIPTEN__
    int canvas_width = EM_ASM_INT({
        return document.getElementById('canvas').width;
    });

    int canvas_height = EM_ASM_INT({
        return document.getElementById('canvas').height;
    });

    background_rectangle.x = 0;
    background_rectangle.y = 0;
    background_rectangle.w = canvas_width;
    background_rectangle.h = canvas_height;
    #else
    background_rectangle.x = 20;
    background_rectangle.y = 20;
    background_rectangle.w = 1600;
    background_rectangle.h = 900;
    #endif

    SDL_Texture *background_texture = nullptr;
    RenderTarget *renderTarget = new RenderTarget(std::vector<Color>(), 0, 0);

    Scene scene = Scene(renderTarget, background_rectangle.w, background_rectangle.h).init();

    RayTracerInterface rayTracerInterface(scene);

    MainLoopArgs args = {
        .frameStart = 0,
        .frameTime = 0,
        .frameDelay = frameDelay,
        .done = false,
        .renderer = renderer,
        .rayTracerInterface = &rayTracerInterface,
        .background_texture = background_texture,
        .background_rectangle = background_rectangle,
        .renderTarget = renderTarget,
    };

    #ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(mainLoop, &args, 0, 1);
    #else
    while (!args.done) {
        mainLoop(&args);
    }
    #endif

    // cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(background_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}