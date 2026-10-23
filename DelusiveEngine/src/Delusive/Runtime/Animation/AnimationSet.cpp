#include <Delusive/Runtime/Animation/AnimationSet.h>
#include <Delusive/Runtime/Core/DelusiveLibrary.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace DelusiveAnimation {

    namespace {
        //Value encodings match Property.inl so every block file reads the same way:
        //quoted strings, "x y" vectors, 0/1 bools, lists as "count item item"

        std::ostringstream ValueStream() {
            std::ostringstream ss;
            ss << std::setprecision(std::numeric_limits<float>::max_digits10);
            return ss;
        }

        std::string Quote(const std::string& s) {
            std::ostringstream ss;
            ss << std::quoted(s);
            return ss.str();
        }

        std::string Vec2(const glm::vec2& v) {
            std::ostringstream ss = ValueStream();
            ss << v.x << " " << v.y;
            return ss.str();
        }

        std::string JoinIDs(const std::vector<UUID>& ids) {
            std::string out;
            for (const UUID& id : ids) {
                if (!out.empty()) out += " ";
                out += id.ToString();
            }
            return out;
        }

        //Returns nullptr when the key is missing so callers keep their defaults
        const std::string* Find(const DelusiveParser::DataBlock& block, const std::string& key) {
            auto it = block.properties.find(key);
            return it == block.properties.end() ? nullptr : &it->second;
        }

        void Read(const DelusiveParser::DataBlock& block, const std::string& key, std::string& value) {
            if (const std::string* s = Find(block, key)) {
                std::istringstream ss(*s);
                ss >> std::quoted(value);
            }
        }

        template<typename T>
        void Read(const DelusiveParser::DataBlock& block, const std::string& key, T& value) {
            if (const std::string* s = Find(block, key)) {
                std::istringstream ss(*s);
                if constexpr (std::is_same_v<T, glm::vec2>) {
                    ss >> value.x >> value.y;
                }
                else if constexpr (std::is_same_v<T, bool>) {
                    int flag = 0;
                    if (ss >> flag) value = (flag != 0);
                }
                else if constexpr (std::is_same_v<T, UUID>) {
                    std::string text;
                    if (ss >> text) value.FromString(text);
                }
                else {
                    ss >> value;
                }
            }
        }

        std::vector<UUID> ReadIDs(const DelusiveParser::DataBlock& block, const std::string& key) {
            std::vector<UUID> ids;
            const std::string* s = Find(block, key);
            if (!s) return ids;

            std::istringstream ss(*s);
            std::string token;
            while (ss >> token) {
                UUID id;
                id.FromString(token);
                if (id.IsValid()) ids.push_back(id);
            }
            return ids;
        }

        const char* BoxTypeName(BoxType type) {
            return type == BoxType::Hit ? "hit" : "hurt";
        }

        const char* PivotModeName(PivotMode mode) {
            switch (mode) {
            case PivotMode::BottomCentre: return "bottomCentre";
            case PivotMode::Centre:       return "centre";
            case PivotMode::TopCentre:    return "topCentre";
            case PivotMode::Custom:       return "custom";
            default:                      return "default";
            }
        }

        PivotMode PivotModeFromName(const std::string& name) {
            if (name == "bottomCentre") return PivotMode::BottomCentre;
            if (name == "centre")       return PivotMode::Centre;
            if (name == "topCentre")    return PivotMode::TopCentre;
            if (name == "custom")       return PivotMode::Custom;
            return PivotMode::SetDefault;
        }

        std::string WriteConditions(const std::vector<Condition>& conditions) {
            std::ostringstream ss;
            ss << conditions.size();
            for (const Condition& cond : conditions) {
                ss << " " << std::quoted(cond.flag) << " " << (cond.value ? 1 : 0);
            }
            return ss.str();
        }

        std::vector<Condition> ReadConditions(const DelusiveParser::DataBlock& block) {
            std::vector<Condition> conditions;
            const std::string* s = Find(block, "conditions");
            if (!s) return conditions;

            std::istringstream ss(*s);
            size_t count = 0;
            ss >> count;
            for (size_t i = 0; i < count; i++) {
                Condition cond;
                int value = 1;
                if (!(ss >> std::quoted(cond.flag) >> value)) break;
                cond.value = (value != 0);
                conditions.push_back(cond);
            }
            return conditions;
        }

        //Swaps in a fresh id when this one was already handed out in this save
        void Claim(UUID& id, std::unordered_set<UUID, UUID::Hash>& used) {
            if (!id.IsValid() || used.contains(id)) {
                id = UUID::GenerateRandom();
            }
            used.insert(id);
        }

        DelusiveParser::DataBlock TransitionBlock(const Transition& t) {
            DelusiveParser::DataBlock block;
            block.category = "AnimationTransition";
            block.id = t.id;
            block.properties["id"] = t.id.ToString();
            block.properties["target"] = t.target.IsValid() ? t.target.ToString() : "";
            block.properties["onFinish"] = t.onFinish ? "1" : "0";
            block.properties["window"] = std::to_string(t.fromFrame) + " " + std::to_string(t.toFrame);
            block.properties["conditions"] = WriteConditions(t.conditions);
            return block;
        }

        Transition ReadTransition(const DelusiveParser::DataBlock& block) {
            Transition t;
            t.id = block.id;
            Read(block, "target", t.target);
            Read(block, "onFinish", t.onFinish);
            if (const std::string* s = Find(block, "window")) {
                std::istringstream(*s) >> t.fromFrame >> t.toFrame;
            }
            t.conditions = ReadConditions(block);
            return t;
        }
    }

#pragma region Save

    void Set::ToBlocks(std::vector<DelusiveParser::DataBlock>& out) {
        std::unordered_set<UUID, UUID::Hash> used;
        Claim(id, used);

        DelusiveParser::DataBlock setBlock;
        setBlock.category = "AnimationSet";
        setBlock.id = id;
        setBlock.properties["name"] = Quote(name);
        {
            std::ostringstream ss = ValueStream();
            ss << pixelsPerUnit;
            setBlock.properties["pixelsPerUnit"] = ss.str();
        }
        setBlock.properties["defaultPivot"] = Vec2(defaultPivot);
        setBlock.properties["character"] = character.IsValid() ? character.ToString() : "";
        {
            std::ostringstream ss;
            ss << flags.size();
            for (const Flag& flag : flags) {
                ss << " " << std::quoted(flag.name) << " " << (flag.kind == FlagKind::Trigger ? "trigger" : "bool");
            }
            setBlock.properties["flags"] = ss.str();
        }

        std::vector<UUID> branchIDs;
        std::vector<DelusiveParser::DataBlock> childBlocks;

        for (Branch& branch : branches) {
            Claim(branch.id, used);
            branchIDs.push_back(branch.id);

            DelusiveParser::DataBlock branchBlock;
            branchBlock.category = "AnimationBranch";
            branchBlock.id = branch.id;
            branchBlock.properties["name"] = Quote(branch.name);
            branchBlock.properties["loop"] = branch.loop ? "1" : "0";
            branchBlock.properties["lockInput"] = branch.lockInput ? "1" : "0";

            std::vector<UUID> frameIDs;
            std::vector<DelusiveParser::DataBlock> ownedBlocks;

            for (Frame& frame : branch.frames) {
                Claim(frame.id, used);
                frameIDs.push_back(frame.id);

                DelusiveParser::DataBlock frameBlock;
                frameBlock.category = "AnimationFrame";
                frameBlock.id = frame.id;
                //Forward slashes load on every platform
                std::string texture = frame.texturePath;
                std::replace(texture.begin(), texture.end(), '\\', '/');
                frameBlock.properties["texture"] = Quote(texture);
                frameBlock.properties["ticks"] = std::to_string(frame.ticks);
                frameBlock.properties["pivotMode"] = PivotModeName(frame.pivotMode);
                frameBlock.properties["pivot"] = Vec2(frame.pivot);
                frameBlock.properties["inputLock"] = frame.inputLock ? "1" : "0";

                std::ostringstream boxes = ValueStream();
                boxes << frame.boxes.size();
                for (const Box& box : frame.boxes) {
                    boxes << " " << BoxTypeName(box.type)
                        << " " << box.position.x << " " << box.position.y
                        << " " << box.size.x << " " << box.size.y;
                }
                frameBlock.properties["boxes"] = boxes.str();

                ownedBlocks.push_back(std::move(frameBlock));
            }

            std::vector<UUID> transitionIDs;
            for (Transition& t : branch.transitions) {
                Claim(t.id, used);
                transitionIDs.push_back(t.id);
                ownedBlocks.push_back(TransitionBlock(t));
            }

            branchBlock.properties["frames"] = JoinIDs(frameIDs);
            branchBlock.properties["transitions"] = JoinIDs(transitionIDs);
            childBlocks.push_back(std::move(branchBlock));
            for (auto& block : ownedBlocks) childBlocks.push_back(std::move(block));
        }

        std::vector<UUID> anyIDs;
        for (Transition& t : anyTransitions) {
            Claim(t.id, used);
            anyIDs.push_back(t.id);
            childBlocks.push_back(TransitionBlock(t));
        }

        setBlock.properties["branches"] = JoinIDs(branchIDs);
        setBlock.properties["anyTransitions"] = JoinIDs(anyIDs);
        out.push_back(std::move(setBlock));
        for (auto& block : childBlocks) out.push_back(std::move(block));
    }

    bool Set::SaveToFile(const std::string& path, DelusiveLibrary& library) {
        std::vector<DelusiveParser::DataBlock> blocks;
        ToBlocks(blocks);

        DelusiveLibrary::IDRemap remap;
        if (!library.WriteFile(path, std::move(blocks), &remap)) return false;

        //Saved as a copy - carry the copy's fresh ids from here on, including the
        //transition targets that point at remapped branches
        auto apply = [&remap](UUID& blockID) {
            auto fresh = remap.find(blockID);
            if (fresh != remap.end()) blockID = fresh->second;
        };
        apply(id);
        for (Branch& branch : branches) {
            apply(branch.id);
            for (Frame& frame : branch.frames) apply(frame.id);
            for (Transition& t : branch.transitions) { apply(t.id); apply(t.target); }
        }
        for (Transition& t : anyTransitions) { apply(t.id); apply(t.target); }

        return true;
    }

    bool Set::Save(DelusiveLibrary& library) {
        const std::string path = library.GetSourceFile(id);
        if (path.empty()) {
            std::cerr << "[Animation] " << name << " has no source file - use SaveToFile" << std::endl;
            return false;
        }

        return SaveToFile(path, library);
    }

#pragma endregion

#pragma region Load

    bool Set::FromBlocks(const std::vector<DelusiveParser::DataBlock>& blocks) {
        const DelusiveParser::DataBlock* setBlock = nullptr;
        std::unordered_map<UUID, const DelusiveParser::DataBlock*, UUID::Hash> byID;

        for (const DelusiveParser::DataBlock& block : blocks) {
            if (block.category == "AnimationSet" && !setBlock) setBlock = &block;
            if (block.id.IsValid()) byID[block.id] = &block;
        }

        if (!setBlock) {
            std::cerr << "[Animation] No AnimationSet block" << std::endl;
            return false;
        }

        return Build(*setBlock, [&byID](const UUID& blockID) -> const DelusiveParser::DataBlock* {
            auto it = byID.find(blockID);
            return it == byID.end() ? nullptr : it->second;
        });
    }

    bool Set::FromLibrary(const DelusiveLibrary& library, const UUID& setID) {
        const DelusiveParser::DataBlock* setBlock = library.Find(setID);
        if (!setBlock || setBlock->category != "AnimationSet") {
            std::cerr << "[Animation] No AnimationSet " << setID.ToString() << std::endl;
            return false;
        }

        return Build(*setBlock, [&library](const UUID& blockID) { return library.Find(blockID); });
    }

    bool Set::LoadFromFile(const std::string& path, DelusiveLibrary& library) {
        if (!library.LoadFile(path)) return false;

        for (const DelusiveParser::DataBlock* block : library.ListFile(path)) {
            if (block->category == "AnimationSet") {
                return FromLibrary(library, block->id);
            }
        }

        std::cerr << "[Animation] No AnimationSet block in " << path << std::endl;
        return false;
    }

    bool Set::Build(const DelusiveParser::DataBlock& setBlock, const BlockLookup& find) {
        //Build into a fresh set so a half read file never leaves this one mixed
        Set out;
        out.id = setBlock.id.IsValid() ? setBlock.id : UUID::GenerateRandom();
        Read(setBlock, "name", out.name);
        Read(setBlock, "pixelsPerUnit", out.pixelsPerUnit);
        Read(setBlock, "defaultPivot", out.defaultPivot);
        Read(setBlock, "character", out.character);

        if (const std::string* s = Find(setBlock, "flags")) {
            std::istringstream ss(*s);
            size_t count = 0;
            ss >> count;
            for (size_t i = 0; i < count; i++) {
                Flag flag;
                std::string kind;
                if (!(ss >> std::quoted(flag.name) >> kind)) break;
                flag.kind = (kind == "trigger") ? FlagKind::Trigger : FlagKind::Bool;
                out.flags.push_back(flag);
            }
        }

        auto ReadTransitions = [&find](const DelusiveParser::DataBlock& owner, std::vector<Transition>& into) {
            for (const UUID& transitionID : ReadIDs(owner, owner.category == "AnimationSet" ? "anyTransitions" : "transitions")) {
                const DelusiveParser::DataBlock* block = find(transitionID);
                if (!block) {
                    std::cerr << "[Animation] Missing transition " << transitionID.ToString() << std::endl;
                    continue;
                }
                into.push_back(ReadTransition(*block));
            }
        };

        for (const UUID& branchID : ReadIDs(setBlock, "branches")) {
            const DelusiveParser::DataBlock* branchFound = find(branchID);
            if (!branchFound) {
                std::cerr << "[Animation] Missing branch " << branchID.ToString() << std::endl;
                continue;
            }
            const DelusiveParser::DataBlock& branchBlock = *branchFound;

            Branch branch;
            branch.id = branchID;
            Read(branchBlock, "name", branch.name);
            Read(branchBlock, "loop", branch.loop);
            Read(branchBlock, "lockInput", branch.lockInput);

            for (const UUID& frameID : ReadIDs(branchBlock, "frames")) {
                const DelusiveParser::DataBlock* frameFound = find(frameID);
                if (!frameFound) {
                    std::cerr << "[Animation] Missing frame " << frameID.ToString()
                        << " in branch " << branch.name << std::endl;
                    continue;
                }
                const DelusiveParser::DataBlock& frameBlock = *frameFound;

                Frame frame;
                frame.id = frameID;
                Read(frameBlock, "texture", frame.texturePath);
                Read(frameBlock, "ticks", frame.ticks);
                std::string mode;
                Read(frameBlock, "pivotMode", mode);
                frame.pivotMode = PivotModeFromName(mode);
                Read(frameBlock, "pivot", frame.pivot);
                Read(frameBlock, "inputLock", frame.inputLock);
                if (frame.ticks < 1) frame.ticks = 1;

                if (const std::string* s = Find(frameBlock, "boxes")) {
                    std::istringstream ss(*s);
                    size_t count = 0;
                    ss >> count;
                    for (size_t i = 0; i < count; i++) {
                        Box box;
                        std::string type;
                        if (!(ss >> type >> box.position.x >> box.position.y
                            >> box.size.x >> box.size.y)) break;
                        box.type = (type == "hit") ? BoxType::Hit : BoxType::Hurt;
                        frame.boxes.push_back(box);
                    }
                }

                branch.frames.push_back(std::move(frame));
            }

            ReadTransitions(branchBlock, branch.transitions);
            out.branches.push_back(std::move(branch));
        }

        ReadTransitions(setBlock, out.anyTransitions);

        *this = std::move(out);
        return true;
    }

#pragma endregion

    glm::vec2 Set::PresetFraction(PivotMode mode) {
        switch (mode) {
        case PivotMode::Centre:    return { 0.5f, 0.5f };
        case PivotMode::TopCentre: return { 0.5f, 0.0f };
        default:                   return { 0.5f, 1.0f };
        }
    }

    glm::vec2 Set::PivotFor(const Frame& frame, glm::ivec2 imageSize) const {
        switch (frame.pivotMode) {
        case PivotMode::Custom:     return frame.pivot;
        case PivotMode::SetDefault: return defaultPivot * glm::vec2(imageSize);
        default:                    return PresetFraction(frame.pivotMode) * glm::vec2(imageSize);
        }
    }

    Branch* Set::FindBranch(const std::string& branchName) {
        for (Branch& branch : branches) {
            if (branch.name == branchName) return &branch;
        }
        return nullptr;
    }

    Branch* Set::FindBranch(const UUID& branchID) {
        for (Branch& branch : branches) {
            if (branch.id == branchID) return &branch;
        }
        return nullptr;
    }

    const Branch* Set::FindBranch(const UUID& branchID) const {
        for (const Branch& branch : branches) {
            if (branch.id == branchID) return &branch;
        }
        return nullptr;
    }

    const Flag* Set::FindFlag(const std::string& flagName) const {
        for (const Flag& flag : flags) {
            if (flag.name == flagName) return &flag;
        }
        return nullptr;
    }
}
