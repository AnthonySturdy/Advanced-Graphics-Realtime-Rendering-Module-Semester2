#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>

class GameObject;

class Component
{
	friend class GameObject;

public:
	Component() = default;
	Component(const Component&) = default;
	Component(Component&&) = default;
	Component& operator=(const Component&) = default;
	Component& operator=(Component&&) = default;
	virtual ~Component() = default;

	virtual void Update() = 0;
	virtual void Render() = 0;

private:
	GameObject* Parent{ nullptr };
};

class GameObject final
{
public:
	GameObject() = default;
	GameObject(const GameObject&) = default;
	GameObject(GameObject&&) = default;
	GameObject& operator=(const GameObject&) = default;
	GameObject& operator=(GameObject&&) = default;
	~GameObject() = default;

	void Update();
	void Render();

	template <typename T> requires std::is_base_of_v<Component, T>
	[[nodiscard]] T* GetComponent()
	{
		const auto type = std::type_index(typeid(T));

		if (!Components.contains(type))
			throw std::invalid_argument("Component does not exist");

		return reinterpret_cast<T*>(Components.at(type)[0].get());
	}

	template <typename T> requires std::is_base_of_v<Component, T>
	[[nodiscard]] std::vector<T*> GetComponents()
	{
		const auto type = std::type_index(typeid(T));

		if (!Components.contains(type))
			throw std::invalid_argument("Component does not exist");

		std::vector<T*> rawPtrList;
		rawPtrList.reserve(Components.at(type).size());
		for (const auto& sPtr : Components.at(type))
			rawPtrList.push_back(reinterpret_cast<T*>(sPtr.get()));

		return rawPtrList;
	}

	template <typename T> requires std::is_base_of_v<Component, T>
	T* AddComponent(Component* component)
	{
		component->Parent = this;
		Components[std::type_index(typeid(T))].push_back(std::shared_ptr<Component>(component));
		return reinterpret_cast<T*>(component);
	}

	template <typename T> requires std::is_base_of_v<Component, T>
	void RemoveComponent(T* component)
	{
		const auto type = std::type_index(typeid(T));

		if (!Components.contains(type))
			throw std::invalid_argument("No components of specified type are assigned to this GameObject");

		for (int i = 0; i < Components[type].size(); i++)
		{
			if (Components[type][i].get() == component)
			{
				Components[type].erase(Components[type].begin() + i);
				if (Components[type].size() == 0)
					Components.erase(type);
				return;
			}
		}

		throw std::invalid_argument("The specific component requested for removal is not assigned to this GameObject");
	}

	template <typename T> requires std::is_base_of_v<Component, T>
	[[nodiscard]] bool HasComponent() const
	{
		return Components.contains(std::type_index(typeid(T)));
	}

private:
	std::unordered_map<std::type_index, std::vector<std::shared_ptr<Component>>> Components{};
};
