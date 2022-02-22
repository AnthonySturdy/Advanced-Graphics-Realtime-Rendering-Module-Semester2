#include "Source/pch.h"
#include "Source/Game.h"

#include "Source/Game/Components/TransformComponent.h"
#include "Source/Rendering/RenderPassGeometry.h"

extern void ExitGame() noexcept;

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
	DX::DeviceResources::Instance()->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
	DX::DeviceResources::Instance()->SetWindow(window, width, height);

	DX::DeviceResources::Instance()->CreateDeviceResources();
	CreateDeviceDependentResources();

	DX::DeviceResources::Instance()->CreateWindowSizeDependentResources();
	CreateWindowSizeDependentResources();

	// Create and Initialise render pipeline
	RenderPipeline.push_back(std::make_unique<RenderPassGeometry>(GameObjects));
	for (auto& rp : RenderPipeline)
		rp->Initialise();

	// Vsync
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	m_timer.Tick([&]() {
		Update(m_timer);
	});

	Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
	const float elapsedTime = static_cast<float>(timer.GetElapsedSeconds());

	for (auto& go : GameObjects)
		go.Update(elapsedTime);
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return;
	}

	Clear();

	DX::DeviceResources::Instance()->PIXBeginEvent(L"Render");

	for (const auto& rp : RenderPipeline)
		rp->Render();

	DX::DeviceResources::Instance()->PIXEndEvent();

	// Show the new frame.
	DX::DeviceResources::Instance()->Present();
}

// Helper method to clear the back buffers.
void Game::Clear()
{
	DX::DeviceResources::Instance()->PIXBeginEvent(L"Clear");

	// Clear the views.
	const auto context = DX::DeviceResources::Instance()->GetD3DDeviceContext();
	const auto renderTarget = DX::DeviceResources::Instance()->GetRenderTargetView();
	const auto depthStencil = DX::DeviceResources::Instance()->GetDepthStencilView();

	context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);

	// Set the viewport.
	const auto viewport = DX::DeviceResources::Instance()->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	DX::DeviceResources::Instance()->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
	// TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
	// TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
	// TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
	m_timer.ResetElapsedTime();

	// TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
	auto r = DX::DeviceResources::Instance()->GetOutputSize();
	DX::DeviceResources::Instance()->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
	if (!DX::DeviceResources::Instance()->WindowSizeChanged(width, height))
		return;

	CreateWindowSizeDependentResources();

	// TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
	// TODO: Change to desired default window size (note minimum size is 320x200).
	width = 800;
	height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
	auto device = DX::DeviceResources::Instance()->GetD3DDevice();

	// TODO: Initialize device dependent objects here (independent of window size).
	device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
	// TODO: Initialize windows-size dependent objects here.
}

void Game::OnDeviceLost()
{
	// TODO: Add Direct3D resource cleanup here.
}

void Game::OnDeviceRestored()
{
	CreateDeviceDependentResources();

	CreateWindowSizeDependentResources();
}
#pragma endregion
