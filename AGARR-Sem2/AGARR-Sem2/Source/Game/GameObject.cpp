#include "pch.h"
#include "Game/GameObject.h"

#include "Game/Components/TransformComponent.h"

GameObject::GameObject()
{
	if (!GetComponent<TransformComponent>())
	{
		auto* comp = AddComponent<TransformComponent>(new TransformComponent());
		comp->Removable = false;
	}
}

void GameObject::Update(float deltaTime)
{
	for (const auto& [type, components] : Components)
	{
		for (const auto& component : components)
		{
			component->Update(deltaTime);
		}
	}
}

void GameObject::Render()
{
	for (const auto& [type, components] : Components)
	{
		for (const auto& component : components)
		{
			component->Render();
		}
	}
}
