#pragma once
#include <Delusive/Runtime/Components/ColliderComponent.h>
#include <iostream>

class SolidCollider : public ColliderComponent {
public:
	SolidCollider(DelusiveInstance&);
	SolidCollider() = delete;

	std::unique_ptr<Component> Clone() const override {
		//TODO: Properly copy over values
		return std::make_unique<SolidCollider>(instance);
	}

	ColliderType GetColliderType() const override {
		return ColliderType::Solid;
	}

	void Update(float) override{};
	void OnCollision(ColliderComponent*) override;

	const char* GetType() const override {
		return "SolidCollider";
	}
private:
};