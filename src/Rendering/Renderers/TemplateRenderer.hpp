//
// Created by carlo on 2025-03-26.
//

#ifndef TEMPLATERENDERER_HPP
#define TEMPLATERENDERER_HPP

namespace Rendering
{
using namespace ENGINE;
class TemplateRenderer : public BaseRenderer
{
  public:
	~TemplateRenderer() override = default;

	TemplateRenderer(Core *core, WindowProvider *windowProvider)
	{
		this->core           = core;
		this->renderGraph    = core->renderGraphRef;
		this->windowProvider = windowProvider;

		CreateResources();
		CreateBuffers();
		CreatePipelines();
	}

	void CreateResources()
	{
	}

	void CreateBuffers()
	{
	}

	void CreatePipelines()
	{
		AttachmentInfo colInfo = GetColorAttachmentInfo(
		    glm::vec4(0.0f), renderGraph->core->swapchainRef->GetFormat());
		Shader *vShader = renderGraph->resourcesManager->GetShader("SomeName", ShaderStage::S_VERT);
		Shader *fShader = renderGraph->resourcesManager->GetShader("SomeName", ShaderStage::S_FRAG);

		auto       imageInfo        = Image::CreateInfo2d(windowProvider->GetWindowSize(), 1, 1, ENGINE::g_32bFormat, ENGINE::colorImageUsage);
		ImageView *attachmentOutput = renderGraph->resourcesManager->GetImage("TemplateOutput", imageInfo, 0, 0);

		auto renderNode = renderGraph->AddPass(passName);
		renderNode->SetConfigs({true});
		renderNode->SetVertShader(vShader);
		renderNode->SetFragShader(fShader);
		renderNode->SetFramebufferSize(windowProvider->GetWindowSize());
		renderNode->SetVertexInput(Vertex2D::GetVertexInput());
		// change this
		renderNode->SetPushConstantSize(4);
		renderNode->AddColorAttachmentOutput("default_attachment", colInfo, BlendConfigs::B_OPAQUE);
		// only if we want to use custom attachment
		//  renderNode->AddColorImageResource("default_attachment", attachmentOutput);
		renderNode->BuildRenderGraphNode();
	}

	void RecreateSwapChainResources() override
	{
	}

	void SetRenderOperation() override
	{
		auto taskOp = new std::function<void()>(
		    [this] {
			    auto renderNode = renderGraph->GetNode(passName);
			    renderNode->AddColorImageResource("default_attachment", renderGraph->currentBackBuffer);
		    });
		auto renderOp = new std::function<void()>(
		    [this]() {
			    auto &renderNode = renderGraph->renderNodes.at(passName);
			    renderGraph->currentFrameResources->commandBuffer->bindDescriptorSets(renderNode->pipelineType,
			                                                                          renderNode->pipelineLayout.get(), 0,
			                                                                          1,
			                                                                          &renderNode->descCache->dstSet, 0, nullptr);
			    renderGraph->currentFrameResources->commandBuffer->bindPipeline(renderNode->pipelineType, renderNode->pipeline.get());
			    vk::DeviceSize offset = 0;
			    renderGraph->currentFrameResources->commandBuffer->bindVertexBuffers(
			        0, 1,
			        &renderGraph->resourcesManager->GetStagedBuffFromName("quad_default")->deviceBuffer->bufferHandle.get(),
			        &offset);
			    renderGraph->currentFrameResources->commandBuffer->bindIndexBuffer(
			        renderGraph->resourcesManager->GetStagedBuffFromName("quad_index_default")->deviceBuffer->bufferHandle.get(), 0, vk::IndexType::eUint32);
			    renderGraph->currentFrameResources->commandBuffer->drawIndexed(
			        Vertex2D::GetQuadIndices().size(), 1, 0, 0, 0);
		    });
		renderGraph->GetNode(passName)->SetRenderOperation(renderOp);
		renderGraph->GetNode(passName)->AddTask(taskOp);
	}

	void ReloadShaders() override
	{
	}

	std::string passName = "passName";

	Core           *core;
	RenderGraph    *renderGraph;
	WindowProvider *windowProvider;

	ImageView *colAttachmentView;
};
}        // namespace Rendering

#endif        // TEMPLATERENDERER_HPP
