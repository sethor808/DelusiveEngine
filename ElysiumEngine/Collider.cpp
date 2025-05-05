#include "Collider.h"

glm::vec2 Collider::GetPosition() {
	return position;
}

glm::vec2& Collider::GetMutableOffset() {
	return offset;
}

glm::vec2 Collider::GetOffset() {
	return offset;
}

void Collider::SetOffset(glm::vec2 newOffset) {
	offset = newOffset;
}

void Collider::SetPosition(glm::vec2 newPos) {
	position = newPos;
}

glm::vec2 Collider::GetSize() {
	return size;
}

glm::vec2& Collider::GetMutableSize() {
	return size;
}

void Collider::SetSize(glm::vec2 newSize) {
	size = newSize;
}