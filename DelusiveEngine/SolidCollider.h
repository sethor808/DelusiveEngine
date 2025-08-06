#pragma once
#include "ColliderComponent.h"
#include <iostream>

class SolidCollider : public ColliderComponent {
public:
	std::unique_ptr<Component> Clone() const override {
		return std::make_unique<SolidCollider>(*this);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Solid;
	}

	void Update(float) override{};
	void DrawImGui() override;
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "SolidCollider";
	}

	void Serialize(std::ofstream& out) const override;
	void Deserialize(std::ifstream& in) override;
private:
};