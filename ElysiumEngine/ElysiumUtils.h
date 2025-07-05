#pragma once
#include <glm/glm.hpp>

glm::vec2 ScreenToWorld2D(int, int);
bool IsInsideCircle(const glm::vec2&, const glm::vec2&, float);
bool IsNearLine(float, float, float);