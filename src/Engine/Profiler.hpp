//
// Created by carlo on 2024-11-28.
//

#ifndef PROFILER_HPP
#define PROFILER_HPP

namespace engine
{
    class profiler
    {
    public:
        profiler() = default;


        void add_profiler_cpu_spot(uint32_t color, std::string name)
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto now_time = std::chrono::duration<double>(now.time_since_epoch()).count();

            cpu_names[name] = cpu_update_info.size();
            cpu_update_info.emplace_back(legit::ProfilerTask{now_time, -1.0, name, color});
        }
        
        void add_profiler_gpu_spot(uint32_t color, std::string name)
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto now_time = std::chrono::duration<double>(now.time_since_epoch()).count();

            gpu_names[name] = gpu_update_info.size();
            gpu_update_info.emplace_back(legit::ProfilerTask{now_time, -1.0, name, color});
        }
        void end_profiler_cpu_spot(std::string name)
        {
            assert(cpu_names.contains(name) &&"Profiler cpu task did not start");
            
            auto now = std::chrono::high_resolution_clock::now();
            auto now_time = std::chrono::duration<double>(now.time_since_epoch()).count();

            auto& task = cpu_update_info.at(cpu_names.at(name));
            task.endTime = now_time;
        }

        void end_profiler_gpu_spot(std::string name)
        {
            
            assert(gpu_names.contains(name) &&"Profiler gpu task did not start");
            auto now = std::chrono::high_resolution_clock::now();
            auto now_time = std::chrono::duration<double>(now.time_since_epoch()).count();

            auto& task = gpu_update_info.at(gpu_names.at(name));
            task.endTime = now_time;
        }
        
        void start_profiler()
        {
            auto now = std::chrono::high_resolution_clock::now();
            start_frame_time = std::chrono::duration<double>(now.time_since_epoch()).count();
        }
        void update_profiler()
        {
            cpu_tasks.resize(cpu_update_info.size());
            gpu_tasks.resize(gpu_update_info.size());
            double cpu_start_time = get_first_tracked_start_time(cpu_update_info);
            double gpu_start_time = get_first_tracked_start_time(gpu_update_info);
            for (int i = 0; i < cpu_tasks.size(); ++i)
            {
                cpu_tasks[i] = legit::ProfilerTask();
                cpu_tasks[i].startTime = cpu_update_info[i].startTime - cpu_start_time;
                cpu_tasks[i].endTime = cpu_update_info[i].endTime - cpu_start_time;
                cpu_tasks[i].name = cpu_update_info[i].name;
                cpu_tasks[i].color = cpu_update_info[i].color;
            }
            for (int i = 0; i < gpu_tasks.size(); ++i)
            {
                gpu_tasks[i].startTime = gpu_update_info[i].startTime - gpu_start_time;
                gpu_tasks[i].endTime = gpu_update_info[i].endTime - gpu_start_time;
                gpu_tasks[i].name = gpu_update_info[i].name;
                gpu_tasks[i].color = gpu_update_info[i].color;
            }
            cpu_update_info.clear();
            gpu_update_info.clear();
            cpu_names.clear();
            gpu_names.clear();
        }

        static profiler* get_instance()
        {
            if (instance==nullptr)
            {
                instance = new profiler();
            }
            return instance;
        }

        double start_frame_time;
        
        std::vector<legit::ProfilerTask> cpu_tasks;
        std::vector<legit::ProfilerTask> gpu_tasks;
        std::vector<legit::ProfilerTask> cpu_update_info;
        std::vector<legit::ProfilerTask> gpu_update_info;
        std::map<std::string, int> cpu_names;
        std::map<std::string, int> gpu_names;
        static profiler* instance;

    private:
        double get_first_tracked_start_time(const std::vector<legit::ProfilerTask>& tasks)
        {
            if (tasks.empty())
            {
                return 0.0;
            }

            double first_start_time = tasks[0].startTime;
            for (const auto& task : tasks)
            {
                first_start_time = std::min(first_start_time, task.startTime);
            }
            return first_start_time;
        }
    };
    profiler* profiler::instance = nullptr;
    
}

#endif //PROFILER_HPP
