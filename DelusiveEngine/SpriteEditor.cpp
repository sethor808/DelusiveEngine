#include "SpriteEditor.h"
#include "DelusiveUtils.h"
#include <SDL3/SDL.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp> 
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

template<typename T>
const T& Clamp(const T& v, const T& lo, const T& hi) {
	return (v < lo) ? lo : (hi < v) ? hi : v;
}

void SpriteEditor::DrawUI(Sprite& sprite) {
	ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);
	ImGui::Begin("Sprite Editor");

	auto& colliders = sprite.GetColliders();

	if (colliders.empty()) {
		ImGui::Text("No colliders available!");
		ImGui::End();
		return;
	}

    if (selectedColliderIndex == -1) {
        selectedColliderIndex = 0;
    }

	ImGui::Text("Collider Count: %d", (int)colliders.size());
	if (ImGui::SliderInt("SelectedCollider", &selectedColliderIndex, 0, (int)colliders.size() - 1)) {
		selectedColliderIndex = Clamp(selectedColliderIndex, 0, (int)colliders.size() - 1);
	}

	Collider& collider = colliders[selectedColliderIndex];
	glm::vec2 offset = collider.GetOffset();
	glm::vec2 size = collider.GetSize();

	if (ImGui::DragFloat2("Collider Offset", glm::value_ptr(offset), 1.0f)) {
		collider.SetOffset(offset);
	}

	if (ImGui::DragFloat2("Collider Size", glm::value_ptr(size), 1.0f)) {
		collider.SetSize(size);
	}

    if (ImGui::BeginPopupContextWindow("Collider Popup", ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Add Collider")) {
            Collider newCol;
            newCol.SetOffset(glm::vec2(0.0f, 0.0f));
            newCol.SetSize(glm::vec2(50.0f, 50.0f));
            sprite.GetColliders().push_back(newCol);
            selectedColliderIndex = (int)sprite.GetColliders().size() - 1;
        }
        if (!sprite.GetColliders().empty()) {
            if (ImGui::MenuItem("Remove Selected Collider")) {
                auto& colliders = sprite.GetColliders();
                colliders.erase(colliders.begin() + selectedColliderIndex);

                if (!colliders.empty())
                    selectedColliderIndex = Clamp(selectedColliderIndex, 0, (int)colliders.size() - 1);
                else
                    selectedColliderIndex = -1;
            }
        }
        ImGui::EndPopup();
    }

    if (ImGui::Button("Add Collider")) {
        Collider newCollider;
        newCollider.SetOffset(glm::vec2(0.0f, 0.0f));
        newCollider.SetSize(glm::vec2(50.0f, 50.0f));
        sprite.GetColliders().push_back(newCollider);

        selectedColliderIndex = (int)sprite.GetColliders().size() - 1;
    }

    if (!sprite.GetColliders().empty()) {
        if (ImGui::Button("Remove Selected Collider")) {
            auto& colliders = sprite.GetColliders();
            colliders.erase(colliders.begin() + selectedColliderIndex);

            if (!colliders.empty()) {
                selectedColliderIndex = Clamp(selectedColliderIndex, 0, (int)colliders.size() - 1);
            }
            else {
                selectedColliderIndex = -1;
            }
        }
    }

	ImGui::End();
}

void SpriteEditor::HandleMouse(SDL_Window* window, Sprite& sprite, const glm::mat4& projection) {
	if (ImGui::GetIO().WantCaptureMouse) {
		currentAction = ColliderAction::None;
		return;
	}

	float mouseX, mouseY;
	Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);
	glm::vec2 mouseWorld = ScreenToWorld2D((int)mouseX, (int)mouseY);

	auto& colliders = sprite.GetColliders();
	bool mouseDown = (mouseState & SDL_BUTTON_LMASK);

    if (colliders.empty()) {
        return;
    }

	if (!mouseDown) {
		currentAction = ColliderAction::None;
		return;
	}

    if (currentAction == ColliderAction::None) {
        // --- NEW --- Always prioritize picking
        for (size_t i = 0; i < colliders.size(); ++i) {
            Collider& collider = colliders[i];
            glm::vec2 min = collider.position;
            glm::vec2 max = collider.position + collider.size;
            glm::vec2 center = collider.position + collider.size * 0.5f;

            if (IsInsideCircle(mouseWorld, center, 6.0f)) {
                currentAction = ColliderAction::Drag;
                dragOffset = mouseWorld - collider.position;
                selectedColliderIndex = static_cast<int>(i);
                break;
            }
            else if (IsNearLine(mouseWorld.x, min.x, resizeThreshold)) {
                currentAction = ColliderAction::ResizeLeft;
                selectedColliderIndex = static_cast<int>(i);
                break;
            }
            else if (IsNearLine(mouseWorld.x, max.x, resizeThreshold)) {
                currentAction = ColliderAction::ResizeRight;
                selectedColliderIndex = static_cast<int>(i);
                break;
            }
            else if (IsNearLine(mouseWorld.y, min.y, resizeThreshold)) {
                currentAction = ColliderAction::ResizeTop;
                selectedColliderIndex = static_cast<int>(i);
                break;
            }
            else if (IsNearLine(mouseWorld.y, max.y, resizeThreshold)) {
                currentAction = ColliderAction::ResizeBottom;
                selectedColliderIndex = static_cast<int>(i);
                break;
            }
        }
    }

    if (selectedColliderIndex >= 0 && selectedColliderIndex < (int)colliders.size()) {
        Collider& collider = colliders[selectedColliderIndex];
        switch (currentAction) {
        case ColliderAction::Drag:
            collider.SetOffset(mouseWorld - dragOffset);
            break;
        case ColliderAction::ResizeLeft: {
            glm::vec2 currentPosition = collider.GetOffset();
            glm::vec2 currentSize = collider.GetSize();
            float delta = mouseWorld.x - currentPosition.x;
            currentPosition.x += delta;
            currentSize.x -= delta;
            collider.SetOffset(currentPosition);
            collider.SetSize(currentSize);
            break;
        }
        case ColliderAction::ResizeRight: {
            glm::vec2 currentSize = collider.GetSize();
            currentSize.x = mouseWorld.x - collider.GetOffset().x;
            collider.SetSize(currentSize);
            break;
        }
        case ColliderAction::ResizeTop: {
            glm::vec2 currentPosition = collider.GetOffset();
            glm::vec2 currentSize = collider.GetSize();
            float delta = mouseWorld.y - currentPosition.y;
            currentPosition.y += delta;
            currentSize.y -= delta;
            collider.SetOffset(currentPosition);
            collider.SetSize(currentSize);
            break;
        }
        case ColliderAction::ResizeBottom: {
            glm::vec2 currentSize = collider.GetSize();
            currentSize.y = mouseWorld.y - collider.GetOffset().y;
            collider.SetSize(currentSize);
            break;
        }
        default: break;
        }
    }
}