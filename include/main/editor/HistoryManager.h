#pragma once
#include <vector>
#include <deque>
#include <external/nlohmann/json.hpp>
#include "main/World.h"
#include "main/scenes/SaveScene.h"
#include "main/scenes/LoadScene.h"

namespace Editor {

class HistoryManager {
public:
    static HistoryManager& Get() {
        static HistoryManager instance;
        return instance;
    }

    void RecordState(World& world) {
        nlohmann::json snapshot = SaveScene::SerializeScene(world);
        
        // If the new snapshot is identical to the last one, don't record it
        if (!undoStack.empty() && undoStack.back() == snapshot) {
            return;
        }

        undoStack.push_back(snapshot);
        if (undoStack.size() > maxHistory) {
            undoStack.pop_front();
        }
        redoStack.clear();
    }

    void Undo(World& world) {
        if (undoStack.size() < 2) return; // Need at least current + previous

        redoStack.push_back(undoStack.back());
        undoStack.pop_back();

        ApplyState(world, undoStack.back());
    }

    void Redo(World& world) {
        if (redoStack.empty()) return;

        undoStack.push_back(redoStack.back());
        ApplyState(world, redoStack.back());
        redoStack.pop_back();
    }

    bool CanUndo() const { return undoStack.size() > 1; }
    bool CanRedo() const { return !redoStack.empty(); }

private:
    HistoryManager() = default;

    void ApplyState(World& world, const nlohmann::json& state) {
        // Capture selection before reloading to try and restore it
        // Note: This is simplified; we might need more logic if IDs change
        world.isPaused = true;
        LoadScene::LoadFromJSON(state, world, Config::screenWidth, Config::screenHeight);
    }

    std::deque<nlohmann::json> undoStack;
    std::vector<nlohmann::json> redoStack;
    const size_t maxHistory = 50;
};

} // namespace Editor
