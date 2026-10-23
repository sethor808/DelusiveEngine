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

	//The builder edits the prototype; the copies are rebuilt from it
	std::vector<UIElement*> GetEditorChildren() override;
	bool AcceptsEditorChild() const override { return !prototype; }
	void AddEditorChild(std::unique_ptr<UIElement>) override;
	bool RemoveEditorChild(UIElement*) override;
	void NotifyEdited() override;

	void SetPrototype(std::unique_ptr<UIElement>);

	//A container inside another bound item takes its count from that item
	void ApplyBinding(const UIBindItem&) override;

	void SetCount(int newCount) { if (newCount < 0) newCount = 0; count = newCount; }
	void SetRows(int newRows) { if (newRows < 1) newRows = 1; rows = newRows; }
	void SetSpacing(float newSpacing) { spacing = newSpacing; }
	int GetCount() { return count; }
	int GetRows() { return rows; }
	float GetSpacing() { return spacing; }

	void RegenerateChildren();
protected:
	void DrawBindingImGui() override;
	void DrawChildrenImGui() override;
private:
	int count = 1;
	int rows = 1;
	float spacing = 0.0f;

	DelusiveObject<UIElement> prototype;

	//Binding - a source repeats over game data, bindCount reads a field of the parent's item
	std::string source;
	std::string bindCount;
	std::vector<UIBindItem> boundItems;
	bool hasParentItem = false;

	//Where the copies were laid out from
	glm::vec2 placedAt = { 0.0f, 0.0f };

	//Regenerates the copies and gives each its data again
	void Rebuild();
};
