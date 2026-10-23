#include <Delusive/Runtime/Editor/AnimatorEditor.h>
#include <Delusive/Runtime/Agents/Agent.h>
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <Delusive/Runtime/Core/PhysicsSystem.h>
#include <Delusive/Internal/Rendering/DelusiveRenderer.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <imgui/imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iomanip>
#include <sstream>

using namespace DelusiveAnimation;

namespace {
    //ImGui's default font is ASCII only, so labels here stay ASCII too

    bool InputString(const char* label, std::string& value) {
        char buffer[256];
        std::snprintf(buffer, sizeof(buffer), "%s", value.c_str());
        if (!ImGui::InputText(label, buffer, sizeof(buffer))) return false;
        value = buffer;
        return true;
    }

    ImTextureRef Tex(unsigned int id) {
        return ImTextureRef(static_cast<ImTextureID>(id));
    }

    int TicksToMs(int ticks) {
        return static_cast<int>(std::lround(ticks * 1000.0 / DELUSIVE_TICKS_PER_SECOND));
    }

    std::string Unquote(const std::string& raw) {
        std::istringstream in(raw);
        std::string out;
        in >> std::quoted(out);
        return out;
    }

    glm::vec2 Round(glm::vec2 v) {
        return { std::round(v.x), std::round(v.y) };
    }

    //Frame boxes match the collider renderer's colours; character colliders are dashed
    const ImU32 HurtColour = IM_COL32(55, 138, 221, 255);
    const ImU32 HitColour = IM_COL32(212, 83, 126, 255);
    const ImU32 SolidColour = IM_COL32(226, 75, 74, 255);
    const ImU32 TriggerColour = IM_COL32(239, 159, 39, 255);
    const ImU32 PivotColour = IM_COL32(250, 199, 117, 255);
    const ImU32 HandleColour = IM_COL32(240, 240, 240, 255);

    ImU32 WithAlpha(ImU32 colour, int alpha) {
        return (colour & 0x00FFFFFF) | (static_cast<ImU32>(alpha) << 24);
    }

    ImU32 ColliderColour(ColliderType type) {
        switch (type) {
        case ColliderType::Hitbox:  return HitColour;
        case ColliderType::Hurtbox: return HurtColour;
        case ColliderType::Trigger: return TriggerColour;
        default:                    return SolidColour;
        }
    }

    void DashedLine(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 colour, float thickness) {
        const float dash = 6.0f, gap = 4.0f;
        const ImVec2 d(b.x - a.x, b.y - a.y);
        const float length = std::sqrt(d.x * d.x + d.y * d.y);
        if (length < 0.001f) return;
        const ImVec2 u(d.x / length, d.y / length);
        for (float s = 0.0f; s < length; s += dash + gap) {
            const float e = std::min(s + dash, length);
            dl->AddLine(ImVec2(a.x + u.x * s, a.y + u.y * s), ImVec2(a.x + u.x * e, a.y + u.y * e), colour, thickness);
        }
    }

    void DashedRect(ImDrawList* dl, ImVec2 mn, ImVec2 mx, ImU32 colour, float thickness) {
        DashedLine(dl, mn, ImVec2(mx.x, mn.y), colour, thickness);
        DashedLine(dl, ImVec2(mx.x, mn.y), mx, colour, thickness);
        DashedLine(dl, mx, ImVec2(mn.x, mx.y), colour, thickness);
        DashedLine(dl, ImVec2(mn.x, mx.y), mn, colour, thickness);
    }

    bool Near(ImVec2 a, ImVec2 b, float radius) {
        const float dx = a.x - b.x, dy = a.y - b.y;
        return dx * dx + dy * dy <= radius * radius;
    }

    bool Inside(ImVec2 p, ImVec2 mn, ImVec2 mx) {
        return p.x >= mn.x && p.x <= mx.x && p.y >= mn.y && p.y <= mx.y;
    }

    float SegmentDistance(ImVec2 p, ImVec2 a, ImVec2 b) {
        const glm::vec2 ab(b.x - a.x, b.y - a.y), ap(p.x - a.x, p.y - a.y);
        const float lengthSq = glm::dot(ab, ab);
        const float t = lengthSq > 0.0f ? std::clamp(glm::dot(ap, ab) / lengthSq, 0.0f, 1.0f) : 0.0f;
        return glm::length(ap - ab * t);
    }

    //Rect handles in image space (y down): 1-4 edges left right top bottom, 5-8 corners
    glm::vec2 HandlePos(int handle, glm::vec2 mn, glm::vec2 mx) {
        const glm::vec2 c = (mn + mx) * 0.5f;
        switch (handle) {
        case 1: return { mn.x, c.y };
        case 2: return { mx.x, c.y };
        case 3: return { c.x, mn.y };
        case 4: return { c.x, mx.y };
        case 5: return mn;
        case 6: return { mx.x, mn.y };
        case 7: return { mn.x, mx.y };
        case 8: return mx;
        default: return c;
        }
    }

    //The grabbed edges follow the point, the opposite ones stay put
    void ApplyHandle(int handle, glm::vec2 p, glm::vec2& mn, glm::vec2& mx, float minSize) {
        const bool left = handle == 1 || handle == 5 || handle == 7;
        const bool right = handle == 2 || handle == 6 || handle == 8;
        const bool top = handle == 3 || handle == 5 || handle == 6;
        const bool bottom = handle == 4 || handle == 7 || handle == 8;
        if (left)   mn.x = std::min(p.x, mx.x - minSize);
        if (right)  mx.x = std::max(p.x, mn.x + minSize);
        if (top)    mn.y = std::min(p.y, mx.y - minSize);
        if (bottom) mx.y = std::max(p.y, mn.y + minSize);
    }

    //Maps image pixels to the screen. The view is anchored on the pivot, so the character
    //stays planted while stepping through frames - registration errors show as movement.
    struct CanvasView {
        glm::vec2 centre;
        glm::vec2 pan;
        float zoom;
        glm::vec2 pivot;
        bool flip;

        ImVec2 ToScreen(glm::vec2 px) const {
            float dx = (px.x - pivot.x) * zoom;
            if (flip) dx = -dx;
            return ImVec2(centre.x + pan.x + dx, centre.y + pan.y + (px.y - pivot.y) * zoom);
        }

        glm::vec2 ToImage(ImVec2 s) const {
            float dx = (s.x - centre.x - pan.x) / zoom;
            if (flip) dx = -dx;
            return { pivot.x + dx, pivot.y + (s.y - centre.y - pan.y) / zoom };
        }

        void ScreenRect(glm::vec2 mn, glm::vec2 mx, ImVec2& smn, ImVec2& smx) const {
            const ImVec2 a = ToScreen(mn), b = ToScreen(mx);
            smn = ImVec2(std::min(a.x, b.x), std::min(a.y, b.y));
            smx = ImVec2(std::max(a.x, b.x), std::max(a.y, b.y));
        }
    };

    glm::vec2 SafeScale(glm::vec2 s) {
        auto safe = [](float v) { return std::abs(v) < 1e-6f ? (v < 0.0f ? -1e-6f : 1e-6f) : v; };
        return { safe(s.x), safe(s.y) };
    }

    //Character colliders live in agent units around the agent's origin, which the pivot
    //marks in the image; x right and y up there, y down in the image
    struct ColliderInImage {
        ShapeType shape;
        glm::vec2 min, max, centre, end;
        float radius;
    };

    ColliderInImage ToImage(const ColliderComponent& collider, glm::vec2 pivot, float ppu) {
        const WorldShape s = PhysicsSystem::BuildShape(collider);
        const glm::vec2 origin = collider.GetOwner()->GetTransform().position;
        auto P = [&](glm::vec2 world) {
            const glm::vec2 r = world - origin;
            return glm::vec2(pivot.x + r.x * ppu, pivot.y - r.y * ppu);
        };

        ColliderInImage out;
        out.shape = s.shape;
        const glm::vec2 a = P(s.min), b = P(s.max);
        out.min = glm::min(a, b);
        out.max = glm::max(a, b);
        out.centre = P(s.center);
        out.end = P(s.end);
        out.radius = s.radius * ppu;
        return out;
    }

    void SetBoxFromImage(ColliderComponent& collider, glm::vec2 mn, glm::vec2 mx, glm::vec2 pivot, float ppu) {
        const glm::vec2 scale = SafeScale(collider.GetOwner()->GetTransform().scale);
        const glm::vec2 a((mn.x - pivot.x) / ppu, (pivot.y - mx.y) / ppu);
        const glm::vec2 b((mx.x - pivot.x) / ppu, (pivot.y - mn.y) / ppu);
        collider.transform->position = (a + b) * 0.5f / scale;
        collider.transform->scale = (b - a) / glm::abs(scale);
    }

    const char* PivotModeLabels[] = { "Set default", "Bottom centre", "Centre", "Top centre", "Custom" };
    const char* DefaultPivotLabels[] = { "Bottom centre", "Centre", "Top centre", "Custom" };

    template<typename F>
    void ForEachTransition(Set& set, F&& visit) {
        for (Branch& branch : set.branches) {
            for (Transition& t : branch.transitions) visit(t);
        }
        for (Transition& t : set.anyTransitions) visit(t);
    }
}

AnimatorEditor::AnimatorEditor(DelusiveInstance& instance)
    : instance(instance)
{
}

AnimatorEditor::~AnimatorEditor() = default;

#pragma region Files

bool AnimatorEditor::Open(const std::string& path) {
    Close();

    auto loaded = std::make_unique<Set>();
    if (!loaded->LoadFromFile(path, instance.delusiveLibrary)) return false;

    set = std::move(loaded);
    LinkCharacter(set->character);

    customDefaultPivot = true;
    for (int mode = 1; mode <= 3; ++mode) {
        if (set->defaultPivot == Set::PresetFraction(static_cast<PivotMode>(mode))) customDefaultPivot = false;
    }

    branch = set->branches.empty() ? -1 : 0;
    SelectFrame(CurrentBranch() && !CurrentBranch()->frames.empty() ? 0 : -1);
    history.Reset(Capture());
    return true;
}

bool AnimatorEditor::Save(const std::string& path) {
    if (!set) return false;

    bool saved = set->SaveToFile(path, instance.delusiveLibrary);
    if (saved) setDirty = false;

    if (character && characterDirty && !characterPath.empty()) {
        if (character->SaveToFile(characterPath)) characterDirty = false;
        else saved = false;
    }
    return saved;
}

bool AnimatorEditor::Create(const std::string& path, const std::string& name) {
    Close();

    set = std::make_unique<Set>();
    set->name = name;
    Branch idle;
    idle.name = "Idle";
    idle.loop = true;
    set->branches.push_back(idle);
    branch = 0;
    history.Reset(Capture());

    return Save(path);
}

void AnimatorEditor::Close() {
    set.reset();
    character.reset();
    characterPath.clear();
    collider = nullptr;
    setDirty = characterDirty = false;
    branch = frame = box = transition = -1;
    transitionIsAny = false;
    playing = false;
    playTick = 0;
    tickAccumulator = 0.0f;
    drag = Drag::None;
    tool = Tool::Select;
    fitPending = true;
    history.Reset(Snapshot{});
    pendingChange = false;
}

void AnimatorEditor::LinkCharacter(const UUID& characterID) {
    character.reset();
    characterPath.clear();
    collider = nullptr;
    characterDirty = false;
    if (!characterID.IsValid()) return;

    const std::string path = instance.delusiveLibrary.GetSourceFile(characterID);
    if (path.empty()) return;

    //A standalone copy, so editing its colliders touches nothing else until saved
    character = Agent::LoadFromFile(path, instance, nullptr);
    if (character) characterPath = path;
}

#pragma endregion

#pragma region History

AnimatorEditor::Snapshot AnimatorEditor::Capture() {
    Snapshot snap;
    snap.set = *set;
    snap.branch = branch;
    snap.frame = frame;
    if (character) {
        for (ColliderComponent* c : character->GetComponentsOfType<ColliderComponent>()) {
            snap.colliders.push_back({ c->transform->position, c->transform->scale, c->transform->rotation,
                static_cast<int>(c->GetShapeType()) });
        }
    }
    return snap;
}

void AnimatorEditor::Restore(const Snapshot& snap) {
    const UUID linked = set->character;
    *set = snap.set;
    if (set->character != linked) LinkCharacter(set->character);

    if (character) {
        const std::vector<ColliderComponent*> colliders = character->GetComponentsOfType<ColliderComponent>();
        if (colliders.size() == snap.colliders.size()) {
            for (size_t i = 0; i < colliders.size(); ++i) {
                ColliderComponent& c = *colliders[i];
                const ColliderState now{ c.transform->position, c.transform->scale, c.transform->rotation, static_cast<int>(c.GetShapeType()) };
                if (now == snap.colliders[i]) continue;
                c.transform->position = snap.colliders[i].position;
                c.transform->scale = snap.colliders[i].scale;
                c.transform->rotation = snap.colliders[i].rotation;
                c.SetShapeType(static_cast<ShapeType>(snap.colliders[i].shape));
                characterDirty = true;
            }
        }
    }

    //Back to where the edit was made, as far as it still exists
    branch = std::clamp(snap.branch, -1, (int)set->branches.size() - 1);
    const Branch* b = CurrentBranch();
    playing = false;
    drag = Drag::None;
    transition = -1;
    SelectFrame(b ? std::clamp(snap.frame, -1, (int)b->frames.size() - 1) : -1);
    setDirty = true;
}

void AnimatorEditor::SettleEdits() {
    if (!pendingChange || ImGui::IsAnyItemActive() || drag != Drag::None) return;
    history.Commit(Capture());
    pendingChange = false;
}

void AnimatorEditor::Undo() {
    if (!set || drag != Drag::None) return;
    if (pendingChange) {
        history.Commit(Capture());
        pendingChange = false;
    }
    if (auto snap = history.Undo(Capture())) Restore(*snap);
}

void AnimatorEditor::Redo() {
    if (!set || drag != Drag::None) return;
    if (pendingChange) {
        history.Commit(Capture());
        pendingChange = false;
    }
    if (auto snap = history.Redo(Capture())) Restore(*snap);
}

#pragma endregion

#pragma region Selection

Branch* AnimatorEditor::CurrentBranch() {
    if (!set || branch < 0 || branch >= (int)set->branches.size()) return nullptr;
    return &set->branches[branch];
}

Frame* AnimatorEditor::CurrentFrame() {
    Branch* b = CurrentBranch();
    if (!b || frame < 0 || frame >= (int)b->frames.size()) return nullptr;
    return &b->frames[frame];
}

int AnimatorEditor::BranchTicks() {
    const Branch* b = CurrentBranch();
    int total = 0;
    if (b) for (const Frame& f : b->frames) total += f.ticks;
    return total;
}

int AnimatorEditor::FrameStartTick(int index) {
    const Branch* b = CurrentBranch();
    int start = 0;
    for (int i = 0; b && i < index && i < (int)b->frames.size(); ++i) start += b->frames[i].ticks;
    return start;
}

int AnimatorEditor::FrameAtTick(int tick) {
    const Branch* b = CurrentBranch();
    if (!b || b->frames.empty()) return -1;
    int start = 0;
    for (int i = 0; i < (int)b->frames.size(); ++i) {
        start += b->frames[i].ticks;
        if (tick < start) return i;
    }
    return (int)b->frames.size() - 1;
}

void AnimatorEditor::SelectFrame(int index) {
    frame = index;
    box = -1;
    if (index >= 0) playTick = FrameStartTick(index);
}

void AnimatorEditor::AdvancePreview() {
    if (!playing) return;

    const int total = BranchTicks();
    if (total <= 0) {
        playing = false;
        return;
    }

    tickAccumulator += ImGui::GetIO().DeltaTime * DELUSIVE_TICKS_PER_SECOND;
    while (tickAccumulator >= 1.0f) {
        tickAccumulator -= 1.0f;
        if (++playTick >= total) {
            if (loopPreview) {
                playTick = 0;
            }
            else {
                playTick = total - 1;
                playing = false;
                break;
            }
        }
    }

    const int shown = FrameAtTick(playTick);
    if (shown != frame) {
        frame = shown;
        box = -1;
    }
}

#pragma endregion

void AnimatorEditor::Render() {
    if (set) AdvancePreview();

    //Four dockable windows - EngineUI arranges them the first time the animator opens
    auto Panel = [this](const char* name, ImGuiWindowFlags flags, void (AnimatorEditor::*draw)(), bool sidePanel) {
        if (!ImGui::Begin(name, nullptr, flags) || !set) {
            if (!set && draw == &AnimatorEditor::CanvasPanel)
                ImGui::TextDisabled("Pick an animation set from the list above, or make one with Add New...");
            ImGui::End();
            return;
        }
        //Side panels keep room for labels to the right of their inputs
        if (sidePanel) {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.55f);
            ImGui::PushTextWrapPos(0.0f);
        }
        (this->*draw)();
        if (sidePanel) {
            ImGui::PopTextWrapPos();
            ImGui::PopItemWidth();
        }
        ImGui::End();
    };

    Panel(SetWindow, ImGuiWindowFlags_None, &AnimatorEditor::SetPanel, true);
    Panel(CanvasWindow, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, &AnimatorEditor::CanvasPanel, false);
    Panel(FrameWindow, ImGuiWindowFlags_None, &AnimatorEditor::InspectorPanel, true);
    Panel(TimelineWindow, ImGuiWindowFlags_NoScrollbar, &AnimatorEditor::TimelinePanel, false);

    if (set) SettleEdits();
}

#pragma region Set panel

void AnimatorEditor::SetPanel() {
    DelusiveLibrary& library = instance.delusiveLibrary;

    ImGui::SeparatorText("Set");
    if (InputString("Name", set->name)) Touch();
    if (ImGui::DragFloat("PPU", &set->pixelsPerUnit, 1.0f, 1.0f, 4096.0f, "%.0f")) Touch();

    //Character: agent files the library knows. Linking reloads the library entries for
    //that file, so the pick is applied after the list is done with
    std::string current = "<none>";
    if (set->character.IsValid()) {
        const DelusiveParser::DataBlock* block = library.Find(set->character);
        auto name = block ? block->properties.find("name") : decltype(block->properties.end()){};
        current = block && name != block->properties.end()
            ? Unquote(name->second) + " (" + std::filesystem::path(library.GetSourceFile(set->character)).filename().string() + ")"
            : "<missing agent>";
    }
    bool pickCharacter = false;
    UUID picked;
    if (ImGui::BeginCombo("Character", current.c_str())) {
        if (ImGui::Selectable("<none>", !set->character.IsValid())) pickCharacter = true;
        for (const DelusiveParser::DataBlock* block : library.List("Agent")) {
            const std::filesystem::path file = library.GetSourceFile(block->id);
            if (file.extension() != AGENT_EXT) continue; //Agents placed in scenes are not characters

            auto name = block->properties.find("name");
            const std::string label = (name != block->properties.end() ? Unquote(name->second) : "Agent")
                + " (" + file.filename().string() + ")";
            ImGui::PushID(block);
            if (ImGui::Selectable(label.c_str(), block->id == set->character)) {
                pickCharacter = true;
                picked = block->id;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    if (pickCharacter) {
        set->character = picked;
        LinkCharacter(picked);
        Touch();
    }

    int defaultMode = 3;
    if (!customDefaultPivot) {
        for (int mode = 0; mode < 3; ++mode) {
            if (set->defaultPivot == Set::PresetFraction(static_cast<PivotMode>(mode + 1))) defaultMode = mode;
        }
    }
    if (ImGui::Combo("Default pivot", &defaultMode, DefaultPivotLabels, 4)) {
        customDefaultPivot = defaultMode == 3;
        if (!customDefaultPivot) set->defaultPivot = Set::PresetFraction(static_cast<PivotMode>(defaultMode + 1));
        Touch();
    }
    if (defaultMode == 3 && ImGui::DragFloat2("Fraction", &set->defaultPivot.x, 0.01f, 0.0f, 1.0f)) Touch();

    //Flags - what gameplay sets for transitions to read
    ImGui::SeparatorText("Flags");
    int eraseFlag = -1;
    for (int i = 0; i < (int)set->flags.size(); ++i) {
        Flag& flag = set->flags[i];
        ImGui::PushID(i);
        const std::string before = flag.name;
        const float kindWidth = ImGui::CalcTextSize("Trigger").x + ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float removeWidth = ImGui::CalcTextSize("x").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - kindWidth - removeWidth - ImGui::GetStyle().ItemSpacing.x * 2.0f);
        if (InputString("##name", flag.name)) {
            //Conditions follow a rename
            ForEachTransition(*set, [&](Transition& t) {
                for (Condition& c : t.conditions) if (c.flag == before) c.flag = flag.name;
            });
            Touch();
        }
        ImGui::SameLine();
        int kind = static_cast<int>(flag.kind);
        ImGui::SetNextItemWidth(kindWidth);
        if (ImGui::Combo("##kind", &kind, "Bool\0Trigger\0")) {
            flag.kind = static_cast<FlagKind>(kind);
            Touch();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("x")) eraseFlag = i;
        ImGui::PopID();
    }
    if (eraseFlag >= 0) {
        set->flags.erase(set->flags.begin() + eraseFlag);
        Touch();
    }
    if (ImGui::SmallButton("+ Add flag")) {
        set->flags.push_back({ "Flag " + std::to_string(set->flags.size() + 1), FlagKind::Bool });
        Touch();
    }

    //Branches
    ImGui::SeparatorText("Branches");
    for (int i = 0; i < (int)set->branches.size(); ++i) {
        ImGui::PushID(i);
        if (ImGui::Selectable(set->branches[i].name.c_str(), branch == i)) {
            branch = i;
            transition = -1;
            playing = false;
            SelectFrame(set->branches[i].frames.empty() ? -1 : 0);
        }
        ImGui::PopID();
    }

    if (ImGui::SmallButton("Add")) {
        Branch added;
        added.name = "Branch " + std::to_string(set->branches.size() + 1);
        set->branches.push_back(added);
        branch = (int)set->branches.size() - 1;
        SelectFrame(-1);
        Touch();
    }
    if (Branch* b = CurrentBranch()) {
        ImGui::SameLine();
        if (ImGui::SmallButton("Copy")) {
            Branch copy = *b;
            copy.id = UUID::GenerateRandom();
            copy.name += " copy";
            for (Frame& f : copy.frames) f.id = UUID::GenerateRandom();
            for (Transition& t : copy.transitions) t.id = UUID::GenerateRandom();
            set->branches.insert(set->branches.begin() + branch + 1, copy);
            ++branch;
            Touch();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Delete")) {
            const UUID gone = b->id;
            set->branches.erase(set->branches.begin() + branch);
            ForEachTransition(*set, [&](Transition& t) { if (t.target == gone) t.target = UUID(); });
            branch = std::min(branch, (int)set->branches.size() - 1);
            transition = -1;
            SelectFrame(CurrentBranch() && !CurrentBranch()->frames.empty() ? 0 : -1);
            Touch();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##branchUp", ImGuiDir_Up) && branch > 0) {
            std::swap(set->branches[branch], set->branches[branch - 1]);
            --branch;
            Touch();
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##branchDown", ImGuiDir_Down) && branch + 1 < (int)set->branches.size()) {
            std::swap(set->branches[branch], set->branches[branch + 1]);
            ++branch;
            Touch();
        }
    }

    if (Branch* b = CurrentBranch()) {
        ImGui::SeparatorText(b->name.c_str());
        if (InputString("Name##branch", b->name)) Touch();
        if (ImGui::Checkbox("Loop", &b->loop)) Touch();
        ImGui::SameLine();
        if (ImGui::Checkbox("Lock input", &b->lockInput)) Touch();

        ImGui::TextDisabled("Goes to (first match wins)");
        ImGui::PushID("branchTransitions");
        TransitionList(b->transitions, false);
        ImGui::PopID();
    }

    ImGui::SeparatorText("Any branch");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("Checked first, from every branch");
    ImGui::PopTextWrapPos();
    ImGui::PushID("anyTransitions");
    TransitionList(set->anyTransitions, true);
    ImGui::PopID();
}

std::string AnimatorEditor::Summary(const Transition& t, bool fromAny) {
    const Branch* target = set->FindBranch(t.target);
    std::string text = "-> " + (target ? target->name : std::string("<choose target>"));

    for (const Condition& c : t.conditions) {
        const Flag* flag = set->FindFlag(c.flag);
        text += ", ";
        if (!flag)                               text += c.flag + " (missing)";
        else if (flag->kind == FlagKind::Trigger) text += c.flag;
        else                                     text += c.flag + (c.value ? "" : " off");
    }
    if (!fromAny && (t.fromFrame > 0 || t.toFrame > 0)) {
        text += ", frames " + (t.fromFrame > 0 ? std::to_string(t.fromFrame) : std::string("start"))
            + "-" + (t.toFrame > 0 ? std::to_string(t.toFrame) : std::string("end"));
    }
    if (t.onFinish) text += ", when finished";
    return text;
}

void AnimatorEditor::TransitionList(std::vector<Transition>& list, bool fromAny) {
    const bool ownsSelection = transitionIsAny == fromAny;

    for (int i = 0; i < (int)list.size(); ++i) {
        ImGui::PushID(i);
        const std::string summary = Summary(list[i], fromAny);
        if (ImGui::Selectable(summary.c_str(), ownsSelection && transition == i)) {
            transition = (ownsSelection && transition == i) ? -1 : i;
            transitionIsAny = fromAny;
        }
        ImGui::SetItemTooltip("%s", summary.c_str()); //Long summaries clip in a narrow panel
        ImGui::PopID();
    }

    if (ImGui::SmallButton("+ Add transition")) {
        list.push_back(Transition());
        transition = (int)list.size() - 1;
        transitionIsAny = fromAny;
        Touch();
    }

    if (transitionIsAny != fromAny || transition < 0 || transition >= (int)list.size()) return;

    Transition& t = list[transition];
    ImGui::Indent();

    const Branch* target = set->FindBranch(t.target);
    if (ImGui::BeginCombo("Target", target ? target->name.c_str() : "<choose>")) {
        for (const Branch& b : set->branches) {
            ImGui::PushID(&b);
            if (ImGui::Selectable(b.name.c_str(), b.id == t.target)) {
                t.target = b.id;
                Touch();
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if (!fromAny) {
        if (ImGui::Checkbox("When finished", &t.onFinish)) Touch();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (ImGui::InputInt("From frame", &t.fromFrame)) { t.fromFrame = std::max(t.fromFrame, 0); Touch(); }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (ImGui::InputInt("To frame", &t.toFrame)) { t.toFrame = std::max(t.toFrame, 0); Touch(); }
        ImGui::TextDisabled("0 leaves that end open");
    }

    ImGui::Text("Conditions");
    int eraseCondition = -1;
    for (int c = 0; c < (int)t.conditions.size(); ++c) {
        Condition& cond = t.conditions[c];
        ImGui::PushID(c);
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.55f);
        if (ImGui::BeginCombo("##flag", cond.flag.empty() ? "<flag>" : cond.flag.c_str())) {
            for (const Flag& flag : set->flags) {
                if (ImGui::Selectable(flag.name.c_str(), flag.name == cond.flag)) {
                    cond.flag = flag.name;
                    Touch();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        const Flag* flag = set->FindFlag(cond.flag);
        if (flag && flag->kind == FlagKind::Bool) {
            if (ImGui::Checkbox("on", &cond.value)) Touch();
        }
        else {
            ImGui::TextDisabled(flag ? "fired" : "missing");
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("x")) eraseCondition = c;
        ImGui::PopID();
    }
    if (eraseCondition >= 0) {
        t.conditions.erase(t.conditions.begin() + eraseCondition);
        Touch();
    }
    if (ImGui::SmallButton("+ Condition")) {
        t.conditions.push_back({ set->flags.empty() ? std::string() : set->flags.front().name, true });
        Touch();
    }

    //Order is priority
    if (ImGui::ArrowButton("##transitionUp", ImGuiDir_Up) && transition > 0) {
        std::swap(list[transition], list[transition - 1]);
        --transition;
        Touch();
    }
    ImGui::SameLine();
    if (ImGui::ArrowButton("##transitionDown", ImGuiDir_Down) && transition + 1 < (int)list.size()) {
        std::swap(list[transition], list[transition + 1]);
        ++transition;
        Touch();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Delete transition")) {
        list.erase(list.begin() + transition);
        transition = -1;
        Touch();
    }

    ImGui::Unindent();
}

#pragma endregion

#pragma region Canvas

void AnimatorEditor::CanvasPanel() {
    auto ToolButton = [this](const char* label, Tool which) {
        const bool active = tool == which;
        if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(label)) tool = which;
        if (active) ImGui::PopStyleColor();
        ImGui::SameLine();
    };
    ToolButton("Select", Tool::Select);
    ToolButton("+ Hurt", Tool::DrawHurt);
    ToolButton("+ Hit", Tool::DrawHit);
    ImGui::Checkbox("Character colliders", &showColliders);
    ImGui::SameLine();
    ImGui::Checkbox("Flip", &flip);
    ImGui::SameLine();
    if (ImGui::Button("Fit")) fitPending = true;
    ImGui::SameLine();
    ImGui::TextDisabled("%d%%", static_cast<int>(std::lround(zoom * 100.0f)));

    const ImVec2 p0 = ImGui::GetCursorScreenPos();
    const ImVec2 size = ImGui::GetContentRegionAvail();
    if (size.x < 50.0f || size.y < 50.0f) return;
    const ImVec2 p1(p0.x + size.x, p0.y + size.y);

    ImGui::InvisibleButton("canvas", size,
        ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool hovered = ImGui::IsItemHovered();
    const bool active = ImGui::IsItemActive();

    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->PushClipRect(p0, p1, true);
    dl->AddRectFilled(p0, p1, IM_COL32(28, 28, 32, 255));

    Frame* f = CurrentFrame();
    if (!f) {
        dl->AddText(ImVec2(p0.x + 12.0f, p0.y + 12.0f), IM_COL32(150, 150, 150, 255),
            CurrentBranch() ? "Add a frame on the timeline below" : "Select or add a branch");
        dl->PopClipRect();
        return;
    }

    const unsigned int texture = f->texturePath.empty() ? 0u : instance.renderer.GetTexture(f->texturePath);
    glm::ivec2 imageSize = f->texturePath.empty() ? glm::ivec2(0) : instance.renderer.GetTextureSize(f->texturePath);
    const bool hasImage = texture != 0 && imageSize.x > 0 && imageSize.y > 0;
    if (!hasImage) imageSize = { 128, 128 };
    const glm::vec2 image = imageSize;
    const glm::vec2 pivot = set->PivotFor(*f, imageSize);
    const float ppu = set->pixelsPerUnit > 0.0f ? set->pixelsPerUnit : DELUSIVE_PIXEL_SCALE;

    if (fitPending) {
        zoom = std::clamp(std::min(size.x * 0.8f / image.x, size.y * 0.8f / image.y), 0.05f, 32.0f);
        pan = { (pivot.x - image.x * 0.5f) * zoom * (flip ? -1.0f : 1.0f), (pivot.y - image.y * 0.5f) * zoom };
        fitPending = false;
    }
    const CanvasView view{ { p0.x + size.x * 0.5f, p0.y + size.y * 0.5f }, pan, zoom, pivot, flip };

    //Image
    ImVec2 imageMin, imageMax;
    view.ScreenRect({ 0.0f, 0.0f }, image, imageMin, imageMax);
    if (hasImage) {
        //Textures are stored bottom row first, so v runs 1 to 0 down the screen
        const ImVec2 uv0 = flip ? ImVec2(1, 1) : ImVec2(0, 1);
        const ImVec2 uv1 = flip ? ImVec2(0, 0) : ImVec2(1, 0);
        dl->AddImage(Tex(texture), imageMin, imageMax, uv0, uv1);
    }
    else {
        dl->AddText(ImVec2(imageMin.x + 6.0f, imageMin.y + 6.0f), IM_COL32(150, 150, 150, 255), "No image");
    }
    dl->AddRect(imageMin, imageMax, IM_COL32(90, 90, 96, 255));

    //Frame boxes
    for (int i = 0; i < (int)f->boxes.size(); ++i) {
        const Box& b = f->boxes[i];
        ImVec2 mn, mx;
        view.ScreenRect(b.position, b.position + b.size, mn, mx);
        const ImU32 colour = b.type == BoxType::Hit ? HitColour : HurtColour;
        dl->AddRectFilled(mn, mx, WithAlpha(colour, b.type == BoxType::Hit ? 48 : 24));
        dl->AddRect(mn, mx, colour, 0.0f, 0, i == box ? 2.5f : 1.5f);
    }

    //Character colliders, dashed - they belong to the agent, not the frame
    std::vector<ColliderComponent*> colliders;
    if (showColliders && character) colliders = character->GetComponentsOfType<ColliderComponent>();
    if (collider && std::find(colliders.begin(), colliders.end(), collider) == colliders.end()) collider = nullptr;

    for (ColliderComponent* c : colliders) {
        const ColliderInImage shape = ToImage(*c, pivot, ppu);
        const ImU32 colour = ColliderColour(c->GetColliderType());
        const float thickness = c == collider ? 2.5f : 1.5f;
        switch (shape.shape) {
        case ShapeType::Circle:
            dl->AddCircle(view.ToScreen(shape.centre), shape.radius * zoom, colour, 48, thickness);
            break;
        case ShapeType::Line:
            DashedLine(dl, view.ToScreen(shape.centre), view.ToScreen(shape.end), colour, thickness);
            break;
        default: {
            ImVec2 mn, mx;
            view.ScreenRect(shape.min, shape.max, mn, mx);
            DashedRect(dl, mn, mx, colour, thickness);
            break;
        }
        }
    }

    //Handles on whatever box is selected
    auto DrawHandles = [&](glm::vec2 mn, glm::vec2 mx) {
        for (int h = 1; h <= 8; ++h) {
            const ImVec2 s = view.ToScreen(HandlePos(h, mn, mx));
            dl->AddRectFilled(ImVec2(s.x - 3.5f, s.y - 3.5f), ImVec2(s.x + 3.5f, s.y + 3.5f), HandleColour);
        }
    };
    if (!flip && box >= 0 && box < (int)f->boxes.size()) {
        DrawHandles(f->boxes[box].position, f->boxes[box].position + f->boxes[box].size);
    }
    if (!flip && collider && collider->GetShapeType() == ShapeType::Box) {
        const ColliderInImage shape = ToImage(*collider, pivot, ppu);
        DrawHandles(shape.min, shape.max);
    }

    //Pivot
    const ImVec2 pivotScreen = view.ToScreen(pivot);
    dl->AddLine(ImVec2(pivotScreen.x - 10.0f, pivotScreen.y), ImVec2(pivotScreen.x + 10.0f, pivotScreen.y), PivotColour, 2.0f);
    dl->AddLine(ImVec2(pivotScreen.x, pivotScreen.y - 10.0f), ImVec2(pivotScreen.x, pivotScreen.y + 10.0f), PivotColour, 2.0f);
    dl->AddCircle(pivotScreen, 4.0f, PivotColour, 12, 1.5f);

    //View: wheel zooms around the cursor, middle or right drag pans
    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse = io.MousePos;
    const glm::vec2 mouseImage = view.ToImage(mouse);
    if (hovered && io.MouseWheel != 0.0f) {
        zoom = std::clamp(zoom * (io.MouseWheel > 0.0f ? 1.15f : 1.0f / 1.15f), 0.05f, 32.0f);
        float dx = (mouseImage.x - pivot.x) * zoom;
        if (flip) dx = -dx;
        pan = { mouse.x - view.centre.x - dx, mouse.y - view.centre.y - (mouseImage.y - pivot.y) * zoom };
    }
    if ((hovered || active) && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))) {
        pan += glm::vec2(io.MouseDelta.x, io.MouseDelta.y);
    }

    if (flip) {
        //Mirrored around the pivot, as the character looks facing left
        dl->AddText(ImVec2(p0.x + 10.0f, p1.y - 24.0f), IM_COL32(180, 180, 180, 255), "Flip is a preview - turn it off to edit");
        drag = Drag::None;
        dl->PopClipRect();
        return;
    }

    const float grab = 7.0f;

    //Press: the tool decides, otherwise handles first, then the pivot, then what is under the cursor
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        drag = Drag::None;

        if (tool != Tool::Select) {
            Box added;
            added.type = tool == Tool::DrawHit ? BoxType::Hit : BoxType::Hurt;
            added.position = Round(mouseImage);
            f->boxes.push_back(added);
            box = (int)f->boxes.size() - 1;
            collider = nullptr;
            dragStart = Round(mouseImage);
            drag = Drag::BoxCreate;
            Touch();
        }
        else {
            if (box >= 0 && box < (int)f->boxes.size()) {
                const glm::vec2 mn = f->boxes[box].position, mx = mn + f->boxes[box].size;
                for (int h = 1; h <= 8 && drag == Drag::None; ++h) {
                    if (Near(view.ToScreen(HandlePos(h, mn, mx)), mouse, grab)) {
                        drag = Drag::BoxResize;
                        dragHandle = h;
                        dragMin = mn;
                        dragMax = mx;
                    }
                }
            }
            if (drag == Drag::None && collider && collider->GetShapeType() == ShapeType::Box) {
                const ColliderInImage shape = ToImage(*collider, pivot, ppu);
                for (int h = 1; h <= 8 && drag == Drag::None; ++h) {
                    if (Near(view.ToScreen(HandlePos(h, shape.min, shape.max)), mouse, grab)) {
                        drag = Drag::Collider;
                        dragHandle = h;
                        dragMin = shape.min;
                        dragMax = shape.max;
                    }
                }
            }
            if (drag == Drag::None && Near(pivotScreen, mouse, grab + 3.0f)) {
                drag = Drag::Pivot;
            }
            for (int i = (int)f->boxes.size() - 1; i >= 0 && drag == Drag::None; --i) {
                ImVec2 mn, mx;
                view.ScreenRect(f->boxes[i].position, f->boxes[i].position + f->boxes[i].size, mn, mx);
                if (Inside(mouse, mn, mx)) {
                    box = i;
                    collider = nullptr;
                    drag = Drag::BoxMove;
                    dragStart = mouseImage;
                    dragMin = f->boxes[i].position;
                }
            }
            for (int i = (int)colliders.size() - 1; i >= 0 && drag == Drag::None; --i) {
                const ColliderInImage shape = ToImage(*colliders[i], pivot, ppu);
                bool hit = false;
                if (shape.shape == ShapeType::Circle) {
                    hit = Near(mouse, view.ToScreen(shape.centre), shape.radius * zoom);
                }
                else if (shape.shape == ShapeType::Line) {
                    hit = SegmentDistance(mouse, view.ToScreen(shape.centre), view.ToScreen(shape.end)) <= grab;
                }
                else {
                    ImVec2 mn, mx;
                    view.ScreenRect(shape.min, shape.max, mn, mx);
                    hit = Inside(mouse, mn, mx);
                }
                if (hit) {
                    collider = colliders[i];
                    box = -1;
                    drag = Drag::Collider;
                    dragHandle = 0;
                    dragStart = mouseImage;
                    dragOrigin = collider->transform->position;
                }
            }
            if (drag == Drag::None) {
                box = -1;
                collider = nullptr;
            }
        }
    }

    //Drag
    if (drag != Drag::None && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        const bool boxValid = box >= 0 && box < (int)f->boxes.size();
        switch (drag) {
        case Drag::Pivot: {
            //The view anchors on the pivot, so shift it with the pivot to keep the image still
            const glm::vec2 before = pivot;
            f->pivotMode = PivotMode::Custom;
            f->pivot = Round(mouseImage);
            pan += glm::vec2((f->pivot.x - before.x) * zoom * (flip ? -1.0f : 1.0f), (f->pivot.y - before.y) * zoom);
            Touch();
            break;
        }
        case Drag::BoxMove:
            if (boxValid) {
                f->boxes[box].position = Round(dragMin + (mouseImage - dragStart));
                Touch();
            }
            break;
        case Drag::BoxResize:
            if (boxValid) {
                glm::vec2 mn = dragMin, mx = dragMax;
                ApplyHandle(dragHandle, Round(mouseImage), mn, mx, 1.0f);
                f->boxes[box].position = mn;
                f->boxes[box].size = mx - mn;
                Touch();
            }
            break;
        case Drag::BoxCreate:
            if (boxValid) {
                const glm::vec2 p = Round(mouseImage);
                f->boxes[box].position = glm::min(dragStart, p);
                f->boxes[box].size = glm::abs(p - dragStart);
            }
            break;
        case Drag::Collider:
            if (collider) {
                if (dragHandle == 0) {
                    const glm::vec2 d = mouseImage - dragStart;
                    const glm::vec2 units(d.x / ppu, -d.y / ppu);
                    collider->transform->position = dragOrigin + units / SafeScale(collider->GetOwner()->GetTransform().scale);
                }
                else {
                    glm::vec2 mn = dragMin, mx = dragMax;
                    ApplyHandle(dragHandle, mouseImage, mn, mx, 1.0f);
                    SetBoxFromImage(*collider, mn, mx, pivot, ppu);
                }
                TouchCharacter();
            }
            break;
        default:
            break;
        }
    }

    //Release
    if (drag != Drag::None && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if (drag == Drag::BoxCreate && box >= 0 && box < (int)f->boxes.size()) {
            const glm::vec2 s = f->boxes[box].size;
            if (s.x < 2.0f || s.y < 2.0f) {
                f->boxes.erase(f->boxes.begin() + box);
                box = -1;
            }
            tool = Tool::Select;
        }
        drag = Drag::None;
    }

    if (hovered && box >= 0 && box < (int)f->boxes.size() && ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        f->boxes.erase(f->boxes.begin() + box);
        box = -1;
        Touch();
    }

    dl->PopClipRect();
}

#pragma endregion

#pragma region Inspector

void AnimatorEditor::InspectorPanel() {
    Branch* b = CurrentBranch();
    Frame* f = CurrentFrame();

    if (!b) {
        ImGui::TextDisabled("Select a branch");
    }
    else if (!f) {
        ImGui::TextDisabled("Select or add a frame");
    }
    else {
        ImGui::SeparatorText(("Frame " + std::to_string(frame + 1) + " of " + std::to_string(b->frames.size())).c_str());

        const std::string imageLabel = f->texturePath.empty()
            ? std::string("Choose image...")
            : std::filesystem::path(f->texturePath).filename().string();
        if (ImGui::Button(imageLabel.c_str(), ImVec2(-1.0f, 0.0f))) ImGui::OpenPopup("FrameImage");
        if (ImagePicker("FrameImage", f->texturePath)) {
            fitPending = true;
            Touch();
        }

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
        if (ImGui::InputInt("Ticks", &f->ticks)) {
            f->ticks = std::max(f->ticks, 1);
            Touch();
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%d ms", TicksToMs(f->ticks));
        if (ImGui::Checkbox("Input lock", &f->inputLock)) Touch();

        ImGui::SeparatorText("Pivot");
        int mode = static_cast<int>(f->pivotMode);
        if (ImGui::Combo("##pivotMode", &mode, PivotModeLabels, 5)) {
            //Switching to custom starts from where the pivot already is
            if (static_cast<PivotMode>(mode) == PivotMode::Custom) {
                const glm::ivec2 size = f->texturePath.empty() ? glm::ivec2(128) : instance.renderer.GetTextureSize(f->texturePath);
                f->pivot = set->PivotFor(*f, size);
            }
            f->pivotMode = static_cast<PivotMode>(mode);
            Touch();
        }
        if (f->pivotMode == PivotMode::Custom) {
            if (ImGui::DragFloat2("px", &f->pivot.x, 1.0f)) Touch();
        }
        else {
            ImGui::TextDisabled("Drag the pivot on the canvas to set it exactly");
        }

        ImGui::SeparatorText("Frame boxes (px)");
        int eraseBox = -1;
        for (int i = 0; i < (int)f->boxes.size(); ++i) {
            Box& bx = f->boxes[i];
            ImGui::PushID(i);
            const char* typeName = bx.type == BoxType::Hit ? "Hit" : "Hurt";
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(bx.type == BoxType::Hit ? HitColour : HurtColour));
            if (ImGui::Selectable((std::string(typeName) + " " + std::to_string(i + 1)).c_str(), box == i)) {
                box = i;
                collider = nullptr;
            }
            ImGui::PopStyleColor();
            if (box == i) {
                int type = static_cast<int>(bx.type);
                if (ImGui::Combo("Type", &type, "Hurt\0Hit\0")) { bx.type = static_cast<BoxType>(type); Touch(); }
                if (ImGui::DragFloat2("Position", &bx.position.x, 1.0f)) Touch();
                if (ImGui::DragFloat2("Size", &bx.size.x, 1.0f, 1.0f, 100000.0f)) Touch();
                if (ImGui::SmallButton("Remove")) eraseBox = i;
            }
            ImGui::PopID();
        }
        if (eraseBox >= 0) {
            f->boxes.erase(f->boxes.begin() + eraseBox);
            box = -1;
            Touch();
        }

        //New boxes start standing on the pivot, a third of the image in size
        auto AddBox = [&](BoxType type) {
            const glm::ivec2 size = f->texturePath.empty() ? glm::ivec2(128) : instance.renderer.GetTextureSize(f->texturePath);
            const glm::vec2 pivot = set->PivotFor(*f, size);
            Box added;
            added.type = type;
            added.size = Round(glm::max(glm::vec2(size) / 3.0f, glm::vec2(4.0f)));
            added.position = Round(pivot - glm::vec2(added.size.x * 0.5f, added.size.y));
            f->boxes.push_back(added);
            box = (int)f->boxes.size() - 1;
            collider = nullptr;
            Touch();
        };
        if (ImGui::SmallButton("+ Hurt")) AddBox(BoxType::Hurt);
        ImGui::SameLine();
        if (ImGui::SmallButton("+ Hit")) AddBox(BoxType::Hit);
    }

    if (collider) {
        ImGui::SeparatorText("Character collider");
        ImGui::Text("%s", collider->GetName().c_str());
        ImGui::TextDisabled("%s", collider->GetType());
        int shape = static_cast<int>(collider->GetShapeType());
        if (ImGui::Combo("Shape", &shape, "Box\0Circle\0Line\0")) {
            collider->SetShapeType(static_cast<ShapeType>(shape));
            TouchCharacter();
        }
        if (ImGui::DragFloat2("Offset", &collider->transform->position.x, 0.01f)) TouchCharacter();
        if (ImGui::DragFloat2("Size", &collider->transform->scale.x, 0.01f)) TouchCharacter();
        ImGui::TextDisabled("Saved to %s", std::filesystem::path(characterPath).filename().string().c_str());
    }
    else if (set->character.IsValid() && !character) {
        ImGui::SeparatorText("Character collider");
        ImGui::TextDisabled("Character file not found");
    }
}

#pragma endregion

#pragma region Timeline

void AnimatorEditor::TimelinePanel() {
    Branch* b = CurrentBranch();
    if (!b) {
        ImGui::TextDisabled("Select a branch");
        return;
    }

    const int total = BranchTicks();
    if (ImGui::Button(playing ? "Pause" : "Play")) {
        if (!playing && playTick >= total - 1) playTick = 0;
        playing = !playing;
        tickAccumulator = 0.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step") && total > 0) {
        playing = false;
        playTick = (playTick + 1) % total;
        frame = FrameAtTick(playTick);
        box = -1;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Loop preview", &loopPreview);
    ImGui::SameLine();
    ImGui::TextDisabled("Tick %d of %d   %d of %d ms", total > 0 ? playTick + 1 : 0, total, TicksToMs(playTick), TicksToMs(total));

    //Frame strip - each tile is as wide as the frame lasts
    const float tileHeight = ImGui::GetTextLineHeight() * 5.5f + 8.0f;
    const float labelHeight = ImGui::GetTextLineHeight() + 6.0f;
    const float gap = 4.0f;
    ImGui::BeginChild("Strip", ImVec2(0.0f, tileHeight + ImGui::GetStyle().ScrollbarSize + 8.0f), ImGuiChildFlags_None,
        ImGuiWindowFlags_HorizontalScrollbar);

    const float available = ImGui::GetContentRegionAvail().x - tileHeight - gap * (b->frames.size() + 1);
    const float pxPerTick = total > 0 ? std::max(14.0f, available / total) : 14.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();

    int moveFrom = -1, moveTo = -1, duplicate = -1, erase = -1;
    std::vector<std::pair<float, float>> tiles;

    for (int i = 0; i < (int)b->frames.size(); ++i) {
        const Frame& fr = b->frames[i];
        const float w = std::max(36.0f, fr.ticks * pxPerTick);
        ImGui::PushID(i);

        const ImVec2 tp = ImGui::GetCursorScreenPos();
        tiles.push_back({ tp.x, w });
        ImGui::InvisibleButton("tile", ImVec2(w, tileHeight));
        if (ImGui::IsItemClicked()) {
            playing = false;
            SelectFrame(i);
        }

        //Drag a tile onto another to move it there
        if (ImGui::BeginDragDropSource()) {
            ImGui::SetDragDropPayload("ANIM_FRAME", &i, sizeof(int));
            ImGui::Text("Frame %d", i + 1);
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ANIM_FRAME")) {
                moveFrom = *static_cast<const int*>(payload->Data);
                moveTo = i;
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::BeginPopupContextItem("tileMenu")) {
            if (ImGui::MenuItem("Duplicate")) duplicate = i;
            if (ImGui::MenuItem("Delete")) erase = i;
            ImGui::EndPopup();
        }

        const ImVec2 te(tp.x + w, tp.y + tileHeight);
        dl->AddRectFilled(tp, te, IM_COL32(40, 40, 46, 255), 3.0f);

        //Thumbnail, fitted into the space above the label
        const unsigned int texture = fr.texturePath.empty() ? 0u : instance.renderer.GetTexture(fr.texturePath);
        const glm::ivec2 size = fr.texturePath.empty() ? glm::ivec2(0) : instance.renderer.GetTextureSize(fr.texturePath);
        if (texture && size.x > 0 && size.y > 0) {
            const float boxW = w - 8.0f, boxH = tileHeight - labelHeight - 6.0f;
            const float scale = std::min(boxW / size.x, boxH / size.y);
            const ImVec2 thumb(size.x * scale, size.y * scale);
            const ImVec2 at(tp.x + (w - thumb.x) * 0.5f, tp.y + 4.0f + (boxH - thumb.y) * 0.5f);
            dl->AddImage(Tex(texture), at, ImVec2(at.x + thumb.x, at.y + thumb.y), ImVec2(0, 1), ImVec2(1, 0));
        }

        char label[48];
        std::snprintf(label, sizeof(label), "%d  %dt  %dms", i + 1, fr.ticks, TicksToMs(fr.ticks));
        dl->PushClipRect(tp, te, true);
        dl->AddText(ImVec2(tp.x + 4.0f, te.y - labelHeight + 2.0f), i == frame ? IM_COL32(133, 183, 235, 255) : IM_COL32(170, 170, 170, 255), label);
        dl->PopClipRect();
        dl->AddRect(tp, te, i == frame ? IM_COL32(55, 138, 221, 255) : IM_COL32(80, 80, 88, 255), 3.0f, 0, i == frame ? 2.0f : 1.0f);

        ImGui::PopID();
        ImGui::SameLine(0.0f, gap);
    }

    if (ImGui::Button("+##addFrame", ImVec2(tileHeight, tileHeight))) ImGui::OpenPopup("AddFrame");
    std::string addedImage;
    if (ImagePicker("AddFrame", addedImage)) {
        Frame added;
        added.texturePath = addedImage;
        b->frames.push_back(added);
        playing = false;
        SelectFrame((int)b->frames.size() - 1);
        fitPending = true;
        Touch();
    }

    //Playhead
    const int shown = FrameAtTick(playTick);
    if (shown >= 0 && shown < (int)tiles.size()) {
        const float into = static_cast<float>(playTick - FrameStartTick(shown)) / b->frames[shown].ticks;
        const float x = tiles[shown].first + tiles[shown].second * into;
        const float y = ImGui::GetItemRectMin().y;
        dl->AddLine(ImVec2(x, y - 2.0f), ImVec2(x, y + tileHeight + 2.0f), PivotColour, 2.0f);
    }

    ImGui::EndChild();

    //Frame edits wait until the strip is drawn
    if (moveFrom >= 0 && moveTo >= 0 && moveFrom != moveTo) {
        Frame moved = b->frames[moveFrom];
        b->frames.erase(b->frames.begin() + moveFrom);
        b->frames.insert(b->frames.begin() + moveTo, moved);
        SelectFrame(moveTo);
        Touch();
    }
    if (duplicate >= 0) {
        Frame copy = b->frames[duplicate];
        copy.id = UUID::GenerateRandom();
        b->frames.insert(b->frames.begin() + duplicate + 1, copy);
        SelectFrame(duplicate + 1);
        Touch();
    }
    if (erase >= 0) {
        b->frames.erase(b->frames.begin() + erase);
        SelectFrame(std::min(erase, (int)b->frames.size() - 1));
        Touch();
    }
}

#pragma endregion

bool AnimatorEditor::ImagePicker(const char* popupID, std::string& path) {
    bool picked = false;
    if (!ImGui::BeginPopup(popupID)) return false;

    if (ImGui::IsWindowAppearing()) {
        imageFiles.clear();
        imageFilter[0] = '\0';
        std::error_code ec;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(SPRITE_FOLDER, ec)) {
            if (!entry.is_regular_file()) continue;
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga") {
                imageFiles.push_back(entry.path().generic_string());
            }
        }
        std::sort(imageFiles.begin(), imageFiles.end());
        ImGui::SetKeyboardFocusHere();
    }

    ImGui::InputText("Filter", imageFilter, sizeof(imageFilter));
    std::string filter = imageFilter;
    std::transform(filter.begin(), filter.end(), filter.begin(), ::tolower);

    ImGui::BeginChild("files", ImVec2(420.0f, 300.0f), ImGuiChildFlags_Borders);
    const std::string root = std::filesystem::path(SPRITE_FOLDER).generic_string();
    for (const std::string& file : imageFiles) {
        std::string shown = file.rfind(root, 0) == 0 ? file.substr(root.size()) : file;
        std::string lowered = shown;
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);
        if (!filter.empty() && lowered.find(filter) == std::string::npos) continue;

        if (ImGui::Selectable(shown.c_str(), file == path)) {
            path = file;
            picked = true;
            ImGui::CloseCurrentPopup();
        }
    }
    if (imageFiles.empty()) ImGui::TextDisabled("No images in %s", SPRITE_FOLDER);
    ImGui::EndChild();

    ImGui::EndPopup();
    return picked;
}
