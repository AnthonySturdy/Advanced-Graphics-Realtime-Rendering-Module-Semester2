#include "pch.h"
#include "Game/GameObject.h"

#include <format>
#include <string>

#include "Game/Components/TransformComponent.h"

GameObject::GameObject()
{
	if (!GetComponent<TransformComponent>())
	{
		auto* comp = AddComponent(new TransformComponent());
		comp->Removable = false;
	}
}

void GameObject::Update(float deltaTime)
{
	for (const auto& [type, components] : Components)
		for (const auto& component : components)
			component->Update(deltaTime);
}

void GameObject::Render()
{
	for (const auto& [type, components] : Components)
		for (const auto& component : components)
			component->Render();
}

void GameObject::RenderGUI()
{
	const std::string goName = "GameObject (0x" + std::format("{:x}", reinterpret_cast<intptr_t>(this)) + ")";
	if (ImGui::CollapsingHeader(goName.c_str()))
		for (const auto& [type, components] : Components)
			for (const auto& component : components)
			{
				ImGui::PushID(component.get());
				if (ImGui::TreeNode(component->GetComponentName().c_str()))
				{
					ImGui::BeginChild("##", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowHeight() / 4), true);
					component->RenderGUI();
					ImGui::EndChild();

					ImGui::TreePop();
				}
				ImGui::PopID();
			}
}
