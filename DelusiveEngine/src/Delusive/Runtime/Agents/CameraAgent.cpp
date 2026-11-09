#include <Delusive/Runtime/Agents/CameraAgent.h>
#include <Delusive/Runtime/Core/DelusiveCoreIncludes.h>
#include <imgui/imgui.h>
#include <cmath>
#include <random>

CameraAgent::CameraAgent(DelusiveInstance& instance)
	: Agent(instance)
{
	SetName("New Camera");

	RegisterProperties();
}

std::string CameraAgent::GetType() const{
	return "CameraAgent";
}

void CameraAgent::RegisterProperties() {
	Agent::RegisterProperties();
	//panOffset is runtime dragging, not authored, so it is not saved
	registry->Register("zoom", &zoom);
}

void CameraAgent::Update(float deltaTime) {
	if (shakeTimer <= 0.0f) {
		shakeOffset = glm::vec2(0.0f);
		return;
	}

	//A fresh direction every tick, shrinking as the shake runs out
	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> angle(0.0f, 6.2831853f);
	const float a = angle(rng);
	shakeOffset = glm::vec2(std::cos(a), std::sin(a)) * shakeStrength * (shakeTimer / shakeDuration);
	shakeTimer -= deltaTime;
}

void CameraAgent::Shake(float strength, float seconds) {
	if (seconds <= 0.0f) return;
	//A weaker shake never cuts a stronger one short
	if (shakeTimer > 0.0f && strength < shakeStrength * (shakeTimer / shakeDuration)) return;
	shakeStrength = strength;
	shakeTimer = shakeDuration = seconds;
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

	const glm::vec2 centre = panOffset + shakeOffset;
	return glm::ortho(
		-halfWidthUnits + centre.x,
		halfWidthUnits + centre.x,
		-halfHeightUnits + centre.y,
		halfHeightUnits + centre.y,
		-1.0f, 1.0f
	);
}

glm::mat4 CameraAgent::GetViewProjectionFromWindow(SDL_Window* window) const {
	int w, h;
	SDL_GetWindowSize(window, &w, &h);
	return GetViewProjection(w, h);
}
