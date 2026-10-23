#pragma once
#include <Delusive/Runtime/Core/DelusiveParser.h>
#include <Delusive/Runtime/Core/UUID.h>
#include <Delusive/Runtime/Utils/DelusiveMacros.h>
#include <glm/glm.hpp>
#include <functional>
#include <string>
#include <vector>

//Per character animation data in the flat block format.
//
//File layout - one .anim per character:
//  [AnimationSet <id>]         name, pixelsPerUnit, defaultPivot, character, flags,
//                              branches=<id> <id>, anyTransitions=<id> <id>
//  [AnimationBranch <id>]      name, loop, lockInput, frames=<id> <id>, transitions=<id> <id>
//  [AnimationFrame <id>]       texture, ticks, pivotMode, pivot, inputLock, boxes
//  [AnimationTransition <id>]  target, onFinish, window, conditions
//
//Pivot and boxes are in the frame image's pixels (origin top left, y down) so the
//editor can place them over the drawing. pixelsPerUnit converts them to world units.
//
//Loading and saving go through DelusiveLibrary, so a set can be resolved by UUID and a
//reload always returns to what is on disk.
class DelusiveLibrary;

namespace DelusiveAnimation {

    enum class BoxType {
        Hurt,
        Hit
    };

    struct Box {
        BoxType type = BoxType::Hurt;
        glm::vec2 position = { 0.0f, 0.0f }; //Top left corner
        glm::vec2 size = { 0.0f, 0.0f };
    };

    //Flags are what gameplay tells the animator. A bool holds its value; a trigger is a
    //one tick event - consumed by the transition that uses it, cleared at the end of the
    //tick otherwise. Buffering input belongs to the input code, not here.
    enum class FlagKind {
        Bool,
        Trigger
    };

    struct Flag {
        std::string name;
        FlagKind kind = FlagKind::Bool;
    };

    struct Condition {
        std::string flag;
        bool value = true; //Triggers ignore this - they just have to have fired
    };

    //Branches flow into each other through transitions, checked every tick in list order;
    //the first match wins. A set's anyTransitions are checked first, from every branch, so
    //interrupts such as taking damage can happen anywhere.
    struct Transition {
        UUID id = UUID::GenerateRandom();
        UUID target;
        std::vector<Condition> conditions;
        bool onFinish = false; //Only when the branch ends (or completes a loop)
        //Frame window the transition is open in, 1 based and inclusive; 0 leaves that end open
        int fromFrame = 0;
        int toFrame = 0;
    };

    enum class PivotMode {
        SetDefault,
        BottomCentre,
        Centre,
        TopCentre,
        Custom
    };

    struct Frame {
        UUID id = UUID::GenerateRandom();
        std::string texturePath;
        int ticks = 6; //Game ticks at DELUSIVE_TICKS_PER_SECOND
        //Most frames use the set's default; Custom is an exact pixel position in the image
        PivotMode pivotMode = PivotMode::SetDefault;
        glm::vec2 pivot = { 0.0f, 0.0f };
        bool inputLock = false;
        std::vector<Box> boxes;
    };

    struct Branch {
        UUID id = UUID::GenerateRandom();
        std::string name = "New Branch";
        bool loop = false;
        bool lockInput = false;
        std::vector<Frame> frames;
        std::vector<Transition> transitions;
    };

    struct Set {
        UUID id = UUID::GenerateRandom();
        std::string name = "New Animation Set";
        float pixelsPerUnit = DELUSIVE_PIXEL_SCALE;
        //Fraction of the image, so it lands in the same place on any canvas size.
        //0.5,1 is bottom centre - the feet, for art drawn standing on the canvas bottom
        glm::vec2 defaultPivot = { 0.5f, 1.0f };
        //The agent file this set animates - the editor shows and edits its colliders
        UUID character;
        std::vector<Flag> flags;
        std::vector<Branch> branches;
        std::vector<Transition> anyTransitions;

        Branch* FindBranch(const std::string& branchName);
        Branch* FindBranch(const UUID& branchID);
        const Branch* FindBranch(const UUID& branchID) const;
        const Flag* FindFlag(const std::string& flagName) const;
        //The pivot a frame actually uses, in image pixels
        glm::vec2 PivotFor(const Frame& frame, glm::ivec2 imageSize) const;
        //Fraction of the image for the preset modes
        static glm::vec2 PresetFraction(PivotMode mode);

        //Copied branches, frames and transitions share ids, so saving gives duplicates
        //fresh ones in place - otherwise every save would invent different ids
        void ToBlocks(std::vector<DelusiveParser::DataBlock>& out);

        //Rebuilds this set from scratch; on failure the set is left untouched
        bool FromBlocks(const std::vector<DelusiveParser::DataBlock>& blocks);
        bool FromLibrary(const DelusiveLibrary& library, const UUID& setID);

        //Re-reads the file, discarding unsaved edits
        bool LoadFromFile(const std::string& path, DelusiveLibrary& library);
        bool SaveToFile(const std::string& path, DelusiveLibrary& library);
        //Saves back to the file this set was loaded from; fails for a set never saved
        bool Save(DelusiveLibrary& library);

    private:
        using BlockLookup = std::function<const DelusiveParser::DataBlock*(const UUID&)>;
        bool Build(const DelusiveParser::DataBlock& setBlock, const BlockLookup& find);
    };
}
