#pragma once
#include <memory>
#include "Scene.h"
#include "DelusiveRenderer.h"

class Scene;

class GameManager {
public:
	GameManager(DelusiveRenderer&);

    void Init();
    void Update(float deltaTime);
    void Draw(const ColliderRenderer& renderer, const glm::mat4& projection);
    void HandleInput(const PlayerInputState& input);
    void HandleMouse(const glm::vec2& worldMouse, bool leftClick);

    void Play();
    void Stop();
    bool IsPlaying();

    Scene& GetActiveScene();
    Scene& GetEditorScene();
    void SetEditorScene(const Scene&);

private:
    DelusiveRenderer& renderer;
    Scene editorScene;
    Scene playScene;
    Scene* activeScene = nullptr;
    bool isPlaying;
};