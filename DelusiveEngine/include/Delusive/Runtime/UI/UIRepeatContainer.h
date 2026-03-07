#pragma once
#include <Delusive/Runtime/UI/UIElement.h>

struct DelusiveUIPrototype;

class UIRepeatContainer : public UIElement {
public:
	UIRepeatContainer(DelusiveRenderer&);
	UIRepeatContainer() = delete;

	const std::string GetType() const override { return "UIRepeatContainer"; }
	std::unique_ptr<UIElement> Clone() const override;

	void RegisterProperties() override;
	void Draw(const glm::mat4&) override;
	void DrawImGui() override;

	void SetPrototype(std::unique_ptr<UIElement>);

	void SetCount(int newCount) { if (newCount < 1) newCount = 1; count = newCount; }
	void SetRows(int newRows) { if (newRows < 1) newRows = 1; rows = newRows; }
	void SetSpacing(float newSpacing) { spacing = newSpacing; }
	int GetCount() { return count; }
	int GetRows() { return rows; }
	float GetSpacing() { return spacing; }

	void RegenerateChildren();

    void Serialize(std::ostream& out) const override;
    void Deserialize(std::istream& in) override;
private:
	int count = 1;
	int rows = 1;
	float spacing = 0.0f;

	std::unique_ptr<DelusiveUIPrototype> prototype;
};