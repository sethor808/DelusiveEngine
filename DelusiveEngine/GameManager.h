#pragma once
#include <memory>
#include "Scene.h"

class Scene;

class GameManager {
public:
    static void Init();
    static void Update(float deltaTime);
    static void Draw(const ColliderRenderer& renderer, const glm::mat4& projection);
    static void HandleInput(const PlayerInputState& input);
    static void HandleMouse(const glm::vec2& worldMouse, bool leftClick);

    static void Play();
    static void Stop();
    static bool IsPlaying();

    static Scene& GetActiveScene();
    static Scene& GetEditorScene();
    static void SetEditorScene(const Scene&);

private:
    static Scene editorScene;
    static Scene playScene;
    static Scene* activeScene;
    static bool isPlaying;
};