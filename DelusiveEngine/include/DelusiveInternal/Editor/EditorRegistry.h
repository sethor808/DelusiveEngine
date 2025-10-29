#pragma once
#include <unordered_map>
#include <glm/glm.hpp>
#include <cstdint>

class EditorRegistry {
	struct EditorState {
		bool selected = false;
		bool dragging = false;
		glm::vec2 dragOffset = glm::vec2(0.0f);
	};

	EditorState& Get(uint32_t entityID) {
		return states[entityID];
	}

	void ClearSelection() {
		for(auto& [id, state] : states) {
			state.selected = false;
			state.dragging = false;
			state.dragOffset = glm::vec2(0.0f);
		}
	}

	uint64_t GetSelected() const {
		for(auto& [id, state] : states) {
			if(state.selected) return id;
		}
		return 0;
	}
private:
	std::unordered_map<uint64_t, EditorState> states;
};