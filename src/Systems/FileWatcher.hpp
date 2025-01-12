//
// Created by carlo on 2025-01-12.
//

#ifndef FILEWATCHER_HPP
#define FILEWATCHER_HPP

namespace SYSTEMS
{
    class File
    {
        //lastTime write it
        std::filesystem::path path;
        std::string name;
        std::string extension;
    };
    class FileWatcher 
    {
        void Run()
        {
            while (true)
            {
                
            }
        }
        

        std::vector<File> filesToWatch;
    };

}

#endif //FILEWATCHER_HPP
