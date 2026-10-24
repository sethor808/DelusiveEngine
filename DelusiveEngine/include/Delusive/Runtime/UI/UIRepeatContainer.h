#pragma once
#include <Delusive/Runtime/UI/UIElement.h>
#include <Delusive/Runtime/Core/IDLink.h>

class UIRepeatContainer : public UIElement {
public:
	UIRepeatContainer(DelusiveInstance&);
	UIRepeatContainer() = delete;

	const std::string GetType() const override { return "UIRepeatContainer"; }

	void RegisterProperties() override;
	void Update(float) override;
	void Draw(const glm::mat4&) override;

	//Children are rebuilt from the prototype, so only the prototype is saved
	bool SavesChildren() const override { return false; }

	//Keeps the copies with the container
	void Move(const glm::vec2&) override;

	//The prototype is edited in the inspector, so the hierarchy shows no copies
	std::vector<UIElement*> GetEditorChildren() override { return {}; }
	bool AcceptsEditorChild() const override { return false; }
	//Copies are rebuilt after an edit so they match the prototype
	void NotifyEdited() override { RegenerateChildren(); }

	void SetPrototype(std::unique_ptr<UIElement>);

	void SetCount(int newCount) { if (newCount < 1) newCount = 1; count = newCount; }
	void SetRows(int newRows) { if (newRows < 1) newRows = 1; rows = newRows; }
	void SetSpacing(float newSpacing) { spacing = newSpacing; }
	int GetCount() { return count; }
	int GetRows() { return rows; }
	float GetSpacing() { return spacing; }

	void RegenerateChildren();
protected:
	void DrawChildrenImGui() override;
private:
	int count = 1;
	int rows = 1;
	float spacing = 0.0f;

	DelusiveObject<UIElement> prototype;

	//Where the copies were laid out from
	glm::vec2 placedAt = { 0.0f, 0.0f };
};
