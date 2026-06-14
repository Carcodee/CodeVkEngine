//
// Created by carlo on 2024-11-28.
//

#ifndef PROFILER_HPP
#define PROFILER_HPP

namespace ENGINE
{
    class Profiler
    {
    public:
        Profiler() = default;


        void AddProfilerCpuSpot(uint32_t color, std::string name)
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            cpuNames[name] = cpuUpdateInfo.size();
            cpuUpdateInfo.emplace_back(legit::ProfilerTask{nowTime, -1.0, name, color});
        }
        
        void AddProfilerGpuSpot(uint32_t color, std::string name)
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            gpuNames[name] = gpuUpdateInfo.size();
            gpuUpdateInfo.emplace_back(legit::ProfilerTask{nowTime, -1.0, name, color});
        }
        void EndProfilerCpuSpot(std::string name)
        {
            assert(cpuNames.contains(name) &&"Profiler cpu task did not start");
            
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            auto& task = cpuUpdateInfo.at(cpuNames.at(name));
            task.endTime = nowTime;
        }

        void EndProfilerGpuSpot(std::string name)
        {
            
            assert(gpuNames.contains(name) &&"Profiler gpu task did not start");
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            auto& task = gpuUpdateInfo.at(gpuNames.at(name));
            task.endTime = nowTime;
        }
        
        void StartProfiler()
        {
            auto now = std::chrono::high_resolution_clock::now();
            startFrameTime = std::chrono::duration<double>(now.time_since_epoch()).count();
        }
        void UpdateProfiler()
        {
            cpuTasks.resize(cpuUpdateInfo.size());
            gpuTasks.resize(gpuUpdateInfo.size());
            double cpuStartTime = GetFirstTrackedStartTime(cpuUpdateInfo);
            double gpuStartTime = GetFirstTrackedStartTime(gpuUpdateInfo);
            for (int i = 0; i < cpuTasks.size(); ++i)
            {
                cpuTasks[i] = legit::ProfilerTask();
                cpuTasks[i].startTime = cpuUpdateInfo[i].startTime - cpuStartTime;
                cpuTasks[i].endTime = cpuUpdateInfo[i].endTime - cpuStartTime;
                cpuTasks[i].name = cpuUpdateInfo[i].name;
                cpuTasks[i].color = cpuUpdateInfo[i].color;
            }
            for (int i = 0; i < gpuTasks.size(); ++i)
            {
                gpuTasks[i].startTime = gpuUpdateInfo[i].startTime - gpuStartTime;
                gpuTasks[i].endTime = gpuUpdateInfo[i].endTime - gpuStartTime;
                gpuTasks[i].name = gpuUpdateInfo[i].name;
                gpuTasks[i].color = gpuUpdateInfo[i].color;
            }
            cpuUpdateInfo.clear();
            gpuUpdateInfo.clear();
            cpuNames.clear();
            gpuNames.clear();
        }

        static Profiler* GetInstance()
        {
            if (instance==nullptr)
            {
                instance = new Profiler();
            }
            return instance;
        }

        double startFrameTime;
        
        std::vector<legit::ProfilerTask> cpuTasks;
        std::vector<legit::ProfilerTask> gpuTasks;
        std::vector<legit::ProfilerTask> cpuUpdateInfo;
        std::vector<legit::ProfilerTask> gpuUpdateInfo;
        std::map<std::string, int> cpuNames;
        std::map<std::string, int> gpuNames;
        static Profiler* instance;

    private:
        double GetFirstTrackedStartTime(const std::vector<legit::ProfilerTask>& tasks)
        {
            if (tasks.empty())
            {
                return 0.0;
            }

            double firstStartTime = tasks[0].startTime;
            for (const auto& task : tasks)
            {
                firstStartTime = std::min(firstStartTime, task.startTime);
            }
            return firstStartTime;
        }
    };
    Profiler* Profiler::instance = nullptr;
    
}

#endif //PROFILER_HPP
