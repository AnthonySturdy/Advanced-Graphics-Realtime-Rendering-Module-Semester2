#include "Source/pch.h"
#include "Source/Rendering/RenderPassGeometry.h"

#include "Source/Game/GameObject.h"

RenderPassGeometry::RenderPassGeometry(std::vector<GameObject>& gameObjects)
	: GameObjects(gameObjects) {}

void RenderPassGeometry::Initialise()
{
	// TODO: Create render texture
}

void RenderPassGeometry::Render()
{
	// TODO: Bind render texture
	// TODO: Clear render target

	for (auto& go : GameObjects)
		go.Render();

	// TODO: Unbind render target
}
