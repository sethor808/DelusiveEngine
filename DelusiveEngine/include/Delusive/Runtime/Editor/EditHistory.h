#pragma once
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

//Undo and redo for an editor that snapshots its whole document. The editor commits once an
//edit settles - a drag released, a field left, a button clicked - so one gesture is one step.
template<typename T>
class EditHistory {
public:
    explicit EditHistory(size_t limit = 200) : limit(limit) {}

    //Starts over from this state, forgetting every step
    void Reset(const T& state) {
        undoSteps.clear();
        redoSteps.clear();
        committed = state;
    }

    //Records that the document went from the last committed state to this one
    void Commit(const T& state) {
        undoSteps.push_back(std::move(committed));
        if (undoSteps.size() > limit) undoSteps.erase(undoSteps.begin());
        redoSteps.clear();
        committed = state;
    }

    //The state the last step left the document in
    const T& Committed() const { return committed; }

    bool CanUndo() const { return !undoSteps.empty(); }
    bool CanRedo() const { return !redoSteps.empty(); }

    //current is the live state, kept so redo can come back to it. Returns what to restore.
    std::optional<T> Undo(const T& current) {
        if (undoSteps.empty()) return std::nullopt;
        redoSteps.push_back(current);
        committed = std::move(undoSteps.back());
        undoSteps.pop_back();
        return committed;
    }

    std::optional<T> Redo(const T& current) {
        if (redoSteps.empty()) return std::nullopt;
        undoSteps.push_back(current);
        committed = std::move(redoSteps.back());
        redoSteps.pop_back();
        return committed;
    }

private:
    size_t limit;
    std::vector<T> undoSteps;
    std::vector<T> redoSteps;
    T committed{};
};
