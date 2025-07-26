//

// Created by carlo on 2024-12-01.
//

#ifndef TASKTHREAT_HPP
#define TASKTHREAT_HPP
namespace SYSTEMS
{
class TaskThread
{
  public:
	TaskThread() = default;
	~TaskThread()
	{
		Stop();
	};

	TaskThread(const TaskThread &)            = delete;
	TaskThread &operator=(const TaskThread &) = delete;
	TaskThread(TaskThread &&)                 = default;
	TaskThread &operator=(TaskThread &&)      = default;

	void Run()
	{
		while (running)
		{
			std::cout << "Running\n";
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				conditionVariable.wait(lock, [this]() { return !taskQueue.empty() || !running; });
				if (taskQueue.empty() || !running)
				{
					std::cout << "TaskQueue was empty and it shouldn't be!\n";
					return;
				}
				task = std::move(taskQueue.front());
				taskQueue.pop();
			}
			task();
		}
	}
	void Start()
	{
		assetThreat = std::thread(&TaskThread::Run, this);
	}
	void Stop()
	{
		running = false;
		conditionVariable.notify_one();
		if (assetThreat.joinable())
		{
			assetThreat.join();
		}
	}
	void AddTask(std::function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lockGuard(queueMutex);
			taskQueue.push(task);
		}
		conditionVariable.notify_one();
	}
	std::atomic<bool>                 running           = true;
	std::queue<std::function<void()>> taskQueue         = {};
	std::thread                       assetThreat       = {};
	std::mutex                        queueMutex        = {};
	std::condition_variable           conditionVariable = {};
};

}        // namespace SYSTEMS

#endif        // TASKTHREAT_HPP
