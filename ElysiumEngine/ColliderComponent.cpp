#include "ColliderComponent.h"
ColliderRenderer* ColliderComponent::renderer = nullptr;

ColliderComponent::ColliderComponent(){ 
	ColliderComponent::SetRenderer(renderer);  // Called ONCE 
}

void ColliderComponent::Draw(const glm::mat4& projection) const{
	renderer->Draw(*this, projection);
}