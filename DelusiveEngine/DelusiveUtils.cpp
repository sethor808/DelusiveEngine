#include "DelusiveUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <imgui/imgui.h>
#include "Renderer.h"

glm::vec2 ScreenToWorld2D(int mouseX, int mouseY) {
    ImVec2 display = ImGui::GetIO().DisplaySize;

    // Normalize from screen (pixels) to NDC
    float x = (2.0f * mouseX) / display.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / display.y; // Note the flip of y

    glm::vec4 ndc = glm::vec4(x, y, 0.0f, 1.0f);

    glm::mat4 inverseProj = glm::inverse(Renderer::GetProjection());
    glm::vec4 worldPos = inverseProj * ndc;

    return glm::vec2(worldPos);
}

glm::vec2 ScreenToWorld2D(int mouseX, int mouseY, glm::mat4 projection) {
    ImVec2 display = ImGui::GetIO().DisplaySize;

    // Normalize from screen (pixels) to NDC
    float x = (2.0f * mouseX) / display.x - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / display.y; // Note the flip of y

    glm::vec4 ndc = glm::vec4(x, y, 0.0f, 1.0f);

    glm::mat4 inverseProj = glm::inverse(projection);
    glm::vec4 worldPos = inverseProj * ndc;

    return glm::vec2(worldPos);
}

bool IsInsideCircle(const glm::vec2& point, const glm::vec2& center, float radius) {
    return glm::length(point - center) <= radius;
}

bool IsNearLine(float point, float line, float threshold) {
    return std::abs(point - line) <= threshold;
}