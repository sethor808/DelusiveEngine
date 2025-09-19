#include "CameraAgent.h"
#include "DelusiveRenderer.h"
#include <imgui/imgui.h>

CameraAgent::CameraAgent() {
	SetName("NewCamera");
}

CameraAgent::CameraAgent(const std::string& name) {
	SetName(name);
}

std::string CameraAgent::GetType() const{
	return "CameraAgent";
}

std::unique_ptr<Agent> CameraAgent::Clone(Scene* scene) const {
	auto cam = std::make_unique<CameraAgent>();
	cam->SetName(GetName());
	cam->SetZoom(zoom);
	cam->panOffset = panOffset;
	cam->SetScene(scene);
	return cam;
}

void CameraAgent::Update(float deltaTime) {

}

void CameraAgent::DrawImGui() {
	bool changed = false;

	ImGui::Text("Transform");
	ImGui::Text("Position: ");
	ImGui::SameLine();
	glm::vec2 pos = panOffset;
	if (ImGui::DragFloat2("##position", glm::value_ptr(pos), 1.0f)) {
		panOffset = pos;
	}
}

void CameraAgent::HandleInput(const glm::vec2& mousePos, bool middleMouseDown, float scrollDelta) {
	if (middleMouseDown) {
		if (!dragging) {
			dragging = true;
			lastMousePos = mousePos;
		}
		else {
			glm::vec2 delta = (lastMousePos - mousePos);
			delta.y = -delta.y;  // Flip Y

			panOffset += delta / (64.0f * zoom);
			lastMousePos = mousePos;
		}
	}
	else {
		dragging = false;
	}

	if (scrollDelta != 0.0f) {
		zoom *= (scrollDelta > 0) ? 0.9f : 1.1f;
		zoom = std::clamp(zoom, 0.1f, 10.0f);
	}
}

void CameraAgent::Follow(Agent* target) {
	followTarget = target;
}

void CameraAgent::SetZoom(float z) {
	zoom = std::clamp(z, 0.1f, 10.0f);
}

float CameraAgent::GetZoom() const {
	return zoom;
}

glm::mat4 CameraAgent::GetViewProjection(int width, int height) const {
	float pixelsPerUnit = 64.0f * zoom;
	float halfWidthUnits = width / (2.0f * pixelsPerUnit);
	float halfHeightUnits = height / (2.0f * pixelsPerUnit);

	return glm::ortho(
		-halfWidthUnits + panOffset.x,
		halfWidthUnits + panOffset.x,
		-halfHeightUnits + panOffset.y,
		halfHeightUnits + panOffset.y,
		-1.0f, 1.0f
	);
}

glm::mat4 CameraAgent::GetViewProjectionFromWindow(SDL_Window* window) const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return GetViewProjection(w, h);
}
