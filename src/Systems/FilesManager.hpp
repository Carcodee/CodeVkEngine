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

        std::deque<std::string> pathsToDelete;
    };
}

#endif //FILESMANAGER_HPP
