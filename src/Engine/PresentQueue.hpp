//

// Created by carlo on 2024-09-24.
//

#ifndef PRESENTQUEUE_HPP
#define PRESENTQUEUE_HPP

namespace ENGINE
{
struct PresentQueue
{
  public:
	PresentQueue(Core *core, WindowDesc windowDesc, uint32_t imagesCount, vk::PresentModeKHR preferredMode, glm::uvec2 windowSize)
	{
		this->core                = core;
		this->swapChain           = core->CreateSwapchain(preferredMode, imagesCount, windowDesc, windowSize);
		this->swapchainImageViews = swapChain->GetImageViews();
		this->swapchainRect       = vk::Rect2D(vk::Offset2D(), swapChain->extent);
		this->imageIndex          = -1;
	}
	ImageView *AcquireImage(vk::Semaphore signalSemaphore)
	{
		auto res         = swapChain->AcquireNextImage(signalSemaphore);
		this->imageIndex = res.value;
		return swapchainImageViews[imageIndex];
	}
	void PresentImage(vk::Semaphore waitSemaphore)
	{
		vk::SwapchainKHR swapchains[]     = {swapChain.get()->swapchainHandle.get()};
		vk::Semaphore    waitSemaphores[] = {waitSemaphore};

		auto presentInfo = vk::PresentInfoKHR()
		                       .setSwapchainCount(1)
		                       .setPSwapchains(swapchains)
		                       .setPImageIndices(&imageIndex)
		                       .setPResults(nullptr)
		                       .setWaitSemaphoreCount(1)
		                       .setPWaitSemaphores(waitSemaphores);

		auto res = core->queueWorkerManager->GetWorkerQueue("Present")->workerQueue.presentKHR(presentInfo);
	}

	vk::Extent2D GetImageSize()
	{
		return swapChain.get()->extent;
	}
	Core                      *core;
	std::unique_ptr<SwapChain> swapChain;
	std::vector<ImageView *>   swapchainImageViews;
	uint32_t                   imageIndex;
	vk::Rect2D                 swapchainRect;
};

struct InFlightQueue
{
	InFlightQueue(Core *core, RenderGraph *renderGraph, WindowDesc windowDesc, uint32_t inflightCount, vk::PresentModeKHR preferredMode, glm::uvec2 windowSize)
	{
		this->core        = core;
		this->renderGraph = renderGraph;
		presentQueue.reset(new PresentQueue(this->core, windowDesc, inflightCount, preferredMode, windowSize));

		core->queueWorkerManager->AllocateCmds(inflightCount);
		timelineSemaphore = core->CreateVulkanTimelineSemaphore(0);

		for (int frameIndex = 0; frameIndex < inflightCount; frameIndex++)
		{
			FrameResources frame;
			frame.inflightFence              = core->CreateFence(true);
			frame.imageAcquiredSemaphore     = core->CreateVulkanSemaphore();
			frame.renderingFinishedSemaphore = core->CreateVulkanSemaphore();
			frameResources.push_back(std::move(frame));
		}
		frameIndex = 0;
		core->queueWorkerManager->SetFrameIndex(frameIndex);
	}
	void BeginFrame()
	{
		auto &currFrame = frameResources[frameIndex];
		{
			core->WaitForFence(currFrame.inflightFence.get());
			core->ResetFence(currFrame.inflightFence.get());
		}
		{
			currentSwapchainImageView = presentQueue->AcquireImage(currFrame.imageAcquiredSemaphore.get());
		}
		core->queueWorkerManager->BeginCmds();
		renderGraph->currentBackBufferSwapchain = currentSwapchainImageView;
		renderGraph->frameIndex                 = frameIndex;
		// add pass info from my data
	}
	void EndFrame()
	{
		auto &currFrame = frameResources[frameIndex];

		std::vector<std::string> queueNames;

		TransitionImage(currentSwapchainImageView->imageData, PRESENT, currentSwapchainImageView->GetSubresourceRange(),
		                renderGraph->sortedQueueBatches.back().commandBuffer);

		core->queueWorkerManager->EndCmds();

		{
			vk::Semaphore timeline = timelineSemaphore.get();

			for (int i = 0; i < renderGraph->sortedQueueBatches.size(); ++i)
			{
				auto* queueRef = renderGraph->core->queueWorkerManager->GetWorkerQueue(renderGraph->sortedQueueBatches[i].queueName);
				if (renderGraph->sortedQueueBatches.size() == 1)
				{
					vk::Semaphore          waitSemaphores[]   = {currFrame.imageAcquiredSemaphore.get()};
					vk::PipelineStageFlags waitStages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
					vk::Semaphore          signalSemaphores[] = {currFrame.renderingFinishedSemaphore.get()};

					auto submitInfo = vk::SubmitInfo()
					                      .setWaitSemaphoreCount(1)
					                      .setPWaitSemaphores(waitSemaphores)
					                      .setPWaitDstStageMask(waitStages)
					                      .setCommandBufferCount(1)
					                      .setPCommandBuffers(&renderGraph->sortedQueueBatches[i].commandBuffer)
					                      .setSignalSemaphoreCount(1)
					                      .setPSignalSemaphores(signalSemaphores);

					queueRef->workerQueue.submit({submitInfo}, currFrame.inflightFence.get());
					break;
				}
				if (i == 0)
				{
					vk::Semaphore          waitSemaphores[] = {currFrame.imageAcquiredSemaphore.get()};
					vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
					vk::Semaphore          signalTimeline[] = {timeline};

					uint64_t signalValue = ++timelineValue;

					vk::TimelineSemaphoreSubmitInfo timelineInfo = {};
					timelineInfo.setSignalSemaphoreValueCount(1);
					timelineInfo.setPSignalSemaphoreValues(&signalValue);

					auto submitInfo = vk::SubmitInfo()
					                      .setWaitSemaphoreCount(1)
					                      .setPNext(&timelineInfo)
					                      .setPWaitSemaphores(waitSemaphores)
					                      .setPWaitDstStageMask(waitStages)
					                      .setCommandBufferCount(1)
					                      .setPCommandBuffers(&renderGraph->sortedQueueBatches[i].commandBuffer)
					                      .setSignalSemaphoreCount(1)
					                      .setPSignalSemaphores(signalTimeline);
					queueRef->workerQueue.submit(submitInfo);
				}
				else if (i > 0 && i != renderGraph->sortedQueueBatches.size() - 1)
				{
					vk::Semaphore          waitSemaphores[] = {timeline};
					vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
					vk::Semaphore          signalTimeline[] = {timeline};

					uint64_t waitValue   = timelineValue;
					uint64_t signalValue = ++timelineValue;

					vk::TimelineSemaphoreSubmitInfo timelineInfo = {};
					timelineInfo.setWaitSemaphoreValueCount(1);
					timelineInfo.setPWaitSemaphoreValues(&waitValue);
					timelineInfo.setSignalSemaphoreValueCount(1);
					timelineInfo.setPSignalSemaphoreValues(&signalValue);

					auto submitInfo = vk::SubmitInfo()
					                      .setWaitSemaphoreCount(1)
					                      .setPNext(&timelineInfo)
					                      .setPWaitSemaphores(waitSemaphores)
					                      .setPWaitDstStageMask(waitStages)
					                      .setCommandBufferCount(1)
					                      .setPCommandBuffers(&renderGraph->sortedQueueBatches[i].commandBuffer)
					                      .setSignalSemaphoreCount(1)
					                      .setPSignalSemaphores(signalTimeline);
					queueRef->workerQueue.submit(submitInfo);
				}
				else if (i == renderGraph->sortedQueueBatches.size() - 1)
				{
					vk::Semaphore          waitSemaphores[] = {timeline};
					vk::PipelineStageFlags waitStages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
					vk::Semaphore          signalTimeline[] = {currFrame.renderingFinishedSemaphore.get()};

					uint64_t waitValue = timelineValue;

					vk::TimelineSemaphoreSubmitInfo timelineInfo = {};
					timelineInfo.setWaitSemaphoreValueCount(1);
					timelineInfo.setPWaitSemaphoreValues(&waitValue);

					auto submitInfo = vk::SubmitInfo()
					                      .setWaitSemaphoreCount(1)
					                      .setPNext(&timelineInfo)
					                      .setPWaitSemaphores(waitSemaphores)
					                      .setPWaitDstStageMask(waitStages)
					                      .setCommandBufferCount(1)
					                      .setPCommandBuffers(&renderGraph->sortedQueueBatches[i].commandBuffer)
					                      .setSignalSemaphoreCount(1)
					                      .setPSignalSemaphores(signalTimeline);
					queueRef->workerQueue.submit(submitInfo, currFrame.inflightFence.get());
				}
			}
		}
		presentQueue->PresentImage(currFrame.renderingFinishedSemaphore.get());
		frameIndex = (frameIndex + 1) % frameResources.size();
		core->queueWorkerManager->SetFrameIndex(frameIndex);
	}
	void BeginParallelThreads()
	{
		for (auto &worker : core->queueWorkerManager.get()->workersQueues)
		{
			if (worker.second.isMainThreat)
			{
				continue;
			}
			std::function<void()> workerStartTask([&worker, this] {
				// worker.second.commandBuffers = std::move(core->AllocateCommandBuffers(worker.second.workerCommandPool.get(), 1));
				// auto bufferBeginInfo         = vk::CommandBufferBeginInfo()
				//                            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
			});
			worker.second.taskThreat.AddTask(workerStartTask);
		}
	}

	void EndParallelThreads()
	{
		for (auto &worker : core->queueWorkerManager.get()->workersQueues)
		{
			if (worker.second.isMainThreat)
			{
				continue;
			}
			std::function<void()> workerStartTask([&worker, this] {
			});
			worker.second.taskThreat.AddTask(workerStartTask);
		}
	}

	std::vector<FrameResources> frameResources;
	size_t                      frameIndex;
	vk::UniqueSemaphore         timelineSemaphore;
	uint64_t                    timelineValue = 0;

	Core                         *core;
	RenderGraph                  *renderGraph;
	std::unique_ptr<PresentQueue> presentQueue;
	ImageView                    *currentSwapchainImageView;
};

}        // namespace ENGINE

#endif        // PRESENTQUEUE_HPP
