#pragma once
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/DelusiveData.h>

class UIImage : public UIElement {
public:
	UIImage(DelusiveInstance&);

	void RegisterProperties() override;
	std::unique_ptr<UIElement> Clone() const override;

	void Update(float) override;
	void DrawImGui() override;

	void Init();
	void SetTexturePath(const std::string&);

	void Draw(const glm::mat4& proj) override;
	void HandleMouse(const glm::vec2&, bool) override {}

	const std::string GetType() const override;
private:
	DelusiveTexture textureData;
};