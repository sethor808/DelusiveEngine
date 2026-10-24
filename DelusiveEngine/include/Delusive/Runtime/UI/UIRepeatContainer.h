#pragma once
#include <Delusive/Runtime/UI/UIElement.h>

//Lays its children out in a grid. A script that needs more or fewer calls SetCount,
//which copies the first child (the template) and hides the extras. The template always shows.
class UIRepeatContainer : public UIElement {
public:
	UIRepeatContainer(DelusiveInstance&);
	UIRepeatContainer() = delete;

	const std::string GetType() const override { return "UIRepeatContainer"; }

	void RegisterProperties() override;
	void Update(float) override;

	void SetCount(int);
	void SetRows(int newRows) { if (newRows < 1) newRows = 1; rows = newRows; }
	void SetSpacing(float newSpacing) { spacing = newSpacing; }
		int GetRows() const { return rows; }
	float GetSpacing() const { return spacing; }
private:
	int rows = 1;
	float spacing = 0.0f;
};
