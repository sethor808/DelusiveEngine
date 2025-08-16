#include <GL/glew.h>
#include <GL/GL.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include "Renderer.h"
#include "EngineUI.h"
#include "DelusiveAgents.h"
#include "GameManager.h"
#include "include/imgui/imgui.h"
#include "include/imgui/backend/imgui_impl_sdl3.h"
#include "include/imgui/backend/imgui_impl_opengl3.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>


int main(int argc, char** argv) {
    //Set up window
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_Window* window = SDL_CreateWindow("OpenGL Sprite", 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glctx);
    glewExperimental = GL_TRUE;
    glewInit();
    Renderer::Init();
    GameManager::Init();

    //IMGUI stuff
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault()->Scale = 1.5f;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, glctx);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    ColliderRenderer* gRenderer = new ColliderRenderer();
    ColliderComponent::SetRenderer(gRenderer);  // Called ONCE

    const float speed = 0.1f;
    float xVel = 0.0f, yVel = 0.0f;
    float deltaTime = 0.0f;
    uint64_t lastTicks = SDL_GetTicks();

    ColliderRenderer colliderRenderer;
    EngineUI ui;
    auto editorCamera = std::make_unique<CameraAgent>();
    CameraAgent* editorCamPtr = editorCamera.get();

    float scrollDelta = 0.0f;
    bool running = true;
    SDL_Event e;

    //Main loop
    while (running) {
        scrollDelta = 0.0f;

        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT || e.key.key == SDLK_ESCAPE) { running = false; }
            else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                int newWidth = e.window.data1;
                int newHeight = e.window.data2;

                Renderer::OnResize(newWidth, newHeight);
            }
            else if(e.type == SDL_EVENT_MOUSE_WHEEL){
                scrollDelta = (float)e.wheel.y;
            }
        }

        float mouseX, mouseY;
        uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        //Build input package
        PlayerInputState inputState;
        const bool* keys = SDL_GetKeyboardState(nullptr);
        inputState.moveDir.y += keys[SDL_SCANCODE_W];
        inputState.moveDir.y -= keys[SDL_SCANCODE_S];
        inputState.moveDir.x += keys[SDL_SCANCODE_D];
        inputState.moveDir.x -= keys[SDL_SCANCODE_A];
        if (glm::length(inputState.moveDir) > 0.0f)
            inputState.moveDir = glm::normalize(inputState.moveDir);

        // Attack / Dodge (keyboard)
        inputState.lightAttackPressed = keys[SDL_SCANCODE_J];  //TODO: change this
        inputState.heavyAttackPressed = keys[SDL_SCANCODE_K];
        inputState.dodgePressed = keys[SDL_SCANCODE_LSHIFT];

        GameManager::HandleInput(inputState);

        CameraAgent* cam = nullptr;
        if (GameManager::IsPlaying()) {
            cam = GameManager::GetActiveScene().GetMainCamera();
        }
        else {
            cam = editorCamPtr;
        }

        if (cam) {
            cam->HandleInput({ mouseX, mouseY }, mouseState & SDL_BUTTON_MIDDLE, scrollDelta);
        }

        uint64_t currentTicks = SDL_GetTicks();
        deltaTime = (currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;

        Renderer::Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        GameManager::Update(deltaTime);

        int width, height;
        Renderer::GetWindowSize(width, height);
        glm::mat4 projection = cam->GetViewProjectionFromWindow(window);
        glm::vec2 worldMouse = ScreenToWorld2D((int)mouseX, (int)mouseY, projection);

        GameManager::HandleMouse(worldMouse, mouseState & SDL_BUTTON_LEFT);
        GameManager::Draw(colliderRenderer, projection);

        ui.Render(GameManager::GetActiveScene());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_MakeCurrent(window, nullptr);
    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    _CrtDumpMemoryLeaks();
    return 0;
}
