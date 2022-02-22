#pragma once
#include "Source/Rendering/RenderPass.h"

class GameObject;

class RenderPassGeometry : public RenderPass
{
public:
	RenderPassGeometry(std::vector<GameObject>& gameObjects);
	RenderPassGeometry(const RenderPassGeometry&) = default;
	RenderPassGeometry(RenderPassGeometry&&) = default;
	RenderPassGeometry& operator=(const RenderPassGeometry&) = delete;
	RenderPassGeometry& operator=(RenderPassGeometry&&) = delete;
	~RenderPassGeometry() override = default;

	void Initialise() override;
	void Render() override;

private:
	std::vector<GameObject>& GameObjects;
};
