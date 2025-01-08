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

            cpuNames.try_emplace(name, cpuUpdateInfo.size());
            cpuUpdateInfo.emplace_back(legit::ProfilerTask{nowTime, -1.0, name, color});
        }
        
        void AddProfilerGpuSpot(uint32_t color, std::string name)
        {
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            gpuNames.try_emplace(name, gpuUpdateInfo.size());
            gpuUpdateInfo.emplace_back(legit::ProfilerTask{nowTime, -1.0, name, color});
        }
        void EndProfilerCpuSpot(std::string name)
        {
            assert(cpuNames.contains(name) &&"Profiler cpu task did not start");
            
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            double trueEnd = nowTime - cpuUpdateInfo.at(cpuNames.at(name)).startTime;
            cpuUpdateInfo.at(cpuNames.at(name)).endTime = trueEnd;
            cpuUpdateInfo.at(cpuNames.at(name)).startTime = 0.0f;
            if (cpuUpdateInfo.size() > 1)
            {
                cpuUpdateInfo.at(cpuNames.at(name)).startTime = cpuUpdateInfo.at(cpuNames.at(name) - 1).endTime;
                cpuUpdateInfo.at(cpuNames.at(name)).endTime =cpuUpdateInfo.at(cpuNames.at(name) - 1).endTime + trueEnd;
            }
        }

        void EndProfilerGpuSpot(std::string name)
        {
            
            assert(gpuNames.contains(name) &&"Profiler gpu task did not start");
            auto now = std::chrono::high_resolution_clock::now();
            auto nowTime = std::chrono::duration<double>(now.time_since_epoch()).count();

            double trueEnd = nowTime - gpuUpdateInfo.at(gpuNames.at(name)).startTime;
            gpuUpdateInfo.at(gpuNames.at(name)).endTime = trueEnd;
            gpuUpdateInfo.at(gpuNames.at(name)).startTime = 0.0f;
            if (gpuUpdateInfo.size() > 1)
            {
                gpuUpdateInfo.at(gpuNames.at(name)).startTime = gpuUpdateInfo.at(gpuNames.at(name) - 1).endTime;
                gpuUpdateInfo.at(gpuNames.at(name)).endTime =gpuUpdateInfo.at(gpuNames.at(name) - 1).endTime + trueEnd;
            }
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
            for (int i = 0; i < cpuTasks.size(); ++i)
            {
                cpuTasks[i] = legit::ProfilerTask();
                cpuTasks[i].startTime = cpuUpdateInfo[i].startTime;
                cpuTasks[i].endTime = cpuUpdateInfo[i].endTime;
                cpuTasks[i].name = cpuUpdateInfo[i].name;
                cpuTasks[i].color = cpuUpdateInfo[i].color;
            }
            for (int i = 0; i < gpuTasks.size(); ++i)
            {
                gpuTasks[i].startTime = gpuUpdateInfo[i].startTime;
                gpuTasks[i].endTime = gpuUpdateInfo[i].endTime;
                gpuTasks[i].name = gpuUpdateInfo[i].name;
                gpuTasks[i].color = gpuUpdateInfo[i].color;
            }
            cpuUpdateInfo.clear();
            gpuUpdateInfo.clear();
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
    };
    Profiler* Profiler::instance = nullptr;
    
}

#endif //PROFILER_HPP
