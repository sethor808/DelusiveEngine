#pragma once
#include "UIElement.h"
#include "DelusiveData.h"

class UIImage : public UIElement {
public:
	UIImage();
	UIImage(const glm::vec2& pos, const glm::vec2& size);

	void RegisterProperties() override;
	std::unique_ptr<UIElement> Clone() const override;

	void Init();
	void SetTexturePath(const std::string&);

	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2&, bool) override {}

	const std::string GetType() const override;

private:
	DelusiveTexture textureData;
};