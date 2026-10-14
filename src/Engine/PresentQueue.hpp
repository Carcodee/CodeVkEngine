//

// Created by carlo on 2024-09-24.
//

#ifndef PRESENT_QUEUE_HPP
#define PRESENT_QUEUE_HPP

namespace engine
{
struct Present_Queue
{
  public:
	Present_Queue(engine::Core *core, Window_Desc window_desc, uint32_t images_count, vk::PresentModeKHR preferred_mode, glm::uvec2 window_size)
	{
		this->core                = core;
		this->swap_chain           = core->Create_Swapchain(preferred_mode, images_count, window_desc, window_size);
		this->swapchain_image_views = swap_chain->Get_Image_Views();
		this->swapchain_rect       = vk::Rect2D(vk::Offset2D(), swap_chain->extent);
		this->image_index          = -1;
	}
	engine::Image_View *Acquire_Image(vk::Semaphore signal_semaphore)
	{
		auto res         = swap_chain->Acquire_Next_Image(signal_semaphore);
		this->image_index = res.value;
		return swapchain_image_views[image_index];
	}
	void Present_Image(vk::Semaphore wait_semaphore)
	{
		vk::SwapchainKHR swapchains[]     = {swap_chain.get()->swapchain_handle.get()};
		vk::Semaphore    wait_semaphores[] = {wait_semaphore};

		auto present_info = vk::PresentInfoKHR()
		                       .setSwapchainCount(1)
		                       .setPSwapchains(swapchains)
		                       .setPImageIndices(&image_index)
		                       .setPResults(nullptr)
		                       .setWaitSemaphoreCount(1)
		                       .setPWaitSemaphores(wait_semaphores);

		auto res = core->queue_worker_manager->Get_Worker_Queue("Present")->queue_handle.presentKHR(present_info);
	}

	vk::Extent2D Get_Image_Size()
	{
		return swap_chain.get()->extent;
	}
	engine::Core                      *core;
	std::unique_ptr<engine::Swap_Chain> swap_chain;
	std::vector<engine::Image_View *>   swapchain_image_views;
	uint32_t                   image_index;
	vk::Rect2D                 swapchain_rect;
};

struct In_Flight_Queue
{
	In_Flight_Queue(engine::Core *core, engine::Render_Graph *render_graph, Window_Desc window_desc, uint32_t inflight_count, vk::PresentModeKHR preferred_mode, glm::uvec2 window_size)
	{
		this->core        = core;
		this->render_graph = render_graph;
		presentation_queue.reset(new engine::Present_Queue(this->core, window_desc, inflight_count, preferred_mode, window_size));

		core->queue_worker_manager->Allocate_Cmds(inflight_count);

		for (int frame_index = 0; frame_index < inflight_count; frame_index++)
		{
			engine::Frame_Resources frame;
			frame.inflight_fence              = core->Create_Fence(true);
			frame.image_acquired_semaphore     = core->Create_Vulkan_Semaphore();
			frame.rendering_finished_semaphore = core->Create_Vulkan_Semaphore();
			frame_state.push_back(std::move(frame));
		}
		frame_index = 0;
		core->queue_worker_manager->Set_Frame_Index(frame_index);
	}
	void Begin_Frame()
	{
		auto &curr_frame = frame_state[frame_index];
		{
			core->Wait_For_Fence(curr_frame.inflight_fence.get());
			core->Reset_Fence(curr_frame.inflight_fence.get());
		}
		{
			current_swapchain_image_view = presentation_queue->Acquire_Image(curr_frame.image_acquired_semaphore.get());
		}
		core->queue_worker_manager->Begin_Cmds();
		render_graph->current_back_buffer_swapchain = current_swapchain_image_view;
		render_graph->frame_index                 = frame_index;
		// add pass info from my data
	}
	
	void Sync_Queues()
	{
		auto &curr_frame = frame_state[frame_index];
		for (int i = 0; i < render_graph->sorted_queue_batches.size(); ++i)
		{
			if (render_graph->sorted_queue_batches.size() == 1)
			{
				auto                  *queue_ref           = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[0].queue_name);
				vk::Semaphore          wait_semaphores[]   = {curr_frame.image_acquired_semaphore.get()};
				vk::PipelineStageFlags wait_stages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
				vk::Semaphore          signal_semaphores[] = {curr_frame.rendering_finished_semaphore.get()};

				auto submit_info = vk::SubmitInfo()
				                      .setWaitSemaphoreCount(1)
				                      .setPWaitSemaphores(wait_semaphores)
				                      .setPWaitDstStageMask(wait_stages)
				                      .setCommandBufferCount(1)
				                      .setPCommandBuffers(&render_graph->sorted_queue_batches[0].command_buffer)
				                      .setSignalSemaphoreCount(1)
				                      .setPSignalSemaphores(signal_semaphores);

				queue_ref->queue_handle.submit({submit_info}, curr_frame.inflight_fence.get());
				break;
			}
			auto *queue_ref = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[i].queue_name);
			if (i == 0)
			{
				//in case the next queue is cuda we need to signal that queue
				Worker_Queue* queue_to_signal_ref = queue_ref;
				if (render_graph->sorted_queue_batches[i + 1].queue_name == "CUDA")
				{
					queue_to_signal_ref = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[i + 1].queue_name);
				}
				assert(queue_ref->name != "CUDA" && "First queue must never be cuda");
				vk::Semaphore          wait_semaphores[] = {curr_frame.image_acquired_semaphore.get()};
				vk::PipelineStageFlags wait_stages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
				vk::Semaphore          signal_timeline[] = {queue_to_signal_ref->timeline_sem.get()};

				uint64_t signal_value = ++queue_to_signal_ref->timeline_value;

				vk::TimelineSemaphoreSubmitInfo timeline_info = {};
				timeline_info.setSignalSemaphoreValueCount(1);
				timeline_info.setPSignalSemaphoreValues(&signal_value);

				auto submit_info = vk::SubmitInfo()
				                      .setWaitSemaphoreCount(1)
				                      .setPNext(&timeline_info)
				                      .setPWaitSemaphores(wait_semaphores)
				                      .setPWaitDstStageMask(wait_stages)
				                      .setCommandBufferCount(1)
				                      .setPCommandBuffers(&render_graph->sorted_queue_batches[i].command_buffer)
				                      .setSignalSemaphoreCount(1)
				                      .setPSignalSemaphores(signal_timeline);
				queue_ref->queue_handle.submit({submit_info});
				continue;
			}
			if (i == render_graph->sorted_queue_batches.size() - 1)
			{
				Worker_Queue *last_queue_ref = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[i - 1].queue_name);
				assert(queue_ref->name != "CUDA" && "Last queue must never be cuda");
				vk::Semaphore          wait_semaphores[] = {last_queue_ref->timeline_sem.get()};
				vk::PipelineStageFlags wait_stages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
				vk::Semaphore          signal_semaphore[] = {curr_frame.rendering_finished_semaphore.get()};

				uint64_t wait_value = last_queue_ref->timeline_value;

				vk::TimelineSemaphoreSubmitInfo timeline_info = {};
				timeline_info.setWaitSemaphoreValueCount(1);
				timeline_info.setPWaitSemaphoreValues(&wait_value);

				auto submit_info = vk::SubmitInfo()
				                      .setWaitSemaphoreCount(1)
				                      .setPNext(&timeline_info)
				                      .setPWaitSemaphores(wait_semaphores)
				                      .setPWaitDstStageMask(wait_stages)
				                      .setCommandBufferCount(1)
				                      .setPCommandBuffers(&render_graph->sorted_queue_batches[i].command_buffer)
				                      .setSignalSemaphoreCount(1)
				                      .setPSignalSemaphores(signal_semaphore);
				queue_ref->queue_handle.submit({submit_info}, curr_frame.inflight_fence.get());
				continue;
			}

			Worker_Queue *last_queue_ref = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[i - 1].queue_name);
			if (queue_ref->name == "CUDA")
			{
				// this is the only context where cuda queues should appear
				render_graph->sorted_queue_batches[i].Execute_Cuda(core);
			}
			else
			{
				
				//in case the next queue is cuda we need to signal that queue
				Worker_Queue* queue_to_signal_ref = queue_ref;
				if (render_graph->sorted_queue_batches[i + 1].queue_name == "CUDA")
				{
					queue_to_signal_ref = render_graph->core->queue_worker_manager->Get_Worker_Queue(render_graph->sorted_queue_batches[i + 1].queue_name);
				}
				vk::Semaphore          wait_semaphores[] = {last_queue_ref->timeline_sem.get()};
				vk::PipelineStageFlags wait_stages[]     = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
				vk::Semaphore          signal_timeline[] = {queue_to_signal_ref->timeline_sem.get()};

				uint64_t wait_value   = last_queue_ref->timeline_value;
				uint64_t signal_value = ++queue_to_signal_ref->timeline_value;

				vk::TimelineSemaphoreSubmitInfo timeline_info = {};
				timeline_info.setWaitSemaphoreValueCount(1);
				timeline_info.setPWaitSemaphoreValues(&wait_value);
				timeline_info.setSignalSemaphoreValueCount(1);
				timeline_info.setPSignalSemaphoreValues(&signal_value);

				auto submit_info = vk::SubmitInfo()
				                      .setWaitSemaphoreCount(1)
				                      .setPNext(&timeline_info)
				                      .setPWaitSemaphores(wait_semaphores)
				                      .setPWaitDstStageMask(wait_stages)
				                      .setCommandBufferCount(1)
				                      .setPCommandBuffers(&render_graph->sorted_queue_batches[i].command_buffer)
				                      .setSignalSemaphoreCount(1)
				                      .setPSignalSemaphores(signal_timeline);
				queue_ref->queue_handle.submit({submit_info});
			}
		}
		presentation_queue->Present_Image(curr_frame.rendering_finished_semaphore.get());
	}

	void End_Frame()
	{

		std::vector<std::string> queue_names;

		Transition_Image(current_swapchain_image_view->image_metadata, Present, current_swapchain_image_view->Get_Subresource_Range(),
		                render_graph->sorted_queue_batches.back().command_buffer);

		core->queue_worker_manager->End_Cmds();
		
		Sync_Queues();
		
		frame_index = (frame_index + 1) % frame_state.size();
		core->queue_worker_manager->Set_Frame_Index(frame_index);
	}
	void Begin_Parallel_Threads()
	{
		for (auto &worker : core->queue_worker_manager.get()->workers_queues)
		{
			if (worker.second.is_main_threat)
			{
				continue;
			}
			std::function<void()> worker_start_task([&worker, this] {
				// worker.second.command_buffers = std::Move(core->Allocate_Command_Buffers(worker.second.worker_command_pool.get(), 1));
				// auto buffer_begin_info         = vk::CommandBufferBeginInfo()
				//                            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
			});
			worker.second.task_threat.Add_Pre_Rendering_Task(worker_start_task);
		}
	}

	void End_Parallel_Threads()
	{
		for (auto &worker : core->queue_worker_manager.get()->workers_queues)
		{
			if (worker.second.is_main_threat)
			{
				continue;
			}
			std::function<void()> worker_start_task([&worker, this] {
			});
			worker.second.task_threat.Add_Pre_Rendering_Task(worker_start_task);
		}
	}

	std::vector<engine::Frame_Resources> frame_state;
	size_t                      frame_index;

	engine::Core                         *core;
	engine::Render_Graph                  *render_graph;
	std::unique_ptr<engine::Present_Queue> presentation_queue;
	engine::Image_View                    *current_swapchain_image_view;
};

}        // namespace engine

#endif        // PRESENT_QUEUE_HPP
