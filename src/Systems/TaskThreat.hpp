//

// Created by carlo on 2024-12-01.
//

#ifndef TASK_THREAT_HPP
#define TASK_THREAT_HPP
namespace systems
{
class task_thread
{
  public:
	task_thread() = default;
	~task_thread()
	{
		stop();
	};

	task_thread(const task_thread &)            = delete;
	task_thread &operator=(const task_thread &) = delete;
	task_thread(task_thread &&)                 = default;
	task_thread &operator=(task_thread &&)      = default;

	void run()
	{
		while (running)
		{
			// std::cout << "Running\n";
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(queue_mutex);
				condition_variable.wait(lock, [this]() { return !task_queue.empty() || !running; });
				if (task_queue.empty() || !running)
				{
					std::cout << "TaskQueue was empty and it shouldn't be!\n";
					return;
				}
				task = std::move(task_queue.front());
				task_queue.pop();
			}
			task();
		}
	}
	void start()
	{
		asset_threat = std::thread(&task_thread::run, this);
	}
	void stop()
	{
		running = false;
		condition_variable.notify_one();
		if (asset_threat.joinable())
		{
			asset_threat.join();
		}
	}
	void add_pre_rendering_task(std::function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lock_guard(queue_mutex);
			task_queue.push(task);
		}
		condition_variable.notify_one();
	}
	std::atomic<bool>                 running           = true;
	std::queue<std::function<void()>> task_queue         = {};
	std::thread                       asset_threat       = {};
	std::mutex                        queue_mutex        = {};
	std::condition_variable           condition_variable = {};
};

}        // namespace systems

#endif        // TASK_THREAT_HPP
