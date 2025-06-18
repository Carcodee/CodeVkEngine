//

// Created by carlo on 2024-12-01.
//

#ifndef TASKTHREAT_HPP
#define TASKTHREAT_HPP
namespace SYSTEMS
{
class TaskThreat
{
	TaskThreat()
	{
		running = true;
		// assetThreat = std::thread(&Run, this);
	}

	void Run()
	{
		while (running)
		{
			std::cout << "Running\n";
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				conditionVariable.wait(lock, [this, task]() { return !taskQueue.empty() || !running; });
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

	void AddTask(std::function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lockGuard(queueMutex);
			taskQueue.push(task);
		}
		conditionVariable.notify_one();
	}
	std::atomic<bool>                 running;
	std::queue<std::function<void()>> taskQueue;
	std::thread                       assetThreat;
	std::mutex                        queueMutex;
	std::condition_variable           conditionVariable;
};

}        // namespace SYSTEMS

#endif        // TASKTHREAT_HPP
