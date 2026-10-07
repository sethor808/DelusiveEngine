#pragma once
#include <Delusive/Runtime/Animation/AnimationSet.h>
#include <Delusive/Runtime/Core/DelusiveInstance.h>
#include <Delusive/Runtime/Editor/EditHistory.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class Agent;
class ColliderComponent;

//The editor's animator mode: one character's AnimationSet. Branches and the transitions
//between them on the left, the selected frame with its pivot and boxes on the canvas, the
//frame's details on the right, and the branch's frames on a timeline whose tiles are as
//wide as they last. The character's own colliders can be adjusted here too; they are
//saved back to its agent file.
class AnimatorEditor {
public:
    explicit AnimatorEditor(DelusiveInstance&);
    ~AnimatorEditor();

    bool Open(const std::string& path);
    //Writes the set to path, and the character's agent file if its colliders changed
    bool Save(const std::string& path);
    bool Create(const std::string& path, const std::string& name);
    void Close();
    bool HasSet() const { return set != nullptr; }
    bool IsDirty() const { return setDirty || characterDirty; }

    void Render();
    //The top bar's Undo/Redo and their shortcuts land here
    void Undo();
    void Redo();
    bool CanUndo() const { return set && (history.CanUndo() || pendingChange); }
    bool CanRedo() const { return set && history.CanRedo(); }

    //Window names - EngineUI docks them into the animator layout the first time
    static constexpr const char* SetWindow = "Animation set###AnimatorSet";
    static constexpr const char* CanvasWindow = "Canvas###AnimatorCanvas";
    static constexpr const char* FrameWindow = "Frame###AnimatorFrame";
    static constexpr const char* TimelineWindow = "Timeline###AnimatorTimeline";

private:
    enum class Tool { Select, DrawHurt, DrawHit };
    enum class Drag { None, Pan, Pivot, BoxMove, BoxResize, BoxCreate, Collider };

    DelusiveInstance& instance;
    std::unique_ptr<DelusiveAnimation::Set> set;
    //Loaded from the set's character file for its colliders - never placed in a scene
    std::unique_ptr<Agent> character;
    std::string characterPath;
    bool setDirty = false;
    bool characterDirty = false;
    //The default pivot combo stays on Custom even when the fraction matches a preset
    bool customDefaultPivot = false;

    //Selection
    int branch = -1;
    int frame = -1;
    int box = -1;
    int transition = -1;
    bool transitionIsAny = false;
    ColliderComponent* collider = nullptr;

    //Canvas view
    Tool tool = Tool::Select;
    bool showColliders = true;
    bool flip = false;
    float zoom = 1.0f;
    glm::vec2 pan{ 0.0f };
    bool fitPending = true;

    //Preview playback runs at the game's tick rate
    bool playing = false;
    bool loopPreview = true;
    float tickAccumulator = 0.0f;
    int playTick = 0;

    //Canvas drag in progress
    Drag drag = Drag::None;
    int dragHandle = 0;
    glm::vec2 dragStart{ 0.0f };
    glm::vec2 dragMin{ 0.0f };
    glm::vec2 dragMax{ 0.0f };
    glm::vec2 dragOrigin{ 0.0f };

    std::vector<std::string> imageFiles;
    char imageFilter[64] = "";

    //Undo snapshots the whole set plus the character's collider shapes - both are small,
    //and copying them is simpler and safer than recording each kind of edit
    struct ColliderState {
        glm::vec2 position{ 0.0f };
        glm::vec2 scale{ 1.0f };
        float rotation = 0.0f;
        int shape = 0;
        bool operator==(const ColliderState&) const = default;
    };
    struct Snapshot {
        DelusiveAnimation::Set set;
        std::vector<ColliderState> colliders;
        int branch = -1;
        int frame = -1;
    };
    EditHistory<Snapshot> history;
    //An edit happened that is not committed yet - it commits once nothing is being dragged or typed
    bool pendingChange = false;

    Snapshot Capture();
    void Restore(const Snapshot&);
    void SettleEdits();

    void SetPanel();
    void CanvasPanel();
    void InspectorPanel();
    void TimelinePanel();
    void TransitionList(std::vector<DelusiveAnimation::Transition>& list, bool fromAny);
    std::string Summary(const DelusiveAnimation::Transition&, bool fromAny);
    //Popup listing the sprite folder; true when an image was picked
    bool ImagePicker(const char* popupID, std::string& path);
    void LinkCharacter(const UUID& characterID);
    void AdvancePreview();

    DelusiveAnimation::Branch* CurrentBranch();
    DelusiveAnimation::Frame* CurrentFrame();
    int BranchTicks();
    int FrameStartTick(int index);
    int FrameAtTick(int tick);
    void SelectFrame(int index);
    void Touch() { setDirty = true; pendingChange = true; }
    void TouchCharacter() { characterDirty = true; pendingChange = true; }
};
