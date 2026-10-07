#pragma once
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/IDLink.h>

struct DelusiveUIPrototype;

class UIRepeatContainer : public UIElement {
public:
	UIRepeatContainer(DelusiveInstance&);
	UIRepeatContainer() = delete;

	const std::string GetType() const override { return "UIRepeatContainer"; }

	void RegisterProperties() override;
	void Draw(const glm::mat4&) override;
	void DrawImGui() override;

	//Children are rebuilt from the prototype, so only the prototype is saved
	bool SavesChildren() const override { return false; }

	void SetPrototype(std::unique_ptr<UIElement>);

	void SetCount(int newCount) { if (newCount < 1) newCount = 1; count = newCount; }
	void SetRows(int newRows) { if (newRows < 1) newRows = 1; rows = newRows; }
	void SetSpacing(float newSpacing) { spacing = newSpacing; }
	int GetCount() { return count; }
	int GetRows() { return rows; }
	float GetSpacing() { return spacing; }

	void RegenerateChildren();
private:
	int count = 1;
	int rows = 1;
	float spacing = 0.0f;

	DelusiveObject<UIElement> prototype;
};