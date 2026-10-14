//

// Created by carlo on 2024-12-01.
//

#ifndef TASK_THREAT_HPP
#define TASK_THREAT_HPP
namespace systems
{
class Task_Thread
{
  public:
	Task_Thread() = default;
	~Task_Thread()
	{
		Stop();
	};

	Task_Thread(const Task_Thread &)            = delete;
	Task_Thread &operator=(const Task_Thread &) = delete;
	Task_Thread(Task_Thread &&)                 = default;
	Task_Thread &operator=(Task_Thread &&)      = default;

	void Run()
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
	void Start()
	{
		asset_threat = std::thread(&Task_Thread::Run, this);
	}
	void Stop()
	{
		running = false;
		condition_variable.notify_one();
		if (asset_threat.joinable())
		{
			asset_threat.join();
		}
	}
	void Add_Pre_Rendering_Task(std::function<void()> task)
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
