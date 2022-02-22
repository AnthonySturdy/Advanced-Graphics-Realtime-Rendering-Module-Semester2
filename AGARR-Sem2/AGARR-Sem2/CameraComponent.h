#pragma once
#include "Source/Game/GameObject.h"

class CameraComponent : public Component
{
public:
	CameraComponent();
	CameraComponent(const CameraComponent&) = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	~CameraComponent() override = default;

private:
	float FOV;
	float AspectRatio;
	float NearPlane;
	float FarPlane;
};
