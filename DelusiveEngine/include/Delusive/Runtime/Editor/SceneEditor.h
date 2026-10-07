#pragma once
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <Delusive/Runtime/Editor/EditHistory.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <vector>

class Scene;
class Agent;
class Component;
class SceneSystem;
class CameraAgent;

//The editor's scene mode: the scene's systems and agents on the left, the selection's
//fields and components on the right, and the game view between them where agents are
//clicked to select and dragged to move. Every edit can be undone - the whole scene is
//snapshotted as the blocks it would save, and restored by rebuilding from them.
class SceneEditor {
public:
    explicit SceneEditor(DelusiveInstance&);

    //playing: scene is the play copy - shown live, but not recorded or picked in the view
    void Render(Scene&, bool playing);
    //Strip along the bottom of the window - saved state, counts, cursor position
    void RenderStatusBar(Scene&, bool playing);

    //The scene was replaced (opened, created, cleared) - history and the saved mark start over
    void SceneLoaded(Scene&);
    //The scene was written to its file as it is now
    void SceneSaved(Scene&);
    void ClearSelection(Scene&);

    //The top bar's Undo/Redo and their shortcuts land here
    void Undo(Scene&);
    void Redo(Scene&);
    bool CanUndo() const { return history.CanUndo() || checkPending; }
    bool CanRedo() const { return history.CanRedo(); }
    bool IsDirty() const { return history.Committed().blocks != saved; }

    void LinkEditorCamera(CameraAgent* cam) { editorCamera = cam; }

    //Window names - EngineUI docks them into the scene layout the first time
    static constexpr const char* HierarchyWindow = "Hierarchy###SceneHierarchy";
    static constexpr const char* InspectorWindow = "Inspector###SceneInspector";

private:
    using Blocks = std::vector<DelusiveParser::DataBlock>;

    //Held by id, never by pointer - undo rebuilds every agent and component
    struct Selection {
        enum class Kind { None, Agent, System } kind = Kind::None;
        UUID id;            //The agent or system
        UUID component;     //Within the agent; invalid when the agent itself is selected
    };
    struct Snapshot {
        Blocks blocks;
        Selection selection;
    };

    DelusiveInstance& instance;
    CameraAgent* editorCamera = nullptr;

    Selection selection;
    //The component whose editor mode is on (collider handles); resolved by id to turn it off
    UUID editingAgent;
    UUID editingComponent;
    //Open the selected component's header and scroll the inspector to it once
    bool revealComponent = false;

    EditHistory<Snapshot> history;
    Blocks saved;
    bool started = false;
    //Something may have changed - checked against the last step once nothing is active
    bool checkPending = false;
    bool itemWasActive = false;

    //View mouse
    bool pressInView = false;
    bool movingAgent = false;
    bool movedAgent = false;
    bool collidersWereDragging = false;
    glm::vec2 grabOffset{ 0.0f };
    ImVec2 pressScreen{ 0.0f, 0.0f };

    ImGuiTextFilter filter;
    char sceneName[128] = "";
    bool editingSceneName = false;

    //Structural edits wait until the panels are done drawing the lists they change
    std::vector<std::function<void(Scene&)>> deferred;

    Blocks Capture(Scene&) const;
    void Restore(Scene&, const Snapshot&);
    //Rebuilds the scene from its own blocks so nothing keeps a pointer to what was removed
    void Rebuild(Scene&);
    void Touch() { checkPending = true; }
    void SettleEdits(Scene&);
    bool Resolves(Scene&, const Selection&) const;

    Agent* SelectedAgent(Scene&) const;
    Component* SelectedComponent(Scene&) const;
    SceneSystem* SelectedSystem(Scene&) const;
    void SelectAgent(const UUID& agent, const UUID& component = UUID());
    void SelectSystem(const UUID& system);
    void SyncEditorMode(Scene&);

    void ViewMouse(Scene&);
    void DrawSelectionOutline(Scene&);
    void HierarchyPanel(Scene&, bool playing);
    void InspectorPanel(Scene&, bool playing);
    void AddMenu(Scene&);
    void Shortcuts(Scene&);

    //Structural edits - deferred, each ends with Touch
    void AddAgent(const std::string& type);
    void AddPrefab(const std::string& path);
    void AddSystem(const std::string& type);
    void DuplicateAgent(const UUID& agent);
    void DeleteAgent(const UUID& agent);
    void RemoveComponent(const UUID& agent, const UUID& component);
    void MoveComponent(const UUID& agent, const UUID& component, int delta);
    void AddComponent(const UUID& agent, const std::string& type);
    void RemoveSystem(const UUID& system);
    void Focus(Scene&, const UUID& agent);

    glm::vec2 ViewCentre() const;
};
