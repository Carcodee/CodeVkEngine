//

// Created by carlo on 2025-02-27.
//

#ifndef FILESMANAGER_HPP
#define FILESMANAGER_HPP

namespace SYSTEMS
{
    class FilesManager
    {
    public:
        FilesManager()
        {
        }

        ~FilesManager()
        {
            for (auto& path : pathsToDelete)
            {
                OS::GetInstance()->DeleteExistingFile(path);
            }
            pathsToDelete.clear();
        }

        void AddPathToDelete(std::string path)
        {
            pathsToDelete.push_back(path);
        }
        void CheckAllChanges()
        {
            for (auto& file : filesWatched)
            {
                //calling some callbacks
                file->CheckLastTimeWrite();
            }
        }
        void AddFileToWatch(std::string path, std::function<void()> function)
        {
            assert(!path.empty() && "Path is empty");
            filesWatched.emplace_back(std::make_unique<FileInfo>(path, function));
        }

        std::deque<std::string> pathsToDelete;
        std::vector<std::unique_ptr<FileInfo>> filesWatched;
    };
}

#endif //FILESMANAGER_HPP
