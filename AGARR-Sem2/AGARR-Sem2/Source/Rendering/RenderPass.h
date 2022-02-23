#pragma once
class RenderPass
{
public:
	RenderPass() = default;
	RenderPass(const RenderPass&) = default;
	RenderPass(RenderPass&&) = default;
	RenderPass& operator=(const RenderPass&) = default;
	RenderPass& operator=(RenderPass&&) = default;
	virtual ~RenderPass() = default;

	virtual void Initialise() = 0;
	virtual void Render() = 0;
	virtual void RenderGUI() = 0;
};
