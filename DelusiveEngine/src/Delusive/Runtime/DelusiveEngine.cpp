#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <Delusive/Internal/DelusiveEngine.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Internal/Rendering/ColliderRenderer.h>
#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Editor/EngineUI.h>
#include <Delusive/Runtime/Agents/DelusiveAgents.h>
#include <imgui/imgui.h>
#include <imgui/backend/imgui_impl_sdl3.h>
#include <imgui/backend/imgui_impl_opengl3.h>
#include <Delusive/Runtime/Agents/CameraAgent.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace {
    //Developer launch options, read from the environment only. They let a test run open
    //on a chosen monitor without taking focus, run at a low frame cap, start in an editor
    //mode with an asset open, and save one frame of its own window before quitting.
    //  DELUSIVE_DISPLAY      monitor name (or part of it) or index
    //  DELUSIVE_MAX_FPS      render cap for this run
    //  DELUSIVE_EDITOR_MODE  scene, agent, animator or ui
    //  DELUSIVE_OPEN         asset name to open in that mode
    //  DELUSIVE_CAPTURE      .ppm path written on the last frame
    //  DELUSIVE_QUIT_AFTER   frames to run before quitting
    //  DELUSIVE_WINDOW_SIZE  window size as WxH
    struct DevLaunch {
        std::string display;
        int maxFPS = 0;
        std::string editorMode;
        std::string openAsset;
        std::string capturePath;
        int quitAfter = 0;
        int windowWidth = 0;
        int windowHeight = 0;

        bool Active() const {
            return !display.empty() || maxFPS > 0 || !editorMode.empty() || quitAfter > 0;
        }
    };

    std::string Env(const char* name) {
        const char* value = std::getenv(name);
        return value ? value : "";
    }

    DevLaunch ReadDevLaunch() {
        DevLaunch dev;
        dev.display = Env("DELUSIVE_DISPLAY");
        dev.maxFPS = std::atoi(Env("DELUSIVE_MAX_FPS").c_str());
        dev.editorMode = Env("DELUSIVE_EDITOR_MODE");
        dev.openAsset = Env("DELUSIVE_OPEN");
        dev.capturePath = Env("DELUSIVE_CAPTURE");
        dev.quitAfter = std::atoi(Env("DELUSIVE_QUIT_AFTER").c_str());
        std::sscanf(Env("DELUSIVE_WINDOW_SIZE").c_str(), "%dx%d", &dev.windowWidth, &dev.windowHeight);
        return dev;
    }

    SDL_DisplayID FindDisplay(const std::string& wanted) {
        int count = 0;
        SDL_DisplayID* ids = SDL_GetDisplays(&count);
        SDL_DisplayID found = 0;

        for (int i = 0; i < count; ++i) {
            const char* name = SDL_GetDisplayName(ids[i]);
            SDL_Rect bounds{};
            SDL_GetDisplayBounds(ids[i], &bounds);
            std::cout << "[DevLaunch] Display " << i << ": " << (name ? name : "?") << " at "
                << bounds.x << "," << bounds.y << " " << bounds.w << "x" << bounds.h << "\n";

            if (!found && (wanted == std::to_string(i) || (name && std::string(name).find(wanted) != std::string::npos))) {
                found = ids[i];
            }
        }

        SDL_free(ids);
        return found;
    }

    //Reads back this window's own framebuffer - nothing else on screen is touched
    bool CaptureWindow(SDL_Window* window, const std::string& path) {
        int width = 0, height = 0;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

        std::ofstream out(path, std::ios::binary);
        out << "P6\n" << width << " " << height << "\n255\n";
        for (int y = height - 1; y >= 0; --y) {
            out.write(reinterpret_cast<const char*>(&pixels[static_cast<size_t>(y) * width * 3]), width * 3);
        }
        return static_cast<bool>(out);
    }

    bool ParseEditorMode(const std::string& name, EditorMode& mode) {
        if (name == "scene")    { mode = EditorMode::SceneEditor; return true; }
        if (name == "agent")    { mode = EditorMode::AgentEditor; return true; }
        if (name == "animator") { mode = EditorMode::AnimatorEditor; return true; }
        if (name == "ui")       { mode = EditorMode::UIBuilder; return true; }
        return false;
    }
}

namespace DelusiveEngine {
    
    int Run(const DelusiveContext& context) {
#ifdef _MSC_VER
        _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

        // --- SDL / OpenGL Setup ---
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
            return -1;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        const DevLaunch dev = ReadDevLaunch();

        SDL_Window* window = nullptr;
        if (!dev.display.empty()) {
            //Opened straight onto the chosen monitor, never shown elsewhere, never focused
            SDL_SetHint(SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN, "0");
            SDL_DisplayID display = FindDisplay(dev.display);
            if (!display) std::cerr << "[DevLaunch] No display matches '" << dev.display << "'\n";

            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, context.windowTitle);
            SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, dev.windowWidth > 0 ? dev.windowWidth : context.windowWidth);
            SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, dev.windowHeight > 0 ? dev.windowHeight : context.windowHeight);
            if (display) {
                SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));
                SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED_DISPLAY(display));
            }
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
            //The hint alone does not stop every window manager focusing a new window;
            //a non focusable window tells X11 it never takes input focus at all
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FOCUSABLE_BOOLEAN, false);
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
            window = display ? SDL_CreateWindowWithProperties(props) : nullptr;
            SDL_DestroyProperties(props);
        }
        else {
            window = SDL_CreateWindow(
                context.windowTitle,
                context.windowWidth,
                context.windowHeight,
                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
            );
        }
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

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        if (dev.Active()) {
            //Test runs keep every ImGui window inside this one and leave imgui.ini alone
            io.IniFilename = nullptr;
        }
        else {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        }

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f; // opaque
        }

        io.Fonts->AddFontDefault()->Scale = 1.5f;
        ImGui::StyleColorsDark();

        ImGui_ImplSDL3_InitForOpenGL(window, glctx);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        // --- Collider Renderer ---
        ColliderRenderer colliderRenderer;
        
        // --- Editor Camera ---
        auto editorCamera = std::make_unique<CameraAgent>(game.GetInstance());
        CameraAgent* editorCamPtr = editorCamera.get();

        float scrollDelta = 0.0f;
        bool running = true;
        SDL_Event e;
        uint64_t lastTicks = SDL_GetTicksNS();
        double tickAccumulator = 0.0;

        EngineUI ui(game);
        ui.LinkEditorCamera(editorCamPtr);

        EditorMode startMode;
        if (context.editorMode && ParseEditorMode(dev.editorMode, startMode)) {
            ui.StartIn(game.GetEditorScene(), startMode, dev.openAsset);
        }
        int frameCount = 0;

        const int maxFPS = dev.maxFPS > 0 ? dev.maxFPS : context.maxFPS;
        const uint64_t frameBudgetNS = maxFPS > 0 ? SDL_NS_PER_SECOND / maxFPS : 0;

        while (running) {
            const uint64_t frameStart = SDL_GetTicksNS();
            scrollDelta = 0.0f;

            // --- Event Polling ---
            Uint32 mainWindowID = SDL_GetWindowID(window);
            while (SDL_PollEvent(&e)) {
                ImGui_ImplSDL3_ProcessEvent(&e);
                if (e.type == SDL_EVENT_QUIT) running = false;
                if (e.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                    if (e.window.windowID == mainWindowID) {
                        running = false;
                    }
                }
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

            ImGuiIO& io = ImGui::GetIO();
            if (cam && !io.WantCaptureMouse) {
                cam->HandleInput({ mouseX, mouseY }, mouseState & SDL_BUTTON_MIDDLE, scrollDelta);
            }

            // --- Fixed Timestep ---
            //Gameplay runs in whole ticks so frame data plays the same at any refresh rate
            uint64_t currentTicks = SDL_GetTicksNS();
            double frameSeconds = (currentTicks - lastTicks) / 1e9;
            lastTicks = currentTicks;
            //A stall (breakpoint, window drag) would otherwise replay seconds of ticks at once
            tickAccumulator += std::min(frameSeconds, 0.25);

            // --- Clear / Update / Draw ---
            renderer.Clear();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

            //Each editor mode submits its own dock space - see EngineUI::DockSpace

            while (tickAccumulator >= DELUSIVE_TICK_SECONDS) {
                game.Update(DELUSIVE_TICK_SECONDS);
                tickAccumulator -= DELUSIVE_TICK_SECONDS;
            }

            int width, height;
            renderer.GetWindowSize(width, height);
            glm::mat4 projection = cam->GetViewProjectionFromWindow(window);
            glm::mat4 view = glm::inverse(cam->GetTransform().ToMatrix());
            renderer.SetViewProjection(view, projection);
            glm::vec2 worldMouse = ScreenToWorld2D(static_cast<int>(mouseX), static_cast<int>(mouseY), projection);

            //In the editor the mode being shown owns the mouse until Play
            if (!context.editorMode || game.IsPlaying())
                game.HandleMouse(worldMouse, mouseState & SDL_BUTTON_LEFT);
            game.Draw(colliderRenderer, projection);

            if (context.editorMode) {
                ui.Render(game.GetActiveScene());
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault(); // safely renders detached windows
                SDL_GL_MakeCurrent(window, glctx); // restore main context
            }

            if (dev.quitAfter > 0 && ++frameCount >= dev.quitAfter) {
                if (!dev.capturePath.empty()) CaptureWindow(window, dev.capturePath);
                running = false;
            }

            SDL_GL_SwapWindow(window);

            //Frame cap - sleep off whatever is left of this frame's budget
            if (frameBudgetNS > 0) {
                const uint64_t frameTime = SDL_GetTicksNS() - frameStart;
                if (frameTime < frameBudgetNS) {
                    SDL_DelayPrecise(frameBudgetNS - frameTime);
                }
            }
        }

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::DestroyPlatformWindows();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_MakeCurrent(window, nullptr);
        SDL_GL_DestroyContext(glctx);
        SDL_DestroyWindow(window);
        SDL_Quit();

#ifdef _MSC_VER
        _CrtDumpMemoryLeaks();
#endif
        return 0;
    }

    void Shutdown() {
        // Additional cleanup if needed
    }

}