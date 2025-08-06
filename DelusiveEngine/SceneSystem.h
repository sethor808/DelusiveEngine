#pragma once
#include <string>

class SceneSystem {
public:
	virtual ~SceneSystem() = default;

	virtual void Update(float) = 0;
	virtual void Draw(const glm::mat4&) = 0;
	virtual void Reset() = 0;
	virtual void DrawImGui() {}

	virtual void SetName(std::string _name) { name = _name; }
	virtual std::string GetName() { return name; }

	virtual std::unique_ptr<SceneSystem> Clone() const = 0;

	virtual void Serialize(std::ostream& out) const = 0;
	virtual void Deserialize(std::istream& in) = 0;

protected:
	std::string name;
};