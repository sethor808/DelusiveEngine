#pragma once
#include "UIElement.h"
#include "DelusiveData.h"

class UIImage : public UIElement {
public:
	UIImage(DelusiveRenderer&);

	void RegisterProperties() override;
	std::unique_ptr<UIElement> Clone() const override;

	void Update(float) override;

	void Init();
	void SetTexturePath(const std::string&);

	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2&, bool) override {}

	const std::string GetType() const override;

	void Deserialize(std::istream& in) override;
private:
	DelusiveTexture textureData;
};