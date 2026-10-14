//
// Created by carlo on 2024-11-24.
//

#ifndef OBSERVER_SYSTEM_HPP
#define OBSERVER_SYSTEM_HPP
namespace systems
{

class Watcher
{
  public:
	virtual ~Watcher()           = default;
	virtual void Update_Watcher() = 0;
};

class Subject
{
  public:
	virtual ~Subject()                    = default;
	virtual void Attach(Watcher *watcher) = 0;
	virtual void Notify()                 = 0;
	virtual void Detach(Watcher *watcher) = 0;
};

}        // namespace systems
#endif        // OBSERVER_SYSTEM_HPP
