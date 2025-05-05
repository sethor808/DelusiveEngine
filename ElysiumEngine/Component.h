#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Component {
public:
	virtual void Update(float);
	virtual ~Component() = default;
protected:
};