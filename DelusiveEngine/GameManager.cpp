#include "GameManager.h"

Scene GameManager::editorScene;
Scene GameManager::playScene;
Scene* GameManager::activeScene = nullptr;
bool GameManager::isPlaying = false;

void GameManager::Init() {
    editorScene = Scene();
    activeScene = &editorScene;
    isPlaying = false;
}

void GameManager::Play() {
    editorScene.CloneInto(playScene);
    activeScene = &playScene;
    isPlaying = true;
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
}

void GameManager::HandleInput(const PlayerInputState& input) {
    activeScene->HandleInput(input);
}

void GameManager::HandleMouse(const glm::vec2& pos, bool leftClick) {
    activeScene->HandleMouse(pos, leftClick);
}