#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include "DelusiveEngine.h"
#include "DelusiveRenderer.h"
#include "GameManager.h"
#include "EngineUI.h"
#include "DelusiveAgents.h"
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>
#include "CameraAgent.h"
#include <crtdbg.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace DelusiveEngine {
    
    int Run(const DelusiveContext& context) {
        _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);

        // --- SDL / OpenGL Setup ---
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
            return -1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        SDL_Window* window = SDL_CreateWindow(
            context.windowTitle,
            context.windowWidth,
            context.windowHeight,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );
        if (!window) {
            std::cerr << "SDL_CreateWindow failed\n";
            return -1;
        }

        SDL_GLContext glctx = SDL_GL_CreateContext(window);
        SDL_GL_MakeCurrent(window, glctx);

        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            std::cerr << "glewInit failed\n";
            return -1;
        }

        DelusiveRenderer renderer;
        renderer.Init();
        GameManager game(renderer);

        // --- ImGui Setup ---
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.Fonts->AddFontDefault()->Scale = 1.5f;
        ImGui::StyleColorsDark();

        ImGui_ImplSDL3_InitForOpenGL(window, glctx);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        // --- Collider Renderer ---
        ColliderRenderer colliderRenderer;
        

        // --- Editor Camera ---
        auto editorCamera = std::make_unique<CameraAgent>();
        CameraAgent* editorCamPtr = editorCamera.get();

        float scrollDelta = 0.0f;
        bool running = true;
        SDL_Event e;
        uint64_t lastTicks = SDL_GetTicks();

        EngineUI ui(game, renderer);

        while (running) {
            scrollDelta = 0.0f;

            // --- Event Polling ---
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL3_ProcessEvent(&e);
                if (e.type == SDL_EVENT_QUIT) running = false;
                else if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                    int newWidth = e.window.data1;
                    int newHeight = e.window.data2;
                    renderer.OnResize(newWidth, newHeight);
                }
                else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
                    scrollDelta = static_cast<float>(e.wheel.y);
                }
            }

            // --- Mouse / Keyboard ---
            float mouseX, mouseY;
            uint32_t mouseState = SDL_GetMouseState(&mouseX, &mouseY);

            PlayerInputState inputState;
            const bool* keys = SDL_GetKeyboardState(nullptr);
            inputState.moveDir.y += keys[SDL_SCANCODE_W];
            inputState.moveDir.y -= keys[SDL_SCANCODE_S];
            inputState.moveDir.x += keys[SDL_SCANCODE_D];
            inputState.moveDir.x -= keys[SDL_SCANCODE_A];

            if (glm::length(inputState.moveDir) > 0.0f)
                inputState.moveDir = glm::normalize(inputState.moveDir);

            inputState.lightAttackPressed = keys[SDL_SCANCODE_J];
            inputState.heavyAttackPressed = keys[SDL_SCANCODE_K];
            inputState.dodgePressed = keys[SDL_SCANCODE_LSHIFT];

            game.HandleInput(inputState);

            CameraAgent* cam = nullptr;
            if (!context.editorMode && game.IsPlaying())
                cam = game.GetActiveScene().GetMainCamera();
            else
                cam = editorCamPtr;

            if (cam) {
                cam->HandleInput({ mouseX, mouseY }, mouseState & SDL_BUTTON_MIDDLE, scrollDelta);
            }

            // --- Delta Time ---
            uint64_t currentTicks = SDL_GetTicks();
            float deltaTime = (currentTicks - lastTicks) / 1000.0f;
            lastTicks = currentTicks;

            // --- Clear / Update / Draw ---
            renderer.Clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            game.Update(deltaTime);

            int width, height;
            renderer.GetWindowSize(width, height);
            glm::mat4 projection = cam->GetViewProjectionFromWindow(window);
            glm::vec2 worldMouse = ScreenToWorld2D(static_cast<int>(mouseX), static_cast<int>(mouseY), projection);

            game.HandleMouse(worldMouse, mouseState & SDL_BUTTON_LEFT);
            game.Draw(colliderRenderer, projection);

            if (context.editorMode) {
                ui.Render(game.GetActiveScene());
            }

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

    void Shutdown() {
        // Additional cleanup if needed
    }

}