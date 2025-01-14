//
// Created by carlo on 2025-01-12.
//


#ifndef FILEWATCHER_HPP
#define FILEWATCHER_HPP

namespace SYSTEMS
{
    struct FileInfo
    {
        //lastTime write it
        FileInfo(const std::string& path)
        {
            assert(std::filesystem::exists(path) && "invalid path");
            
            this->path = path;
            this->name = this->path.filename().string();
            this->extension = this->path.extension().string();
            this->modified = false;
            lastTimeCheck = std::filesystem::last_write_time(this->path);
            
            
        }
        void CheckLastTimeWrite()
        {
            auto fTime = std::filesystem::last_write_time(this->path);
            if (fTime != lastTimeCheck)
            {
                modified = true;
                lastTimeCheck = fTime;
            }
        }
        void FlushModification()
        {
            modified = false;
        }
        std::filesystem::path path;
        std::string name;
        std::string extension;
        std::atomic<bool> modified;
        std::filesystem::file_time_type lastTimeCheck;
        
    };
    
    class FileWatcher 
    {
        void CheckChanges()
        {
            for (auto& file : filesToWatch)
            {
                file.CheckLastTimeWrite();
            }
        }
        // FileInfo& GetFile(const std::string& path)
        // {
        //     filesToWatch.emplace_back(FileInfo(path));
        //     return filesToWatch.back();
        // }

        std::vector<FileInfo> filesToWatch;
        std::thread watcherThreat;
    };

}

#endif //FILEWATCHER_HPP
