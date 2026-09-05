#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/UUIDManager.h>
#include <memory>
#include <Delusive/Runtime/Scene/Scene.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Scripting/ScriptManager.h>
#include <Delusive/Runtime/Player/DelusiveInventory.h>

class Scene;

class GameManager {
public:
	GameManager(DelusiveRenderer&);

    DelusiveInstance& GetInstance() { return instance; }

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
    DelusiveInventory* GetInventoryLink() { return &inventory; }
    void SetEditorScene(const Scene&);

    ScriptManager& GetScriptManager() { return scriptManager; }
private:
    DelusiveInstance instance;
    UUIDManager idManager;
    ScriptManager scriptManager;
    DelusiveInventory inventory;
    Scene editorScene;
    Scene playScene;
    Scene* activeScene = nullptr;
    bool isPlaying;
};