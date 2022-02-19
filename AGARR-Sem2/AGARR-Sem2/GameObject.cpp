#include "pch.h"
#include "GameObject.h"

void GameObject::Update()
{
	for (const auto& [type, components] : Components)
	{
		for (const auto& component : components)
		{
			component->Update();
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
