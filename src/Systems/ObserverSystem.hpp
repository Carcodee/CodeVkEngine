//
// Created by carlo on 2024-11-24.
//

#ifndef OBSERVER_SYSTEM 
#define OBSERVER_SYSTEM 
namespace SYSTEMS{

    class Watcher
    {
    public:
        virtual ~Watcher() = default;
        virtual void UpdateWatcher() = 0;
    };

    class Subject
    {
    public:
        virtual ~Subject() = default;
        virtual void Attach(Watcher* watcher) = 0;
        virtual void Notify() = 0;
        virtual void Detach(Watcher* watcher) = 0;
        
    };
    

}
#endif //OBSERVER_SYSTEM
