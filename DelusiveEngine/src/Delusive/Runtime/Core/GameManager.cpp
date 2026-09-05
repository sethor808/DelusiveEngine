#include <Delusive/Runtime/Core/GameManager.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>


GameManager::GameManager(DelusiveRenderer& renderer)
	: instance(renderer, idManager, scriptManager), editorScene(instance), playScene(instance)
{
    Init();
}

void GameManager::Init() {
    scriptManager.Init();
    isPlaying = false;
    activeScene = &editorScene;
    editorScene.LinkGameManager(this);
	playScene.LinkGameManager(this);
    inventory.Link(this);
}

void GameManager::Play() {
    editorScene.CloneInto(playScene);
    if (playScene.HasCamera()) {
        activeScene = &playScene;
        isPlaying = true;
    }
}

void GameManager::Stop() {
    activeScene = &editorScene;
    playScene.Clear();
    isPlaying = false;
}

bool GameManager::IsPlaying() {
    return isPlaying;
}

Scene& GameManager::GetActiveScene() { return *activeScene; }
Scene& GameManager::GetEditorScene() { return editorScene; }

void GameManager::SetEditorScene(const Scene& newScene) {
    newScene.CloneInto(editorScene);
    if (!isPlaying) {
        activeScene = &editorScene;
    }
}

void GameManager::Update(float deltaTime) {
    if (!isPlaying) return;

    activeScene->Update(deltaTime);
}

void GameManager::Draw(const ColliderRenderer& renderer, const glm::mat4& projection) {
    activeScene->Draw(renderer, projection);
    this->instance.renderer.Flush(); //TODO: Fix the naming here it is confusing
}

void GameManager::HandleInput(const PlayerInputState& input) {
    activeScene->HandleInput(input);
}

void GameManager::HandleMouse(const glm::vec2& pos, bool leftClick) {
    activeScene->HandleMouse(pos, leftClick);
}