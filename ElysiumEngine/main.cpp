#include <GL/glew.h>
#include <GL/GL.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer.h"
#include "Shader.h"
#include "Sprite.h"
#include "Texture.h"
#include "SpriteEditor.h"
#include "EngineUI.h"
#include "PlayerAgent.h"
#include "include/imgui/imgui.h"
#include "include/imgui/backend/imgui_impl_sdl3.h"
#include "include/imgui/backend/imgui_impl_opengl3.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>


int main(int argc, char** argv) {
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

    //IMGUI stuff
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontDefault()->Scale = 1.5f;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForOpenGL(window, glctx);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Sprite sprite("C:/Users/Demon Teddy/Documents/Programs/ElysiumEngine/ElysiumEngine/assets/sprites/star.jpg");
    
    ColliderRenderer* gRenderer = new ColliderRenderer();
    ColliderComponent::SetRenderer(gRenderer);  // Called ONCE

    const float speed = 0.1f;
    sprite.SetVelocity(0.0f, 0.0f);
    float xVel = 0.0f, yVel = 0.0f;
    float deltaTime = 0.0f;
    uint64_t lastTicks = SDL_GetTicks();

    SpriteEditor editor;
    ColliderRenderer colliderRenderer;
    EngineUI ui;
    Scene scene("Test");
    std::vector<std::unique_ptr<Agent>> agents;
    //agents.push_back(std::make_unique<PlayerAgent>("assets/sprites/star.png", "Player1"));
    scene.AddAgent(std::make_unique<PlayerAgent>("Player1"));

    float scrollDelta = 0.0f;
    bool running = true;
    SDL_Event e;
    while (running) {
        scrollDelta = 0.0f;

        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL3_ProcessEvent(&e);
            if (e.type == SDL_EVENT_QUIT || e.key.key == SDLK_ESCAPE) { running = false; }
            else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                int newWidth = e.window.data1;
                int newHeight = e.window.data2;

                Renderer::OnResize(newWidth, newHeight);

                sprite.SetPosition(newWidth / 2.0f, newHeight / 2.0f);
            }
            else if(e.type == SDL_EVENT_MOUSE_WHEEL){
                scrollDelta = (float)e.wheel.y;
            }
            const bool* keys = SDL_GetKeyboardState(0);
            yVel = (keys[SDL_SCANCODE_W]) * speed + (keys[SDL_SCANCODE_S]) * -speed;
            xVel = (keys[SDL_SCANCODE_D]) * speed + (keys[SDL_SCANCODE_A]) * -speed;
            //std::cout << "X vel: " << xVel << " | Y vel: " << yVel << std::endl;
            sprite.SetVelocity(xVel, yVel);
        }

        // Handle camera input
        CameraAgent* cam = scene.GetCamera();
        if (cam) {
            float mx, my;
            uint32_t mouseState = SDL_GetMouseState(&mx, &my);
            cam->HandleInput({ mx, my }, mouseState & SDL_BUTTON_MIDDLE, scrollDelta);
        }

        uint64_t currentTicks = SDL_GetTicks();
        deltaTime = (currentTicks - lastTicks) / 1000.0f;
        lastTicks = currentTicks;
        sprite.Update(deltaTime);

        editor.HandleMouse(window, sprite, Renderer::GetProjection());

        Renderer::Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        /*
        for (const auto& agent : agents) {
            agent->Draw(colliderRenderer, Renderer::GetProjection());
        }
        */
        //editor.DrawUI(sprite);
        scene.Update(deltaTime);
        
        glm::mat4 projection = cam ? cam->GetViewProjection(800, 600) : Renderer::GetProjection();
        scene.Draw(colliderRenderer, projection);

        sprite.UpdateCollider();
        
        //sprite.Draw(colliderRenderer, Renderer::GetProjection());

        ui.Render(scene);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_MakeCurrent(window, nullptr); // disconnect the context
    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    _CrtDumpMemoryLeaks();
    return 0;
}
