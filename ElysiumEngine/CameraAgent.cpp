#include "CameraAgent.h"

CameraAgent::CameraAgent() {

}

std::unique_ptr<Agent> CameraAgent::Clone() const {
	auto cam = std::make_unique<CameraAgent>();
	cam->SetZoom(zoom);
	cam->panOffset = panOffset;
	return cam;
}

void CameraAgent::Update(float deltaTime) {

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
